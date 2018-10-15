#ifndef MBED_H__
#define MBED_H__

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "PinNames.h"
#include "device.h"

typedef uint32_t timestamp_t;

namespace mbed {
class TimerEvent {
public:
};

class Ticker : public TimerEvent {
public:
  void attach_us(void (*fptr)(void), timestamp_t t);
  template <typename T>
    void attach_us(T *tptr, void (T::*mptr)(void), timestamp_t t) { attach_us(0, t); }
  void detach();
};

class Timeout : public Ticker {
public:
};

class DigitalIn {
 public:
  DigitalIn(PinName pin);
  DigitalIn(PinName pin, PinMode mode);
  int read();

  void mode(PinMode pull);
  int is_connected();

  operator int() {
    return read();
  }
 private:
  PinName pin_;
};

class AnalogIn {
public:
  AnalogIn(PinName pin);
  float read();
  unsigned short read_u16();
  operator float() {
    return read();
  }
 private:
  PinName pin_;
};

class Serial {
public:
  Serial(PinName tx, PinName rx, const char *name=NULL);

  // SerialBase:
  int readable();
  void attach(void (*fptr)(void));

  // Stream:
  int (putc)(int c);
  int (getc)();
};

class I2C {
public:
  enum RxStatus { NoData, MasterGeneralCall, MasterWrite, MasterRead };
  enum Acknowledge { NoACK = 0, ACK = 1 };
  I2C(PinName sda, PinName scl);
  void frequency(int hz);
  int read(int address, char *data, int length, bool repeated = false);
  int read(int ack);
  int write(int address, const char *data, int length, bool repeated = false);
  int write(int data);
  void start(void);
  void stop(void);
  i2c_t _i2c;
};

class PwmOut {
public:
  PwmOut(PinName pin);
  void write(float value);
  float read();
  void period(float seconds);
  void period_ms(int ms);
  void period_us(int us);
  void pulsewidth(float seconds);
  void pulsewidth_ms(int ms);
  void pulsewidth_us(int us);
};

class PortOut {
public:
  PortOut(PortName port, int mask = 0xFFFFFFFF) {}

  void write(int value);
  int read();

  PortOut& operator= (int value) {
    write(value);
    return *this;
  }

  PortOut& operator= (PortOut& rhs) {
    write(rhs.read());
    return *this;
  }

  operator int() {
    return read();
  }
};

class PortIn {
public:
  PortIn(PortName port, int mask = 0xFFFFFFFF);

  int read();
  void mode(PinMode mode);

  operator int() {
    return read();
  }
};

class PortInOut {
public:
  PortInOut(PortName port, int mask = 0xFFFFFFFF);
  void write(int value);
  int read();
  void output();
  void input();
  void mode(PinMode mode);

  PortInOut& operator= (int value) {
    write(value);
    return *this;
  }

  PortInOut& operator= (PortInOut& rhs) {
    write(rhs.read());
    return *this;
  }

  operator int() {
    return read();
  }
};

} // namespace mbed

using namespace mbed;
using namespace std;

#endif
