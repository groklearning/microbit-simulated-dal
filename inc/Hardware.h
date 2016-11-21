#ifndef __HARDWARE_H
#define __HARDWARE_H

#include <stddef.h>
#include <stdint.h>

// This file is the interface between the mock implementation of the micro:bit DAL
// and the event loop in Main.cpp.

#include "PinNames.h"

#include "MicroBitAccelerometer.h"

const size_t FLASH_ROM_SIZE = 200*1024;
const size_t MAX_SCRIPT_SIZE = 100*1024;

extern uint8_t* flash_rom;

void serial_add_byte(uint8_t c);

enum GpioPinState {
  GPIO_PIN_OUTPUT_LOW = 0,
  GPIO_PIN_OUTPUT_HIGH,
  GPIO_PIN_OUTPUT_PWM,
  GPIO_PIN_INPUT_FLOATING,
  GPIO_PIN_INPUT_FLOATING_LOW,
  GPIO_PIN_INPUT_FLOATING_HIGH,
  GPIO_PIN_INPUT_UP_LOW,
  GPIO_PIN_INPUT_UP_HIGH,
  GPIO_PIN_INPUT_DOWN_LOW,
  GPIO_PIN_INPUT_DOWN_HIGH,
  GPIO_PIN_RESERVED,
};

class GpioPin {
 private:
  uint32_t _pin;
  bool _output;
  bool _is_output;
  PinMode _pull;
  double _analog;
  bool _is_pwm;
  // 0-1024
  uint32_t _pwm_dutycycle;
  // in microseconds.
  uint32_t _pwm_period;
  uint32_t _on_ticks;
  uint32_t _duration_ticks;
 public:
  GpioPin(uint32_t pin);
  void set_input_mode(PinMode pull);
  void set_output_mode();
  bool set_low();
  bool set_high();
  bool set_digital(bool d);
  bool set_input_voltage(double a);
  void set_pwm(uint32_t dutycycle);
  void set_pwm_period(uint32_t us);
  double get_voltage();
  GpioPinState get_state();
  bool is_high();
  bool is_low();
  bool is_input();
  bool is_output();
  bool is_floating();
  bool is_pullup();
  bool is_pulldown();
  double get_pwm();
  double get_pwm_period();
  uint32_t ticks();
};

GpioPin& get_gpio_pin(uint32_t pin);

class DisplayLed {
private:
  uint32_t _n;
  bool _state;
  uint32_t _b;
  uint32_t _on_ticks;
  uint32_t _duration_ticks;
  bool is_on();
  uint32_t ticks();
public:
  DisplayLed(uint32_t n);
  void update();
  void calculate();
  uint32_t brightness();
};

DisplayLed& get_display_led(uint32_t n);

void set_accelerometer(int16_t x, int16_t y, int16_t z, BasicGesture g);
void get_accelerometer(int16_t* x, int16_t* y, int16_t* z, BasicGesture* g);

void set_magnetometer(int32_t x, int32_t y, int32_t z);
void get_magnetometer(int32_t* x, int32_t* y, int32_t* z);

void set_temperature(int32_t t);
void get_temperature(int32_t* t);

void set_random_state(int32_t next, int32_t repeat);
void set_random_seed(uint32_t seed);
uint32_t get_random();
bool has_exceeded_random_call_limit();
void set_random_choice(int32_t count, const char* result);
bool get_random_choice(int32_t* count, const char** result);


struct simulator_radio_frame_t {
simulator_radio_frame_t() : len(0) {}
  uint32_t len;
  char data[2048];
  uint8_t channel;
  uint32_t base0;
  uint8_t prefix0;
  uint8_t data_rate;
};

void simulator_radio_config(uint8_t channel, uint32_t base0, uint8_t prefix0, uint8_t data_rate);
void simulator_radio_send(const uint8_t* buf, uint32_t len);
bool simulator_radio_receive(uint8_t* buf, uint32_t* len);

bool simulator_radio_get_tx(simulator_radio_frame_t* f);
void simulator_radio_add_rx(simulator_radio_frame_t f);

void set_marker_failure_event(const char* category, const char* message);
bool get_marker_failure_event(const char** category, const char** message);

void set_reset_flag();
bool get_reset_flag();

void set_panic_flag();
bool get_panic_flag();

void set_disconnect_flag();
bool get_disconnect_flag();

void disable_echo();

uint32_t get_ticks();
uint32_t get_macro_ticks();

// Returns how many ticks (16us) until it should next be called.
uint32_t fire_ticker(uint32_t ticks);

void nvmc_tick();

#endif
