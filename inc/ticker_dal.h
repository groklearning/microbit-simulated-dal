#ifndef __TICKER_DAL_H
#define __TICKER_DAL_H

uint32_t get_ticks();
uint32_t get_macro_ticks();

// Returns how many ticks (16us) until it should next be called.
uint32_t fire_ticker(uint32_t ticks);

#endif
