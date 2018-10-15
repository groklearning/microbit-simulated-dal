#ifndef NRF51_H
#define NRF51_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Bits 1..0 : ADC enable. */
#define ADC_ENABLE_ENABLE_Pos (0UL) /*!< Position of ENABLE field. */
#define ADC_ENABLE_ENABLE_Msk (0x3UL << ADC_ENABLE_ENABLE_Pos) /*!< Bit mask of ENABLE field. */
#define ADC_ENABLE_ENABLE_Disabled (0x00UL) /*!< ADC is disabled. */
#define ADC_ENABLE_ENABLE_Enabled (0x01UL) /*!< ADC is enabled. If an analog input pin is selected as source of the conversion, the selected pin is configured as an analog input. */

typedef struct {                                    /*!< ADC Structure                                                         */
  uint32_t  TASKS_START;                       /*!< Start an ADC conversion.                                              */
  uint32_t  TASKS_STOP;                        /*!< Stop ADC.                                                             */
  uint32_t  RESERVED0[62];
  uint32_t  EVENTS_END;                        /*!< ADC conversion complete.                                              */
  uint32_t  RESERVED1[128];
  uint32_t  INTENSET;                          /*!< Interrupt enable set register.                                        */
  uint32_t  INTENCLR;                          /*!< Interrupt enable clear register.                                      */
  uint32_t  RESERVED2[61];
  uint32_t  BUSY;                              /*!< ADC busy register.                                                    */
  uint32_t  RESERVED3[63];
  uint32_t  ENABLE;                            /*!< ADC enable.                                                           */
  uint32_t  CONFIG;                            /*!< ADC configuration register.                                           */
  uint32_t  RESULT;                            /*!< Result of ADC conversion.                                             */
  uint32_t  RESERVED4[700];
  uint32_t  POWER;                             /*!< Peripheral power control.                                             */
} NRF_ADC_Type;

extern NRF_ADC_Type* NRF_ADC;

#ifdef __cplusplus
}
#endif


#endif  /* nrf51_H */
