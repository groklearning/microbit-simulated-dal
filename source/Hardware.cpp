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

#include "Hardware.h"
#include "ticker_dal.h"

namespace {
uint8_t serial_buffer[10240];
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
  fprintf(stderr, "Unsupported: serial_baud\n");
}
void
serial_format(serial_t* obj, int data_bits, SerialParity parity, int stop_bits) {
  fprintf(stderr, "Unsupported: serial_format\n");
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
  if (c != '\r') {
    putc(c, stdout);
    fflush(stdout);
  }
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
  fprintf(stderr, "Unsupported: serial_clear\n");
}
void
serial_break_set(serial_t* obj) {
  fprintf(stderr, "Unsupported: serial_break_set\n");
}
void
serial_break_clear(serial_t* obj) {
  fprintf(stderr, "Unsupported: serial_break_clear\n");
}
void
serial_pinout_tx(PinName tx) {
  fprintf(stderr, "Unsupported: serial_pinout_tx\n");
}
void
serial_set_flow_control(serial_t* obj, FlowControl type, PinName rxflow, PinName txflow) {
  fprintf(stderr, "Unsupported: serial_set_flow_control\n");
}

uint8_t
serial_tx_active(serial_t* obj) {
  fprintf(stderr, "Unsupported: serial_tx_active\n");
}
uint8_t
serial_rx_active(serial_t* obj) {
  fprintf(stderr, "Unsupported: serial_rx_active\n");
}

void
spi_init(spi_t* obj, PinName mosi, PinName miso, PinName sclk, PinName ssel) {
  fprintf(stderr, "Unsupported: spi_init\n");
}
void
spi_free(spi_t* obj) {
  fprintf(stderr, "Unsupported: spi_free\n");
}
void
spi_format(spi_t* obj, int bits, int mode, int slave) {
  fprintf(stderr, "Unsupported: spi_format\n");
}
void
spi_frequency(spi_t* obj, int hz) {
  fprintf(stderr, "Unsupported: spi_frequency\n");
}
int
spi_master_write(spi_t* obj, int value) {
  fprintf(stderr, "Unsupported: spi_master_write\n");
  return 0;
}
int
spi_slave_receive(spi_t* obj) {
  fprintf(stderr, "Unsupported: spi_slave_receive\n");
  return 0;
}
int
spi_slave_read(spi_t* obj) {
  fprintf(stderr, "Unsupported: spi_slave_read\n");
  return 0;
}
void
spi_slave_write(spi_t* obj, int value) {
  fprintf(stderr, "Unsupported: spi_slave_write\n");
}
int
spi_busy(spi_t* obj) {
  fprintf(stderr, "Unsupported: spi_busy\n");
  return 0;
}
uint8_t
spi_get_module(spi_t* obj) {
  fprintf(stderr, "Unsupported: spi_get_module\n");
  return 0;
}
uint8_t
spi_active(spi_t* obj) {
  fprintf(stderr, "Unsupported: spi_active\n");
  return 0;
}

// Misc
namespace mbed {
namespace {
void (*serial_callback)() = NULL;

void
handle_serial_irq(unsigned int, SerialIrq) {
  if (serial_callback) {
    serial_callback();
  }
}
}

Serial::Serial(PinName tx, PinName rx, const char* name) {
  serial_init(NULL, tx, rx);
  serial_irq_handler(NULL, &handle_serial_irq, 0);
}

void
Serial::attach(void (*fn)()) {
  if (fn) {
    serial_callback = fn;
    serial_irq_set(NULL, RxIrq, 1);
  } else {
    serial_irq_set(NULL, RxIrq, 0);
    serial_callback = NULL;
  }
}

int(Serial::getc)() {
  return serial_getc(NULL);
}

int(Serial::putc)(int c) {
  serial_putc(NULL, c);
  return 1;
}

int
Serial::readable() {
  return serial_readable(NULL);
}

void
Ticker::attach_us(void (*fptr)(void), timestamp_t t) {
  fprintf(stderr, "Unsupported: Ticker::%s\n", __FUNCTION__);
}
void
Ticker::detach() {
  // Micro:bit code doesn't use the DAL ticker, and we don't implement it, so nothing to do here.
}

DigitalIn::DigitalIn(PinName pin) : pin_(pin) {
  this->mode(PullDefault);
}

DigitalIn::DigitalIn(PinName pin, PinMode mode) : pin_(pin) {
  if (pin == 17 || pin == 26) {
    // External pull-up.
    mode = PullUp;
  }
  this->mode(mode);
}

int
DigitalIn::read() {
  return !!(get_gpio_state() & (1 << pin_));
}

void
DigitalIn::mode(PinMode pull) {
  set_gpio_pin_input(pin_);
  set_gpio_pin_pull_mode(pin_, pull);
}

int
DigitalIn::is_connected() {
  fprintf(stderr, "Unsupported: DigitalIn::is_connected() on pin %d\n", pin_);
  return true;
}

AnalogIn::AnalogIn(PinName pin) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}
float
AnalogIn::read() {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
  return 0;
}
unsigned short
AnalogIn::read_u16() {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
  return 0;
}

I2C::I2C(PinName sda, PinName scl) {
}
void
I2C::frequency(int hz) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}
int
I2C::read(int address, char* data, int length, bool repeated) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}
int
I2C::read(int ack) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}
int
I2C::write(int address, const char* data, int length, bool repeated) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}
int
I2C::write(int data) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}
void
I2C::start(void) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}
void
I2C::stop(void) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}

PwmOut::PwmOut(PinName pin) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}
void
PwmOut::write(float value) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}
float
PwmOut::read() {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
  return 0;
}
void
PwmOut::period(float seconds) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}
void
PwmOut::period_ms(int ms) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}
void
PwmOut::period_us(int us) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}
void
PwmOut::pulsewidth(float seconds) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}
void
PwmOut::pulsewidth_ms(int ms) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}
void
PwmOut::pulsewidth_us(int us) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}
}

namespace {
volatile uint32_t _gpio_state = (1 << 17) | (1 << 26);
// 0 for input (hi-z), 1 for output (low-z).
volatile uint32_t _gpio_output = 0;
// 0 for down, 1 for up (only applies in hi-z mode).
volatile uint32_t _gpio_pull = 0;
// 0 for not-floating (see _gpio_pull), 1 for floating (only applies in hi-z mode).
volatile uint32_t _gpio_float = 0;

bool gpio_is_output(uint32_t pin_number) {
  return (_gpio_output >> pin_number) & 1;
}

uint32_t _led_brightness[25] = {0};
const uint32_t LED_GPIO_MASK = 0xfff << 4;
uint32_t _last_gpio_led_state = 0;
uint32_t _last_gpio_led_ticks = 0;

// The LED matrix is driven as a 3 rows by 9 columns grid.
// This table is a mapping of the 5x5 grid to the 3x9 grid.
// Each entry is a row pin (13+r) and a column pin (4+c).
// To turn on a LED, it sets the row high and the column low.
// Note that 3x9=27, so two states are unused ({1,7}, {1,8}).
struct LedPins {
  uint8_t r;
  uint8_t c;
};
const LedPins led_pin_map[25] = {
    {0, 0},  // 0, 0
    {1, 3},  // 1, 0
    {0, 1},  // 2, 0
    {1, 4},  // 3, 0
    {0, 2},  // 4, 0
    {2, 3},  // 0, 1
    {2, 4},  // 1, 1
    {2, 5},  // 2, 1
    {2, 6},  // 3, 1
    {2, 7},  // 4, 1
    {1, 1},  // 0, 2
    {0, 8},  // 1, 2
    {1, 2},  // 2, 2
    {2, 8},  // 3, 2
    {1, 0},  // 4, 2
    {0, 7},  // 0, 3
    {0, 6},  // 1, 3
    {0, 5},  // 2, 3
    {0, 4},  // 3, 3
    {0, 3},  // 4, 3
    {2, 2},  // 0, 4
    {1, 6},  // 1, 4
    {2, 0},  // 2, 4
    {1, 5},  // 3, 4
    {2, 1},  // 4, 4
};

void
print_row_col_bits(uint32_t gpio) {
  for (int i = 0; i < 3; ++i) {
    fprintf(stderr, "%c", (gpio & (1 << (13 + i))) ? '1' : '0');
  }
  fprintf(stderr, "  ");
  for (int i = 0; i < 9; ++i) {
    fprintf(stderr, "%c", (gpio & (1 << (4 + i))) ? '1' : '0');
  }
  fprintf(stderr, "\n");
}

bool
is_led_on(int led, uint32_t gpio_state) {
  int row_pin = 13 + led_pin_map[led].r;
  int col_pin = 4 + led_pin_map[led].c;
  return (gpio_state & (1 << row_pin)) && !(gpio_state & (1 << col_pin));
}
}

void
nrf_gpio_range_cfg_output(uint32_t pin_range_start, uint32_t pin_range_end) {
  for (int i = pin_range_start; i < pin_range_end; ++i) {
    set_gpio_pin_output(i);
  }
}
void
nrf_gpio_pin_set(uint32_t pin_number) {
  nrf_gpio_pins_set(1 << pin_number);
}
void
nrf_gpio_pins_set(uint32_t pin_mask) {
  if ((_gpio_output & pin_mask) != pin_mask) {
    fprintf(stderr, "Setting non-output pin (output: %08x pins: %08x).\n", _gpio_output, pin_mask);
    return;
  }

  set_gpio_state(get_gpio_state() | pin_mask);
}
void
nrf_gpio_pin_clear(uint32_t pin_number) {
  nrf_gpio_pins_clear(1 << pin_number);
}
void
nrf_gpio_pins_clear(uint32_t pin_mask) {
  if ((_gpio_output & pin_mask) != pin_mask) {
    fprintf(stderr, "Clearing non-output pin (output: %08x pins: %08x).\n", _gpio_output, pin_mask);
    return;
  }

  set_gpio_state(get_gpio_state() & ~pin_mask);
}

uint32_t get_gpio_state() {
  return _gpio_state;
}

void set_gpio_state(uint32_t state) {
  //if ((new_state & LED_GPIO_MASK) != _last_gpio_led_state) {
  if (state != _last_gpio_led_state) {
    uint32_t now_ticks = get_ticks();
    if (now_ticks > _last_gpio_led_ticks) {
      uint32_t duration = now_ticks - _last_gpio_led_ticks;
      for (int led = 0; led < 25; ++led) {
	if (is_led_on(led, _last_gpio_led_state)) {
	  _led_brightness[led] += duration;
	}
      }
    }

    _last_gpio_led_state = state;//(new_state & LED_GPIO_MASK);
    _last_gpio_led_ticks = now_ticks;
  }

  _gpio_state = state;
}

void set_gpio_pin_input(uint32_t pin) {
  _gpio_output &= ~(1 << pin);
}

void set_gpio_pin_output(uint32_t pin) {
  _gpio_output |= (1 << pin);
}

void set_gpio_pin_pull_mode(uint32_t pin, PinMode mode) {
  if (mode == PullDown) {
    _gpio_float &= ~(1 << pin);
    _gpio_pull &= ~(1 << pin);
  } else if (mode == PullUp) {
    _gpio_float &= ~(1 << pin);
    _gpio_pull |= (1 << pin);
  } else if (mode == PullNone) {
    _gpio_float |= (1 << pin);
    _gpio_pull &= ~(1 << pin);
  }
}

void get_gpio_info(uint32_t* state, uint32_t* output, uint32_t* pull, uint32_t* floating) {
  *state = _gpio_state;
  *output = _gpio_output;
  *pull = _gpio_pull;
  *floating = _gpio_float;
}

void get_led_ticks(uint32_t* leds) {
  memcpy(leds, _led_brightness, sizeof(_led_brightness));
  memset(_led_brightness, 0, sizeof(_led_brightness));
}

namespace {
int16_t _accel_x = 0;
int16_t _accel_y = 1024;
int16_t _accel_z = 0;

int32_t _magnet_x = 0;
int32_t _magnet_y = 80000;
int32_t _magnet_z = 0;
}

void set_accelerometer(int16_t x, int16_t y, int16_t z) {
  _accel_x = x;
  _accel_y = y;
  _accel_z = z;
  set_gpio_state(get_gpio_state() & ~(1 << 28));
}

void get_accelerometer(int16_t* x, int16_t* y, int16_t* z) {
  *x = _accel_x;
  *y = _accel_y;
  *z = _accel_z;
  set_gpio_state(get_gpio_state() | (1 << 28));
}

void set_magnetometer(int32_t x, int32_t y, int32_t z) {
  _magnet_x = x;
  _magnet_y = y;
  _magnet_z = z;
  set_gpio_state(get_gpio_state() & ~(1 << 29));
}

void get_magnetometer(int32_t* x, int32_t* y, int32_t* z) {
  *x = _magnet_x;
  *y = _magnet_y;
  *z = _magnet_z;
  set_gpio_state(get_gpio_state() | (1 << 29));
}

namespace {
volatile bool _reset_flag = false;
volatile bool _panic_flag = false;
}

void set_reset_flag() {
  _reset_flag = true;
}

bool get_reset_flag() {
  return _reset_flag;
}

void set_panic_flag() {
  _panic_flag = true;
}

bool get_panic_flag() {
  return _panic_flag;
}
