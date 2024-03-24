#pragma once

#include "apps/apps.hpp"
#include "driver/abstractradio.hpp"
#include "driver/audio.hpp"
#include "driver/bk1080.hpp"
#include "driver/bk4819.hpp"
#include "driver/gpio.hpp"
#include "driver/system.hpp"
#include "frequency.hpp"
#include "globals.hpp"
#include "helper/battery.hpp"
#include "helper/channels.hpp"
#include "helper/lootlist.hpp"
#include "helper/measurements.hpp"
#include "helpers/lootlist.hpp"
#include <stdint.h>
// #include "helper/msghelper.hpp"
#include "helpers/measurements.hpp"
#include "inc/dp32g030/gpio.hpp"
#include "misc.hpp"
#include "scheduler.hpp"
#include "settings.hpp"
#include <string.h>

class Radio : AbstractRadio {
public:
  constexpr static uint32_t VHF_UHF_BOUND1 = 24000000;
  constexpr static uint32_t VHF_UHF_BOUND2 = 28000000;

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

  typedef enum {
    FILTER_VHF,
    FILTER_UHF,
    FILTER_OFF,
  } Filter;

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
  // getsize(SquelchSettings)

  typedef struct VFO {
    AppType_t app;
    int16_t channel;
    ScanSettings scan;
    Step step : 4;
    uint32_t f : 27;
    uint32_t offset : 27;
    OffsetDirection offsetDir;
    ModulationType modulation : 4;
    FilterBandwidth bw : 2;
    TXOutputPower power : 2;
    uint8_t codeRX;
    uint8_t codeTX;
    uint8_t codeTypeRX : 4;
    uint8_t codeTypeTX : 4;
    SquelchSettings sq;
    uint8_t gainIndex : 5;

    uint32_t getStep() { return StepFrequencyTable[step]; }
  } __attribute__((packed)) VFO;

  VFO vfo;

  void init() {
    bk4819.init();
    bk1080.init();

    mainRadio = &bk4819;
  }

  void setF(uint32_t f) { mainRadio->setF(f); }
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
      bk4819.squelch(level, vfo.f);
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

  uint8_t getBandIndex(uint32_t f) {
    for (uint8_t i = 0; i < ARRAY_SIZE(STOCK_BANDS); ++i) {
      const FRange *b = &STOCK_BANDS[i];
      if (f >= b->start && f <= b->end) {
        return i;
      }
    }
    return 6;
  }

  void onVfoUpdate() {
    TaskRemove(saveCurrentCH);
    TaskAdd("CH save", saveCurrentCH, 2000, false, 0);
  }

  void toggleBK4819(bool on) {
    if (on) {
      bk4819.mute(false);
      delayMs(10);
      AUDIO_ToggleSpeaker(true);
    } else {
      AUDIO_ToggleSpeaker(false);
      delayMs(10);
      bk4819.mute(true);
    }
  }

  void toggleBK1080(bool on) {
    if (on) {
      bk1080.mute(false);
      delayMs(10);
      AUDIO_ToggleSpeaker(true);
    } else {
      AUDIO_ToggleSpeaker(false);
      delayMs(10);
      bk1080.mute(true);
    }
  }

  uint8_t calculateOutputPower(uint32_t f) {
    const uint8_t bi = getBandIndex(f);
    const FRange *range = &STOCK_BANDS[bi];
    const PowerCalibration *pCal = &gSettings.powCalib[bi];
    const uint32_t Middle = range->start + (range->end - range->start) / 2;

    if (f <= range->start) {
      return pCal->s;
    }

    if (f >= range->end) {
      return pCal->e;
    }

    if (f <= Middle) {
      return (uint8_t)(pCal->m + (((pCal->m - pCal->s) * (f - range->start)) /
                                  (Middle - range->start)));
    }

    return (uint8_t)(pCal->m + (((pCal->e - pCal->m) * (f - Middle)) /
                                (range->end - Middle)));
  }

  bool isBK1080Range(uint32_t f) { return f >= 6400000 && f <= 10800000; }

  void toggleRX(bool on) {
    if (gIsListening == on) {
      return;
    }
    gRedrawScreen = true;

    gIsListening = on;

    bk4819.toggleGpioOut(BK4819_GPIO0_PIN28_GREEN, on);

    if (on) {
      if (gSettings.backlightOnSquelch != BL_SQL_OFF) {
        BACKLIGHT_On();
      }
    } else {
      if (gSettings.backlightOnSquelch == BL_SQL_OPEN) {
        BACKLIGHT_Toggle(false);
      }
    }

    if (gIsBK1080) {
      toggleBK1080(on);
    } else {
      toggleBK4819(on);
    }
  }

  void enableCxCSS() {
    switch (vfo.codeTypeTX) {
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

  static TXState getTXState(uint32_t txF) {
    if (gSettings.upconverter) {
      return TX_DISABLED_UPCONVERTER;
    }

    if (gSettings.allowTX == TX_DISALLOW) {
      return TX_DISABLED;
    }

    if (gSettings.allowTX == TX_ALLOW_LPD_PMR && !FreqInRange(txF, &BAND_LPD) &&
        !FreqInRange(txF, &BAND_PMR)) {
      return TX_DISABLED;
    }

    if (gSettings.allowTX == TX_ALLOW_LPD_PMR_SATCOM &&
        !FreqInRange(txF, &BAND_LPD) && !FreqInRange(txF, &BAND_PMR) &&
        !FreqInRange(txF, &BAND_SATCOM)) {
      return TX_DISABLED;
    }

    if (gSettings.allowTX == TX_ALLOW_HAM && !FreqInRange(txF, &BAND_HAM2M) &&
        !FreqInRange(txF, &BAND_HAM70CM)) {
      return TX_DISABLED;
    }

    if (gBatteryPercent == 0) {
      return TX_BAT_LOW;
    }
    if (gChargingWithTypeC || gBatteryVoltage > 880) {
      return TX_VOL_HIGH;
    }

    return TX_ON;
  }

  void toggleTX(bool on) {
    if (gTxState == on) {
      return;
    }

    uint8_t power = 0;
    uint32_t txF = getTXFEx(&vfo);

    if (on) {
      gTxState = getTXState(txF);
      power = calculateOutputPower(txF);
      if (power > 0x91) {
        power = 0;
        gTxState = TX_POW_OVERDRIVE;
        return;
      }
      power >>= 2 - vfo.power;
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
    } else if (gTxState == TX_ON) {
      bk4819.exitDTMF_TX(true);
      enableCxCSS();

      bk4819.setupPowerAmplifier(0, 0);
      bk4819.toggleGpioOut(BK4819_GPIO1_PIN29_PA_ENABLE, false);
      bk4819.toggleGpioOut(BK4819_GPIO0_PIN28_RX_ENABLE, true);

      tuneToPure(vfo.f, true);
      bk4819.rxEnable();
    }

    bk4819.toggleGpioOut(BK4819_GPIO5_PIN1_RED, on);

    gTxState = on;
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
    vfo.modulation = IncDec<uint8_t, ModulationType>(
        vfo.modulation, 0, ARRAY_SIZE(modulationTypeOptions), 1);
    if (vfo.modulation == MOD_WFM) {
      if (isBK1080Range(vfo.f)) {
        toggleBK1080(true);
        return;
      }
      vfo.modulation = MOD_FM;
    }
    toggleBK1080(false);
    bk4819.setModulation(vfo.modulation);
    onVfoUpdate();
  }

  void updateStep(bool inc) {
    vfo.step = IncDec<uint8_t, Step>(
        vfo.step, 0, ARRAY_SIZE(StepFrequencyTable), inc ? 1 : -1);
    onVfoUpdate();
  }

  void toggleListeningBW() {
    if (vfo.bw == FILTER_BW_NARROWER) {
      vfo.bw = FILTER_BW_WIDE;
    } else {
      ++vfo.bw;
    }

    bk4819.setFilterBandwidth(vfo.bw);
    onVfoUpdate();
  }

  void toggleTxPower() {
    if (vfo.power == TX_POW_HIGH) {
      vfo.power = TX_POW_LOW;
    } else {
      ++vfo.power;
    }

    bk4819.setFilterBandwidth(vfo.bw); // TODO: ???
    onVfoUpdate();
  }

  void tuneToPure(uint32_t f, bool precise) {
    if (gIsBK1080) {
      BK1080_SetFrequency(f);
    } else {
      bk4819.tuneTo(f, precise);
    }
  }

  void setupByCurrentCH() {
    setupParams();
    toggleBK1080(vfo.modulation == MOD_WFM && isBK1080Range(vfo.f));
    tuneToPure(vfo.f, true);
  }

  // USE CASE: set vfo temporary for current app
  void tuneTo(uint32_t f) {
    vfo.f = f;
    vfo.vfo.channel = -1;
    setupByCurrentCH();
  }

  // USE CASE: set vfo and use in another app
  void tuneToSave(uint32_t f) {
    tuneTo(f);
    saveCurrentCH();
  }

  void saveCurrentCH() { CHS_Save(gSettings.activeCH, radio); }

  void vfoLoadCH(uint8_t i) {
    CH ch;
    CHANNELS_Load(gCH[i].vfo.channel, &gCH[i]);
    strncpy(gCHNames[i], ch.name, 9);
  }

  void loadCurrentCH() {
    for (uint8_t i = 0; i < 2; ++i) {
      CHS_Load(i, &gCH[i]);
      if (gCH[i].vfo.channel >= 0) {
        vfoLoadCH(i);
      }
      gCHBands[i] = BAND_ByFrequency(gCH[i].f);

      LOOT_Replace(&gLoot[i], gCH[i].f);
    }

    radio = &gCH[gSettings.activeCH];
    gCurrentLoot = &gLoot[gSettings.activeCH];
    setupByCurrentCH();
  }

  void setSquelch(uint8_t sq) {
    vfo.sq.level = sq;
    bk4819.squelch(sq, vfo.f, vfo.sq.openTime, vfo.sq.closeTime);
    onVfoUpdate();
  }

  void setSquelchType(SquelchType t) {
    vfo.sq.type = t;
    onVfoUpdate();
  }

  void setGain(uint8_t gainIndex) {
    bk4819.setGain(vfo.gainIndex = gainIndex);
    onVfoUpdate();
  }

  void setupParams() {
    tuneToPure(vfo.f, true);
    bk4819.squelchType(vfo.sq.type);
    bk4819.squelch(vfo.sq.level, vfo.f, vfo.sq.openTime, vfo.sq.closeTime);
    bk4819.setFilterBandwidth(vfo.bw);
    bk4819.setModulation(vfo.modulation);
    bk4819.setGain(vfo.gainIndex);
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

  bool tuneToCH(int16_t num) {
    if (CHANNELS_Existing(num)) {
      CHANNELS_Load(num, radio);
      vfo.vfo.channel = num;
      onVfoUpdate();
      setupByCurrentCH();
      return true;
    }
    return false;
  }

  void nextCH(bool next) {
    int16_t i;
    if (vfo.vfo.channel >= 0) {
      i = CHANNELS_Next(vfo.vfo.channel, next);
      if (i > -1) {
        vfo.vfo.channel = i;
        vfoLoadCH(gSettings.activeCH);
      }
      onVfoUpdate();
      setupByCurrentCH();
      return;
    }

    i = vfo.vfo.channel;

    if (!CHANNELS_Existing(vfo.vfo.channel)) {
      i = CHANNELS_Next(vfo.vfo.channel, true);
      if (i == -1) {
        return;
      }
    }

    tuneToCH(i);
  }

  uint8_t getActiveVFOGroup() { return vfo.groups; }

  void nextVFO() {
    gSettings.activeCH = !gSettings.activeCH;
    radio = &gCH[gSettings.activeCH];
    gCurrentLoot = &gLoot[gSettings.activeCH];
    setupByCurrentCH();
    toggleRX(false);
    SETTINGS_Save();
  }

  void toggleVfoMR() {
    if (vfo.vfo.channel >= 0) {
      vfo.vfo.channel = -1;
    } else {
      nextCH(true);
    }
    saveCurrentCH();
  }

  void updateSquelchLevel(bool next) {
    uint8_t sq = vfo.sq.level;
    if (!next && sq > 0) {
      sq--;
    }
    if (next && sq < 9) {
      sq++;
    }
    setSquelch(sq);
  }

  // TODO: бесшовное
  void nextFreq(bool next) {
    int8_t dir = next ? 1 : -1;

    if (vfo.vfo.channel >= 0) {
      nextCH(next);
      return;
    }

    Band *nextBand = BAND_ByFrequency(vfo.f + dir);
    if (nextBand != gCurrentBand && nextBand != &defaultBand) {
      if (next) {
        tuneTo(nextBand->band.bounds.start);
      } else {
        tuneTo(nextBand->band.bounds.end -
               nextBand->band.bounds.end %
                   StepFrequencyTable[nextBand->band.step]);
      }
    } else {
      tuneTo(vfo.f + StepFrequencyTable[nextBand->band.step] * dir);
    }
    onVfoUpdate();
  }

  void nextBandFreq(bool next) {
    uint32_t steps = BANDS_GetSteps(gCurrentBand);
    uint32_t step = BANDS_GetChannel(gCurrentBand, vfo.f);
    IncDec32(&step, 0, steps, next ? 1 : -1);
    vfo.f = BANDS_GetF(gCurrentBand, step);
    tuneToPure(vfo.f, true);
  }

  void nextBandFreqEx(bool next, bool precise) {
    uint32_t steps = BANDS_GetSteps(gCurrentBand);
    uint32_t step = BANDS_GetChannel(gCurrentBand, vfo.f);
    IncDec32(&step, 0, steps, next ? 1 : -1);
    vfo.f = BANDS_GetF(gCurrentBand, step);
    tuneToPure(vfo.f, precise);
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
