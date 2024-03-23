#pragma once

#include "../inc/dp32g030/gpio.h"
#include "bk4819.hpp"
#include "gpio.hpp"
#include "system.hpp"
#include <stdint.h>

class Audio {
public:
  bool speakerOn = false;

  void toggleSpeaker(bool on) {
    speakerOn = on;
    if (on) {
      GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
    } else {
      GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
    }
  }

  void playTone(uint32_t frequency, uint16_t duration) {
    bool isSpeakerWasOn = speakerOn;
    uint16_t ToneConfig = BK4819_ReadRegister(BK4819_REG_71);

    toggleSpeaker(false);
    // BK4819_RX_TurnOn();

    delayMs(20);
    BK4819_PlayTone(frequency, true);
    delayMs(2);

    toggleSpeaker(true);
    delayMs(60);

    BK4819_ExitTxMute();
    delayMs(duration);
    BK4819_EnterTxMute();

    delayMs(20);
    toggleSpeaker(false);

    delayMs(5);
    BK4819_TurnsOffTones_TurnsOnRX();
    delayMs(5);

    BK4819_WriteRegister(BK4819_REG_71, ToneConfig);
    toggleSpeaker(isSpeakerWasOn);
  }
};
