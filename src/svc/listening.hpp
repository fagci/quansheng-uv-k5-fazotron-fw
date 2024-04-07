#pragma once

#include "../board.hpp"
#include "../frequency.hpp"
#include "backlight.hpp"
#include "driver/abstractradio.hpp"
#include "svc/apps.hpp"

namespace svc::listen {

struct Measurement {
  uint16_t r;
  uint8_t n;
  uint8_t g;
};

namespace {
TXState gTxState;

Measurement *msm;
uint32_t lastTailTone = 0;
uint16_t rssi = 0;
bool _isListening = false;
bool _monitorMode = false;
} // namespace

bool isSqOpenSimple(uint16_t r) {
  const VFO *vfo = Board::radio.vfo();
  uint8_t band = vfo->f > Board::settings.getFilterBound() ? 1 : 0;
  uint8_t sq = vfo->sq.level;
  uint16_t ro = SQ[band][0][sq];
  uint16_t rc = SQ[band][1][sq];

  bool open = r >= ro;
  if (r < rc) {
    open = false;
  }

  return open;
}

bool isSquelchOpen() {
  const SquelchSettings *sq = &Board::radio.vfo()->sq;
  if (sq->openTime || sq->closeTime) {
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
  uint32_t txF = Board::radio.getTXFEx(Board::radio.vfo());

  if (on) {
    gTxState = getTXState(txF);
    power = Band::calculateOutputPower(txF, Board::settings.powCalib);
    if (power > 0x91) {
      power = 0;
      gTxState = TX_POW_OVERDRIVE;
      return;
    }
    power >>= 2 - Board::radio.vfo()->power;
  }
}

void intrHandler(uint16_t intBits) {
  // MSG_StorePacket(intBits);

  if (intBits & BK4819_REG_02_CxCSS_TAIL) {
    _isListening = false;
    lastTailTone = Now();
  }
}

void deinit() { Board::radio.idle(true); }
void toggleMonitorMode() { _monitorMode = !_monitorMode; }
bool isListening() { return _isListening; }

void update() {
  rssi = Board::radio.getRSSI();
  Board::radio.handleInterrupts(intrHandler);

  // else sql reopens
  if ((Now() - lastTailTone) < 250) {
    _isListening = false;
  }

  if (Board::radio.getTxState() != TX_ON) {
    if (_monitorMode) {
      _isListening = true;
    } else if (Board::settings.noListen &&
               (svc::apps::currentAppId() == APP_SPECTRUM ||
                svc::apps::currentAppId() == APP_ANALYZER)) {
      _isListening = false;
    } else if (Board::settings.skipGarbageFrequencies &&
               (Board::radio.vfo()->f % 1300000 == 0)) {
      _isListening = false;
    }
    Board::radio.toggleRX(_isListening);
  }

  if (Board::radio.vfo()->scan.timeout < 10) {
    Board::radio.resetRSSI();
  }
}
void stop() { Scheduler::taskRemove(update); }
void start(uint32_t interval = 10) {
  stop();
  Scheduler::taskAdd("Svc", update, interval, true, 50);
}
}; // namespace svc::listen
