// ticker.h
#include "nrf.h"
typedef void (*callback_ptr)(void);
typedef int32_t (*ticker_callback_ptr)(void);
#define CYCLES_PER_MICROSECONDS 16

#define MICROSECONDS_PER_TICK 16
#define CYCLES_PER_TICK (CYCLES_PER_MICROSECONDS * MICROSECONDS_PER_TICK)
// This must be an integer multiple of MICROSECONDS_PER_TICK
#define MICROSECONDS_PER_MACRO_TICK 6000
#define MILLISECONDS_PER_MACRO_TICK 6

#include <stdio.h>

namespace {
volatile bool _slow_callback_enabled;
callback_ptr _slow_callback;
ticker_callback_ptr _callbacks[3] = {NULL, NULL, NULL};
callback_ptr _low_pri_callbacks[4] = {NULL, NULL, NULL, NULL};
}

extern "C" {

void
ticker_init(callback_ptr slow_ticker_callback) {
  _slow_callback_enabled = false;
  _slow_callback = slow_ticker_callback;
}

void
ticker_start(void) {
  _slow_callback_enabled = true;
}

void
ticker_stop(void) {
  _slow_callback_enabled = false;
}

int
clear_ticker_callback(uint32_t index) {
  _callbacks[index] = NULL;
}

int
set_ticker_callback(uint32_t index, ticker_callback_ptr func, int32_t initial_delay_us) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  _callbacks[index] = func;
}

int
set_low_priority_callback(callback_ptr callback, int id) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  _low_pri_callbacks[id] = callback;
}
}

void
ticker_on_tick() {
  for (int i = 0; i < 3; ++i) {
    //if (
  }
}

void
ticker_on_macrotick() {
  if (_slow_callback && _slow_callback_enabled) {
    _slow_callback();
  }
}
