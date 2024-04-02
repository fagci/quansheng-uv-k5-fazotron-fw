#pragma once

#include "driver/abstractradio.hpp"
#include "driver/audio.hpp"
#include "driver/bk1080.hpp"
#include "driver/bk4819.hpp"
#include "driver/system.hpp"
#include "globals.hpp"
#include "lib/utils.hpp"
#include "misc.hpp"
#include <stdint.h>

typedef enum {
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
} ScanTimeout;

typedef struct {
  uint8_t timeout : 8;
  ScanTimeout openedTimeout : 4;
  ScanTimeout closedTimeout : 4;
} __attribute__((packed)) ScanSettings;

typedef struct {
  uint8_t level : 6;
  uint8_t openTime : 2;
  AbstractRadio::SquelchType type;
  uint8_t closeTime : 3;
} __attribute__((packed)) SquelchSettings;

struct VFO {
  int16_t channel;
  ScanSettings scan;
  Step step : 4;
  uint32_t f : 27;
  uint32_t offset : 27;
  OffsetDirection offsetDir;
  AbstractRadio::ModulationType modulation : 4;
  AbstractRadio::FilterBandwidth bw : 2;
  TXOutputPower power : 2;
  uint8_t codeRX;
  uint8_t codeTX;
  uint8_t codeTypeRX : 4;
  uint8_t codeTypeTX : 4;
  SquelchSettings sq;
  uint8_t gainIndex : 5;
} __attribute__((packed));

class Radio : AbstractRadio, VFO {
public:
  constexpr static uint32_t VHF_UHF_BOUND1 = 24000000;
  constexpr static uint32_t VHF_UHF_BOUND2 = 28000000;

  typedef enum {
    FILTER_VHF,
    FILTER_UHF,
    FILTER_OFF,
  } Filter;

  void init() {
    bk4819.init();
    bk1080.init();

    mainRadio = &bk4819;
  }

  void setF(uint32_t freq) { mainRadio->setF(freq); }
  uint32_t getF() { mainRadio->getF(); }
  void rxEnable() { mainRadio->rxEnable(); }
  void idle() { mainRadio->idle(); }
  void resetRSSI() {
    if (mainRadio == &bk4819) {
      bk4819.resetRSSI();
    }
  }
  bool isSquelchOpen() { return mainRadio->isSquelchOpen(); }
  TXState getTxState() { return gTxState; }
  void squelch(uint8_t level) {
    if (mainRadio == &bk4819) {
      bk4819.squelch(level, f);
    }
  }
  uint32_t getStep() { return StepFrequencyTable[step]; }

  virtual bool inRange(uint32_t f) { return mainRadio->inRange(f); }

  virtual uint16_t getRSSI() { return mainRadio->getRSSI(); }
  virtual uint8_t getNoise() { return mainRadio->getNoise(); }
  virtual uint8_t getGlitch() { return mainRadio->getGlitch(); }
  virtual uint8_t getSNR() { return mainRadio->getSNR(); }

  virtual void mute(bool m) { mainRadio->mute(m); }

  void selectFilter(Filter filterNeeded) {
    if (selectedFilter == filterNeeded) {
      return;
    }

    selectedFilter = filterNeeded;
    bk4819.toggleGpioOut(BK4819_GPIO4_PIN32_VHF_LNA,
                         filterNeeded == FILTER_VHF);
    bk4819.toggleGpioOut(BK4819_GPIO3_PIN31_UHF_LNA,
                         filterNeeded == FILTER_UHF);
  }

  void toggleBK4819(bool on) {
    if (on) {
      bk4819.mute(false);
      delayMs(10);
      Audio::toggleSpeaker(true);
    } else {
      Audio::toggleSpeaker(false);
      delayMs(10);
      bk4819.mute(true);
    }
  }

  void toggleBK1080(bool on) {
    if (on) {
      bk1080.mute(false);
      delayMs(10);
      Audio::toggleSpeaker(true);
    } else {
      Audio::toggleSpeaker(false);
      delayMs(10);
      bk1080.mute(true);
    }
  }

  void toggleRX(bool on) {
    if (gIsListening == on) {
      return;
    }

    gIsListening = on;

    bk4819.toggleGpioOut(BK4819_GPIO0_PIN28_GREEN, on);

    if (gIsBK1080) {
      toggleBK1080(on);
    } else {
      toggleBK4819(on);
    }
  }

  void enableCxCSS() {
    switch (codeTypeTX) {
    /* case CODE_TYPE_DIGITAL:
    case CODE_TYPE_REVERSE_DIGITAL:
            bk4819.enableCDCSS();
            break; */
    default:
      bk4819.enableCTCSS();
      break;
    }

    delayMs(200);
  }

  static uint32_t getTXFEx(VFO *vfo) {
    if (vfo->offset == 0 || vfo->offsetDir == OFFSET_NONE) {
      return vfo->f;
    }

    return vfo->f +
           (vfo->offsetDir == OFFSET_PLUS ? vfo->offset : -vfo->offset);
  }

  void toggleTX(bool on, uint32_t txF, uint8_t power) {
    if (on) {
      toggleRX(false);

      bk4819.toggleGpioOut(BK4819_GPIO0_PIN28_RX_ENABLE, false);
      setupParams();

      tuneToPure(txF, true);

      bk4819.prepareTransmit();

      delayMs(10);
      bk4819.toggleGpioOut(BK4819_GPIO1_PIN29_PA_ENABLE, true);
      delayMs(5);
      bk4819.setupPowerAmplifier(power, txF);
      delayMs(10);
      bk4819.exitSubAu();
    } else {
      bk4819.exitDTMF_TX(true);
      enableCxCSS();

      bk4819.setupPowerAmplifier(0, 0);
      bk4819.toggleGpioOut(BK4819_GPIO1_PIN29_PA_ENABLE, false);
      bk4819.toggleGpioOut(BK4819_GPIO0_PIN28_RX_ENABLE, true);

      tuneToPure(f, true);
      bk4819.rxEnable();
    }

    bk4819.toggleGpioOut(BK4819_GPIO5_PIN1_RED, on);
  }

  void toggleBK1080(bool on) {
    if (on == gIsBK1080) {
      return;
    }
    gIsBK1080 = on;

    if (gIsBK1080) {
      toggleBK4819(false);
      bk4819.idle();
    } else {
      toggleBK1080(false);
      bk4819.rxEnable();
    }
  }

  void toggleModulation() {
    ++modulation;
    modulation = IncDec<uint8_t, ModulationType>(
        modulation, 0, ARRAY_SIZE(modulationTypeOptions), 1);
    if (modulation == MOD_WFM) {
      if (bk1080.inRange(f)) {
        toggleBK1080(true);
        return;
      }
      modulation = MOD_FM;
    }
    toggleBK1080(false);
    bk4819.setModulation(modulation);
  }

  void updateStep(bool inc) {
    step = IncDec<uint8_t, Step>(step, 0, ARRAY_SIZE(StepFrequencyTable),
                                 inc ? 1 : -1);
  }

  void toggleListeningBW() {
    if (bw == FILTER_BW_NARROWER) {
      bw = FILTER_BW_WIDE;
    } else {
      ++bw;
    }

    bk4819.setFilterBandwidth(bw);
  }

  void toggleTxPower() {
    if (power == TX_POW_HIGH) {
      power = TX_POW_LOW;
    } else {
      ++power;
    }

    bk4819.setFilterBandwidth(bw); // TODO: ???
  }

  void tuneToPure(uint32_t f, bool precise) {
    if (gIsBK1080) {
      bk1080.setF(f);
    } else {
      bk4819.tuneTo(f, precise);
    }
  }

  void setSquelch(uint8_t sql) { bk4819.squelch(sq.level = sql, f); }

  void setSquelchType(SquelchType t) { sq.type = t; }

  void setGain(uint8_t index) { bk4819.setGain(gainIndex = index); }

  void setupParams() {
    tuneToPure(f, true);
    bk4819.squelchType(sq.type);
    bk4819.squelch(sq.level, f);
    bk4819.setFilterBandwidth(bw);
    bk4819.setModulation(modulation);
    bk4819.setGain(gainIndex);
  }

  uint16_t getRSSI() { return gIsBK1080 ? 128 : bk4819.getRSSI(); }

  void handleInterrupts(void *handler(uint16_t intBits)) {
    while (bk4819.readRegister(BK4819_REG_0C) & 1u) {
      bk4819.writeRegister(BK4819_REG_02, 0);

      handler(bk4819.readRegister(BK4819_REG_02));
    }
  }

  bool isSqOpen(bool manual) {
    return gIsBK1080 ? true
                     : (manual ? bk4819.isSquelchOpen()
                               : isSqOpenSimple(
                                     msm->rssi)); // FIXME: actual RSSI problem
  }

  void enableToneDetection() {
    bk4819.setCTCSSFrequency(670);
    bk4819.setTailDetection(550);
    bk4819.writeRegister(BK4819_REG_3F, BK4819_REG_3F_CxCSS_TAIL);
  }

  void updateSquelchLevel(bool next) {
    uint8_t sql = sq.level;
    if (!next && sql > 0) {
      sql--;
    }
    if (next && sql < 9) {
      sql++;
    }
    setSquelch(sql);
  }

private:
  BK4819 bk4819;
  BK1080 bk1080;

  AbstractRadio *mainRadio;

  bool gIsListening = false;

  bool gIsBK1080 = false;

  TXState gTxState = TX_UNKNOWN;
  Filter selectedFilter = FILTER_OFF;
};
