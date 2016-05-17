#ifndef __TICKER_DAL_H
#define __TICKER_DAL_H

void ticker_on_tick();
void ticker_on_macrotick();

uint32_t get_ticks();
uint32_t get_macro_ticks();

#endif
