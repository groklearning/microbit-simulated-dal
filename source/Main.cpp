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

// Interface to the hardware simulation (gpio, ticker, etc).
#include "Hardware.h"

// For the MICROBIT_PIN_* constants.
#include "MicroBitPin.h"

extern "C" {
#include "json.h"
}

namespace {
// Used to wake up the code thread from __WFI.
pthread_cond_t interrupt_signal;
pthread_mutex_t interrupt_lock;

// Used to provide mutex for all state accessed by both the micropython VM and the main simulator thread.
pthread_mutex_t code_lock;

// Set by the INT handler to cleanly shutdown the simulator.
volatile bool shutdown = false;

jmp_buf code_quit_jmp;
}

extern "C" {
// Called on every jump in the VM.
// We briefly bounce the code lock to allow the main thread to alter any state necessary.
// And handle the shutdown/panic/reset flags if necessary.
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

// The __WFI() macro calls this.
// Blocks the code thread until signal_interrupt() is called.
// On real hardware, this polls a register.
// Mostly used to halt until serial data is available in the REPL.
// We also use this to handle the shutdown (or reset or panic) flags being set.
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
  // This should probably be in Hardware.cpp.
  // We need to provide a definition, but there's currently no usages
  // that we need to do anything.
}

void
__disable_irq() {
}

namespace {
// Unblock the VM if it's sitting in __WFI().
void
signal_interrupt() {
  pthread_mutex_lock(&interrupt_lock);
  pthread_cond_signal(&interrupt_signal);
  pthread_mutex_unlock(&interrupt_lock);
}

// Run the MicroPython VM.
void*
code_thread(void*) {
  while (true) {
    // If we attempt to shutdown or reboot, we will longjump back to here.
    if (setjmp(code_quit_jmp) == 0) {
      // Jump set, run the VM.

      // Must be holding the lock while running the VM.
      pthread_mutex_lock(&code_lock);
      app_main();
      
      // Normal termination (this should never happen - app_main doesn't return).
      return NULL;
    } else {
      // A longjmp happened.
      if (shutdown) {
	// Ctrl-C or panic().
	return NULL;
      } else {
	// Reset. (From within MicroPython, e.g. reset()).
	continue;
      }
    }
  }
  return NULL;
}

// In the same style as MICROBIT_PIN_* (defined in MicroBitPin.h), add two special ones
// for pins 17&18 and 21&22.
const uint32_t MICROBIT_PIN_3V = INT_MAX;
const uint32_t MICROBIT_PIN_GND = INT_MAX - 1;

// Mapping of micro:bit pin number (0-20) to actual NRF pin number.
uint32_t MICROBIT_PIN_MAP[23] = {MICROBIT_PIN_P0,  MICROBIT_PIN_P1,  MICROBIT_PIN_P2,  MICROBIT_PIN_P3,  MICROBIT_PIN_P4,  MICROBIT_PIN_P5,
                                 MICROBIT_PIN_P6,  MICROBIT_PIN_P7,  MICROBIT_PIN_P8,  MICROBIT_PIN_P9,  MICROBIT_PIN_P10, MICROBIT_PIN_P11,
                                 MICROBIT_PIN_P12, MICROBIT_PIN_P13, MICROBIT_PIN_P14, MICROBIT_PIN_P15, MICROBIT_PIN_P16, MICROBIT_PIN_3V,
                                 MICROBIT_PIN_3V,  MICROBIT_PIN_P19, MICROBIT_PIN_P20, MICROBIT_PIN_GND, MICROBIT_PIN_GND};

// Generate json string from a list of uint32_t.
// {1,2,3} --> '"<field>:" [1,2,3]'
void
list_to_json(const char* field, char** json_ptr, char* json_end, uint32_t* values, size_t len) {
  if (len == 0) {
    snprintf(*json_ptr, json_end - *json_ptr, "\"%s\": []", field);
    *json_ptr += strnlen(*json_ptr, json_end - *json_ptr);
  } else {
    snprintf(*json_ptr, json_end - *json_ptr, "\"%s\": [", field);
    *json_ptr += strnlen(*json_ptr, json_end - *json_ptr);

    for (int i = 0; i < len; ++i) {
      snprintf(*json_ptr, json_end - *json_ptr, "%d,", values[i]);
      *json_ptr += strnlen(*json_ptr, json_end - *json_ptr);
    }

    // Replace trailing comma with ']'.
    *(*json_ptr - 1) = ']';
  }
}

// Called periodically (currently every macro tick) to send GPIO pin state back to the client.
// We're less strict about waiting for changes to stabilize (compared to the LED matrix) because we're not dealing with things like row/column scanning.
// There are 25 pins on the micro:bit, but we exclude the final two (3.3V and GND).
// Pin states are in the GpioPinState enum in Hardware.h.
// TODO(jim): Break this up into mode and value.
void
check_gpio_updates(int updates_fd) {
  static uint32_t prev_pins[23] = {0};

  pthread_mutex_lock(&code_lock);

  uint32_t pins[23] = {0};
  for (int i = 0; i < sizeof(pins) / sizeof(uint32_t); ++i) {
    int pin = MICROBIT_PIN_MAP[i];
    if (pin == MICROBIT_PIN_3V) {
      // pins 17 & 18 -- fixed to 3.3V
      pins[i] = 1;
    } else if (pin == MICROBIT_PIN_GND) {
      // pins 21 & 22 -- fixed to 0V
      pins[i] = 0;
    } else {
      // Get the simulated pin state.
      pins[i] = get_gpio_pin(pin).get_state();
    }
  }

  pthread_mutex_unlock(&code_lock);


  if (memcmp(pins, prev_pins, sizeof(prev_pins)) != 0) {
    char json[1024];
    char* json_ptr = json;
    char* json_end = json + sizeof(json);
    snprintf(json_ptr, json_end - json_ptr, "[{ \"type\": \"microbit_pins\", \"ticks\": %d, \"data\": {", get_macro_ticks());
    json_ptr += strnlen(json_ptr, json_end - json_ptr);

    list_to_json("p", &json_ptr, json_end, pins, sizeof(pins) / sizeof(uint32_t));

    snprintf(json_ptr, json_end - json_ptr, "}}]\n");
    json_ptr += strnlen(json_ptr, json_end - json_ptr);

    write(updates_fd, json, json_ptr - json);

    memcpy(prev_pins, pins, sizeof(prev_pins));
  }
}

// Convert the 0-375 scale (ticks on per frame) to the 0-9 scale used by micropython
// and our web UI.
uint32_t
ticks_to_brightness(int ticks) {
  static uint32_t limits[] = {0,2,4,8,15,28,53,102,199,375};
  for (int i = 9; i >= 0; --i) {
    if (ticks >= limits[i]) {
      return i;
    }
  }
  return 0;
}

// Called periodically (currently every macro tick) to send LED matrix changes back
// to the client.
// Sometimes it takes a few frames to stabilize (i.e. if an image change occurs mid-frame), so we require at least 3 identical frames before sending to the client.
// The DisplayLed class returns brightness() as a number of ticks that it was turned on for the last complete frame. This will be a maximum of 375 ticks (1/3 of the time) - with three rows, a frame takes 3 macro ticks (1125 ticks).
void
check_led_updates(int updates_fd) {
  uint32_t leds[25] = {0};
  static uint32_t leds1[25] = {INT_MAX};
  static uint32_t count = 0;

  // Get the LED brightness, and convert to our 0-9 scale.
  pthread_mutex_lock(&code_lock);
  for (int i = 0; i < 25; ++i) {
    leds[i] = ticks_to_brightness(get_display_led(i).brightness());
  }
  pthread_mutex_unlock(&code_lock);

  // Update count (the number of identical frames we've seen).
  if (memcmp(leds, leds1, sizeof(leds)) == 0) {
    ++count;
  } else {
    count = 0;
    memcpy(leds1, leds, sizeof(leds)); 
  }

  // If we see three, send it to the client.
  if (count == 3) {
    char json[1024];
    char* json_ptr = json;
    char* json_end = json + sizeof(json);
    snprintf(json_ptr, json_end - json_ptr, "[{ \"type\": \"microbit_leds\", \"ticks\": %d, \"data\": {", get_macro_ticks());
    json_ptr += strnlen(json_ptr, json_end - json_ptr);

    list_to_json("b", &json_ptr, json_end, leds, sizeof(leds) / sizeof(uint32_t));

    snprintf(json_ptr, json_end - json_ptr, "}}]\n");
    json_ptr += strnlen(json_ptr, json_end - json_ptr);

    write(updates_fd, json, json_ptr - json);
  }
}

// Button updates are formatted as:
// { id: <0 or 1>, state: <0 or 1> }
// where 1 means 'down'.
void
process_client_button(const json_value* data) {
  const json_value* id = json_value_get(data, "id");
  const json_value* state = json_value_get(data, "state");
  if (!id || !state || id->type != JSON_VALUE_TYPE_NUMBER || state->type != JSON_VALUE_TYPE_NUMBER) {
    fprintf(stderr, "Button event missing id and/or state\n");
    return;
  }

  pthread_mutex_lock(&code_lock);

  int pin = (id->as.number == 0) ? BUTTON_A : BUTTON_B;
  // Buttons have (external) pull-up resistors (so pressing the button sets the pin low).
  // See comment in main() but we rely on the fact that set_input_voltage() overrides
  // the pin's pull-up/down state (i.e. this is a perfect voltage source).
  // See Hardware.cpp GpioPin::get_voltage() which ignores pull mode if analog voltage
  // is non-NaN.
  double v = (state->as.number == 0) ? 3.3 : 0;
  get_gpio_pin(pin).set_input_voltage(v);

  pthread_mutex_unlock(&code_lock);
}

// Accelerometer updates are formatted as:
// { x: <number>, y: <number>, z: <number> }
// The values correspond to the values read by accelerometer.get_*().
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

// Magnetometer updates are formatted as:
// { x: <number>, y: <number>, z: <number> }
// The values correspond to the values read by compass.get_*().
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

// Pin updates are formatted as:
// { a: [<voltage>, ...] }
// The voltage can be 'null' for disconnected.
void
process_client_pins(const json_value* data) {
  const json_value* analog_values = json_value_get(data, "a");
  if (!analog_values || analog_values->type != JSON_VALUE_TYPE_ARRAY) {
    fprintf(stderr, "Pin analog values missing.\n");
    return;
  }
  
  pthread_mutex_lock(&code_lock);

  int i = 0;
  const json_value_list* pin = analog_values->as.pairs;
  while (pin && i <= 20) {
    if (pin->value->type != JSON_VALUE_TYPE_NULL) {
      double a = pin->value->as.number;
      get_gpio_pin(i).set_input_voltage(a);
    }
    pin = pin->next;
    ++i;
  }
  
  pthread_mutex_unlock(&code_lock);
}

// Handle an array of json events that we read from the pipe/file.
// All json events are at a minimum:
//   { "type": "<string>", "data": { <object> } }
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
	// Button state change.
        process_client_button(event_data);
      } else if (strncmp(event_type->as.string, "microbit_accel", 14) == 0) {
	// Accelerometer values change.
        process_client_accel(event_data);
      } else if (strncmp(event_type->as.string, "microbit_magnet", 15) == 0) {
	// Compass values change.
        process_client_magnet(event_data);
      } else if (strncmp(event_type->as.string, "microbit_pins", 13) == 0) {
	// Something driving the GPIO pins.
        process_client_pins(event_data);
      } else {
        fprintf(stderr, "Unknown event type: %s\n", event_type->as.string);
      }
    }
    event = event->next;
  }
}

// Handle an epoll event from either the pipe or the file.
// Each line of the file/pipe should be a JSON-formatted array of events.
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

// Main thread - does everything except for running the micropython VM.
// The epoll loop is responsible for:
//  - Reading client events from both
//    - a file called ___client_events
//    - a pipe (fd provided as GROK_CLIENT_PIPE)
//  - Reading serial data from STDIN
//  - Generating timer events.
// We can't epoll a file (so we inotify), but we can't inotify a file on a FUSE filesystem.
// So we provide the client events in the pipe as a backup. The Grok terminal uses the pipe, but the file is useful for local testing. See utils/*.sh for scripts to generate events.
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
  uint32_t macroticks_last_led_update = 0;

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

        if (get_macro_ticks() != macroticks_last_led_update) {
          check_led_updates(updates_fd);
          check_gpio_updates(updates_fd);
          macroticks_last_led_update = get_macro_ticks();
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

// Makes STDIN non-blocking.
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

// Custom handler for INT, allows cleanup to happen. In particular, restoring
// the terminal.
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

}  // namespace

// Copy of the definition of the _appended_script_t struct in mprun.c.
// In a real microbit-micropython firmware, one of these is expected to
// be found this at 0x3e000.
// We instead leave it as an extern and provide one below.
extern "C" {
typedef uint8_t byte;
typedef struct _appended_script_t {
  byte header[2];  // should be "MP"
  uint16_t len;    // length of script stored little endian
  char str[];      // data of script
} appended_script_t;

// Our modification to mprun.c expects to find one of these.
struct _appended_script_t* initial_script;
}

int
main(int argc, char** argv) {
  unbuffered_terminal(true);

  // Emulate the pull-ups on the button pins.
  // Note: MicroBitButton floats the pin on creation, but MicroBitPin doesn't
  // change the mode until the first read() (to PullDown).
  // On a real microbit, this means that if you ever read from pin5 or pin11,
  // you lose the ability to use the buttons.
  // TODO(jim): Send patch.
  get_gpio_pin(BUTTON_A).set_input_voltage(3.3);
  get_gpio_pin(BUTTON_B).set_input_voltage(3.3);

  // Load the program from the command line args, defaulting to microbit import
  // if nothing specified.
  char script[102400] = "from microbit import *\n";
  if (argc > 1 && argv[1] != NULL && strlen(argv[1]) > 0) {
    int program_fd = open("program.py", O_RDONLY);
    if (program_fd != -1) {
      ssize_t len = read(program_fd, script, sizeof(script));
      script[len] = 0;
    }
    close(program_fd);
  }

  // Create the "appended_script_t" struct that mprun.c expects.
  initial_script = static_cast<_appended_script_t*>(malloc(sizeof(initial_script) + strlen(script) + 2));
  initial_script->header[0] = 'M';
  initial_script->header[1] = 'P';
  initial_script->len = strlen(script);
  strcpy(initial_script->str, script);

  // Install an INT handler so that we can make Ctrl-C clean up nicely.
  struct sigaction sa;
  sa.sa_handler = handle_sigint;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);

  // Initialize a condvar/mutex for signalling interrupts, and a mutex for
  // synchronizing access to state owned by the code thread (basically anything
  // in the microbit code and Hardware.cpp).
  pthread_cond_init(&interrupt_signal, NULL);
  pthread_mutex_init(&interrupt_lock, NULL);
  pthread_mutex_init(&code_lock, NULL);

  // Create threads.
  typedef void* (*thread_start_fn)(void*);
  thread_start_fn thread_fns[] = {&code_thread};
  const int NUM_THREADS = sizeof(thread_fns) / sizeof(thread_start_fn);
  pthread_t threads[NUM_THREADS];
  for (int i = 0; i < NUM_THREADS; ++i) {
    pthread_create(&threads[i], NULL, thread_fns[i], NULL);
  }

  // Run the main thread (on the main thread).
  main_thread(NULL);

  // After main terminates, join on all other threads.
  for (int i = 0; i < NUM_THREADS; ++i) {
    void* result;
    pthread_join(threads[i], &result);
  }

  // Clean up mutexes / condvars and the starting script.
  pthread_cond_destroy(&interrupt_signal);
  pthread_mutex_destroy(&interrupt_lock);
  pthread_mutex_destroy(&code_lock);
  free(initial_script);

  // Reset the terminal.
  unbuffered_terminal(false);

  // If the reset flag is set, signal to the driver that we want to be re-run.
  if (get_reset_flag()) {
    return 99;
  } else {
    return 0;
  }
}
