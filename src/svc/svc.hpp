#pragma once

#include "../scheduler.hpp"
#include "backlight.hpp"
#include "bat_save.hpp"
#include "driver/uart.hpp"
#include "keyboard.hpp"
#include "listening.hpp"
#include "render.hpp"
#include "scan.hpp"
#include "settings.hpp"
#include <stdint.h>

class Svc {
public:
  static SettingsService settings;

  static BacklightService backlight;
  static ListenService listen;
  static RenderService render;
  static KeyboardService keyboard;
  static ScanService scan;

  static void setupServices() {
    keyboard.setPrio(0);
    listen.setPrio(50);
    scan.setPrio(51);
    // batSave.setPrio(52);
    // apps.setPrio(100);
    // sys.setPrio(150);
    render.setPrio(255);
  }

  virtual void update(void) = 0;

  void setPrio(uint8_t prio) { priority = prio; }

  void start(uint32_t interval) {
    stop();
    Scheduler::taskAdd("Svc", update, interval, true, priority);
  }

  void stop() { Scheduler::taskRemove(update); }

private:
  uint8_t priority = 100;
};
