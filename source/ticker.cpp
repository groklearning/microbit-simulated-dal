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
volatile bool _slow_callback_enabled = false;
callback_ptr _slow_callback = NULL;
ticker_callback_ptr _callbacks[3] = {NULL, NULL, NULL};
callback_ptr _low_pri_callbacks[4] = {NULL, NULL, NULL, NULL};

// One tick is 16us. The fast timers are scheduled in increments of this.
uint32_t _ticks = 0;
// One macro tick is 6ms (375 ticks).
uint32_t _macro_ticks = 0;
// When _ticks == _time_ticks[i], then timer[i] fires.
// timer[0-2] are the fast timers, timer[3] is slow.
uint32_t _timer_ticks[4] = {0};
}

extern "C" {
// Set up the 6ms callback.
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
  _timer_ticks[index] = 0;
}

// Install a fast callback.
// This will first fire in 'initial_delay_us' us, then the callback should return
// a delay in us.
int
set_ticker_callback(uint32_t index, ticker_callback_ptr func, int32_t initial_delay_us) {
  _callbacks[index] = func;
  _timer_ticks[index] = _ticks + (initial_delay_us / 16);
}

int
set_low_priority_callback(callback_ptr callback, int id) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  _low_pri_callbacks[id] = callback;
}
}

uint32_t
fire_ticker(uint32_t ticks) {
  //fprintf(stderr, "fire_ticker(%u)\n", ticks);
  _ticks += ticks;
  if (_ticks >= _timer_ticks[3]) {
    ++_macro_ticks;
    if (_slow_callback_enabled) {
      _slow_callback();
    }
    _timer_ticks[3] += 375;
  }
  for (int i = 0; i < 3; ++i) {
    if (_ticks >= _timer_ticks[i] && _callbacks[i]) {
      _timer_ticks[i] += _callbacks[i]();
    }
  }
  uint32_t next = _ticks + 75;
  for (int i = 0; i < 4; ++i) {
    if (_timer_ticks[i] > _ticks && _timer_ticks[i] < next) {
      next = _timer_ticks[i];
    }
  }
  return next - _ticks;
}

uint32_t get_ticks() {
  return _ticks;
}

uint32_t get_macro_ticks() {
  return _macro_ticks;
}
