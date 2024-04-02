#pragma once

#include "../lib/enum.hpp"
#include <stdint.h>

class AbstractRadio {
public:
  enum Chip {
    RADIO_BK4819,
    RADIO_BK1080,
    RADIO_SI4732,
  };

  typedef enum class IncDecEnum {
    MOD_FM,
    MOD_AM,
    MOD_USB,
    MOD_BYP,
    MOD_RAW,
    MOD_WFM,
  } ModulationType;

  typedef enum {
    SQUELCH_RSSI_NOISE_GLITCH,
    SQUELCH_RSSI_GLITCH,
    SQUELCH_RSSI_NOISE,
    SQUELCH_RSSI,
  } SquelchType;

  typedef enum FilterBandwidth {
    FILTER_BW_WIDE,
    FILTER_BW_NARROW,
    FILTER_BW_NARROWER,
  } FilterBandwidth;

  virtual void init() = 0;

  virtual void setF(uint32_t f) = 0;
  virtual bool inRange(uint32_t f) = 0;
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
