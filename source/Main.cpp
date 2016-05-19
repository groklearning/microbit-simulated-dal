extern "C" void app_main();

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
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
#include "MicroBitPin.h"
#include "ticker_dal.h"

extern "C" {
#include "json.h"
}

namespace {
pthread_cond_t interrupt_signal;
pthread_mutex_t interrupt_lock;
pthread_mutex_t code_lock;

volatile bool shutdown = false;

jmp_buf code_quit_jmp;
}

uint64_t
get_usec() {
  uint64_t us = 0;
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  unsigned long ms = ts.tv_sec * 1000;
  ms += ts.tv_nsec / 1000000ULL;
}

extern "C" {
void
simulated_dal_micropy_vm_hook_loop() {
  pthread_mutex_unlock(&code_lock);

  if (get_reset_flag() || get_panic_flag()) {
    shutdown = true;
  }

  if (shutdown) {
    longjmp(code_quit_jmp, 1);
  }

  pthread_mutex_lock(&code_lock);
}
}

void
__wait_for_interrupt() {
  pthread_mutex_unlock(&code_lock);

  pthread_mutex_lock(&interrupt_lock);
  pthread_cond_wait(&interrupt_signal, &interrupt_lock);
  pthread_mutex_unlock(&interrupt_lock);

  if (get_reset_flag() || get_panic_flag()) {
    shutdown = true;
  }

  if (shutdown) {
    longjmp(code_quit_jmp, 1);
  }

  pthread_mutex_lock(&code_lock);
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
  while (true) {
    if (setjmp(code_quit_jmp) == 0) {
      pthread_mutex_lock(&code_lock);
      app_main();
      // Normal termination (this should never happen).
      return NULL;
    }
    // A longjmp happened.
    if (shutdown) {
      return NULL;
    }
  }
  return NULL;
}

const uint32_t MICROBIT_PIN_3V = INT_MAX;
const uint32_t MICROBIT_PIN_GND = INT_MAX - 1;

uint32_t MICROBIT_PIN_MAP[23] = {MICROBIT_PIN_P0,  MICROBIT_PIN_P1,  MICROBIT_PIN_P2,  MICROBIT_PIN_P3,  MICROBIT_PIN_P4,  MICROBIT_PIN_P5,
                                 MICROBIT_PIN_P6,  MICROBIT_PIN_P7,  MICROBIT_PIN_P8,  MICROBIT_PIN_P9,  MICROBIT_PIN_P10, MICROBIT_PIN_P11,
                                 MICROBIT_PIN_P12, MICROBIT_PIN_P13, MICROBIT_PIN_P14, MICROBIT_PIN_P15, MICROBIT_PIN_P16, MICROBIT_PIN_3V,
                                 MICROBIT_PIN_3V,  MICROBIT_PIN_P19, MICROBIT_PIN_P20, MICROBIT_PIN_GND, MICROBIT_PIN_GND};

void
check_gpio_updates(int updates_fd) {
  static uint32_t prev_pins[23] = {0};

  uint32_t state;
  uint32_t output;
  uint32_t pull;
  uint32_t floating;
  pthread_mutex_lock(&code_lock);
  get_gpio_info(&state, &output, &pull, &floating);
  pthread_mutex_unlock(&code_lock);

  uint32_t pins[23] = {0};
  for (int i = 0; i < sizeof(pins) / sizeof(uint32_t); ++i) {
    int pin = MICROBIT_PIN_MAP[i];
    if (i == 3 || i == 4 || i == 6 || i == 7 || i == 9 || i == 10) {
      // LED rows and columns.
      pins[i] = 7;
    } else if (pin == MICROBIT_PIN_3V) {
      pins[i] = 1;
    } else if (pin == MICROBIT_PIN_GND) {
      pins[i] = 0;
    } else {
      if (output & (1 << pin)) {
        // low-z
        if (state & (1 << pin)) {
          // hi
          pins[i] = 1;
        } else {
          // low
          pins[i] = 0;
        }
      } else {
        // hi-z
        if (floating & (1 << pin)) {
          // floating
          pins[i] = 2;
        } else {
          if (pull & (1 << pin)) {
            // pull-up
            if (state & (1 << pin)) {
              pins[i] = 3;
            } else {
              pins[i] = 4;
            }
          } else {
            // pull-down
            if (state & (1 << pin)) {
              pins[i] = 5;
            } else {
              pins[i] = 6;
            }
          }
        }
      }
    }
  }

  if (memcmp(pins, prev_pins, sizeof(prev_pins)) != 0) {
    char json[1024];
    char* jsonp = json;
    char* jsonp_end = json + sizeof(json);
    snprintf(jsonp, jsonp_end - jsonp, "[{ \"type\": \"microbit_pins\", \"data\": { \"p\": [");
    jsonp += strnlen(jsonp, jsonp_end - jsonp);

    for (int i = 0; i < sizeof(pins) / sizeof(uint32_t); ++i) {
      snprintf(jsonp, jsonp_end - jsonp, "%d,", pins[i]);
      jsonp += strnlen(jsonp, jsonp_end - jsonp);
    }

    // Remove trailing comma.
    jsonp -= 1;
    snprintf(jsonp, jsonp_end - jsonp, "]}}]\n");
    jsonp += strnlen(jsonp, jsonp_end - jsonp);
    write(updates_fd, json, jsonp - json);

    memcpy(prev_pins, pins, sizeof(prev_pins));
  }
}

int
ticks_to_brightness(int ticks) {
  if (ticks < 5)
    return 0;
  if (ticks < 11)
    return 1;
  if (ticks < 23)
    return 2;
  if (ticks < 40)
    return 3;
  if (ticks < 80)
    return 4;
  if (ticks < 150)
    return 5;
  if (ticks < 300)
    return 6;
  if (ticks < 590)
    return 7;
  if (ticks < 1100)
    return 8;
  return 9;
}

void
check_led_updates(int updates_fd) {
  static uint32_t leds[25] = {INT_MAX};
  uint32_t led_ticks[25] = {0};

  pthread_mutex_lock(&code_lock);
  get_led_ticks(led_ticks);
  pthread_mutex_unlock(&code_lock);
  /*for (int y = 0; y < 5; ++y) {
    for (int x = 0; x < 5; ++x) {
      fprintf(stderr, "%9d, ", led_ticks[y * 5 + x]);
    }
    fprintf(stderr, "\n");
  }
  fprintf(stderr, "\n");
  */
  bool diff = false;
  for (int i = 0; i < 25; ++i) {
    int b = ticks_to_brightness(led_ticks[i]);
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

  pthread_mutex_lock(&code_lock);
  if (id->as.number == 0) {
    // Buttons have (external) pull-up resistors (so pressing the button sets the pin low).
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
  pthread_mutex_unlock(&code_lock);
}

void
process_client_accel(const json_value* data) {
  const json_value* x = json_value_get(data, "x");
  const json_value* y = json_value_get(data, "y");
  const json_value* z = json_value_get(data, "z");
  if (!x || !y || !z || x->type != JSON_VALUE_TYPE_NUMBER || y->type != JSON_VALUE_TYPE_NUMBER || z->type != JSON_VALUE_TYPE_NUMBER) {
    fprintf(stderr, "Accelerometer event missing x/y/z.\n");
    return;
  }

  pthread_mutex_lock(&code_lock);
  set_accelerometer(x->as.number, y->as.number, z->as.number);
  pthread_mutex_unlock(&code_lock);
}

void
process_client_magnet(const json_value* data) {
  const json_value* x = json_value_get(data, "x");
  const json_value* y = json_value_get(data, "y");
  const json_value* z = json_value_get(data, "z");
  if (!x || !y || !z || x->type != JSON_VALUE_TYPE_NUMBER || y->type != JSON_VALUE_TYPE_NUMBER || z->type != JSON_VALUE_TYPE_NUMBER) {
    fprintf(stderr, "Magnetometer event missing x/y/z.\n");
    return;
  }

  pthread_mutex_lock(&code_lock);
  set_magnetometer(x->as.number, y->as.number, z->as.number);
  pthread_mutex_unlock(&code_lock);
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
      } else if (strncmp(event_type->as.string, "microbit_accel", 14) == 0) {
        process_client_accel(event_data);
      } else if (strncmp(event_type->as.string, "microbit_magnet", 15) == 0) {
        process_client_magnet(event_data);
      } else {
        fprintf(stderr, "Unknown event type: %s\n", event_type->as.string);
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
  timer_spec.it_interval.tv_nsec = 0;
  timer_spec.it_value.tv_sec = 0;
  timer_spec.it_value.tv_nsec = 16000 * 75;
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

  uint32_t ticks_until_fire_timer = 75;
  uint32_t macroticks_next_led_update = 0;

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
        uint8_t buf[10240];
        ssize_t len = read(STDIN_FILENO, &buf, sizeof(buf));
        if (len == -1) {
          continue;
        }
        pthread_mutex_lock(&code_lock);
        for (ssize_t i = 0; i < len; ++i) {
          serial_add_byte(buf[i]);
        }
        pthread_mutex_unlock(&code_lock);
        signal_interrupt();
      } else if (events[n].data.fd == notify_fd) {
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

        pthread_mutex_lock(&code_lock);
        ticks_until_fire_timer = fire_ticker(ticks_until_fire_timer);
        pthread_mutex_unlock(&code_lock);

        signal_interrupt();

        if (get_macro_ticks() > macroticks_next_led_update) {
          check_led_updates(updates_fd);
          check_gpio_updates(updates_fd);
          macroticks_next_led_update = get_macro_ticks() + 10;
        }

        timer_spec.it_value.tv_nsec = 16000 * ticks_until_fire_timer;
        timerfd_settime(timer_fd, 0, &timer_spec, NULL);
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
    // This will stop the code_thread on the next instruction or __WFI().
    shutdown = true;
    // The main thread will get EINTR back from epoll_wait.
  }
}

typedef void* (*thread_start_fn)(void*);
}

extern "C" {
typedef uint8_t byte;
typedef struct _appended_script_t {
  byte header[2];  // should be "MP"
  uint16_t len;    // length of script stored little endian
  char str[];      // data of script
} appended_script_t;

struct _appended_script_t* initial_script;
}

int
main(int argc, char** argv) {
  unbuffered_terminal(true);

  char script[102400] = {0};
  int program_fd = open("program.py", O_RDONLY);
  if (program_fd != -1) {
    ssize_t len = read(program_fd, script, sizeof(script));
    script[len] = 0;
  }

  initial_script = static_cast<_appended_script_t*>(malloc(sizeof(initial_script) + strlen(script) + 2));
  initial_script->header[0] = 'M';
  initial_script->header[1] = 'P';
  initial_script->len = strlen(script);
  strcpy(initial_script->str, script);

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
  pthread_mutex_init(&code_lock, NULL);

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

  free(initial_script);

  unbuffered_terminal(false);

  if (get_reset_flag()) {
    return 99;
  } else {
    return 0;
  }
}
