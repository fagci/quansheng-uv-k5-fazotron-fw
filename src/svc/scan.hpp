#pragma once

#include "../driver/uart.hpp"
#include "../radio.hpp"
#include "listening.hpp"
#include "svc.hpp"
#include "tune.hpp"

class ScanService {
  class CallbackFunctor {
  public:
    void operator()(bool v) {}
  };

public:
  void next() {
    lastListenState = false;
    gScanFn(gScanForward);
    lastSettedF = ServiceManager::board.radio->getF();
    SetTimeout(&timeout, radio->vfo.scan.timeout);
    if (gScanRedraw) {
      gRedrawScreen = true;
    }
  }

  void init() {
    gScanForward = true;
    Log("SCAN init, SF:%u", !!gScanFn);
    if (!gScanFn) {
      if (radio->vfo.channel >= 0) {
        gScanFn = ServiceManager::tuneService.nextCH;
      } else {
        gScanFn = tuneService->nextBandFreq;
      }
    }
    next();
  }

  void update() {
    if (lastListenState != listenService->isListening()) {
      lastListenState = listenService->isListening();
      SetTimeout(&timeout, lastListenState
                               ? SCAN_TIMEOUTS[radio->vfo.scan.openedTimeout]
                               : SCAN_TIMEOUTS[radio->vfo.scan.closedTimeout]);
    }

    if (CheckTimeout(&timeout)) {
      next();
      return;
    }

    if (lastSettedF != radio->getF()) {
      SetTimeout(&timeout, 0);
    }
  }

  void deinit() {
    gScanFn = NULL;
    gScanRedraw = true;
  }

private:
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

  static constexpr char *SCAN_TIMEOUT_NAMES[11] = {
      "0",   "500ms", "1s",   "2s",   "5s",   "10s",
      "30s", "1min",  "2min", "5min", "None",
  };

  uint32_t lastSettedF = 0;
  uint32_t timeout = 0;
  bool lastListenState = false;

  void (*gScanFn)(bool) = NULL;
};
