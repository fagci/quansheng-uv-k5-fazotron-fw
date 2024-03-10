#include "backlight.hpp"
#include "../inc/dp32g030/portcon.h"
#include "../inc/dp32g030/pwmplus.h"
#include "system.hpp"

static uint8_t duration = 2;
static uint8_t countdown;
static bool state = false;
static uint8_t _brightness = 8;

void BACKLIGHT_Init() {
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

  PWM_PLUS0_CFG =
      PWMPLUS_CFG_CNT_REP_BITS_ENABLE | PWMPLUS_CFG_COUNTER_EN_BITS_ENABLE | 0;
}

void BACKLIGHT_SetBrightness(uint8_t brigtness) {
  _brightness = brigtness;
  PWM_PLUS0_CH0_COMP = (1 << brigtness) - 1;
}

void BACKLIGHT_Toggle(bool on) {
  if (state == on) {
    return;
  }
  state = on;
  if (on) {
    BACKLIGHT_SetBrightness(_brightness);
  } else {
    BACKLIGHT_SetBrightness(0);
  }
}

void BACKLIGHT_On() {
  countdown = duration;
  BACKLIGHT_Toggle(countdown);
}

void BACKLIGHT_SetDuration(uint8_t durationSec) {
  duration = durationSec;
  countdown = durationSec;
}

void BACKLIGHT_Update() {
  if (countdown == 0 || countdown == 255) {
    return;
  }

  if (countdown == 1) {
    BACKLIGHT_Toggle(false);
    countdown = 0;
  } else {
    countdown--;
  }
}
