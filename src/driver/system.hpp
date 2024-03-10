#pragma once

extern "C" {
#include "../external/CMSIS_5/Device/ARM/ARMCM0/Include/ARMCM0.h"
#include "../inc/dp32g030/pmu.h"
#include "../inc/dp32g030/syscon.h"
#include <stdint.h>
// #include "misc.h"
}

#define CPU_CLOCK_HZ 48000000

void SYSTEM_ConfigureClocks() {
  // Set source clock from external crystal
  PMU_SRC_CFG =
      (PMU_SRC_CFG & ~(PMU_SRC_CFG_RCHF_SEL_MASK | PMU_SRC_CFG_RCHF_EN_MASK)) |
      PMU_SRC_CFG_RCHF_SEL_BITS_48MHZ | PMU_SRC_CFG_RCHF_EN_BITS_ENABLE;

  // Divide by 2
  SYSCON_CLK_SEL = SYSCON_CLK_SEL_DIV_BITS_2;

  // Disable division clock gate
  SYSCON_DIV_CLK_GATE =
      (SYSCON_DIV_CLK_GATE & ~SYSCON_DIV_CLK_GATE_DIV_CLK_GATE_MASK) |
      SYSCON_DIV_CLK_GATE_DIV_CLK_GATE_BITS_DISABLE;
}

void SYSTEM_ConfigureSysCon() {
  // Enable clock gating of blocks we need.
  SYSCON_DEV_CLK_GATE = 0 | SYSCON_DEV_CLK_GATE_GPIOA_BITS_ENABLE |
                        SYSCON_DEV_CLK_GATE_GPIOB_BITS_ENABLE |
                        SYSCON_DEV_CLK_GATE_GPIOC_BITS_ENABLE |
                        SYSCON_DEV_CLK_GATE_UART1_BITS_ENABLE |
                        SYSCON_DEV_CLK_GATE_SPI0_BITS_ENABLE |
                        SYSCON_DEV_CLK_GATE_SARADC_BITS_ENABLE |
                        SYSCON_DEV_CLK_GATE_CRC_BITS_ENABLE |
                        SYSCON_DEV_CLK_GATE_AES_BITS_ENABLE |
                        SYSCON_DEV_CLK_GATE_PWM_PLUS0_BITS_ENABLE;
}

static const uint32_t tickMultiplier = 48;

void SYSTICK_Init() { SysTick_Config(48000); }

void SYSTICK_DelayUs(uint32_t Delay) {
  uint32_t i;
  uint32_t Start;
  uint32_t Previous;
  uint32_t Current;
  uint32_t Delta;

  i = 0;
  Start = SysTick->LOAD;
  Previous = SysTick->VAL;
  do {
    do {
      Current = SysTick->VAL;
    } while (Current == Previous);
    if (Current < Previous) {
      Delta = -Current;
    } else {
      Delta = Start - Current;
    }
    i += Delta + Previous;
    Previous = Current;
  } while (i < Delay * tickMultiplier);
}

void SYSTEM_DelayMs(uint32_t Delay) { SYSTICK_DelayUs(Delay * 1000); }
