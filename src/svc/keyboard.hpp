#pragma once

#include "../apps/apps.hpp"
#include "../driver/audio.hpp"
#include "../driver/backlight.hpp"
#include "../driver/gpio.hpp"
#include "../driver/keyboard.hpp"
#include "../driver/st7565.hpp"
#include "../inc/dp32g030/gpio.h"
#include "../scheduler.hpp"
#include "../settings.hpp"
#include "backlight.hpp"
#include "svc.hpp"
#include <stdint.h>

class KeyboardService : public Svc {
public:
  // NOTE: Important!
  // If app runs app on keypress, keyup passed to next app
  // Common practice:
  //
  // keypress (up)
  // keyhold
  void onKey(KEY_Code_t key, bool pressed, bool hold) {
    if (key != KEY_INVALID) {
      backlightService->on();
      Scheduler::taskTouch(Svc::backlight.update);
    }

    // apps needed this events:
    // - keyup (!pressed)
    // - keyhold pressed (hold && pressed)
    // - keyup hold (hold && !pressed)
    if ((hold || !pressed) && APPS_key(key, pressed, hold)) {
      if (Board::settings.beep)
        AUDIO_PlayTone(1000, 20);
      render.schedule();
      return;
    }

    if (key == KEY_SIDE2 && !hold && !pressed) {
      GPIO_FlipBit(&GPIOC->DATA, GPIOC_PIN_FLASHLIGHT);
      return;
    }

    if (key != KEY_MENU) {
      return;
    }

    if (pressed) {
      if (hold) {
        APPS_run(APP_SETTINGS);
        return;
      }
    } else {
      if (!hold) {
        APPS_run(APP_APPSLIST);
        return;
      }
    }
  }

  void init() {}
  void update() { KEYBOARD_CheckKeys(onKey); }
  void deinit() {}

private:
  BacklightService *backlightService;
};
