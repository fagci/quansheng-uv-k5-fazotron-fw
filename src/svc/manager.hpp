#pragma once
#include "apps.hpp"
#include "backlight.hpp"
#include "batsave.hpp"
#include "keyboard.hpp"
#include "listening.hpp"
#include "render.hpp"
#include "scan.hpp"
#include "settings.hpp"
#include "svc/tune.hpp"
#include "sys.hpp"

class S {
public:
  static EEPROMService settings;

  static KeyboardService keyboard;
  static BacklightService backlight;
  static RenderService render;
  static TuneService tune;

  static ListenService listen;
  static ScanService scan;
  static SystemService sys;
  static BatSaveService batSave;

  static AppsService apps;

  static void setupServices() {
    keyboard.setPrio(0);
    listen.setPrio(50);
    scan.setPrio(51);
    batSave.setPrio(52);
    apps.setPrio(100);
    sys.setPrio(150);
    render.setPrio(255);
  }
};
