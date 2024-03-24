#pragma once

#include "../inc/dp32g030/gpio.h"
#include "bk4819.hpp"
#include "gpio.hpp"
#include "system.hpp"
#include <stdint.h>

class Audio {
public:
  Audio(BK4819 *bk) : bk4819(bk) {}

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
    uint16_t ToneConfig = bk4819->readRegister(BK4819_REG_71);

    toggleSpeaker(false);
    // BK4819_RX_TurnOn();

    delayMs(20);
    bk4819->playTone(frequency, true);
    delayMs(2);

    toggleSpeaker(true);
    delayMs(60);

    bk4819->exitTxMute();
    delayMs(duration);
    bk4819->enterTxMute();

    delayMs(20);
    toggleSpeaker(false);

    delayMs(5);
    bk4819->turnsOffTones_TurnsOnRX();
    delayMs(5);

    bk4819->writeRegister(BK4819_REG_71, ToneConfig);
    toggleSpeaker(isSpeakerWasOn);
  }

private:
  bool speakerOn = false;
  BK4819 *bk4819;
};
