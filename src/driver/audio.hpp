#pragma once

#include "../board.hpp"
#include "../inc/dp32g030/gpio.h"
#include "gpio.hpp"
#include <stdint.h>

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

  void playTone(uint32_t frequency, uint16_t duration) {
    bool isSpeakerWasOn = isSpeakerOn();
    uint16_t ToneConfig = Board::radio.readRegister(BK4819_REG_71);

    toggleSpeaker(false);
    // BK4819_RX_TurnOn();

    delayMs(20);
    Board::radio.playTone(frequency, true);
    delayMs(2);

    toggleSpeaker(true);
    delayMs(60);

    Board::radio.exitTxMute();
    delayMs(duration);
    Board::radio.enterTxMute();

    delayMs(20);
    toggleSpeaker(false);

    delayMs(5);
    Board::radio.turnsOffTones_TurnsOnRX();
    delayMs(5);

    Board::radio.writeRegister(BK4819_REG_71, ToneConfig);
    toggleSpeaker(isSpeakerWasOn);
  }
};
