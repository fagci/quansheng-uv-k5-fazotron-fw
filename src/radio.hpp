#pragma once

#include "driver/abstractradio.hpp"
#include "driver/audio.hpp"
#include "driver/bk1080.hpp"
#include "driver/bk4819.hpp"
#include "driver/system.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include "misc.hpp"
#include <stdint.h>

class Radio : AbstractRadio {
public:
  static BK4819 bk4819;
  static BK1080 bk1080;

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
  uint32_t getF() { return mainRadio->getF(); }
  void rxEnable() { mainRadio->rxEnable(); }
  void idle(bool on) { mainRadio->idle(on); }
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

  void mute(bool on) {
    if (on) {
      mainRadio->mute(false);
      delayMs(10);
      Audio::toggleSpeaker(true);
    } else {
      Audio::toggleSpeaker(false);
      delayMs(10);
      mainRadio->mute(true);
    }
  }

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

  void toggleRX(bool on) {
    if (gIsListening != on) {
      gIsListening = on;
      bk4819.toggleGpioOut(BK4819_GPIO0_PIN28_GREEN, on);
      mute(!on);
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
    if (chip != RADIO_BK4819) {
      return;
    }
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

  void toggleModulation() { mainRadio->toggleModulation(); }

  void updateStep(bool inc) {
    step = IncDec(step, 0, ARRAY_SIZE(StepFrequencyTable), inc ? 1 : -1);
  }

  void toggleListeningBW() {
    bw = IncDec(bw, FILTER_BW_WIDE, FILTER_BW_NARROWER, 1);

    bk4819.setFilterBandwidth(bw);
  }

  void toggleTxPower() { power = IncDec(power, TX_POW_LOW, TX_POW_HIGH, 1); }

  void tuneToPure(uint32_t f, bool precise) {
    switch (chip) {
    case RADIO_BK4819:
      bk4819.tuneTo(f, precise);
      break;
    default:
      mainRadio->setF(f);
      break;
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

  void handleInterrupts(void *handler(uint16_t intBits)) {
    while (bk4819.readRegister(BK4819_REG_0C) & 1u) {
      bk4819.writeRegister(BK4819_REG_02, 0);

      handler(bk4819.readRegister(BK4819_REG_02));
    }
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
  AbstractRadio *mainRadio;

  bool gIsListening = false;

  TXState gTxState = TX_UNKNOWN;
  Filter selectedFilter = FILTER_OFF;
};
