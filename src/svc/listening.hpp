#pragma once

#include "../radio.hpp"
#include "settings.hpp"

class ListenService {
public:
  ListenService(SettingsService *s, Radio *r) : gSettings(s), radio{r} {
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
      } else if (gSettings.noListen && (gCurrentApp->id == APP_SPECTRUM ||
                                        gCurrentApp->id == APP_ANALYZER)) {
        rx = false;
      } else if (gSettings.skipGarbageFrequencies &&
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
  Loot *msm;
  Radio *radio;
  uint32_t lastTailTone = 0;
  uint16_t rssi = 0;
  bool isListening = false;
  bool gMonitorMode = false;
};
