extern "C" void app_main();

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <sched.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <sys/prctl.h>
#include <sys/timerfd.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
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
pthread_mutex_t interrupt_signal_lock;

volatile bool interrupt_waiting = false;
pthread_cond_t interrupt_delivered;
pthread_mutex_t interrupt_delivered_lock;

pthread_mutex_t updates_file_lock;

// In fast mode, every time we write a client update, we go to sleep until the marker
// resumes via a client event.
// Writing to the updates file locks this, and progress on the code thread blocks on it.
pthread_mutex_t suspend_lock;
volatile bool suspend = false;

// Used to provide mutex for all state accessed by both the micropython VM and the main
// simulator thread.
pthread_mutex_t code_lock;

// Set to cleanly shutdown the simulator.
volatile bool shutdown = false;

// Set to disable checking led/gpio state (e.g. while fastforwarding time waiting for a Ctrl-C to be handled).
volatile bool suppress_pin_led_updates = false;

jmp_buf code_quit_jmp;

// In interactive mode, we let the default behavior happen (microbit-micropython runs the
// main script then goes into the REPL). In non-interactive mode, we send a Ctrl-D which
// will be handled when the REPL first starts, terminating it immediately.
bool interactive = true;
volatile bool sent_ctrl_d = false;

// Rather than putting complicated locking in the signal handler, just flag that it happened
// and on the next timer tick we'll do the right thing.
volatile bool sigint_requested = false;

// Our SIGINT handler sends a Ctrl-C to the serial input. This has the following effect:
// - In the REPL, it clears the current line, and starts a new prompt.
// - If Python code is executing, it stops it and returns to the REPL.
// However, if there's an uncaught exception, microbit-micropython scrolls the message
// on the display. Which is slow and hard to read, and also it doesn't see the Ctrl-C.
// So if it's been more than 20 ticks since we attempted to deliver a Ctrl-C and the
// VM hasn't executed any instructions, then we shutdown.
// Same goes for Ctrl-D.
uint32_t signal_pending_since = 0;

// The test script can only measure time passing by seeing pin/led changes or heartbeats.
// The heartbeat allows it to measure time while the program isn't doing anything.
// 10-tick heartbeats means the marker will be at most 60ms behind.
const uint32_t HEARTBEAT_TICKS = 10;

// For the marker, disable using the timer_fd and instead just advance the ticker whenever:
//  - The micropython code calls WFI
//  - Every 100 branches.
bool fast_mode = false;

// Log every HEARTBEAT_TICKS macro ticks (to keep the marker synchronized).
bool heartbeat_mode = false;

// File descriptor to write ___device_updates.
int updates_fd = -1;

// In fast mode, in either WFI or the branch hook, this is how many ticks the microbit ticker expected.
uint32_t fast_mode_ticks_until_fire_timer = 75;

// When did we last write a heartbeat, in macro ticks (if enabled in heartbeat_mode).
uint32_t last_heartbeat = 0;

uint32_t
handle_timerfd_event(uint32_t ticks);
}

extern "C" {
// Called on every jump in the VM.
// We briefly bounce the code lock to allow the main thread to alter any state necessary.
// And handle the shutdown/panic/reset flags if necessary.
void
simulated_dal_micropy_vm_hook_loop() {
  pthread_mutex_unlock(&code_lock);

  // Code has executed since the a Ctrl-C was delivered (if any) so this means the VM is
  // processing instructions and if there was a Ctrl-C it would have been handled.
  signal_pending_since = 0;

  if (shutdown || get_reset_flag() || get_panic_flag() || get_disconnect_flag()) {
    shutdown = true;
    longjmp(code_quit_jmp, 1);
  }

  static int n = 0;
  n++;
  if (n > 100) {
    if (fast_mode) {
      // In marking mode, fire the ticker every 100 branches.
      fast_mode_ticks_until_fire_timer = handle_timerfd_event(fast_mode_ticks_until_fire_timer);

      while (true) {
	pthread_mutex_lock(&suspend_lock);
	bool b = suspend;
	pthread_mutex_unlock(&suspend_lock);
	if (!b) {
	  break;
	}
	usleep(100);
      }
    } else {
      // Every 100 branches, wait for a timer tick.
      // Should be fairly unnoticable for most programs, but will prevent tight loops using
      // ~any user CPU time.
      // While the marker is running (fast mode), this also enables the main thread to keep up with client events.
      // fast_mode doesn't have the timer_fd, but on epoll timeout, it calls signal_interrupt which achieves
      // the same thing.
      pthread_mutex_lock(&interrupt_signal_lock);
      pthread_cond_wait(&interrupt_signal, &interrupt_signal_lock);
      pthread_mutex_unlock(&interrupt_signal_lock);
    }
    n = 0;
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

  // Every time micro:bit tries to read from serial it calls __WFI first.
  // So if we're non-interactive (i.e. Run button or inline snippet) then
  // trigger micro:bit's soft reboot by sending ctrl-d. We'll detect
  // the reboot (in MicroBitDisplay, which sets the disconnect flag).
  if (!interactive && !sent_ctrl_d) {
    serial_add_byte(0x04);
    sent_ctrl_d = true;
  }

  if (fast_mode) {
    // In fast mode, the most likely reason for WFI is waiting for the timer.
    // e.g. sleep() or synchronous music.
    fast_mode_ticks_until_fire_timer = handle_timerfd_event(fast_mode_ticks_until_fire_timer);
    while (true) {
      pthread_mutex_lock(&suspend_lock);
      bool b = suspend;
      pthread_mutex_unlock(&suspend_lock);
      if (!b) {
	break;
      }
      usleep(100);
    }
  } else {
    // Wait for the interrupt signal, then let the signalling thread know that we're running.
    pthread_mutex_lock(&interrupt_signal_lock);
    interrupt_waiting = true;
    pthread_cond_wait(&interrupt_signal, &interrupt_signal_lock);
    interrupt_waiting = false;
    pthread_cond_broadcast(&interrupt_delivered);
    pthread_mutex_unlock(&interrupt_signal_lock);
  }

  if (shutdown || get_reset_flag() || get_panic_flag() || get_disconnect_flag()) {
    shutdown = true;
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
  bool wait_for_delivery = false;

  // Signal the code thread which may be in WFI or the loop/branch hook.
  pthread_mutex_lock(&interrupt_signal_lock);
  wait_for_delivery = interrupt_waiting;
  pthread_cond_broadcast(&interrupt_signal);
  if (wait_for_delivery) {
    // If the WFI thread was blocked on the signal, yield to it.
    pthread_cond_wait(&interrupt_delivered, &interrupt_signal_lock);
  }
  pthread_mutex_unlock(&interrupt_signal_lock);

  // Bounce the code lock to wait for any currently running code to go through the
  // loop hook or WFI.
  pthread_mutex_lock(&code_lock);
  pthread_mutex_unlock(&code_lock);

  sched_yield();
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
uint32_t MICROBIT_PIN_MAP[23] = {
    MICROBIT_PIN_P0,  MICROBIT_PIN_P1,  MICROBIT_PIN_P2,  MICROBIT_PIN_P3,  MICROBIT_PIN_P4,
    MICROBIT_PIN_P5,  MICROBIT_PIN_P6,  MICROBIT_PIN_P7,  MICROBIT_PIN_P8,  MICROBIT_PIN_P9,
    MICROBIT_PIN_P10, MICROBIT_PIN_P11, MICROBIT_PIN_P12, MICROBIT_PIN_P13, MICROBIT_PIN_P14,
    MICROBIT_PIN_P15, MICROBIT_PIN_P16, MICROBIT_PIN_3V,  MICROBIT_PIN_3V,  MICROBIT_PIN_P19,
    MICROBIT_PIN_P20, MICROBIT_PIN_GND, MICROBIT_PIN_GND};

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

void write_to_updates(const void* buf, size_t count, bool should_suspend = false) {
  pthread_mutex_lock(&updates_file_lock);
  if (should_suspend && fast_mode) {
    pthread_mutex_lock(&suspend_lock);
    suspend = true;
    pthread_mutex_unlock(&suspend_lock);
  }
  int status = write(updates_fd, buf, count);
  pthread_mutex_unlock(&updates_file_lock);
}

// Called periodically (currently every macro tick) to send GPIO pin state back to the client.
// We're less strict about waiting for changes to stabilize (compared to the LED matrix) because
// we're not dealing with things like row/column scanning.
// There are 25 pins on the micro:bit, but we exclude the final two (3.3V and GND).
// Pin states are in the GpioPinState enum in Hardware.h.
// TODO(jim): Break this up into mode and value.
void
check_gpio_updates() {
  static uint32_t prev_pins[23] = {0};
  static uint32_t prev_pwm_dutycycle[23] = {0};
  static uint32_t prev_pwm_period[23] = {0};

  if (suppress_pin_led_updates) {
    return;
  }

  pthread_mutex_lock(&code_lock);

  uint32_t pins[23] = {0};
  uint32_t pwm_dutycycle[23] = {0};
  uint32_t pwm_period[23] = {0};

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
      if (pins[i] == GPIO_PIN_OUTPUT_PWM) {
	pwm_dutycycle[i] = get_gpio_pin(pin).get_pwm();
	pwm_period[i] = get_gpio_pin(pin).get_pwm_period();
      }
    }
  }

  pthread_mutex_unlock(&code_lock);

  if (memcmp(pins, prev_pins, sizeof(prev_pins)) != 0
      || memcmp(pwm_dutycycle, prev_pwm_dutycycle, sizeof(prev_pwm_dutycycle)) != 0
      || memcmp(pwm_period, prev_pwm_period, sizeof(prev_pwm_period)) != 0) {
    char json[1024];
    char* json_ptr = json;
    char* json_end = json + sizeof(json);
    snprintf(json_ptr, json_end - json_ptr,
             "[{ \"type\": \"microbit_pins\", \"ticks\": %d, \"data\": {", get_macro_ticks());
    json_ptr += strnlen(json_ptr, json_end - json_ptr);

    list_to_json("p", &json_ptr, json_end, pins, sizeof(pins) / sizeof(uint32_t));

    snprintf(json_ptr, json_end - json_ptr, ", ");
    json_ptr += strnlen(json_ptr, json_end - json_ptr);
    list_to_json("pwmd", &json_ptr, json_end, pwm_dutycycle, sizeof(pwm_dutycycle) / sizeof(uint32_t));

    snprintf(json_ptr, json_end - json_ptr, ", ");
    json_ptr += strnlen(json_ptr, json_end - json_ptr);
    list_to_json("pwmp", &json_ptr, json_end, pwm_period, sizeof(pwm_period) / sizeof(uint32_t));

    snprintf(json_ptr, json_end - json_ptr, "}}]\n");
    json_ptr += strnlen(json_ptr, json_end - json_ptr);

    write_to_updates(json, json_ptr - json, true);

    memcpy(prev_pins, pins, sizeof(prev_pins));
    memcpy(prev_pwm_dutycycle, pwm_dutycycle, sizeof(prev_pwm_dutycycle));
    memcpy(prev_pwm_period, pwm_period, sizeof(prev_pwm_period));
  }
}

// Convert the 0-375 scale (ticks on per frame) to the 0-9 scale used by micropython
// and our web UI.
uint32_t
ticks_to_brightness(int ticks) {
  static uint32_t limits[] = {0, 2, 4, 8, 15, 28, 53, 102, 199, 375};
  for (int i = 9; i >= 0; --i) {
    if (ticks >= limits[i]) {
      return i;
    }
  }
  return 0;
}

// Called periodically (currently every macro tick) to send LED matrix changes back
// to the client.
// The DisplayLed class returns brightness() as a number of ticks that it was turned on for the last
// complete frame. This will be a maximum of 375 ticks (1/3 of the time) - with three rows, a frame
// takes 3 macro ticks (1125 ticks).
void
check_led_updates() {
  uint32_t leds[25] = {0};
  static uint32_t leds_prev[25] = {INT_MAX};

  if (suppress_pin_led_updates) {
    return;
  }

  // Get the LED brightness, and convert to our 0-9 scale.
  pthread_mutex_lock(&code_lock);
  for (int i = 0; i < 25; ++i) {
    leds[i] = ticks_to_brightness(get_display_led(i).brightness());
  }
  pthread_mutex_unlock(&code_lock);

  // If it's changed since the last update, send update.
  if (memcmp(leds, leds_prev, sizeof(leds)) != 0) {
    char json[1024];
    char* json_ptr = json;
    char* json_end = json + sizeof(json);
    snprintf(json_ptr, json_end - json_ptr,
             "[{ \"type\": \"microbit_leds\", \"ticks\": %d, \"data\": {", get_macro_ticks());
    json_ptr += strnlen(json_ptr, json_end - json_ptr);

    list_to_json("b", &json_ptr, json_end, leds, sizeof(leds) / sizeof(uint32_t));

    snprintf(json_ptr, json_end - json_ptr, "}}]\n");
    json_ptr += strnlen(json_ptr, json_end - json_ptr);

    write_to_updates(json, json_ptr - json, true);

    memcpy(leds_prev, leds, sizeof(leds));
  }
}

// Returns the number of macro ticks that we expect should have passed (based on the real clock).
// This only makes sense in normal mode (i.e. not fast mode).
uint32_t
expected_macro_ticks(bool reset = false) {
  static uint32_t starting_clock = 0;

  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC_COARSE, &t);
  uint32_t clock_ticks = (t.tv_sec * 1000 + (t.tv_nsec / 1000000)) / 6;

  if (starting_clock == 0 || reset) {
    starting_clock = clock_ticks - get_macro_ticks();
  }

  return clock_ticks - starting_clock;
}

void
write_heartbeat() {
  char json[1024];
  char* json_ptr = json;
  char* json_end = json + sizeof(json);

  snprintf(json_ptr, json_end - json_ptr,
	   "[{ \"type\": \"microbit_heartbeat\", \"ticks\": %d, \"data\": { \"real_ticks\": \"%d\" }}]\n", get_macro_ticks(), expected_macro_ticks());
  json_ptr += strnlen(json_ptr, json_end - json_ptr);

  write_to_updates(json, json_ptr - json, true);
}

void
write_bye() {
  char json[1024];
  char* json_ptr = json;
  char* json_end = json + sizeof(json);

  snprintf(json_ptr, json_end - json_ptr,
	   "[{ \"type\": \"microbit_bye\", \"ticks\": %d, \"data\": { \"real_ticks\": \"%d\" }}]\n", get_macro_ticks(), expected_macro_ticks());
  json_ptr += strnlen(json_ptr, json_end - json_ptr);

  write_to_updates(json, json_ptr - json, false);
}

void
write_event_ack() {
  char json[1024];
  char* json_ptr = json;
  char* json_end = json + sizeof(json);

  snprintf(json_ptr, json_end - json_ptr,
	   "[{ \"type\": \"microbit_ack\", \"ticks\": %d, \"data\": {}}]\n", get_macro_ticks());
  json_ptr += strnlen(json_ptr, json_end - json_ptr);

  write_to_updates(json, json_ptr - json, false);
}

// Button updates are formatted as:
// { id: <0 or 1>, state: <0 or 1> }
// where 1 means 'down'.
void
process_client_button(const json_value* data) {
  const json_value* id = json_value_get(data, "id");
  const json_value* state = json_value_get(data, "state");
  if (!id || !state || id->type != JSON_VALUE_TYPE_NUMBER ||
      state->type != JSON_VALUE_TYPE_NUMBER) {
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

  // Make the code thread run with the new state.
  signal_interrupt();

  write_event_ack();
}

// Accelerometer updates are formatted as:
// { x: <number>, y: <number>, z: <number> }
// The values correspond to the values read by accelerometer.get_*().
void
process_client_accel(const json_value* data) {
  const json_value* x = json_value_get(data, "x");
  const json_value* y = json_value_get(data, "y");
  const json_value* z = json_value_get(data, "z");
  if (!x || !y || !z || x->type != JSON_VALUE_TYPE_NUMBER || y->type != JSON_VALUE_TYPE_NUMBER ||
      z->type != JSON_VALUE_TYPE_NUMBER) {
    fprintf(stderr, "Accelerometer event missing x/y/z.\n");
    return;
  }

  pthread_mutex_lock(&code_lock);
  set_accelerometer(x->as.number, y->as.number, z->as.number);
  pthread_mutex_unlock(&code_lock);

  // Make the code thread run with the new state.
  signal_interrupt();

  write_event_ack();
}

// Magnetometer updates are formatted as:
// { x: <number>, y: <number>, z: <number> }
// The values correspond to the values read by compass.get_*().
void
process_client_magnet(const json_value* data) {
  const json_value* x = json_value_get(data, "x");
  const json_value* y = json_value_get(data, "y");
  const json_value* z = json_value_get(data, "z");
  if (!x || !y || !z || x->type != JSON_VALUE_TYPE_NUMBER || y->type != JSON_VALUE_TYPE_NUMBER ||
      z->type != JSON_VALUE_TYPE_NUMBER) {
    fprintf(stderr, "Magnetometer event missing x/y/z.\n");
    return;
  }

  pthread_mutex_lock(&code_lock);
  set_magnetometer(x->as.number, y->as.number, z->as.number);
  pthread_mutex_unlock(&code_lock);

  // Make the code thread run with the new state.
  signal_interrupt();

  write_event_ack();
}

// Pin updates are formatted as:
// { "pin": N, "voltage": V }
// The voltage can be 'null' for disconnected.
void
process_client_pins(const json_value* data) {
  const json_value* pin = json_value_get(data, "pin");
  const json_value* voltage = json_value_get(data, "voltage");
  if (!pin || !voltage || pin->type != JSON_VALUE_TYPE_NUMBER || voltage->type != JSON_VALUE_TYPE_NUMBER) {
    fprintf(stderr, "Pin number or voltage missing.\n");
    return;
  }

  int pin_mb = uint32_t(pin->as.number);
  if (pin_mb > 20) {
    fprintf(stderr, "Invalid pin number.\n");
    return;
  }

  int pin_nrf = MICROBIT_PIN_MAP[pin_mb];
  pthread_mutex_lock(&code_lock);
  get_gpio_pin(pin_nrf).set_input_voltage(voltage->as.number);
  pthread_mutex_unlock(&code_lock);

  // Make the code thread run with the new state.
  signal_interrupt();

  write_event_ack();
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
    if (!event_type || !event_data || event_type->type != JSON_VALUE_TYPE_STRING ||
        event_data->type != JSON_VALUE_TYPE_OBJECT) {
      fprintf(stderr, "Event missing type and/or data.\n");
    } else {
      if (strncmp(event_type->as.string, "resume", 6) == 0) {
	pthread_mutex_lock(&suspend_lock);
	suspend = false;
	pthread_mutex_unlock(&suspend_lock);
      } else if (strncmp(event_type->as.string, "microbit_button", 15) == 0) {
        // Button state change.
        process_client_button(event_data);
      } else if (strncmp(event_type->as.string, "accelerometer", 13) == 0) {
        // Accelerometer values change.
        process_client_accel(event_data);
      } else if (strncmp(event_type->as.string, "magnetometer", 12) == 0) {
        // Compass values change.
        process_client_magnet(event_data);
      } else if (strncmp(event_type->as.string, "microbit_pin", 13) == 0) {
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

// Called when the timerfd fires.
// Fires the hardware timer and periodically updates LED & GPIO state.
// Takes the number of ticks since the last call, and returns the number of ticks
// until the next call.
uint32_t
handle_timerfd_event(uint32_t ticks) {
  static uint32_t macroticks_last_led_update = 0;

  pthread_mutex_lock(&code_lock);
  ticks = fire_ticker(ticks);
  pthread_mutex_unlock(&code_lock);

  if (!fast_mode) {
    signal_interrupt();
  }

  if (get_macro_ticks() != macroticks_last_led_update) {
    check_led_updates();
    check_gpio_updates();
    macroticks_last_led_update = get_macro_ticks();
  }

  // Periodically heartbeat if the '-t' flag is enabled.
  // This is useful for the marker to ensure that it sees an event at least every N ticks.
  if (heartbeat_mode && get_macro_ticks() >= last_heartbeat + HEARTBEAT_TICKS) {
    last_heartbeat = get_macro_ticks();
    write_heartbeat();
  }

  return ticks;
}

// Run the timer with no real sleeps, just simulating the timer events.
// This is used to flush out any LED updates on shutdown, and to fastforward past
// native code that is blocking a Ctrl-C being handled.
void
fastforward_timer(uint32_t ticks, bool stop_on_shutdown) {
  uint32_t start_ticks = get_macro_ticks();
  uint32_t ticks_until_fire_timer = 75;
  while (get_macro_ticks() < start_ticks + ticks) {
    if (stop_on_shutdown && shutdown) {
      return;
    }
    expected_macro_ticks(true);
    ticks_until_fire_timer = handle_timerfd_event(ticks_until_fire_timer);
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
// So we provide the client events in the pipe as a backup. The Grok terminal uses the pipe, but the
// file is useful for local testing. See utils/*.sh for scripts to generate events.
void
main_thread() {
  const int MAX_EVENTS = 10;
  int epoll_fd = epoll_create1(0);

  // Add non-blocking stdin to epoll set.
  struct epoll_event ev_stdin;
  fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL, 0) | O_NONBLOCK);
  ev_stdin.events = EPOLLIN;
  ev_stdin.data.fd = STDIN_FILENO;
  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &ev_stdin);

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
  if (!fast_mode) {
    timerfd_settime(timer_fd, 0, &timer_spec, NULL);
  }

  // Open the events pipe.
  char* client_pipe_str = getenv("GROK_CLIENT_PIPE");
  int client_fd = -1;
  int notify_fd = -1;
  int client_wd = -1;
  if (client_pipe_str != NULL) {
    client_fd = atoi(client_pipe_str);
    fcntl(client_fd, F_SETFL, fcntl(client_fd, F_GETFL, 0) | O_NONBLOCK);
    struct epoll_event ev_client_pipe;
    ev_client_pipe.events = EPOLLIN;
    ev_client_pipe.data.fd = client_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev_client_pipe);
  } else {
    // Create and truncate the client events file.
    client_fd = open("___client_events", O_CREAT | O_TRUNC | O_RDONLY, S_IRUSR | S_IWUSR);

    // Set up a notify for the file and add to epoll set.
    struct epoll_event ev_client;
    notify_fd = inotify_init1(0);
    client_wd = inotify_add_watch(notify_fd, "___client_events", IN_MODIFY);
    if (client_wd == -1) {
      perror("add watch");
      return;
    }
    ev_client.events = EPOLLIN;
    ev_client.data.fd = notify_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, notify_fd, &ev_client) == -1) {
      perror("epoll_ctl");
      return;
    }
  }

  // How long until we next need the timer callback to fire (in ticks).
  uint32_t ticks_until_fire_timer = 75;

  int epoll_timeout = fast_mode ? 50 : 50;

  while (!shutdown) {
    struct epoll_event events[MAX_EVENTS];
    int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, epoll_timeout);

    if (nfds == -1) {
      if (errno == EINTR) {
	// Timeout or interrupted.
	// Allow the vm branch hook to proceed.
        // Continue so that we'll catch the shutdown flag above (if it's set, otherwise continue as normal).
        continue;
      }
      perror("epoll wait\n");
      exit(1);
    }

    // In all modes, make sure Ctrl-C is handled in a timely manner - see if our SIGINT handler has been called.
    if (sigint_requested) {
      sigint_requested = false;

      // Deliver a Ctrl-C to the serial input.
      pthread_mutex_lock(&code_lock);
      serial_add_byte(0x03);
      pthread_mutex_unlock(&code_lock);

      // Make sure microbit-micropython does something with it.
      signal_pending_since = get_macro_ticks();

      // Let any code blocking on interrupts know there's serial data available.
      signal_interrupt();
    }
    // And check how long it's been since we delivered a Ctrl-C or Ctrl-D.
    if (signal_pending_since > 0 && signal_pending_since + 20 < get_macro_ticks()) {
      // We attempted to deliver a Ctrl-C or Ctrl-D but code hasn't executed in the past
      // 20 macro ticks, so it's probably stuck in the unhandled exception marquee display.
      signal_pending_since = 0;
      suppress_pin_led_updates = true;
      fastforward_timer(10000, true);
      suppress_pin_led_updates = false;
    }

    // Handle epoll timeout.
    // In fast mode, use this to keep the code thread running by calling signal_interrupt().
    // In other modes this never happens because the timer_fd fires more quickly than the epoll timeout.
    if (nfds == 0) {
      // Keep the code thread running.
      signal_interrupt();
      continue;
    }

    for (int n = 0; n < nfds; ++n) {
      if (events[n].data.fd == STDIN_FILENO) {
	// Input from stdin.
        uint8_t buf[10240];
        ssize_t len = read(STDIN_FILENO, &buf, sizeof(buf));
        if (len == -1) {
          continue;
        }
        pthread_mutex_lock(&code_lock);
        for (ssize_t i = 0; i < len; ++i) {
	  if (buf[i] == 0x04) {
	    // Make sure that the Ctrl-D gets handled by something.
	    signal_pending_since = get_macro_ticks();
	  }
	  serial_add_byte(buf[i]);
        }
        pthread_mutex_unlock(&code_lock);
        signal_interrupt();
      } else if (notify_fd != -1 && events[n].data.fd == notify_fd) {
	// A change occured to the ___client_events file.
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
      } else if (events[n].data.fd == client_fd) {
	// A write happened to the client events pipe.
        process_client_event(client_fd);
      } else if (events[n].data.fd == timer_fd) {
	// Timer callback.
        uint64_t t;
        int status = read(timer_fd, &t, sizeof(uint64_t));

	// Call the timer, telling it how many ticks have elapsed since the last call.
	// It returns the number of ticks until the next call.
	ticks_until_fire_timer = handle_timerfd_event(ticks_until_fire_timer);

	// Figure out how long to set the timerfd for. One tick is 16us.
	uint32_t sleep_nsec = 16000 * ticks_until_fire_timer;

	// But scale so that we converge on 'real' time.
	uint32_t e = expected_macro_ticks();
	if (get_macro_ticks() > e) {
	  sleep_nsec = (sleep_nsec * 11) / 10;
	} else if (get_macro_ticks() < e) {
	  uint32_t d = std::min(9U, e - get_macro_ticks());
	  sleep_nsec = (sleep_nsec * (10-d)) / 10;
	}

	// Reset the timer_fd.
        timer_spec.it_value.tv_nsec = sleep_nsec;
        timerfd_settime(timer_fd, 0, &timer_spec, NULL);
      }
    }
  }

  // Keep running the timer for 20 more macro ticks (simulates ~120ms of time passing) so
  // that any pending LED and GPIO updates get sent out.
  fastforward_timer(20, false);
  write_bye();

  signal_interrupt();

  if (notify_fd != -1) {
    close(notify_fd);
  }

  close(client_fd);
  close(timer_fd);
}

// Makes STDIN non-blocking.
void
unbuffered_terminal(bool enable) {
  static struct termios old_tio;

  if (enable) {
    tcgetattr(STDIN_FILENO, &old_tio);
    struct termios new_tio = old_tio;

    // Disable echo if this terminal doesn't have it.
    if (!(old_tio.c_lflag & ECHO)) {
      disable_echo();
    }

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
  sigint_requested = true;
}

const int SIMULATOR_RESET = 99;

int
run_simulator() {
  // Emulate the pull-ups on the button pins.
  // Note: MicroBitButton floats the pin on creation, but MicroBitPin doesn't
  // change the mode until the first read() (to PullDown).
  // On a real microbit, this means that if you ever read from pin5 or pin11,
  // you lose the ability to use the buttons.
  // TODO(jim): Send patch.
  get_gpio_pin(BUTTON_A).set_input_voltage(3.3);
  get_gpio_pin(BUTTON_B).set_input_voltage(3.3);

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
  pthread_mutex_init(&interrupt_signal_lock, NULL);
  pthread_cond_init(&interrupt_delivered, NULL);
  pthread_mutex_init(&interrupt_delivered_lock, NULL);
  pthread_mutex_init(&code_lock, NULL);

  pthread_mutex_init(&updates_file_lock, NULL);

  // Open the events pipe.
  char* updates_pipe_str = getenv("GROK_UPDATES_PIPE");
  if (updates_pipe_str != NULL) {
    updates_fd = atoi(updates_pipe_str);
  } else {
    updates_fd = open("___device_updates", O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
  }

  //pthread_mutexattr_init(&suspend_lock_attr);
  //pthread_mutexattr_settype(&suspend_lock_attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(&suspend_lock, NULL);// &suspend_lock_attr);

  if (heartbeat_mode) {
    write_heartbeat();
  }

  // Create threads.
  typedef void* (*thread_start_fn)(void*);
  thread_start_fn thread_fns[] = {&code_thread};
  const int NUM_THREADS = sizeof(thread_fns) / sizeof(thread_start_fn);
  pthread_t threads[NUM_THREADS];
  for (int i = 0; i < NUM_THREADS; ++i) {
    pthread_create(&threads[i], NULL, thread_fns[i], NULL);
  }

  // Run the main thread (on the main thread).
  main_thread();

  // After main terminates, join on all other threads.
  for (int i = 0; i < NUM_THREADS; ++i) {
    void* result;
    pthread_join(threads[i], &result);
  }

  pthread_mutex_destroy(&suspend_lock);
  //pthread_mutexattr_destroy(&suspend_lock_attr);

  close(updates_fd);
  pthread_mutex_destroy(&updates_file_lock);

  // Clean up mutexes / condvars and the starting script.
  pthread_cond_destroy(&interrupt_signal);
  pthread_mutex_destroy(&interrupt_signal_lock);
  pthread_cond_destroy(&interrupt_delivered);
  pthread_mutex_destroy(&interrupt_delivered_lock);
  pthread_mutex_destroy(&code_lock);

  // If the reset flag is set, ask the parent loop to re-run.
  return get_reset_flag() ? SIMULATOR_RESET : 0;
}

}  // namespace

uint8_t* flash_rom;

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
char* initial_script;
}

uint32_t __data_end__ = 0;
uint32_t __data_start__ = 0;
uint32_t __etext = 0;

int
main(int argc, char** argv) {
  unbuffered_terminal(true);

  // Ignore the INT handler so that it's handled by the child.
  struct sigaction sa;
  sa.sa_handler = SIG_IGN;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);

  // Load the program from the command line args, defaulting to microbit import
  // if nothing specified.
  char script[MAX_SCRIPT_SIZE] = "from microbit import *\n";
  bool interactive_override = false;
  bool debug_mode = false;
  bool script_loaded = false;

  for (int i = 1; i < argc; ++i) {
    if (strlen(argv[i]) > 0) {
      if (argv[i][0] == '-') {
	if (argv[i][1] == 'i') {
	  interactive_override = true;
	} else if (argv[i][1] == 't') {
	  heartbeat_mode = true;
	} else if (argv[i][1] == 'f') {
	  fast_mode = true;
	} else if (argv[i][1] == 'd') {
	  debug_mode = true;
	}
      } else {
	script_loaded = true;
	int program_fd = open(argv[i], O_RDONLY);
	if (program_fd != -1) {
	  ssize_t len = read(program_fd, script, sizeof(script));
	  script[len] = 0;
	}
	close(program_fd);
      }
    }
  }

  // When a script is loaded, pay attention to the -i flag.
  if (script_loaded) {
    interactive = interactive_override;
  }

  flash_rom = static_cast<uint8_t*>(malloc(FLASH_ROM_SIZE));
  memset(flash_rom, 0, FLASH_ROM_SIZE);

  __etext = reinterpret_cast<uint32_t>(flash_rom);

  // Create the "appended_script_t" struct that mprun.c expects.
  struct _appended_script_t* initial_script_struct =
      reinterpret_cast<_appended_script_t*>(flash_rom + FLASH_ROM_SIZE - MAX_SCRIPT_SIZE);
  initial_script_struct->header[0] = 'M';
  initial_script_struct->header[1] = 'P';
  initial_script_struct->len = strlen(script);
  strcpy(initial_script_struct->str, script);
  initial_script = reinterpret_cast<char*>(initial_script_struct);

  int status = 0;

  if (debug_mode) {
    run_simulator();
  } else {
    // Micropython provides a 'reset()' method that restarts the simulation.
    // Easiest way to do that is to fork() the simulation and just start a new one when it
    // signals that it wants to restart.
    while (true) {
      pid_t pid = fork();
      if (pid == -1) {
        // Error.
        break;
      } else if (pid == 0) {
        // Child. Return directly from main().
	// If the parent gets killed, term.
	prctl(PR_SET_PDEATHSIG, SIGKILL);
        return run_simulator();
      } else {
        // Parent. Block until the child exits.
        int wstatus = 0;
        waitpid(pid, &wstatus, 0);
        status = WEXITSTATUS(wstatus);
        if (status != SIMULATOR_RESET) {
          break;
        }
      }
    }
  }

  free(flash_rom);

  // Reset the terminal.
  unbuffered_terminal(false);

  return status;
}
