#ifndef __MBED_OBJECTS_H
#define __MBED_OBJECTS_H

#include <stdint.h>
#include "PinNames.h"

#define __WFI __wait_for_interrupt

void __wait_for_interrupt();

void __enable_irq();
void __disable_irq();

#ifdef __cplusplus
extern "C" {
#endif

#define I2C_SPI_PERIPHERAL_FOR_I2C 1
#define I2C_SPI_PERIPHERAL_FOR_SPI 2

typedef struct {
  uint8_t usage; // I2C: 1, SPI: 2
  uint8_t sda_mosi;
  uint8_t scl_miso;
  uint8_t sclk;
} i2c_spi_peripheral_t;

struct serial_s {
  // NRF_UART_Type *uart;
  int index;
};

struct spi_s {
  // NRF_SPI_Type *spi;
  // NRF_SPIS_Type *spis;
  uint8_t peripheral;
};

struct port_s {
  //__IO uint32_t *reg_cnf;
  //__IO uint32_t *reg_out;
  //__I  uint32_t *reg_in;
  //PortName port;
  //uint32_t mask;
};

struct pwmout_s {
  //PWMName pwm;
  //PinName pin;
};

struct i2c_s {
  // NRF_TWI_Type *i2c;
  //PinName sda;
  //PinName scl;
  int freq;
  uint8_t address_set;
  uint8_t peripheral;
};

struct analogin_s {
  //ADCName adc;
  uint8_t adc_pin;
};

struct gpio_irq_s {
  uint32_t ch;
};

#include "gpio_object.h"

#ifdef __cplusplus
}
#endif

#endif
