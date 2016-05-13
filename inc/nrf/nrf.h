#ifndef __NRF_TIMER_H
#define __NRF_TIMER_H

#include <stdint.h>

/*
#define __NOP()

typedef enum {
  Reset_IRQn = -15,
  NonMaskableInt_IRQn = -14,
  HardFault_IRQn = -13,
  SVCall_IRQn = -5,
  DebugMonitor_IRQn = -4,
  PendSV_IRQn = -2,
  SysTick_IRQn = -1,
  POWER_CLOCK_IRQn = 0,
  RADIO_IRQn = 1,
  UART0_IRQn = 2,
  SPI0_TWI0_IRQn = 3,
  SPI1_TWI1_IRQn = 4,
  GPIOTE_IRQn = 6,
  ADC_IRQn = 7,
  TIMER0_IRQn = 8,
  TIMER1_IRQn = 9,
  TIMER2_IRQn = 10,
  RTC0_IRQn = 11,
  TEMP_IRQn = 12,
  RNG_IRQn = 13,
  ECB_IRQn = 14,
  CCM_AAR_IRQn = 15,
  WDT_IRQn = 16,
  RTC1_IRQn = 17,
  QDEC_IRQn = 18,
  LPCOMP_IRQn = 19,
  SWI0_IRQn = 20,
  SWI1_IRQn = 21,
  SWI2_IRQn = 22,
  SWI3_IRQn = 23,
  SWI4_IRQn = 24,
  SWI5_IRQn = 25
} IRQn_Type;

typedef struct NRF_TIMER_Type {
  uint32_t POWER;
  uint32_t TASKS_CLEAR;
  uint32_t CC[4];
  uint32_t MODE;
  uint32_t BITMODE;
  uint32_t PRESCALER;
  uint32_t INTENSET;
  uint32_t INTENCLR;
  uint32_t SHORTS;
  uint32_t TASKS_START;
  uint32_t TASKS_STOP;
  uint32_t EVENTS_COMPARE[4];
  uint32_t TASKS_CAPTURE[4];
} NRF_TIMER_Type;

void NVIC_SetPriority(IRQn_Type IRQn, uint32_t priority);
void NVIC_EnableIRQ(IRQn_Type IRQn);
void NVIC_DisableIRQ(IRQn_Type IRQn);
void NVIC_SetPendingIRQ(IRQn_Type IRQn);

extern NRF_TIMER_Type* NRF_TIMER0;

#define TIMER_MODE_MODE_Timer 0
#define TIMER_BITMODE_BITMODE_24Bit 0
#define TIMER_BITMODE_BITMODE_Pos 0
#define TIMER_INTENCLR_COMPARE0_Msk 0
#define TIMER_INTENCLR_COMPARE1_Msk 0
#define TIMER_INTENCLR_COMPARE2_Msk 0
#define TIMER_INTENCLR_COMPARE3_Msk 0
#define TIMER_INTENSET_COMPARE0_Msk 0
#define TIMER_INTENSET_COMPARE1_Msk 0
#define TIMER_INTENSET_COMPARE2_Msk 0
#define TIMER_INTENSET_COMPARE3_Msk 0
*/

#endif
