#pragma once

#include "../inc/dp32g030/gpio.h"
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
    SYSTEM_DelayMs(0x78);
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
    SYSTEM_DelayMs(1);
    writeByte(0x2E);
    SYSTEM_DelayMs(1);
    writeByte(0x2F);
    writeByte(0x2F);
    writeByte(0x2F);
    writeByte(0x2F);
    SYSTEM_DelayMs(0x28);
    writeByte(0x40);
    writeByte(0xAF);
    SPI_WaitForUndocumentedTxFifoStatusBit();
    SPI_ToggleMasterMode(&SPI0->CR, true);
    fillScreen(0x00);
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

private:
  uint8_t gFrameBuffer[8][LCD_WIDTH];
  bool gRedrawScreen = true;

  void ST7565_Configure_GPIO_B11() {
    GPIO_SetBit(&GPIOB->DATA, GPIOB_PIN_ST7565_RES);
    SYSTEM_DelayMs(1);
    GPIO_ClearBit(&GPIOB->DATA, GPIOB_PIN_ST7565_RES);
    SYSTEM_DelayMs(20);
    GPIO_SetBit(&GPIOB->DATA, GPIOB_PIN_ST7565_RES);
    SYSTEM_DelayMs(120);
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
