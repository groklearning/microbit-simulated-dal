#include <stdio.h>

#include "gpio_api.h"
#include "gpio_object.h"
#include "nrf_gpio.h"
#include "PinNames.h"

#include "buffer.h"
#include "device.h"
#include "dma_api.h"
#include "mbed.h"
#include "objects.h"
#include "serial_api.h"
#include "spi_api.h"

namespace {
uint8_t serial_buffer[1024];
volatile uint serial_buffer_head = 0;
volatile uint serial_buffer_tail = 0;

bool serial_irq_rx_enabled = false;
uart_irq_handler serial_irq = 0;
}

void
serial_add_byte(uint8_t c) {
  if (c == '\n') {
    c = '\r';
  }
  serial_buffer[serial_buffer_head] = c;
  serial_buffer_head = (serial_buffer_head + 1) % sizeof(serial_buffer);

  if (serial_irq_rx_enabled && serial_irq) {
    serial_irq(0, RxIrq);
  }
}

void
serial_init(serial_t* obj, PinName tx, PinName rx) {
}
void
serial_free(serial_t* obj) {
}
void
serial_baud(serial_t* obj, int baudrate) {
  fprintf(stderr, "Unhandled: serial_baud\n");
}
void
serial_format(serial_t* obj, int data_bits, SerialParity parity, int stop_bits) {
  fprintf(stderr, "Unhandled: serial_format\n");
}

void
serial_irq_handler(serial_t* obj, uart_irq_handler handler, uint32_t id) {
  serial_irq = handler;
}
void
serial_irq_set(serial_t* obj, SerialIrq irq, uint32_t enable) {
  serial_irq_rx_enabled = enable;
}

int
serial_getc(serial_t* obj) {
  if (!serial_readable(obj)) {
    return -1;
  }
  int c = serial_buffer[serial_buffer_tail];
  serial_buffer_tail = (serial_buffer_tail + 1) % sizeof(serial_buffer);
  return c;
}
void
serial_putc(serial_t* obj, int c) {
  putc(c, stdout);
  fflush(stdout);
}
int
serial_readable(serial_t* obj) {
  return (serial_buffer_head != serial_buffer_tail);
}
int
serial_writable(serial_t* obj) {
  return 1;
}

void
serial_clear(serial_t* obj) {
  fprintf(stderr, "Unhandled: serial_clear\n");
}
void
serial_break_set(serial_t* obj) {
  fprintf(stderr, "Unhandled: serial_break_set\n");
}
void
serial_break_clear(serial_t* obj) {
  fprintf(stderr, "Unhandled: serial_break_clear\n");
}
void
serial_pinout_tx(PinName tx) {
  fprintf(stderr, "Unhandled: serial_pinout_tx\n");
}
void
serial_set_flow_control(serial_t* obj, FlowControl type, PinName rxflow, PinName txflow) {
  fprintf(stderr, "Unhandled: serial_set_flow_control\n");
}

uint8_t
serial_tx_active(serial_t* obj) {
  fprintf(stderr, "Unhandled: serial_tx_active\n");
}
uint8_t
serial_rx_active(serial_t* obj) {
  fprintf(stderr, "Unhandled: serial_rx_active\n");
}

void
spi_init(spi_t* obj, PinName mosi, PinName miso, PinName sclk, PinName ssel) {
  fprintf(stderr, "Unhandled: spi_init\n");
}
void
spi_free(spi_t* obj) {
  fprintf(stderr, "Unhandled: spi_free\n");
}
void
spi_format(spi_t* obj, int bits, int mode, int slave) {
  fprintf(stderr, "Unhandled: spi_format\n");
}
void
spi_frequency(spi_t* obj, int hz) {
  fprintf(stderr, "Unhandled: spi_frequency\n");
}
int
spi_master_write(spi_t* obj, int value) {
  fprintf(stderr, "Unhandled: spi_master_write\n");
  return 0;
}
int
spi_slave_receive(spi_t* obj) {
  fprintf(stderr, "Unhandled: spi_slave_receive\n");
  return 0;
}
int
spi_slave_read(spi_t* obj) {
  fprintf(stderr, "Unhandled: spi_slave_read\n");
  return 0;
}
void
spi_slave_write(spi_t* obj, int value) {
  fprintf(stderr, "Unhandled: spi_slave_write\n");
}
int
spi_busy(spi_t* obj) {
  fprintf(stderr, "Unhandled: spi_busy\n");
  return 0;
}
uint8_t
spi_get_module(spi_t* obj) {
  fprintf(stderr, "Unhandled: spi_get_module\n");
  return 0;
}
uint8_t
spi_active(spi_t* obj) {
  fprintf(stderr, "Unhandled: spi_active\n");
  return 0;
}
void
nrf_gpio_range_cfg_output(uint32_t pin_range_start, uint32_t pin_range_end) {
  fprintf(stderr, "Unhandled: nrf_gpio_range_cfg_output\n");
}
void
nrf_gpio_pin_set(uint32_t pin_number) {
  fprintf(stderr, "Unhandled: nrf_gpio_pin_set\n");
}
void
nrf_gpio_pins_set(uint32_t pin_mask) {
  fprintf(stderr, "Unhandled: nrf_gpio_pins_set\n");
}
void
nrf_gpio_pin_clear(uint32_t pin_number) {
  fprintf(stderr, "Unhandled: nrf_gpio_pin_clear\n");
}
void
nrf_gpio_pins_clear(uint32_t pin_mask) {
  fprintf(stderr, "Unhandled: nrf_gpio_pins_clear\n");
}
