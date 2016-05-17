extern "C" void app_main();

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <sys/timerfd.h>
#include <termios.h>
#include <unistd.h>

#include <algorithm>
#include <type_traits>

#include "Hardware.h"
#include "ticker_dal.h"

extern "C" {
#include "json.h"
}

namespace {
pthread_cond_t interrupt_signal;
pthread_mutex_t interrupt_lock;

volatile bool shutdown = false;

jmp_buf code_quit_jmp;
}

void
__wait_for_interrupt() {
  pthread_mutex_lock(&interrupt_lock);
  pthread_cond_wait(&interrupt_signal, &interrupt_lock);
  pthread_mutex_unlock(&interrupt_lock);

  if (shutdown) {
    longjmp(code_quit_jmp, 1);
  }
}

void
__enable_irq() {
}

void
__disable_irq() {
}

namespace {
void
signal_interrupt() {
  pthread_mutex_lock(&interrupt_lock);
  pthread_cond_signal(&interrupt_signal);
  pthread_mutex_unlock(&interrupt_lock);
}

void*
code_thread(void*) {
  if (setjmp(code_quit_jmp) == 0) {
    app_main();
  }
  return NULL;
}

void
check_led_updates(int updates_fd) {
  uint32_t led_ticks[25] = {0};
  static uint32_t leds[25] = {0};
  get_led_brightness(led_ticks);
  /*for (int y = 0; y < 5; ++y) {
    for (int x = 0; x < 5; ++x) {
      fprintf(stderr, "%9d, ", led_ticks[y * 5 + x]);
    }
    fprintf(stderr, "\n");
  }
  fprintf(stderr, "\n");*/

  bool diff = false;
  for (int i = 0; i < 25; ++i) {
    int b = (led_ticks[i] > 100) ? 9 : 0;
    if (b != leds[i]) {
      diff = true;
      leds[i] = b;
    }
  }

  if (diff) {
    char json[1024];
    char* jsonp = json;
    char* jsonp_end = json + sizeof(json);
    snprintf(jsonp, jsonp_end - jsonp, "[{ \"type\": \"microbit_leds\", \"data\": { \"b\": [");
    jsonp += strnlen(jsonp, jsonp_end - jsonp);
    for (int y = 0; y < 5; ++y) {
      for (int x = 0; x < 5; ++x) {
        snprintf(jsonp, jsonp_end - jsonp, "%d,", leds[y * 5 + x]);
        jsonp += strnlen(jsonp, jsonp_end - jsonp);
      }
    }
    // Remove trailing comma.
    jsonp -= 1;
    snprintf(jsonp, jsonp_end - jsonp, "]}}]\n");
    jsonp += strnlen(jsonp, jsonp_end - jsonp);
    write(updates_fd, json, jsonp - json);
  }
}

void
process_client_button(const json_value* data) {
  const json_value* id = json_value_get(data, "id");
  const json_value* state = json_value_get(data, "state");
  if (!id || !state || id->type != JSON_VALUE_TYPE_NUMBER || state->type != JSON_VALUE_TYPE_NUMBER) {
    fprintf(stderr, "Button event missing id and/or state\n");
    return;
  }
  if (id->as.number == 0) {
    // Buttons are pull-down.
    if (state->as.number == 0) {
      set_gpio_state(get_gpio_state() | (1 << 17));
    } else {
      set_gpio_state(get_gpio_state() & ~(1 << 17));
    }
  }
  if (id->as.number == 1) {
    if (state->as.number == 0) {
      set_gpio_state(get_gpio_state() | (1 << 26));
    } else {
      set_gpio_state(get_gpio_state() & ~(1 << 26));
    }
  }
}

void
process_client_json(const json_value* json) {
  if (json->type != JSON_VALUE_TYPE_ARRAY) {
    fprintf(stderr, "Client event JSON wasn't a list.\n");
  }
  const json_value_list* event = json->as.pairs;
  while (event) {
    if (event->value->type != JSON_VALUE_TYPE_OBJECT) {
      fprintf(stderr, "Event should be an object.\n");
      continue;
    }
    const json_value* event_type = json_value_get(event->value, "type");
    const json_value* event_data = json_value_get(event->value, "data");
    if (!event_type || !event_data || event_type->type != JSON_VALUE_TYPE_STRING || event_data->type != JSON_VALUE_TYPE_OBJECT) {
      fprintf(stderr, "Event missing type and/or data.\n");
    } else {
      if (strncmp(event_type->as.string, "microbit_button", 15) == 0) {
        process_client_button(event_data);
      }
    }
    event = event->next;
  }
}

void
process_client_event(int fd) {
  char buf[10240];
  ssize_t len = read(fd, buf, sizeof(buf));
  if (len == sizeof(buf)) {
    fprintf(stderr, "Too much data in client event.\n");
    return;
  }
  buf[len] = 0;

  char* line_start = buf;
  while (*line_start) {
    char* line_end = strchrnul(line_start, '\n');

    json_value* json = json_parse_n(line_start, line_end - line_start);
    if (json) {
      process_client_json(json);
      json_value_destroy(json);
    } else {
      fprintf(stderr, "Invalid JSON\n");
    }

    if (!*line_end) {
      break;
    }

    line_start = line_end + 1;
  }
}

void*
main_thread(void*) {
  const int MAX_EVENTS = 10;
  int epoll_fd = epoll_create1(0);

  int updates_fd = open("___device_updates", O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);

  // Add non-blocking stdin to epoll set.
  struct epoll_event ev_stdin;
  fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL, 0) | O_NONBLOCK);
  ev_stdin.events = EPOLLIN;
  ev_stdin.data.fd = STDIN_FILENO;
  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &ev_stdin);

  // Create and truncate the client events file.
  int client_fd = open("___client_events", O_CREAT | O_TRUNC | O_RDONLY, S_IRUSR | S_IWUSR);

  // Set up a notify for the file and add to epoll set.
  struct epoll_event ev_client;
  int notify_fd = inotify_init1(0);
  int client_wd = inotify_add_watch(notify_fd, "___client_events", IN_MODIFY);
  if (client_wd == -1) {
    perror("add watch");
    return NULL;
  }
  ev_client.events = EPOLLIN;
  ev_client.data.fd = notify_fd;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, notify_fd, &ev_client) == -1) {
    perror("epoll_ctl");
    return NULL;
  }

  // Set up a timer for the ticker.
  int timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
  struct epoll_event ev_timer;
  ev_timer.events = EPOLLIN;
  ev_timer.data.fd = timer_fd;
  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timer_fd, &ev_timer);
  struct itimerspec timer_spec;
  timer_spec.it_interval.tv_sec = 0;
  timer_spec.it_interval.tv_nsec = 160000;
  timer_spec.it_value.tv_sec = 0;
  timer_spec.it_value.tv_nsec = 160000;
  timerfd_settime(timer_fd, 0, &timer_spec, NULL);

  // Open the events pipe.
  char* client_pipe_str = getenv("GROK_CLIENT_PIPE");
  int client_pipe_fd = -1;
  if (client_pipe_str != NULL) {
    client_pipe_fd = atoi(client_pipe_str);
    fcntl(client_pipe_fd, F_SETFL, fcntl(client_pipe_fd, F_GETFL, 0) | O_NONBLOCK);
    struct epoll_event ev_client_pipe;
    ev_client_pipe.events = EPOLLIN;
    ev_client_pipe.data.fd = client_pipe_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_pipe_fd, &ev_client_pipe);
  }

  int ticks = 0;
  int macroticks = 0;

  while (!shutdown) {
    struct epoll_event events[MAX_EVENTS];
    int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    if (nfds == -1) {
      if (errno == EINTR) {
        // Continue so that we'll catch the shutdown flag above.
        continue;
      }
      perror("epoll wait\n");
      exit(1);
    }

    for (int n = 0; n < nfds; ++n) {
      if (events[n].data.fd == STDIN_FILENO) {
        uint8_t buf[64];
        ssize_t len = read(STDIN_FILENO, &buf, sizeof(buf));
        if (len == -1) {
          continue;
        }
        for (ssize_t i = 0; i < len; ++i) {
          serial_add_byte(buf[i]);
        }
        signal_interrupt();
      } else if (events[n].data.fd == notify_fd) {
	fprintf(stderr, "inotify event\n");
        uint8_t buf[4096] __attribute__((aligned(__alignof__(inotify_event))));
        ssize_t len = read(notify_fd, buf, sizeof(buf));
        if (len == -1) {
          continue;
        }

        const struct inotify_event* event;
        for (uint8_t* p = buf; p < buf + len; p += sizeof(inotify_event) + event->len) {
          event = reinterpret_cast<inotify_event*>(p);
          if (event->wd == client_wd) {
            process_client_event(client_fd);
          }
        }
      } else if (events[n].data.fd == timer_fd) {
	uint64_t t;
	read(timer_fd, &t, sizeof(uint64_t));

	ticker_on_tick();
        signal_interrupt();
	++ticks;
	if (ticks > 375) {
	  ticker_on_macrotick();
	  ticks = 0;
	  ++macroticks;

	  if (macroticks > 100) {
	    check_led_updates(updates_fd);
	    macroticks = 0;
	  }
	}
      } else if (events[n].data.fd == client_pipe_fd) {
	process_client_event(client_pipe_fd);
      }
    }
  }

  signal_interrupt();

  close(notify_fd);
  close(client_fd);
  close(timer_fd);
  close(updates_fd);

  return NULL;
}

void
unbuffered_terminal(bool enable) {
  static struct termios old_tio;

  if (enable) {
    tcgetattr(STDIN_FILENO, &old_tio);
    struct termios new_tio = old_tio;

    // disable canonical mode (buffered i/o) and local echo
    new_tio.c_lflag &= (~ICANON & ~ECHO);

    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
  } else {
    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
  }
}

void
handle_sigint(int sig) {
  if (shutdown) {
    // Second time, something has gone wrong, so do minimal cleanup and exit.
    unbuffered_terminal(false);
    exit(1);
  } else {
    // Attempt a clean shutdown.
    fprintf(stderr, "\nShutting down...\n");
    // This will stop the code_thread, timer_thread and updates_thread.
    shutdown = true;
    // The main thread will get EINTR back from epoll_wait.
  }
}

typedef void* (*thread_start_fn)(void*);
}

int
main(int, char**) {
  unbuffered_terminal(true);

  struct sigaction sa;
  sa.sa_handler = handle_sigint;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);

  thread_start_fn thread_fns[] = {&code_thread};
  const int NUM_THREADS = sizeof(thread_fns) / sizeof(thread_start_fn);
  pthread_t threads[NUM_THREADS];

  pthread_cond_init(&interrupt_signal, NULL);
  pthread_mutex_init(&interrupt_lock, NULL);

  for (int i = 0; i < NUM_THREADS; ++i) {
    pthread_create(&threads[i], NULL, thread_fns[i], NULL);
  }

  main_thread(NULL);

  for (int i = 0; i < NUM_THREADS; ++i) {
    void* result;
    pthread_join(threads[i], &result);
  }

  pthread_cond_destroy(&interrupt_signal);
  pthread_mutex_destroy(&interrupt_lock);

  unbuffered_terminal(false);
  return 0;
}
