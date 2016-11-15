#include <math.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include "PinNames.h"
#include "gpio_api.h"
#include "gpio_object.h"
#include "nrf_gpio.h"

#include "buffer.h"
#include "device.h"
#include "dma_api.h"
#include "mbed.h"
#include "objects.h"
#include "serial_api.h"
#include "spi_api.h"

extern "C" {
#include "nrf_nvmc.h"
}

#include "Hardware.h"

namespace {
// Basic ring buffer for serial data.
// At least as big as all the other ring buffers involved...
uint8_t serial_buffer[10240];
volatile uint serial_buffer_head = 0;
volatile uint serial_buffer_tail = 0;
volatile uint serial_buffer_echo = 0;

bool serial_irq_rx_enabled = false;
uart_irq_handler serial_irq = 0;
}

namespace {
volatile bool _reset_flag = false;
volatile bool _panic_flag = false;
volatile bool _disconnect_flag = false;
volatile bool _disable_echo = false;
}

// Hardware.h
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

// serial_api.h
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
  // We only send '\r' (see serial_add_byte). They send '\r\n'. So only process the '\r'.
  if (c == '\n') {
    return;
  }

  if (_disable_echo) {
    // If echo is disabled, then ignore any bytes that match recently sent bytes.
    // Search forwards from serial_buffer_echo.

    // ....print('hi')\r.....
    //     ^e   ^t      ^h
    // if we receive 'p', then advance e, and don't print it. Keep doing that while e < t.

    while (serial_buffer_echo != serial_buffer_tail) {
      int b = serial_buffer[serial_buffer_echo];
      serial_buffer_echo = (serial_buffer_echo + 1) % sizeof(serial_buffer);
      // Skip unprintables below ' ' (but include nl/cr).
      if (b < ' ' && !(b == '\r' || b == '\n')) {
	continue;
      }
      // This was a byte we sent, so don't echo it.
      if (b == c) {
	return;
      }
    }
  }
  if (c == '\r') {
    c = '\n';
  }
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

// spi_api.h
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

// Misc -- everything in mbed that microbit-micropython uses directly (but probably shouldn't) or
// used by the microbit-dal code.
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
  if (pin == BUTTON_A || pin == BUTTON_B) {
    // External pull-up.
    mode = PullUp;
  }
  this->mode(mode);
}

int
DigitalIn::read() {
  return get_gpio_pin(pin_).is_high() ? 1 : 0;
}

void
DigitalIn::mode(PinMode pull) {
  get_gpio_pin(pin_).set_input_mode(pull);
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

extern "C" {
// More mbed/nrf stuff used by microbit-micropython directly.
void nrf_gpio_cfg_output(uint8_t pin_number) {
  get_gpio_pin(pin_number).set_output_mode();
}

void
nrf_gpio_range_cfg_output(uint32_t pin_range_start, uint32_t pin_range_end) {
  for (int i = pin_range_start; i < pin_range_end; ++i) {
    get_gpio_pin(i).set_output_mode();
  }
}

void
nrf_gpio_pin_set(uint8_t pin_number) {
  nrf_gpio_pins_set(1 << pin_number);
}

void
nrf_gpio_pins_set(uint32_t pin_mask) {
  // fprintf(stderr, "nrf_gpio_pins_set: %d\n", get_ticks());
  for (int i = 0; i < 32; ++i) {
    if (pin_mask & (1 << i)) {
      get_gpio_pin(i).set_high();
    }
  }

  for (int i = 0; i < 25; ++i) {
    get_display_led(i).update();
  }
}

void
nrf_gpio_pin_clear(uint8_t pin_number) {
  nrf_gpio_pins_clear(1 << pin_number);

  // Detect the start of a new row.
  for (int i = COL1; i <= COL9; ++i) {
    if (get_gpio_pin(i).is_low()) {
      return;
    }
  }
  for (int i = ROW1; i <= ROW3; ++i) {
    if (get_gpio_pin(i).is_high()) {
      return;
    }
  }
  // And if we've just cleared ROW3, then it's the start of a new frame.
  if (pin_number != ROW3) {
    return;
  }

  // At the start of a new frame, get the matrix leds to re-calculate brightness.
  for (int i = 0; i < 25; ++i) {
    get_display_led(i).calculate();
  }
}

void
nrf_gpio_pins_clear(uint32_t pin_mask) {
  for (int i = 0; i < 32; ++i) {
    if (pin_mask & (1 << i)) {
      get_gpio_pin(i).set_low();
    }
  }

  for (int i = 0; i < 25; ++i) {
    get_display_led(i).update();
  }
}
}

extern "C" {
NRF_FICR_t _NRF_FICR = { 4096 };
NRF_RNG_t _NRF_RNG;
NRF_NVMC_t _NRF_NVMC = { 0 };

void nrf_nvmc_write_byte(uint32_t addr, uint8_t b) {
  *reinterpret_cast<uint8_t*>(addr) = b;
}
void nrf_nvmc_write_words(uint32_t addr, const uint32_t* d, size_t len) {
  uint32_t* mem = reinterpret_cast<uint32_t*>(addr);
  for (size_t i = 0; i < len; ++i) {
    *mem++ = *d++;
  }
}
void nrf_nvmc_write_bytes(uint32_t addr, const uint8_t* d, size_t len) {
  uint8_t* mem = reinterpret_cast<uint8_t*>(addr);
  for (size_t i = 0; i < len; ++i) {
    *mem++ = *d++;
  }
}
void nrf_nvmc_page_erase(uint32_t addr) {
  void* mem = reinterpret_cast<void*>(addr);
  memset(mem, 0xff, 4096);
}
}

// Hardware.h

namespace {
GpioPin _gpio_pins[32] = {
    GpioPin(0),  GpioPin(1),  GpioPin(2),  GpioPin(3),  GpioPin(4),  GpioPin(5),  GpioPin(6),
    GpioPin(7),  GpioPin(8),  GpioPin(9),  GpioPin(10), GpioPin(11), GpioPin(12), GpioPin(13),
    GpioPin(14), GpioPin(15), GpioPin(16), GpioPin(17), GpioPin(18), GpioPin(19), GpioPin(20),
    GpioPin(21), GpioPin(22), GpioPin(23), GpioPin(24), GpioPin(25), GpioPin(26), GpioPin(27),
    GpioPin(28), GpioPin(29), GpioPin(30), GpioPin(31)};

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
DisplayLed _display_leds[25] = {
    DisplayLed(0),  DisplayLed(1),  DisplayLed(2),  DisplayLed(3),  DisplayLed(4),
    DisplayLed(5),  DisplayLed(6),  DisplayLed(7),  DisplayLed(8),  DisplayLed(9),
    DisplayLed(10), DisplayLed(11), DisplayLed(12), DisplayLed(13), DisplayLed(14),
    DisplayLed(15), DisplayLed(16), DisplayLed(17), DisplayLed(18), DisplayLed(19),
    DisplayLed(20), DisplayLed(21), DisplayLed(22), DisplayLed(23), DisplayLed(24),
};
}

DisplayLed::DisplayLed(uint32_t n) : _n(n), _state(false), _b(0), _on_ticks(0), _duration_ticks(0) {
}
// Called whenever a LED pin changes state.
void
DisplayLed::update() {
  bool on = is_on();
  if (on == _state) {
    return;
  }
  if (on) {
    _on_ticks = get_ticks();
  } else {
    _duration_ticks += get_ticks() - _on_ticks;
  }
  _state = on;
}
// Called on the start of a new frame.
void
DisplayLed::calculate() {
  _b = ticks();
}
uint32_t
DisplayLed::ticks() {
  uint32_t now = get_ticks();
  if (_state) {
    _duration_ticks += now - _on_ticks;
    _on_ticks = now;
  }
  uint32_t t = _duration_ticks;
  _duration_ticks = 0;
  return t;
}
bool
DisplayLed::is_on() {
  int row_pin = 13 + led_pin_map[_n].r;
  int col_pin = 4 + led_pin_map[_n].c;
  return get_gpio_pin(row_pin).is_high() && !get_gpio_pin(col_pin).is_high();
}
uint32_t
DisplayLed::brightness() {
  return _b;
}

DisplayLed&
get_display_led(uint32_t n) {
  return _display_leds[n];
}

GpioPin::GpioPin(uint32_t pin)
  : _pin(pin), _output(false), _is_output(false), _pull(PullDefault), _analog(NAN), _is_pwm(false), _pwm_dutycycle(0), _pwm_period(1000) {
}

void
GpioPin::set_input_mode(PinMode pull) {
  _is_output = false;
  _is_pwm = false;
  _pull = pull;
}
void
GpioPin::set_output_mode() {
  _is_output = true;
  _is_pwm = false;
  _pull = PullNone;
}
bool
GpioPin::set_low() {
  return set_digital(false);
}
bool
GpioPin::set_high() {
  return set_digital(true);
}
bool
GpioPin::set_digital(bool d) {
  if (!_is_output) {
    return false;
  }
  if (_output == d) {
    return true;
  }
  if (d) {
    _on_ticks = get_ticks();
  } else {
    _duration_ticks += get_ticks() - _on_ticks;
  }
  _output = d;
  return true;
}
uint32_t
GpioPin::ticks() {
  if (_is_output) {
    return 0;
  }
  uint32_t now = get_ticks();
  if (_output) {
    _duration_ticks += now - _on_ticks;
    _on_ticks = now;
  }
  uint32_t t = _duration_ticks;
  _duration_ticks = 0;
  return t;
}
bool
GpioPin::set_input_voltage(double a) {
  if (_is_output) {
    return false;
  }
  _analog = a;
  return true;
}
void
GpioPin::set_pwm(uint32_t dutycycle) {
  set_output_mode();
  _is_pwm = true;
  _pwm_dutycycle = dutycycle;
}
void
GpioPin::set_pwm_period(uint32_t us) {
  _pwm_period = us;
}
double
GpioPin::get_voltage() {
  if (_is_output) {
    return _output ? 3.3 : 0.0;
  } else {
    if (isnan(_analog)) {
      switch (_pull) {
        case PullNone:
          return 1.57 + drand48() - 0.5;
        case PullUp:
          return 3.3;
        case PullDown:
          return 0.0;
      }
    } else {
      return _analog;
    }
  }
}
double GpioPin::get_pwm() {
  return _pwm_dutycycle;
}
double GpioPin::get_pwm_period() {
  return _pwm_period;
}
GpioPinState
GpioPin::get_state() {
  if (_pin >= COL1 && _pin <= ROW3) {
    return GPIO_PIN_RESERVED;
  }
  if (_is_output) {
    if (_is_pwm) {
      return GPIO_PIN_OUTPUT_PWM;
    } else {
      return _output ? GPIO_PIN_OUTPUT_HIGH : GPIO_PIN_OUTPUT_LOW;
    }
  } else {
    switch (_pull) {
      case PullNone:
        if (isnan(_analog)) {
          return GPIO_PIN_INPUT_FLOATING;
        } else {
          return is_high() ? GPIO_PIN_INPUT_FLOATING_HIGH : GPIO_PIN_INPUT_FLOATING_LOW;
        }
      case PullUp:
        return is_high() ? GPIO_PIN_INPUT_UP_HIGH : GPIO_PIN_INPUT_UP_LOW;
      case PullDown:
        return is_high() ? GPIO_PIN_INPUT_DOWN_HIGH : GPIO_PIN_INPUT_DOWN_LOW;
    }
  }
}
bool
GpioPin::is_high() {
  return get_voltage() > 2.0;
}
bool
GpioPin::is_low() {
  return get_voltage() < 1.0;
}
bool
GpioPin::is_input() {
  return !_is_output;
}
bool
GpioPin::is_output() {
  return _is_output;
}
bool
GpioPin::is_floating() {
  return _pull = PullNone;
}
bool
GpioPin::is_pullup() {
  return _pull = PullUp;
}
bool
GpioPin::is_pulldown() {
  return _pull = PullDown;
}

GpioPin&
get_gpio_pin(uint32_t pin) {
  return _gpio_pins[pin];
}

namespace {
int16_t _accel_x = 0;
int16_t _accel_y = 0;
int16_t _accel_z = -1024;
BasicGesture _accel_gesture = GESTURE_FACE_UP;

int32_t _magnet_x = 0;
int32_t _magnet_y = -20000;
int32_t _magnet_z = 25000;

int32_t _temperature = 28;

bool _inject_random = false;
int32_t _next_random = 0;
int32_t _remaining_random = 0;
}

void
set_accelerometer(int16_t x, int16_t y, int16_t z, BasicGesture g) {
  _accel_x = x;
  _accel_y = y;
  _accel_z = z;
  _accel_gesture = g;
  get_gpio_pin(ACCEL_INT1).set_input_voltage(0);
}

void
get_accelerometer(int16_t* x, int16_t* y, int16_t* z, BasicGesture* g) {
  *x = _accel_x;
  *y = _accel_y;
  *z = _accel_z;
  *g = _accel_gesture;
  get_gpio_pin(ACCEL_INT1).set_input_voltage(3.3);
}

void
set_magnetometer(int32_t x, int32_t y, int32_t z) {
  _magnet_x = x;
  _magnet_y = y;
  _magnet_z = z;
  get_gpio_pin(MAG_INT1).set_input_voltage(0);
}

void
get_magnetometer(int32_t* x, int32_t* y, int32_t* z) {
  *x = _magnet_x;
  *y = _magnet_y;
  *z = _magnet_z;
  get_gpio_pin(MAG_INT1).set_input_voltage(3.3);
}

void
set_temperature(int32_t t) {
  _temperature = t;
}

void
get_temperature(int32_t* t) {
  *t = _temperature;
}

void
set_random_state(int32_t next, int32_t repeat) {
  if (next >= 0) {
    _inject_random = true;
    _next_random = next;
    _remaining_random = repeat;
  } else {
    _inject_random = false;
  }
}

void
set_random_seed(uint32_t seed) {
  srand(seed);
}

uint32_t
get_random() {
  if (_inject_random) {
    --_remaining_random;
    return _next_random;
  } else {
    return rand();
  }
}

int32_t
get_random_remaining() {
  return _remaining_random;
}

// Called by microbit.reset()
void
set_reset_flag() {
  _reset_flag = true;
}

// Called by Main.cpp to initiate a reset.
bool
get_reset_flag() {
  return _reset_flag;
}

// Called by microbit.panic()
void
set_panic_flag() {
  _panic_flag = true;
}

// Called by Main.cpp to initiate shutdown.
bool
get_panic_flag() {
  return _panic_flag;
}

// Called by MicroBitDisplay::disable on the first reboot.
void
set_disconnect_flag() {
  _disconnect_flag = true;
}

bool
get_disconnect_flag() {
  return _disconnect_flag;
}

// Called by Main.cpp to disable echo if the terminal doesn't have it enabled.
void
disable_echo() {
  _disable_echo = true;
}
