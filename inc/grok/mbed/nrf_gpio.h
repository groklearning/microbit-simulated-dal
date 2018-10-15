#ifndef MICROBIT_SIMULATED_HAL_NRF_GPIO_H__
#define MICROBIT_SIMULATED_HAL_NRF_GPIO_H__

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "PinNames.h"
#include "nrf51.h"
#include "device.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void nrf_gpio_cfg_output(uint8_t pin_number);
void nrf_gpio_range_cfg_output(uint32_t pin_range_start, uint32_t pin_range_end);
void nrf_gpio_pin_set(uint8_t pin_number);
void nrf_gpio_pins_set(uint32_t pin_mask);
void nrf_gpio_pin_clear(uint8_t pin_number);
void nrf_gpio_pins_clear(uint32_t pin_mask);
uint32_t nrf_gpio_pin_read(uint32_t pin_number);

#ifdef __cplusplus
}
#endif

#endif
