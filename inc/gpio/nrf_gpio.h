#ifndef MICROBIT_SIMULATED_HAL_NRF_GPIO_H__
#define MICROBIT_SIMULATED_HAL_NRF_GPIO_H__

#include "mbed.h"

#include <stdbool.h>

void nrf_gpio_range_cfg_output(uint32_t pin_range_start, uint32_t pin_range_end);
void nrf_gpio_pin_set(uint32_t pin_number);
void nrf_gpio_pins_set(uint32_t pin_mask);
void nrf_gpio_pin_clear(uint32_t pin_number);
void nrf_gpio_pins_clear(uint32_t pin_mask);

#endif
