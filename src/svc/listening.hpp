#pragma once

#include "../board.hpp"
#include "../frequency.hpp"
#include "backlight.hpp"
#include "svc/apps.hpp"

namespace svc::listen {
namespace {
TXState gTxState;

Loot *msm;
uint32_t lastTailTone = 0;
uint16_t rssi = 0;
bool _isListening = false;
bool _monitorMode = false;
} // namespace

bool isSqOpenSimple(uint16_t r) {
  uint8_t band = Board::radio.vfo.f > Board::settings.getFilterBound() ? 1 : 0;
  uint8_t sq = Board::radio.vfo.sq.level;
  uint16_t ro = SQ[band][0][sq];
  uint16_t rc = SQ[band][1][sq];

  bool open = r >= ro;
  if (r < rc) {
    open = false;
  }

  return open;
}

bool isSquelchOpen() {
  if (Board::radio.vfo.sq.openTime || Board::radio.vfo.sq.closeTime) {
    return isSqOpenSimple(rssi);
  }
  return Board::radio.isSquelchOpen();
}

TXState getTXState(uint32_t txF) {
  if (Board::settings.upconverter) {
    return TX_DISABLED_UPCONVERTER;
  }

  if (Board::settings.allowTX == TX_DISALLOW) {
    return TX_DISABLED;
  }

  if (Board::settings.allowTX == TX_ALLOW_LPD_PMR && Band::LPD.contains(txF) &&
      Band::PMR.contains(txF)) {
    return TX_DISABLED;
  }

  if (Board::settings.allowTX == TX_ALLOW_LPD_PMR_SATCOM &&
      !Band::LPD.contains(txF) && !Band::PMR.contains(txF) &&
      !Band::SATCOM.contains(txF)) {
    return TX_DISABLED;
  }

  if (Board::settings.allowTX == TX_ALLOW_HAM && !Band::HAM2M.contains(txF) &&
      !Band::HAM70CM.contains(txF)) {
    return TX_DISABLED;
  }

  if (Board::battery.percentage() == 0) {
    return TX_BAT_LOW;
  }

  if (Board::battery.isCharging() || Board::battery.voltage() > 880) {
    return TX_VOL_HIGH;
  }

  return TX_ON;
}

void toggleRX(bool on) {
  svc::backlight::toggleSquelch(on);
  Board::radio.toggleRX(on);
}

void toggleTX(bool on) {
  if (gTxState == on) {
    return;
  }
  uint8_t power = 0;
  uint32_t txF = Board::radio.getTXFEx(&Board::radio.vfo);

  if (on) {
    gTxState = getTXState(txF);
    power = Board::radio.calculateOutputPower(txF);
    if (power > 0x91) {
      power = 0;
      gTxState = TX_POW_OVERDRIVE;
      return;
    }
    power >>= 2 - Board::radio.vfo.power;
  }
}

void intrHandler(uint16_t intBits) {
  // MSG_StorePacket(intBits);

  if (intBits & BK4819_REG_02_CxCSS_TAIL) {
    _isListening = false;
    lastTailTone = Now();
  }
}

void deinit() { Board::radio.idle(); }
void toggleMonitorMode() { _monitorMode = !_monitorMode; }
bool isListening() { return _isListening; }

void update() {
  rssi = Board::radio.getRSSI();
  Board::radio.handleInterrupts(intrHandler);

  // else sql reopens
  if ((Now() - lastTailTone) < 250) {
    _isListening = false;
  }

  bool rx = msm->open;
  if (Board::radio.getTxState() != TX_ON) {
    if (_monitorMode) {
      rx = true;
    } else if (Board::settings.noListen &&
               (svc::apps::currentAppId() == APP_SPECTRUM ||
                svc::apps::currentAppId() == APP_ANALYZER)) {
      rx = false;
    } else if (Board::settings.skipGarbageFrequencies &&
               (Board::radio.vfo.f % 1300000 == 0)) {
      rx = false;
    }
    Board::radio.toggleRX(rx);
  }

  if (Board::radio.vfo.scan.timeout < 10) {
    Board::radio.resetRSSI();
  }
}
void stop() { Scheduler::taskRemove(update); }
void start(uint32_t interval = 10) {
  stop();
  Scheduler::taskAdd("Svc", update, interval, true, 50);
}
}; // namespace svc::listen
