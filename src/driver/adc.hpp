#pragma once

#include "../external/CMSIS_5/Device/ARM/ARMCM0/Include/ARMCM0.h"
#include "../inc/dp32g030/irq.h"
#include "../inc/dp32g030/saradc.h"
#include "../inc/dp32g030/syscon.h"
#include <stdint.h>

// The firmware thinks W_SARADC_SMPL_CLK_SEL is at [8:7] but the TRM says it's
// at [10:9]
#define FW_R_SARADC_SMPL_SHIFT 7
#define FW_R_SARADC_SMPL_MASK (3U << FW_R_SARADC_SMPL_SHIFT)

class ADC {
public:
  typedef enum ADC_CH_MASK {
    ADC_CH0 = 0x0001U,
    ADC_CH1 = 0x0002U,
    ADC_CH2 = 0x0004U,
    ADC_CH3 = 0x0008U,
    ADC_CH4 = 0x0010U,
    ADC_CH5 = 0x0020U,
    ADC_CH6 = 0x0040U,
    ADC_CH7 = 0x0080U,
    ADC_CH8 = 0x0100U,
    ADC_CH9 = 0x0200U,
    ADC_CH10 = 0x0400U,
    ADC_CH11 = 0x0800U,
    ADC_CH12 = 0x1000U,
    ADC_CH13 = 0x2000U,
    ADC_CH14 = 0x4000U,
    ADC_CH15 = 0x8000U,
  } ADC_CH_MASK;

  typedef struct {
    uint8_t CLK_SEL;
    ADC_CH_MASK CH_SEL;
    uint8_t AVG;
    uint8_t CONT;
    uint8_t MEM_MODE;
    uint8_t SMPL_CLK;
    uint8_t SMPL_SETUP;
    uint8_t SMPL_WIN;
    uint8_t ADC_TRIG;
    uint16_t EXTTRIG_SEL;
    bool CALIB_OFFSET_VALID;
    bool CALIB_KD_VALID;
    uint8_t DMA_EN;
    uint16_t IE_CHx_EOC;
    uint8_t IE_FIFO_HFULL;
    uint8_t IE_FIFO_FULL;
  } ADC_Config_t;

  uint8_t getChannelNumber(ADC_CH_MASK Mask) {
    if (Mask & ADC_CH15)
      return 15U;
    if (Mask & ADC_CH14)
      return 14U;
    if (Mask & ADC_CH13)
      return 13U;
    if (Mask & ADC_CH12)
      return 12U;
    if (Mask & ADC_CH11)
      return 11U;
    if (Mask & ADC_CH10)
      return 10U;
    if (Mask & ADC_CH9)
      return 9U;
    if (Mask & ADC_CH8)
      return 8U;
    if (Mask & ADC_CH7)
      return 7U;
    if (Mask & ADC_CH6)
      return 6U;
    if (Mask & ADC_CH5)
      return 5U;
    if (Mask & ADC_CH4)
      return 4U;
    if (Mask & ADC_CH3)
      return 3U;
    if (Mask & ADC_CH2)
      return 2U;
    if (Mask & ADC_CH1)
      return 1U;
    if (Mask & ADC_CH0)
      return 0U;

    return 0U;
  }

  void disable() {
    SARADC_CFG =
        (SARADC_CFG & ~SARADC_CFG_ADC_EN_MASK) | SARADC_CFG_ADC_EN_BITS_DISABLE;
  }

  void enable() {
    SARADC_CFG =
        (SARADC_CFG & ~SARADC_CFG_ADC_EN_MASK) | SARADC_CFG_ADC_EN_BITS_ENABLE;
  }

  void softReset() {
    SARADC_START = (SARADC_START & ~SARADC_START_SOFT_RESET_MASK) |
                   SARADC_START_SOFT_RESET_BITS_ASSERT;
    SARADC_START = (SARADC_START & ~SARADC_START_SOFT_RESET_MASK) |
                   SARADC_START_SOFT_RESET_BITS_DEASSERT;
  }

  uint32_t getClockConfig() {
    uint32_t Value;

    Value = SYSCON_CLK_SEL;

    Value = 0 | (Value & ~(SYSCON_CLK_SEL_R_PLL_MASK | FW_R_SARADC_SMPL_MASK)) |
            (((Value & SYSCON_CLK_SEL_R_PLL_MASK) >> SYSCON_CLK_SEL_R_PLL_SHIFT)
             << SYSCON_CLK_SEL_W_PLL_SHIFT) |
            (((Value & FW_R_SARADC_SMPL_MASK) >> FW_R_SARADC_SMPL_SHIFT)
             << SYSCON_CLK_SEL_W_SARADC_SMPL_SHIFT);

    return Value;
  }

  void configure(ADC_Config_t *pAdc) {
    SYSCON_DEV_CLK_GATE =
        (SYSCON_DEV_CLK_GATE & ~SYSCON_DEV_CLK_GATE_SARADC_MASK) |
        SYSCON_DEV_CLK_GATE_SARADC_BITS_ENABLE;

    disable();

    SYSCON_CLK_SEL = (getClockConfig() & ~SYSCON_CLK_SEL_W_SARADC_SMPL_MASK) |
                     ((pAdc->CLK_SEL << SYSCON_CLK_SEL_W_SARADC_SMPL_SHIFT) &
                      SYSCON_CLK_SEL_W_SARADC_SMPL_MASK);

    SARADC_CFG =
        0 |
        (SARADC_CFG & ~(0 | SARADC_CFG_CH_SEL_MASK | SARADC_CFG_AVG_MASK |
                        SARADC_CFG_CONT_MASK | SARADC_CFG_SMPL_SETUP_MASK |
                        SARADC_CFG_MEM_MODE_MASK | SARADC_CFG_SMPL_CLK_MASK |
                        SARADC_CFG_SMPL_WIN_MASK | SARADC_CFG_ADC_TRIG_MASK |
                        SARADC_CFG_DMA_EN_MASK)) |
        ((pAdc->CH_SEL << SARADC_CFG_CH_SEL_SHIFT) & SARADC_CFG_CH_SEL_MASK) |
        ((pAdc->AVG << SARADC_CFG_AVG_SHIFT) & SARADC_CFG_AVG_MASK) |
        ((pAdc->CONT << SARADC_CFG_CONT_SHIFT) & SARADC_CFG_CONT_MASK) |
        ((pAdc->SMPL_SETUP << SARADC_CFG_SMPL_SETUP_SHIFT) &
         SARADC_CFG_SMPL_SETUP_MASK) |
        ((pAdc->MEM_MODE << SARADC_CFG_MEM_MODE_SHIFT) &
         SARADC_CFG_MEM_MODE_MASK) |
        ((pAdc->SMPL_CLK << SARADC_CFG_SMPL_CLK_SHIFT) &
         SARADC_CFG_SMPL_CLK_MASK) |
        ((pAdc->SMPL_WIN << SARADC_CFG_SMPL_WIN_SHIFT) &
         SARADC_CFG_SMPL_WIN_MASK) |
        ((pAdc->ADC_TRIG << SARADC_CFG_ADC_TRIG_SHIFT) &
         SARADC_CFG_ADC_TRIG_MASK) |
        ((pAdc->DMA_EN << SARADC_CFG_DMA_EN_SHIFT) & SARADC_CFG_DMA_EN_MASK);

    SARADC_EXTTRIG_SEL = pAdc->EXTTRIG_SEL;

    if (pAdc->CALIB_OFFSET_VALID) {
      SARADC_CALIB_OFFSET =
          (SARADC_CALIB_OFFSET & ~SARADC_CALIB_OFFSET_VALID_MASK) |
          SARADC_CALIB_OFFSET_VALID_BITS_YES;
    } else {
      SARADC_CALIB_OFFSET =
          (SARADC_CALIB_OFFSET & ~SARADC_CALIB_OFFSET_VALID_MASK) |
          SARADC_CALIB_OFFSET_VALID_BITS_NO;
    }
    if (pAdc->CALIB_KD_VALID) {
      SARADC_CALIB_KD = (SARADC_CALIB_KD & ~SARADC_CALIB_KD_VALID_MASK) |
                        SARADC_CALIB_KD_VALID_BITS_YES;
    } else {
      SARADC_CALIB_KD = (SARADC_CALIB_KD & ~SARADC_CALIB_KD_VALID_MASK) |
                        SARADC_CALIB_KD_VALID_BITS_NO;
    }

    SARADC_IF = 0xFFFFFFFF;
    SARADC_IE =
        0 |
        (SARADC_IE & ~(0 | SARADC_IE_CHx_EOC_MASK | SARADC_IE_FIFO_FULL_MASK |
                       SARADC_IE_FIFO_HFULL_MASK)) |
        ((pAdc->IE_CHx_EOC << SARADC_IE_CHx_EOC_SHIFT) &
         SARADC_IE_CHx_EOC_MASK) |
        ((pAdc->IE_FIFO_FULL << SARADC_IE_FIFO_FULL_SHIFT) &
         SARADC_IE_FIFO_FULL_MASK) |
        ((pAdc->IE_FIFO_HFULL << SARADC_IE_FIFO_HFULL_SHIFT) &
         SARADC_IE_FIFO_HFULL_MASK);

    if (SARADC_IE == 0) {
      NVIC_DisableIRQ((IRQn_Type)DP32_SARADC_IRQn);
    } else {
      NVIC_EnableIRQ((IRQn_Type)DP32_SARADC_IRQn);
    }
  }

  void start() {
    SARADC_START = (SARADC_START & ~SARADC_START_START_MASK) |
                   SARADC_START_START_BITS_ENABLE;
  }

  bool checkEndOfConversion(ADC_CH_MASK Mask) {
    volatile ADC_Channel_t *pChannels = (volatile ADC_Channel_t *)&SARADC_CH0;
    uint8_t Channel = getChannelNumber(Mask);

    return (pChannels[Channel].STAT & ADC_CHx_STAT_EOC_MASK) >>
           ADC_CHx_STAT_EOC_SHIFT;
  }

  uint16_t getValue(ADC_CH_MASK Mask) {
    volatile ADC_Channel_t *pChannels = (volatile ADC_Channel_t *)&SARADC_CH0;
    uint8_t Channel = getChannelNumber(Mask);

    SARADC_IF = 1 << Channel; // TODO: Or just use 'Mask'

    return (pChannels[Channel].DATA & ADC_CHx_DATA_DATA_MASK) >>
           ADC_CHx_DATA_DATA_SHIFT;
  }
};
