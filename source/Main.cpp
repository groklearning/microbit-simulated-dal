extern "C" void app_main();

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <termios.h>
#include <unistd.h>

#include <type_traits>
#include <algorithm>

#include "Hardware.h"
#include "ticker_dal.h"

extern "C" {
#include "json.h"
}

namespace {
pthread_cond_t interrupt_signal;
pthread_mutex_t interrupt_lock;
}

void
__wait_for_interrupt() {
  pthread_mutex_lock(&interrupt_lock);
  pthread_cond_wait(&interrupt_signal, &interrupt_lock);
  pthread_mutex_unlock(&interrupt_lock);
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
  app_main();
}

void*
serial_thread(void*) {
  while (true) {
    uint8_t buf[64];
    ssize_t nbytes = read(STDIN_FILENO, &buf, sizeof(buf));
    for (ssize_t i = 0; i < nbytes; ++i) {
      serial_add_byte(buf[i]);
    }
  }
  return NULL;
}

void*
timer_thread(void*) {
  while (true) {
    for (int i = 0; i < 375; ++i) {
      usleep(16);
      ticker_on_tick();
      signal_interrupt();
    }
    ticker_on_macrotick();
  }
}

// The LED matrix is driven as a 3 rows by 9 columns grid.
// This table is a mapping of the 5x5 grid to the 3x9 grid.
// Each entry is a row pin (13+r) and a column pin (4+c).
// To turn on a LED, it sets the row high and the column low.
// Note that 3x9=27, so two states are unused ({1,7}, {1,8}).
struct LedPins {
  uint8_t r;
  uint8_t c;
};
const LedPins led_pin_map[25] = {
    {0, 0},  // 0, 0
    {1, 3},  // 1, 0
    {0, 1},  // 2, 0
    {1, 4},  // 3, 0
    {0, 2},  // 4, 0
    {2, 3},  // 0, 1
    {2, 4},  // 1, 1
    {2, 5},  // 2, 1
    {2, 6},  // 3, 1
    {2, 7},  // 4, 1
    {1, 1},  // 0, 2
    {0, 8},  // 1, 2
    {1, 2},  // 2, 2
    {2, 8},  // 3, 2
    {1, 0},  // 4, 2
    {0, 7},  // 0, 3
    {0, 6},  // 1, 3
    {0, 5},  // 2, 3
    {0, 4},  // 3, 3
    {0, 3},  // 4, 3
    {2, 2},  // 0, 4
    {1, 6},  // 1, 4
    {2, 0},  // 2, 4
    {1, 5},  // 3, 4
    {2, 1},  // 4, 4
};

void
print_row_col_bits(uint32_t gpio) {
  for (int i = 0; i < 3; ++i) {
    fprintf(stderr, "%c", (gpio & (1 << (13 + i))) ? '1' : '0');
  }
  fprintf(stderr, "  ");
  for (int i = 0; i < 9; ++i) {
    fprintf(stderr, "%c", (gpio & (1 << (4 + i))) ? '1' : '0');
  }
  fprintf(stderr, "\n");
}

bool
is_led_on(int led, uint32_t gpio_state) {
  int row_pin = 13 + led_pin_map[led].r;
  int col_pin = 4 + led_pin_map[led].c;
  return (gpio_state & (1 << row_pin)) && !(gpio_state & (1 << col_pin));
}

uint8_t led_brightness[25] = { 0 };

void*
updates_thread(void*) {
  int updates_fd = open("___device_updates", O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);

  while (true) {
    uint32_t leds[25] = {0};
    for (int i = 0; i < 2500; ++i) {
      usleep(10);
      uint32_t gpio = get_gpio_state();
      for (int led = 0; led < 25; ++led) {
        if (is_led_on(led, gpio)) {
          ++leds[led];
        }
      }
    }
    
    bool diff = false;
    for (int i = 0; i < 25; ++i) {
      int b = std::min(9U, leds[i] / 80);
      if (b != led_brightness[i]) {
	diff = true;
	led_brightness[i] = b;
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
	  snprintf(jsonp, jsonp_end - jsonp, "%d,", led_brightness[y*5 + x]);
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
client_thread(void*) {
  // Create and truncate the file.
  int client_fd = open("___client_events", O_CREAT | O_TRUNC | O_RDONLY, S_IRUSR | S_IWUSR);

  // Set up a notify for the file.
  int notify_fd = inotify_init1(0);
  int client_wd = inotify_add_watch(notify_fd, "___client_events", IN_MODIFY);

  while (true) {
    uint8_t buf[4096] __attribute__ ((aligned(__alignof__(inotify_event))));
    ssize_t len = read(notify_fd, buf, sizeof(buf));
    if (len == -1 && errno != EAGAIN) {
      perror("inotify read");
      break;
    }

    const struct inotify_event* event;
    for(uint8_t* p = buf; p < buf + len; p += sizeof(inotify_event) + event->len) {
      event = reinterpret_cast<inotify_event*>(p);
      if (event->wd == client_wd) {
	process_client_event(client_fd);
      }
    }
  }

  close(notify_fd);
  close(client_fd);
  
  while (true) {
    uint8_t buf[1024];
    ssize_t nbytes = read(client_fd, &buf, sizeof(buf));
    fprintf(stderr, "client event %d\n", nbytes);
  }
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

void handle_sigint(int sig) {
  fprintf(stderr, "INT\n");
  unbuffered_terminal(false);
  exit(1);
}

typedef void* (*thread_start_fn)(void*);
}

int
main(int, char**) {
  unbuffered_terminal(true);

  signal(SIGINT, &handle_sigint);

  thread_start_fn thread_fns[] = {&code_thread, &serial_thread, &timer_thread, &updates_thread, &client_thread};
  const int NUM_THREADS = sizeof(thread_fns) / sizeof(thread_start_fn);
  pthread_t threads[NUM_THREADS];

  pthread_cond_init(&interrupt_signal, NULL);
  pthread_mutex_init(&interrupt_lock, NULL);

  for (int i = 0; i < NUM_THREADS; ++i) {
    pthread_create(&threads[i], NULL, thread_fns[i], NULL);
  }

  fprintf(stdout, "Threads running\n");

  for (int i = 0; i < NUM_THREADS; ++i) {
    void* result;
    pthread_join(threads[i], &result);
  }

  pthread_cond_destroy(&interrupt_signal);
  pthread_mutex_destroy(&interrupt_lock);

  unbuffered_terminal(false);
  return 0;
}
