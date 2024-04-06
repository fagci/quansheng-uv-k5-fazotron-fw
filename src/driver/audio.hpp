#pragma once

#include "../inc/dp32g030/gpio.h"
#include "gpio.hpp"

class Audio {
public:
  static void toggleSpeaker(bool on) {
    if (on) {
      GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
    } else {
      GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
    }
  }

  static bool isSpeakerOn() {
    return GPIO_CheckBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
  }
};
