#pragma once

#include "../driver/uart.hpp"
#include "../radio.hpp"
#include "board.hpp"
#include "driver/abstractradio.hpp"
#include "listening.hpp"
#include "render.hpp"
#include "tune.hpp"

namespace svc::scan {

namespace {
uint16_t gScanSwitchT = 10;
bool gScanForward = true;
bool gScanRedraw = true;

uint32_t SCAN_TIMEOUTS[11] = {
    0,
    500,
    1000 * 1,
    1000 * 2,
    1000 * 5,
    1000 * 10,
    1000 * 30,
    1000 * 60,
    1000 * 60 * 2,
    1000 * 60 * 5,
    ((uint32_t)0) - 1,
};

static constexpr const char *SCAN_TIMEOUT_NAMES[11] = {
    "0",   "500ms", "1s",   "2s",   "5s",   "10s",
    "30s", "1min",  "2min", "5min", "None",
};

uint32_t lastSettedF = 0;
uint32_t timeout = 0;
bool lastListenState = false;

void (*gScanFn)(bool) = NULL;
} // namespace

void next() {
  lastListenState = false;
  gScanFn(gScanForward);
  lastSettedF = Board::radio.vfo()->f;
  SetTimeout(&timeout, Board::radio.vfo()->scan.timeout);
  if (gScanRedraw) {
    svc::render::schedule();
  }
}

void init() {
  gScanForward = true;
  Log("SCAN init, SF:%u", !!gScanFn);
  if (!gScanFn) {
    if (Board::radio.vfo()->channel >= 0) {
      gScanFn = svc::tune::nextCH;
    } else {
      gScanFn = svc::tune::nextBandFreq;
    }
  }
  next();
}

void update() {
  ScanSettings *scan = &Board::radio.vfo()->scan;
  if (lastListenState != svc::listen::isListening()) {
    lastListenState = svc::listen::isListening();
    SetTimeout(&timeout, lastListenState ? SCAN_TIMEOUTS[scan->openedTimeout]
                                         : SCAN_TIMEOUTS[scan->closedTimeout]);
  }

  if (CheckTimeout(&timeout)) {
    next();
    return;
  }

  if (lastSettedF != Board::radio.getF()) {
    SetTimeout(&timeout, 0);
  }
}

void deinit() {
  gScanFn = NULL;
  gScanRedraw = true;
}

void stop() { Scheduler::taskRemove(update); }
void start(uint32_t interval) {
  stop();
  Scheduler::taskAdd("Svc", update, interval, true, 51);
}
}; // namespace svc::scan
