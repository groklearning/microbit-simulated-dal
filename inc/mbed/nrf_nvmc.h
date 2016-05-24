#ifndef __NRF_NVMC_H
#define __NRF_NVMC_H

struct NRF_FICR_t {
  int CODEPAGESIZE;
};

struct NRF_RNG_t {
  int TASKS_START;
  int EVENTS_VALRDY;
  int VALUE;
  int TASKS_STOP;
};

struct NRF_NVMC_t {
  int READY;
};

enum {
  NVMC_READY_READY_Ready = 0,
  NVMC_READY_READY_Busy
};

void nrf_nvmc_write_byte(uint32_t, uint8_t);
void nrf_nvmc_write_words(uint32_t, const uint32_t*, size_t);
void nrf_nvmc_write_bytes(uint32_t, const uint8_t*, size_t);
void nrf_nvmc_page_erase(uint32_t);

extern struct NRF_FICR_t _NRF_FICR;
#define NRF_FICR (&_NRF_FICR)
extern struct NRF_RNG_t _NRF_RNG;
#define NRF_RNG (&_NRF_RNG)
extern struct NRF_NVMC_t _NRF_NVMC;
#define NRF_NVMC (&_NRF_NVMC)

#endif
