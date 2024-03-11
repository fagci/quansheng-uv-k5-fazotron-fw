#pragma once

#include "../inc/dp32g030/gpio.h"
#include "../misc.hpp"
#include "abstractradio.hpp"
#include "bk1080-regs.hpp"
#include "gpio.hpp"
#include "i2c.hpp"
#include "system.hpp"
#include <stdint.h>

class BK1080 : public AbstractRadio {
public:
  typedef enum {
    BK1080_BAND_87_5_108,
    BK1080_BAND_76_108,
    BK1080_BAND_76_90,
    BK1080_BAND_64_76,
  } BK1080_Band;

  typedef enum {
    BK1080_CHSP_200,
    BK1080_CHSP_100,
    BK1080_CHSP_50,
  } BK1080_ChannelSpacing;

  uint32_t getF() {}

  uint8_t getNoise() { return 0; }
  uint8_t getGlitch() { return 0; }
  uint8_t getSNR() { return 0; }
  bool isSquelchOpen() { return true; }

  void init() {
    for (uint8_t i = 0; i < ARRAY_SIZE(BK1080_RegisterTable); i++) {
      writeRegister((BK1080_Register_t)i, BK1080_RegisterTable[i]);
    }

    SYSTEM_DelayMs(250);
    writeRegister(BK1080_REG_25_INTERNAL, 0xA83C);
    writeRegister(BK1080_REG_25_INTERNAL, 0xA8BC);
    SYSTEM_DelayMs(60);
    isInitBK1080 = true;
  }

  void rxEnable() {
    mute(false);
    GPIO_SetBit(&GPIOB->DATA, GPIOB_PIN_BK1080);
  }

  void setF(uint32_t f) {
    if (f == currentF) {
      return;
    }
    currentF = f;
    uint8_t vol = 0b1111;
    uint8_t chSp = BK1080_CHSP_100;
    uint8_t seekThres = 0b00001010;

    uint8_t band = f < 7600000 ? BK1080_BAND_64_76 : BK1080_BAND_76_108;

    uint32_t startF = band == BK1080_BAND_64_76 ? 6400000 : 7600000;

    uint16_t channel = (f - startF) / CH_SP_F[chSp];

    uint16_t sysCfg2 =
        (vol << 0) | (chSp << 4) | (band << 6) | (seekThres << 8);

    writeRegister(BK1080_REG_05_SYSTEM_CONFIGURATION2, sysCfg2);
    writeRegister(BK1080_REG_03_CHANNEL, channel);
    SYSTEM_DelayMs(10);
    writeRegister(BK1080_REG_03_CHANNEL, channel | 0x8000);
  }

  uint16_t getRSSI() { return (readRegister(BK1080_REG_10) & 0xFF) << 1; }

  void idle() { GPIO_ClearBit(&GPIOB->DATA, GPIOB_PIN_BK1080); }

  uint16_t readRegister(BK1080_Register_t Register) {
    uint8_t Value[2];

    I2C_Start();
    I2C_Write(0x80);
    I2C_Write((Register << 1) | I2C_READ);
    I2C_ReadBuffer(Value, sizeof(Value));
    I2C_Stop();
    return (Value[0] << 8) | Value[1];
  }

  void writeRegister(BK1080_Register_t Register, uint16_t Value) {
    I2C_Start();
    I2C_Write(0x80);
    I2C_Write((Register << 1) | I2C_WRITE);
    Value = ((Value >> 8) & 0xFF) | ((Value & 0xFF) << 8);
    I2C_WriteBuffer(&Value, sizeof(Value));
    I2C_Stop();
  }

  void mute(bool mute) {
    if (mute) {
      writeRegister(BK1080_REG_02_POWER_CONFIGURATION, 0x4201);
    } else {
      writeRegister(BK1080_REG_02_POWER_CONFIGURATION, 0x0201);
    }
  }

  uint16_t getFrequencyDeviation() { return readRegister(BK1080_REG_07) >> 4; }

private:
  bool isInitBK1080;
  uint32_t currentF = 0;

  static constexpr const uint16_t BK1080_RegisterTable[] = {
      0x0008, 0x1080, 0x0201, 0x0000, 0x40C0, 0x0A1F, 0x002E, 0x02FF, 0x5B11,
      0x0000, 0x411E, 0x0000, 0xCE00, 0x0000, 0x0000, 0x1000, 0x3197, 0x0000,
      0x13FF, 0x9852, 0x0000, 0x0000, 0x0008, 0x0000, 0x51E1, 0xA8BC, 0x2645,
      0x00E4, 0x1CD8, 0x3A50, 0xEAE0, 0x3000, 0x0200, 0x0000,
  };

  static constexpr uint16_t CH_SP_F[] = {20000, 10000, 5000};
};
