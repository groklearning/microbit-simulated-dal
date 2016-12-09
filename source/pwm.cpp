#include "Hardware.h"

#include <stdint.h>

extern "C" {

void
pwm_start() {
}

void
pwm_stop() {
}

int32_t
pwm_set_period_us(int32_t us) {
  return 0;
}

int32_t
pwm_get_period_us() {
  return 20000;
}

int
pwm_set_duty_cycle(int32_t pin, int32_t value) {
  get_gpio_pin(pin).set_pwm(value);
}
}
