#pragma once

#include "../driver/st7565.hpp"
#include "svc.hpp"
#include <stdint.h>

class BacklightService : Svc {
public:
  void setDuration(uint8_t durationSec) {
    duration = durationSec;
    countdown = durationSec;
  }

  void update() {
    if (countdown == 0 || countdown == 255) {
      return;
    }

    if (countdown == 1) {
      toggle(false);
      countdown = 0;
    } else {
      countdown--;
    }
  }

  void toggle(bool on) {
    if (state != on) {
      state = on;
      Board::display.setBrightness(on ? brightness : 0);
    }
  }

  void setBrightness(uint8_t v) { brightness = v; }

  void on() {
    countdown = duration;
    toggle(countdown);
  }

  void toggleSquelch(bool on) {
    if (on) {
      if (Board::settings.backlightOnSquelch != BL_SQL_OFF) {
        Svc::backlight.on();
      }
    } else {
      if (Board::settings.backlightOnSquelch == BL_SQL_OPEN) {
        Svc::backlight.toggle(false);
      }
    }
  }

private:
  uint8_t duration = 2;
  uint8_t countdown;
  bool state = false;
  uint8_t brightness = 8;
};
