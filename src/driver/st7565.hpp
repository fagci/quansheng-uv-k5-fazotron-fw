#pragma once

#include "../inc/dp32g030/gpio.h"
#include "../inc/dp32g030/portcon.h"
#include "../inc/dp32g030/pwmplus.h"
#include "../inc/dp32g030/spi.h"
#include "../misc.hpp"
#include "gpio.hpp"
#include "spi.hpp"
#include "system.hpp"
#include <stdint.h>

#define NEED_WAIT_FIFO                                                         \
  ((SPI0->FIFOST & SPI_FIFOST_TFF_MASK) != SPI_FIFOST_TFF_BITS_NOT_FULL)

#define LCD_WIDTH 128
#define LCD_HEIGHT 64
#define LCD_XCENTER 64
#define LCD_YCENTER 32

class ST7565 {
public:
  void init(uint8_t contrast = 6) {
    SPI0_Init();
    ST7565_Configure_GPIO_B11();
    SPI_ToggleMasterMode(&SPI0->CR, false);
    writeByte(0xE2);
    delayMs(0x78);
    writeByte(0xA2);
    writeByte(0xC0);
    writeByte(0xA1);
    writeByte(0xA6);
    writeByte(0xA4);
    writeByte(0x24);
    writeByte(0x81);
    // ST7565_WriteByte(0x1F); // contrast
    writeByte(23 + contrast); // brightness 0 ~ 63
    writeByte(0x2B);
    delayMs(1);
    writeByte(0x2E);
    delayMs(1);
    writeByte(0x2F);
    writeByte(0x2F);
    writeByte(0x2F);
    writeByte(0x2F);
    delayMs(0x28);
    writeByte(0x40);
    writeByte(0xAF);
    SPI_WaitForUndocumentedTxFifoStatusBit();
    SPI_ToggleMasterMode(&SPI0->CR, true);
    fillScreen(0x00);

    initBacklight();
  }

  void initBacklight() {
    // 48MHz / 94 / 1024 ~ 500Hz
    const uint32_t PWM_FREQUENCY_HZ = 1000;
    PWM_PLUS0_CLKSRC |= ((CPU_CLOCK_HZ / 1024 / PWM_FREQUENCY_HZ) << 16);
    PWM_PLUS0_PERIOD = 1023;

    PORTCON_PORTB_SEL0 &= ~(0
                            // Back light
                            | PORTCON_PORTB_SEL0_B6_MASK);
    PORTCON_PORTB_SEL0 |= 0
                          // Back light PWM
                          | PORTCON_PORTB_SEL0_B6_BITS_PWMP0_CH0;

    PWM_PLUS0_GEN =
        PWMPLUS_GEN_CH0_OE_BITS_ENABLE | PWMPLUS_GEN_CH0_OUTINV_BITS_ENABLE | 0;

    PWM_PLUS0_CFG = PWMPLUS_CFG_CNT_REP_BITS_ENABLE |
                    PWMPLUS_CFG_COUNTER_EN_BITS_ENABLE | 0;
  }

  void render() {
    if (gRedrawScreen) {
      blit();
      gRedrawScreen = false;
    }
  }

  void fillScreen(uint8_t Value) {
    uint8_t i, j;

    SPI_ToggleMasterMode(&SPI0->CR, false);
    for (i = 0; i < 8; i++) {
      selectColumnAndLine(0, i);
      GPIO_SetBit(&GPIOB->DATA, GPIOB_PIN_ST7565_A0);
      for (j = 0; j < 132; j++) {
        while (NEED_WAIT_FIFO)
          continue;
        SPI0->WDR = Value;
      }
      SPI_WaitForUndocumentedTxFifoStatusBit();
    }
    SPI_ToggleMasterMode(&SPI0->CR, true);
  }

  void blit() {
    uint8_t Line;
    uint8_t Column;

    SPI_ToggleMasterMode(&SPI0->CR, false);
    writeByte(0x40);

    for (Line = 0; Line < ARRAY_SIZE(gFrameBuffer); Line++) {
      selectColumnAndLine(4U, Line);
      GPIO_SetBit(&GPIOB->DATA, GPIOB_PIN_ST7565_A0);
      for (Column = 0; Column < ARRAY_SIZE(gFrameBuffer[0]); Column++) {
        while (NEED_WAIT_FIFO)
          continue;
        SPI0->WDR = gFrameBuffer[Line][Column];
      }
      SPI_WaitForUndocumentedTxFifoStatusBit();
    }

    SPI_ToggleMasterMode(&SPI0->CR, true);
  }

  void setBrightness(uint8_t brigtness) {
    PWM_PLUS0_CH0_COMP = (1 << brigtness) - 1;
  }

private:
  uint8_t gFrameBuffer[8][LCD_WIDTH];
  bool gRedrawScreen = true;

  void ST7565_Configure_GPIO_B11() {
    GPIO_SetBit(&GPIOB->DATA, GPIOB_PIN_ST7565_RES);
    delayMs(1);
    GPIO_ClearBit(&GPIOB->DATA, GPIOB_PIN_ST7565_RES);
    delayMs(20);
    GPIO_SetBit(&GPIOB->DATA, GPIOB_PIN_ST7565_RES);
    delayMs(120);
  }

  void selectColumnAndLine(uint8_t Column, uint8_t Line) {
    GPIO_ClearBit(&GPIOB->DATA, GPIOB_PIN_ST7565_A0);
    while (NEED_WAIT_FIFO)
      continue;
    SPI0->WDR = Line + 0xB0;
    while (NEED_WAIT_FIFO)
      continue;
    SPI0->WDR = ((Column >> 4) & 0x0F) | 0x10;
    while (NEED_WAIT_FIFO)
      continue;
    SPI0->WDR = ((Column >> 0) & 0x0F);
    SPI_WaitForUndocumentedTxFifoStatusBit();
  }

  void writeByte(uint8_t Value) {
    GPIO_ClearBit(&GPIOB->DATA, GPIOB_PIN_ST7565_A0);
    while (NEED_WAIT_FIFO)
      continue;
    SPI0->WDR = Value;
  }
};

class Backlight {
public:
  void init() {}
};
