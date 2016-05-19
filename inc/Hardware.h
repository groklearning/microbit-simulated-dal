#ifndef __HARDWARE_H
#define __HARDWARE_H

#include "PinNames.h"

void serial_add_byte(uint8_t c);

void set_gpio_state(uint32_t);
uint32_t get_gpio_state();

void set_gpio_pin_input(uint32_t);
void set_gpio_pin_output(uint32_t);
void set_gpio_pin_pull_mode(uint32_t, PinMode);

void get_led_ticks(uint32_t* b);

void set_accelerometer(int16_t x, int16_t y, int16_t z);
void get_accelerometer(int16_t* x, int16_t* y, int16_t* z);

void set_magnetometer(int32_t x, int32_t y, int32_t z);
void get_magnetometer(int32_t* x, int32_t* y, int32_t* z);

#endif
