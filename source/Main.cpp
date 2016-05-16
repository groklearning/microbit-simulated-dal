extern "C" void app_main();

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <termios.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/inotify.h>

#include <type_traits>

#include "Hardware.h"
#include "ticker_dal.h"

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
led_thread(void*) {
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
      int b = leds[i] / 200;
      if (b != led_brightness[i]) {
	diff = true;
	led_brightness[i] = b;
      }
    }

    if (diff) {
      fprintf(stderr, "\n");
      for (int y = 0; y < 5; ++y) {
	for (int x = 0; x < 5; ++x) {
	  fprintf(stderr, "%01d ", led_brightness[y*5 + x]);
	}
	fprintf(stderr, "\n");
      }
    }
  }
}

void
process_client_event(int fd) {
  char buf[4096];
  ssize_t len = read(fd, buf, sizeof(buf));
  buf[len] = 0;
  if (buf[0] == 'A') {
    // Buttons are pull-down.
    if (buf[1] == '0') {
      set_gpio_state(get_gpio_state() | (1 << 17));
    } else {
      set_gpio_state(get_gpio_state() & ~(1 << 17));
    }
  }
  if (buf[0] == 'B') {
    if (buf[1] == '0') {
      set_gpio_state(get_gpio_state() | (1 << 26));
    } else {
      set_gpio_state(get_gpio_state() & ~(1 << 26));
    }
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

typedef void* (*thread_start_fn)(void*);
}

int
main(int, char**) {
  unbuffered_terminal(true);

  thread_start_fn thread_fns[] = {&code_thread, &serial_thread, &timer_thread, &led_thread, &client_thread};
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
