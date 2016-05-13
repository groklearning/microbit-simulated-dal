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

namespace {}

extern "C" {

void
ticker_init(callback_ptr slow_ticker_callback) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}

void
ticker_start(void) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}

void
ticker_stop(void) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}

int
clear_ticker_callback(uint32_t index) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}

int
set_ticker_callback(uint32_t index, ticker_callback_ptr func, int32_t initial_delay_us) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}

int
set_low_priority_callback(callback_ptr callback, int id) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
}
