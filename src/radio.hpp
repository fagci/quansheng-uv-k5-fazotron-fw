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
#include "inc/dp32g030/gpio.hpp"
#include "misc.hpp"
#include "scheduler.hpp"
#include "settings.hpp"
#include <string.h>

class Radio : AbstractRadio {
public:
  BK4819 bk4819;
  BK1080 bk1080;

  AbstractRadio *mainRadio;

  void init() {
    bk4819.init();
    bk1080.init();

    mainRadio = &bk4819;
  }

  void setF(uint32_t f) { mainRadio->setF(f); }


  CH *radio;
  uint8_t gCurrentVfo;
  Loot *gCurrentLoot;
  FRange *gCurrentBounds;

  bool gIsListening = false;
  bool gMonitorMode = false;

  bool gIsBK1080 = false;

  TXState gTxState = TX_UNKNOWN;

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
    TaskRemove(RADIO_SaveCurrentCH);
    TaskAdd("CH save", RADIO_SaveCurrentCH, 2000, false, 0);
  }

  void toggleBK4819(bool on) {
    if (on) {
      bk4819.mute(false);
      SYSTEM_DelayMs(10);
      AUDIO_ToggleSpeaker(true);
    } else {
      AUDIO_ToggleSpeaker(false);
      SYSTEM_DelayMs(10);
      bk4819.mute(true);
    }
  }

  void toggleBK1080(bool on) {
    if (on) {
      BK1080_Init(radio->f, true);
      BK1080_Mute(false);
      SYSTEM_DelayMs(10);
      AUDIO_ToggleSpeaker(true);
    } else {
      AUDIO_ToggleSpeaker(false);
      SYSTEM_DelayMs(10);
      BK1080_Mute(true);
      BK1080_Init(0, false);
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

  bool isSqOpenSimple(uint16_t r) {
    uint8_t band = radio->f > SETTINGS_GetFilterBound() ? 1 : 0;
    uint8_t sq = radio->sq.level;
    uint16_t ro = SQ[band][0][sq];
    uint16_t rc = SQ[band][1][sq];
    uint8_t no = SQ[band][2][sq];
    uint8_t nc = SQ[band][3][sq];
    uint8_t go = SQ[band][4][sq];
    uint8_t gc = SQ[band][5][sq];

    uint8_t n, g;

    bool open;

    switch (radio->sq.type) {
    case SQUELCH_RSSI_NOISE_GLITCH:
      n = BK4819_GetNoise();
      g = BK4819_GetGlitch();
      open = r >= ro && n <= no && g <= go;
      if (r < rc || n > nc || g > gc) {
        open = false;
      }
      break;
    case SQUELCH_RSSI_NOISE:
      n = BK4819_GetNoise();
      open = r >= ro && n <= no;
      if (r < rc || n > nc) {
        open = false;
      }
      break;
    case SQUELCH_RSSI_GLITCH:
      g = BK4819_GetGlitch();
      open = r >= ro && g <= go;
      if (r < rc || g > gc) {
        open = false;
      }
      break;
    case SQUELCH_RSSI:
      open = r >= ro;
      if (r < rc) {
        open = false;
      }
      break;
    }

    return open;
  }

  void setupRegisters() {
    uint32_t Frequency = 0;

    GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
    BK4819_ToggleGpioOut(BK4819_GPIO0_PIN28_GREEN, false);
    BK4819_ToggleGpioOut(BK4819_GPIO5_PIN1_RED, false);
    BK4819_ToggleGpioOut(BK4819_GPIO1_PIN29_PA_ENABLE, false);

    BK4819_SetFilterBandwidth(BK4819_FILTER_BW_WIDE);

    BK4819_SetupPowerAmplifier(0, 0);

    while (BK4819_ReadRegister(BK4819_REG_0C) & 1U) {
      BK4819_WriteRegister(BK4819_REG_02, 0);
      SYSTEM_DelayMs(1);
    }
    BK4819_WriteRegister(BK4819_REG_3F, 0);
    BK4819_WriteRegister(BK4819_REG_7D, 0xE94F);
    tuneToPure(Frequency, true);
    BK4819_ToggleGpioOut(BK4819_GPIO0_PIN28_RX_ENABLE, true);
    BK4819_WriteRegister(
        BK4819_REG_48,
        (11u << 12) |    // ??? .. 0 ~ 15, doesn't seem to make any difference
            (1u << 10) | // AF Rx Gain-1
            (56 << 4) |  // AF Rx Gain-2
            (8 << 0));   // AF DAC Gain (after Gain-1 and Gain-2)

    BK4819_DisableScramble();

    BK4819_DisableVox();
    BK4819_DisableDTMF();

    BK4819_WriteRegister(BK4819_REG_3F, 0);
    BK4819_WriteRegister(0x40, (BK4819_ReadRegister(0x40) & ~(0b11111111111)) |
                                   0b10110101010 | (1 << 12));
    // BK4819_WriteRegister(0x40, (1 << 12) | (1450));
  }

  bool isBK1080Range(uint32_t f) { return f >= 6400000 && f <= 10800000; }

  void toggleRX(bool on) {
    if (gIsListening == on) {
      return;
    }
    gRedrawScreen = true;

    gIsListening = on;

    BK4819_ToggleGpioOut(BK4819_GPIO0_PIN28_GREEN, on);

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
    switch (radio->codeTypeTX) {
    /* case CODE_TYPE_DIGITAL:
    case CODE_TYPE_REVERSE_DIGITAL:
            BK4819_EnableCDCSS();
            break; */
    default:
      BK4819_EnableCTCSS();
      break;
    }

    SYSTEM_DelayMs(200);
  }

  uint32_t getTXFEx(CH *vfo) {
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
    uint32_t txF = getTXFEx(radio);

    if (on) {
      gTxState = getTXState(txF);
      power = calculateOutputPower(txF);
      if (power > 0x91) {
        power = 0;
        gTxState = TX_POW_OVERDRIVE;
        return;
      }
      power >>= 2 - radio->power;
    }

    if (on) {
      toggleRX(false);

      BK4819_ToggleGpioOut(BK4819_GPIO0_PIN28_RX_ENABLE, false);
      setupParams();

      tuneToPure(txF, true);

      BK4819_PrepareTransmit();

      SYSTEM_DelayMs(10);
      BK4819_ToggleGpioOut(BK4819_GPIO1_PIN29_PA_ENABLE, true);
      SYSTEM_DelayMs(5);
      BK4819_SetupPowerAmplifier(power, txF);
      SYSTEM_DelayMs(10);
      BK4819_ExitSubAu();
    } else if (gTxState == TX_ON) {
      BK4819_ExitDTMF_TX(true);
      enableCxCSS();

      BK4819_SetupPowerAmplifier(0, 0);
      BK4819_ToggleGpioOut(BK4819_GPIO1_PIN29_PA_ENABLE, false);
      BK4819_ToggleGpioOut(BK4819_GPIO0_PIN28_RX_ENABLE, true);

      tuneToPure(radio->f, true);
      BK4819_RX_TurnOn();
    }

    BK4819_ToggleGpioOut(BK4819_GPIO5_PIN1_RED, on);

    gTxState = on;
  }

  void toggleBK1080(bool on) {
    if (on == gIsBK1080) {
      return;
    }
    gIsBK1080 = on;

    if (gIsBK1080) {
      toggleBK4819(false);
      BK4819_Idle();
    } else {
      toggleBK1080(false);
      BK4819_RX_TurnOn();
    }
  }

  void toggleModulation() {
    if (radio->modulation == MOD_WFM) {
      radio->modulation = MOD_FM;
    } else {
      ++radio->modulation;
    }
    if (radio->modulation == MOD_WFM) {
      if (isBK1080Range(radio->f)) {
        toggleBK1080(true);
        return;
      }
      radio->modulation = MOD_FM;
    }
    toggleBK1080(false);
    BK4819_SetModulation(radio->modulation);
    onVfoUpdate();
  }

  void updateStep(bool inc) {
    uint8_t step = radio->vfo.step;
    IncDec8(&step, 0, ARRAY_SIZE(StepFrequencyTable), inc ? 1 : -1);
    radio->vfo.step = step;
    onVfoUpdate();
  }

  void toggleListeningBW() {
    if (radio->bw == BK4819_FILTER_BW_NARROWER) {
      radio->bw = BK4819_FILTER_BW_WIDE;
    } else {
      ++radio->bw;
    }

    BK4819_SetFilterBandwidth(radio->bw);
    onVfoUpdate();
  }

  void toggleTxPower() {
    if (radio->power == TX_POW_HIGH) {
      radio->power = TX_POW_LOW;
    } else {
      ++radio->power;
    }

    BK4819_SetFilterBandwidth(radio->bw); // TODO: ???
    onVfoUpdate();
  }

  void tuneToPure(uint32_t f, bool precise) {
    if (gIsBK1080) {
      BK1080_SetFrequency(f);
    } else {
      BK4819_TuneTo(f, precise);
    }
  }

  void setupByCurrentCH() {
    setupParams();
    toggleBK1080(radio->modulation == MOD_WFM && isBK1080Range(radio->f));
    tuneToPure(radio->f, true);
  }

  // USE CASE: set vfo temporary for current app
  void tuneTo(uint32_t f) {
    radio->f = f;
    radio->vfo.channel = -1;
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
    radio->sq.level = sq;
    BK4819_Squelch(sq, radio->f, radio->sq.openTime, radio->sq.closeTime);
    onVfoUpdate();
  }

  void setSquelchType(SquelchType t) {
    radio->sq.type = t;
    onVfoUpdate();
  }

  void setGain(uint8_t gainIndex) {
    BK4819_SetGain(radio->gainIndex = gainIndex);
    onVfoUpdate();
  }

  void setupParams() {
    tuneToPure(radio->f, true);
    BK4819_SquelchType(radio->sq.type);
    BK4819_Squelch(radio->sq.level, radio->f, radio->sq.openTime,
                   radio->sq.closeTime);
    BK4819_SetFilterBandwidth(radio->bw);
    BK4819_SetModulation(radio->modulation);
    BK4819_SetGain(radio->gainIndex);
  }

  uint16_t getRSSI() { return gIsBK1080 ? 128 : BK4819_GetRSSI(); }

  static uint32_t lastTailTone = 0;
  Loot *updateMeasurements() {
    Loot *msm = LOOT_Get(radio->f);
    msm->rssi = getRSSI();
    msm->open = gIsBK1080 ? true
                          : (radio->sq.openTime || radio->sq.closeTime
                                 ? BK4819_IsSquelchOpen()
                                 : isSqOpenSimple(msm->rssi));

    while (BK4819_ReadRegister(BK4819_REG_0C) & 1u) {
      BK4819_WriteRegister(BK4819_REG_02, 0);

      uint16_t intBits = BK4819_ReadRegister(BK4819_REG_02);

      // MSG_StorePacket(intBits);

      if (intBits & BK4819_REG_02_CxCSS_TAIL) {
        msm->open = false;
        lastTailTone = Now();
      }
    }

    // else sql reopens
    if ((Now() - lastTailTone) < 250) {
      msm->open = false;
    }

    if (!gMonitorMode && radio->sq.level != 0) {
      LOOT_Update(msm);
    }

    bool rx = msm->open;
    if (gTxState != TX_ON) {
      if (gMonitorMode) {
        rx = true;
      } else if (gSettings.noListen && (gCurrentApp->id == APP_SPECTRUM ||
                                        gCurrentApp->id == APP_ANALYZER)) {
        rx = false;
      } else if (gSettings.skipGarbageFrequencies &&
                 (radio->f % 1300000 == 0)) {
        rx = false;
      }
      toggleRX(rx);
    }
    return msm;
  }

  bool updateMeasurementsEx(Loot *dest) {
    Loot *msm = LOOT_Get(radio->f);
    updateMeasurements();
    LOOT_UpdateEx(dest, msm);
    return msm->open;
  }

  void enableToneDetection() {
    BK4819_SetCTCSSFrequency(670);
    BK4819_SetTailDetection(550);
    BK4819_WriteRegister(BK4819_REG_3F, BK4819_REG_3F_CxCSS_TAIL/*  |
                                          BK4819_REG_3F_SQUELCH_LOST |
                                          BK4819_REG_3F_SQUELCH_FOUND */);
  }

  bool tuneToCH(int16_t num) {
    if (CHANNELS_Existing(num)) {
      CHANNELS_Load(num, radio);
      radio->vfo.channel = num;
      onVfoUpdate();
      setupByCurrentCH();
      return true;
    }
    return false;
  }

  void nextCH(bool next) {
    int16_t i;
    if (radio->vfo.channel >= 0) {
      i = CHANNELS_Next(radio->vfo.channel, next);
      if (i > -1) {
        radio->vfo.channel = i;
        vfoLoadCH(gSettings.activeCH);
      }
      onVfoUpdate();
      setupByCurrentCH();
      return;
    }

    i = radio->vfo.channel;

    if (!CHANNELS_Existing(radio->vfo.channel)) {
      i = CHANNELS_Next(radio->vfo.channel, true);
      if (i == -1) {
        return;
      }
    }

    tuneToCH(i);
  }

  uint8_t getActiveVFOGroup() { return radio->groups; }

  void nextVFO() {
    gSettings.activeCH = !gSettings.activeCH;
    radio = &gCH[gSettings.activeCH];
    gCurrentLoot = &gLoot[gSettings.activeCH];
    setupByCurrentCH();
    toggleRX(false);
    SETTINGS_Save();
  }

  void toggleVfoMR() {
    if (radio->vfo.channel >= 0) {
      radio->vfo.channel = -1;
    } else {
      nextCH(true);
    }
    saveCurrentCH();
  }

  void updateSquelchLevel(bool next) {
    uint8_t sq = radio->sq.level;
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

    if (radio->vfo.channel >= 0) {
      nextCH(next);
      return;
    }

    Band *nextBand = BAND_ByFrequency(radio->f + dir);
    if (nextBand != gCurrentBand && nextBand != &defaultBand) {
      if (next) {
        tuneTo(nextBand->band.bounds.start);
      } else {
        tuneTo(nextBand->band.bounds.end -
               nextBand->band.bounds.end %
                   StepFrequencyTable[nextBand->band.step]);
      }
    } else {
      tuneTo(radio->f + StepFrequencyTable[nextBand->band.step] * dir);
    }
    onVfoUpdate();
  }

  void nextBandFreq(bool next) {
    uint32_t steps = BANDS_GetSteps(gCurrentBand);
    uint32_t step = BANDS_GetChannel(gCurrentBand, radio->f);
    IncDec32(&step, 0, steps, next ? 1 : -1);
    radio->f = BANDS_GetF(gCurrentBand, step);
    tuneToPure(radio->f, true);
  }

  void nextBandFreqEx(bool next, bool precise) {
    uint32_t steps = BANDS_GetSteps(gCurrentBand);
    uint32_t step = BANDS_GetChannel(gCurrentBand, radio->f);
    IncDec32(&step, 0, steps, next ? 1 : -1);
    radio->f = BANDS_GetF(gCurrentBand, step);
    tuneToPure(radio->f, precise);
  }
};
