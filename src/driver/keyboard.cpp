/* Copyright 2023 Manuel Jinger
 * Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#include "keyboard.hpp"
#include "../inc/dp32g030/gpio.h"
#include "../misc.hpp"
// #include "../scheduler.h"
#include "gpio.hpp"
#include "i2c.hpp"
#include "system.hpp"

KEY_Code_t gKeyReading0 = KEY_INVALID;
KEY_Code_t gKeyReading1 = KEY_INVALID;
uint16_t gDebounceCounter;

typedef const struct {
  // Using a 16 bit pre-calculated shift and invert is cheaper
  // than using 8 bit and doing shift and invert in code.
  uint16_t set_to_zero_mask;

  // We are very fortunate.
  // The key and pin defines fit together in a single u8,
  // making this very efficient
  struct {
    uint8_t key : 5; // Key 23 is highest
    uint8_t pin : 3; // Pin 6 is highest
  } pins[4];
} Keyboard;

Keyboard keyboard[5] = {
    /* Zero row  */
    {// Set to zero to handle special case of nothing pulled down.
     .set_to_zero_mask = 0xffff,
     .pins =
         {
             {.key = KEY_SIDE1, .pin = GPIOA_PIN_KEYBOARD_0},
             {.key = KEY_SIDE2, .pin = GPIOA_PIN_KEYBOARD_1},
             /* Duplicate to fill the array with valid values */
             {.key = KEY_SIDE2, .pin = GPIOA_PIN_KEYBOARD_1},
             {.key = KEY_SIDE2, .pin = GPIOA_PIN_KEYBOARD_1},
         }},
    /* First row  */
    {.set_to_zero_mask = ~(1 << GPIOA_PIN_KEYBOARD_4) & 0xffff,
     .pins =
         {
             {.key = KEY_MENU, .pin = GPIOA_PIN_KEYBOARD_0},
             {.key = KEY_1, .pin = GPIOA_PIN_KEYBOARD_1},
             {.key = KEY_4, .pin = GPIOA_PIN_KEYBOARD_2},
             {.key = KEY_7, .pin = GPIOA_PIN_KEYBOARD_3},
         }},
    /* Second row */
    {.set_to_zero_mask = ~(1 << GPIOA_PIN_KEYBOARD_5) & 0xffff,
     .pins =
         {
             {.key = KEY_UP, .pin = GPIOA_PIN_KEYBOARD_0},
             {.key = KEY_2, .pin = GPIOA_PIN_KEYBOARD_1},
             {.key = KEY_5, .pin = GPIOA_PIN_KEYBOARD_2},
             {.key = KEY_8, .pin = GPIOA_PIN_KEYBOARD_3},
         }},
    /* Third row */
    {.set_to_zero_mask = ~(1 << GPIOA_PIN_KEYBOARD_6) & 0xffff,
     .pins =
         {
             {.key = KEY_DOWN, .pin = GPIOA_PIN_KEYBOARD_0},
             {.key = KEY_3, .pin = GPIOA_PIN_KEYBOARD_1},
             {.key = KEY_6, .pin = GPIOA_PIN_KEYBOARD_2},
             {.key = KEY_9, .pin = GPIOA_PIN_KEYBOARD_3},
         }},
    /* Fourth row */
    {.set_to_zero_mask = ~(1 << GPIOA_PIN_KEYBOARD_7) & 0xffff,
     .pins =
         {
             {.key = KEY_EXIT, .pin = GPIOA_PIN_KEYBOARD_0},
             {.key = KEY_STAR, .pin = GPIOA_PIN_KEYBOARD_1},
             {.key = KEY_0, .pin = GPIOA_PIN_KEYBOARD_2},
             {.key = KEY_F, .pin = GPIOA_PIN_KEYBOARD_3},
         }},
};

KEY_Code_t KEYBOARD_Poll() {
  KEY_Code_t Key = KEY_INVALID;

  // *****************

  for (unsigned int j = 0; j < ARRAY_SIZE(keyboard); j++) {
    uint16_t reg;
    uint8_t i;
    uint8_t k;

    // Set all high
    GPIOA->DATA |= 1u << GPIOA_PIN_KEYBOARD_4 | 1u << GPIOA_PIN_KEYBOARD_5 |
                   1u << GPIOA_PIN_KEYBOARD_6 | 1u << GPIOA_PIN_KEYBOARD_7;

    // Clear the pin we are selecting
    GPIOA->DATA &= keyboard[j].set_to_zero_mask;

    // Read all 4 GPIO pins at once .. with de-noise, max of 8 sample loops
    for (i = 0, k = 0, reg = 0; i < 3 && k < 8; i++, k++) {
      uint16_t reg2;
      SYSTICK_DelayUs(1);
      reg2 = GPIOA->DATA;
      if (reg != reg2) { // noise
        reg = reg2;
        i = 0;
      }
    }
    if (i < 3)
      break; // noise is too bad

    for (i = 0; i < ARRAY_SIZE(keyboard[j].pins); i++) {
      const uint16_t mask = 1u << keyboard[j].pins[i].pin;
      if (!(reg & mask)) {
        Key = keyboard[j].pins[i].key;
        break;
      }
    }

    if (Key != KEY_INVALID)
      break;
  }

  // Create I2C stop condition since we might have toggled I2C pins
  // This leaves GPIOA_PIN_KEYBOARD_4 and GPIOA_PIN_KEYBOARD_5 high
  I2C_Stop();

  // Reset VOICE pins
  GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_6);
  GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_7);

  return Key;
}

bool gKeyBeingHeld;
bool gPttIsPressed;
bool gPttWasReleased;
uint8_t gPttDebounceCounter;
bool gRepeatHeld = false;

// static uint8_t gSerialConfigCountDown_500ms = 0;
static uint8_t KEY_DEBOUNCE = 4;
static uint8_t KEY_REPEAT_DELAY = 40;
static uint8_t KEY_REPEAT = 8;

void KEYBOARD_CheckKeys(void onKey(KEY_Code_t, bool, bool)) {

  // -------------------- PTT ------------------------

  if (gPttIsPressed) {
    // PTT released or serial comms config in progress
    if (GPIO_CheckBit(&GPIOC->DATA, GPIOC_PIN_PTT)) {
      if (++gPttDebounceCounter >= 3) {
        onKey(KEY_PTT, false, false);
        gPttIsPressed = false;
        if (gKeyReading1 != KEY_INVALID) {
          gPttWasReleased = true;
        }
      }
    } else {
      gPttDebounceCounter = 0;
    }
  } else if (!GPIO_CheckBit(&GPIOC->DATA, GPIOC_PIN_PTT)) {
    if (++gPttDebounceCounter >= 3) {
      gPttDebounceCounter = 0;
      gPttIsPressed = true;
      onKey(KEY_PTT, true, true);
    }
  } else {
    gPttDebounceCounter = 0;
  }

  // --------------------- OTHER KEYS ----------------------------

  KEY_Code_t Key = KEYBOARD_Poll();

  // new key pressed
  if (gKeyReading0 != Key) {
    // key pressed without releasing previous key
    if (gKeyReading0 != KEY_INVALID && Key != KEY_INVALID) {
      onKey(gKeyReading1, false, gKeyBeingHeld);
    }

    gKeyReading0 = Key;
    gDebounceCounter = 0;
    return;
  }

  gDebounceCounter++;

  // debounced new key pressed
  if (gDebounceCounter == KEY_DEBOUNCE) {
    // all non PTT keys released
    if (Key == KEY_INVALID) {
      // some button was pressed before
      if (gKeyReading1 != KEY_INVALID) {
        // process last button released event
        onKey(gKeyReading1, false, gKeyBeingHeld);
        gKeyReading1 = KEY_INVALID;
      }
    } else { // process new key pressed
      gKeyReading1 = Key;
      onKey(Key, true, false);
    }

    gKeyBeingHeld = false;
    gRepeatHeld = false;
    return;
  }

  // the button is not held long enough for repeat yet, or not really pressed
  if (gDebounceCounter < KEY_REPEAT_DELAY || Key == KEY_INVALID) {
    return;
  }

  // initial key repeat with longer delay
  if (gDebounceCounter == KEY_REPEAT_DELAY) {
    if (Key != KEY_PTT) {
      gKeyBeingHeld = true;
      gRepeatHeld = false;
      onKey(Key, true, true); // key held event
    }
  } else {
    // subsequent fast key repeats
    // fast key repeats for up/down buttons
    // if (Key == KEY_UP || Key == KEY_DOWN) {
    gKeyBeingHeld = true;
    gRepeatHeld = true;
    if ((gDebounceCounter % KEY_REPEAT) == 0)
      onKey(Key, true, true); // key held event
    // }

    if (gDebounceCounter < 0xFFFF)
      return;

    gDebounceCounter = KEY_REPEAT_DELAY + 1;
  }
}
