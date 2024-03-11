#pragma once

#include <stdint.h>

class AbstractRadio {
public:
  enum Chip {
    RADIO_BK4819,
    RADIO_BK1080,
    RADIO_SI4732,
  };

  virtual void init() = 0;

  virtual void setF(uint32_t f) = 0;
  virtual uint32_t getF() = 0;

  virtual uint16_t getRSSI() = 0;
  virtual uint8_t getNoise() = 0;
  virtual uint8_t getGlitch() = 0;
  virtual uint8_t getSNR() = 0;
  virtual bool isSquelchOpen() = 0;

  virtual void idle() = 0;
  virtual void rxEnable() = 0;
  virtual void mute(bool) = 0;
};
