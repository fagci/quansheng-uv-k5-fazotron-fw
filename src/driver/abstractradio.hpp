#pragma once

#include <stddef.h>
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

static const char *MOD_NAMES[9] = {
    "NFM", "WFM", "AM", "DSB", "USB", "LSB", "CW", "RAW", "BYP",
};

enum Chip {
  RADIO_BK4819,
  RADIO_BK1080,
  RADIO_SI4732,
};

enum ScanTimeout {
  SCAN_TO_0,
  SCAN_TO_500ms,
  SCAN_TO_1s,
  SCAN_TO_2s,
  SCAN_TO_5s,
  SCAN_TO_10s,
  SCAN_TO_30s,
  SCAN_TO_1min,
  SCAN_TO_2min,
  SCAN_TO_5min,
  SCAN_TO_NONE,
};

enum SquelchType {
  SQUELCH_RSSI_NOISE_GLITCH,
  SQUELCH_RSSI_GLITCH,
  SQUELCH_RSSI_NOISE,
  SQUELCH_RSSI,
};

enum FilterBandwidth {
  FILTER_BW_WIDE,
  FILTER_BW_NARROW,
  FILTER_BW_NARROWER,
};

typedef enum {
  STEP_0_01kHz,
  STEP_0_1kHz,
  STEP_1_0kHz,
  STEP_2_5kHz,
  STEP_5_0kHz,
  STEP_6_25kHz,
  STEP_8_33kHz,
  STEP_9kHz,
  STEP_10_0kHz,
  STEP_12_5kHz,
  STEP_25_0kHz,
  STEP_100_0kHz,
} Step;

typedef enum {
  OFFSET_NONE,
  OFFSET_PLUS,
  OFFSET_MINUS,
} OffsetDirection;

typedef enum {
  TX_POW_LOW,
  TX_POW_MID,
  TX_POW_HIGH,
} TXOutputPower;

typedef struct {
  uint8_t timeout : 8;
  ScanTimeout openedTimeout : 4;
  ScanTimeout closedTimeout : 4;
} __attribute__((packed)) ScanSettings;

typedef struct {
  uint8_t level : 6;
  uint8_t openTime : 2;
  SquelchType type;
  uint8_t closeTime : 3;
} __attribute__((packed)) SquelchSettings;

struct VFO {
  int16_t channel;
  ScanSettings scan;
  Step step : 4;
  uint32_t f : 27;
  uint32_t offset : 27;
  OffsetDirection offsetDir;
  uint8_t modulation : 4;
  FilterBandwidth bw : 2;
  TXOutputPower power : 2;
  uint8_t codeRX;
  uint8_t codeTX;
  uint8_t codeTypeRX : 4;
  uint8_t codeTypeTX : 4;
  SquelchSettings sq;
  uint8_t gainIndex : 5;
  Chip chip : 2;

  VFO *getVfo() { return this; }

  static constexpr size_t size() { return sizeof(VFO); };
} __attribute__((packed));

class AbstractRadio : protected VFO {
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

  VFO *vfo() { return getVfo(); }
};
