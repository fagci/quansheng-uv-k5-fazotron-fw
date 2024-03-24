#pragma once

#include "../board.hpp"
#include "../radio.hpp"
#include "settings.hpp"

class ListenService {
public:
  ListenService(SettingsService *s, Board *b) : gSettings(s), board{b} {
    radio = board->radio;
    radio->squelch(radio->vfo.sq.level);
    radio->rxEnable();
  }

  void update() {
    rssi = radio->getRSSI();
    radio->handleInterrupts(intrHandler);

    // else sql reopens
    if ((Now() - lastTailTone) < 250) {
      isListening = false;
    }

    bool rx = msm->open;
    if (radio->getTxState() != TX_ON) {
      if (gMonitorMode) {
        rx = true;
      } else if (gSettings->noListen && (gCurrentApp->id == APP_SPECTRUM ||
                                         gCurrentApp->id == APP_ANALYZER)) {
        rx = false;
      } else if (gSettings->skipGarbageFrequencies &&
                 (radio->vfo.f % 1300000 == 0)) {
        rx = false;
      }
      radio->toggleRX(rx);
    }

    if (radio->vfo.scan.timeout < 10) {
      radio->resetRSSI();
    }
  }

  bool isSquelchOpen() {
    if (radio->vfo.sq.openTime || radio->vfo.sq.closeTime) {
      return isSqOpenSimple(rssi);
    }
    return radio->isSquelchOpen();
  }

  bool isSqOpenSimple(uint16_t r) {
    uint8_t band = radio->vfo.f > SETTINGS_GetFilterBound() ? 1 : 0;
    uint8_t sq = radio->vfo.sq.level;
    uint16_t ro = SQ[band][0][sq];
    uint16_t rc = SQ[band][1][sq];

    bool open = r >= ro;
    if (r < rc) {
      open = false;
    }

    return open;
  }

  TXState getTXState(uint32_t txF) {
    if (gSettings->upconverter) {
      return TX_DISABLED_UPCONVERTER;
    }

    if (gSettings->allowTX == TX_DISALLOW) {
      return TX_DISABLED;
    }

    if (gSettings->allowTX == TX_ALLOW_LPD_PMR &&
        !FreqInRange(txF, &BAND_LPD) && !FreqInRange(txF, &BAND_PMR)) {
      return TX_DISABLED;
    }

    if (gSettings->allowTX == TX_ALLOW_LPD_PMR_SATCOM &&
        !FreqInRange(txF, &BAND_LPD) && !FreqInRange(txF, &BAND_PMR) &&
        !FreqInRange(txF, &BAND_SATCOM)) {
      return TX_DISABLED;
    }

    if (gSettings->allowTX == TX_ALLOW_HAM && !FreqInRange(txF, &BAND_HAM2M) &&
        !FreqInRange(txF, &BAND_HAM70CM)) {
      return TX_DISABLED;
    }

    if (board->battery.percentage() == 0) {
      return TX_BAT_LOW;
    }
    if (gChargingWithTypeC || gBatteryVoltage > 880) {
      return TX_VOL_HIGH;
    }

    return TX_ON;
  }

  void toggleTX(bool on) {
    uint8_t power = 0;
    uint32_t txF = radio->getTXFEx(&radio->vfo);

    if (gTxState == on) {
      return;
    }

    if (on) {
      gTxState = getTXState(txF);
      power = radio->calculateOutputPower(txF);
      if (power > 0x91) {
        power = 0;
        gTxState = TX_POW_OVERDRIVE;
        return;
      }
      power >>= 2 - radio->vfo.power;
    }
  }

  void intrHandler(uint16_t intBits) {
    // MSG_StorePacket(intBits);

    if (intBits & BK4819_REG_02_CxCSS_TAIL) {
      isListening = false;
      lastTailTone = Now();
    }
  }

  Loot *updateMeasurements() { return msm; }

  void deinit() { radio->idle(); }
  void toggleMonitorMode() { gMonitorMode = !gMonitorMode; }

private:
  SettingsService *gSettings;
  Board *board;
  Radio *radio;

  TXState gTxState;

  Loot *msm;
  uint32_t lastTailTone = 0;
  uint16_t rssi = 0;
  bool isListening = false;
  bool gMonitorMode = false;
};
