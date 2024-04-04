#pragma once

#include <stdint.h>

enum ModulationType {
  MOD_NFM,
  MOD_WFM,
  MOD_AM,
  MOD_DSB,
  MOD_USB,
  MOD_LSB,
  MOD_CW,
  MOD_RAW,
  MOD_BYP,
};

class AbstractRadio {
public:
  virtual void init() = 0;

  virtual void setF(uint32_t f) = 0;
  virtual bool inRange(uint32_t f) = 0;
  virtual uint32_t getF() = 0;

  virtual uint16_t getRSSI() = 0;
  virtual uint8_t getNoise() = 0;
  virtual uint8_t getGlitch() = 0;
  virtual uint8_t getSNR() = 0;
  virtual bool isSquelchOpen() = 0;

  virtual void rxEnable() = 0;
  virtual void mute(bool) = 0;
  virtual void sleep(bool) = 0;
  virtual void idle(bool) = 0;
};
