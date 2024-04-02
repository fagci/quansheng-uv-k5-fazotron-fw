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
  SvcDescription services[7] = {
      {"Keyboard", keyboardService.init, keyboardService.update,
       keyboardService.deinit, 0},
      {"Listen", SVC_LISTEN_Init, SVC_LISTEN_Update, SVC_LISTEN_Deinit, 50},
      {"Scan", SVC_SCAN_Init, SVC_SCAN_Update, SVC_SCAN_Deinit, 51},
      {"Bat save", SVC_BAT_SAVE_Init, SVC_BAT_SAVE_Update, SVC_BAT_SAVE_Deinit,
       52},
      {"Apps", SVC_APPS_Init, SVC_APPS_Update, SVC_APPS_Deinit, 100},
      {"Sys", SVC_SYS_Init, SVC_SYS_Update, SVC_SYS_Deinit, 150},
      {"Render", SVC_RENDER_Init, SVC_RENDER_Update, SVC_RENDER_Deinit, 255},
  };

  static SettingsService settings;

  static BacklightService backlight;
  static ListenService listen;
  static RenderService render;
  static KeyboardService keyboard;
  static ScanService scan;

  static void setupServices() { keyboard.setPrio(0); }

  void setPrio(uint8_t prio) { priority = prio; }

protected:
  uint8_t priority = 100;
};
