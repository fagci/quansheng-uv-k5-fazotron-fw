#pragma once

#include "../driver/st7565.hpp"
#include "board.hpp"
#include "scheduler.hpp"
#include <stdint.h>

namespace svc::backlight {

namespace {
uint8_t duration = 2;
uint8_t countdown;
bool state = false;
uint8_t brightness = 8;
} // namespace

void setDuration(uint8_t durationSec) {
  duration = durationSec;
  countdown = durationSec;
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
      backlight::on();
    }
  } else {
    if (Board::settings.backlightOnSquelch == BL_SQL_OPEN) {
      backlight::toggle(false);
    }
  }
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

void stop() { Scheduler::taskRemove(update); }
void start(uint32_t interval = 1000) {
  stop();
  Scheduler::taskAdd("Svc", update, interval, true, 150);
}

} // namespace svc::backlight
