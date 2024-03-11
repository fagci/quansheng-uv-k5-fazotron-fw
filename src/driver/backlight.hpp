#pragma once

#include "../inc/dp32g030/portcon.h"
#include "../inc/dp32g030/pwmplus.h"
#include "system.hpp"
#include <stdint.h>

class Backlight {
public:
  void init() {
    // 48MHz / 94 / 1024 ~ 500Hz
    const uint32_t PWM_FREQUENCY_HZ = 1000;
    PWM_PLUS0_CLKSRC |= ((CPU_CLOCK_HZ / 1024 / PWM_FREQUENCY_HZ) << 16);
    PWM_PLUS0_PERIOD = 1023;

    PORTCON_PORTB_SEL0 &= ~(0
                            // Back light
                            | PORTCON_PORTB_SEL0_B6_MASK);
    PORTCON_PORTB_SEL0 |= 0
                          // Back light PWM
                          | PORTCON_PORTB_SEL0_B6_BITS_PWMP0_CH0;

    PWM_PLUS0_GEN =
        PWMPLUS_GEN_CH0_OE_BITS_ENABLE | PWMPLUS_GEN_CH0_OUTINV_BITS_ENABLE | 0;

    PWM_PLUS0_CFG = PWMPLUS_CFG_CNT_REP_BITS_ENABLE |
                    PWMPLUS_CFG_COUNTER_EN_BITS_ENABLE | 0;
  }

  void setBrightness(uint8_t brigtness) {
    _brightness = brigtness;
    PWM_PLUS0_CH0_COMP = (1 << brigtness) - 1;
  }

  void toggle(bool on) {
    if (state == on) {
      return;
    }
    state = on;
    if (on) {
      setBrightness(_brightness);
    } else {
      setBrightness(0);
    }
  }

  void on() {
    countdown = duration;
    toggle(countdown);
  }

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

private:
  uint8_t duration = 2;
  uint8_t countdown;
  bool state = false;
  uint8_t _brightness = 8;
};
