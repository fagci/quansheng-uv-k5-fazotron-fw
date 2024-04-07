#pragma once

#include "../apps/apps.hpp"
#include "../driver/gpio.hpp"
#include "../driver/keyboard.hpp"
#include "../inc/dp32g030/gpio.h"
#include "../scheduler.hpp"
#include "backlight.hpp"
#include "render.hpp"

namespace svc::keyboard {
// NOTE: Important!
// If app runs app on keypress, keyup passed to next app
// Common practice:
//
// keypress (up)
// keyhold
void onKey(KEY_Code_t key, bool pressed, bool hold) {
  if (key != KEY_INVALID) {
    svc::backlight::on();
    Scheduler::taskTouch(svc::backlight::update);
  }

  // apps needed this events:
  // - keyup (!pressed)
  // - keyhold pressed (hold && pressed)
  // - keyup hold (hold && !pressed)
  if ((hold || !pressed) && APPS_key(key, pressed, hold)) {
    svc::render::schedule();
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

void update() { KEYBOARD_CheckKeys(onKey); }
void stop() { Scheduler::taskRemove(update); }
void start(uint32_t interval = 10) {
  stop();
  Scheduler::taskAdd("Svc", update, interval, true, 0);
}
} // namespace svc::keyboard
