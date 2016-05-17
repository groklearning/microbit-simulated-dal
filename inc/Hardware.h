#ifndef __HARDWARE_H
#define __HARDWARE_H

void serial_add_byte(uint8_t c);

void set_gpio_state(uint32_t);
uint32_t get_gpio_state();

void get_led_brightness(uint32_t* b);

#endif
