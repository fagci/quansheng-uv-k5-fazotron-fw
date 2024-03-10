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

#ifndef DRIVER_KEYBOARD_H
#define DRIVER_KEYBOARD_H

#include <stdint.h>

typedef enum {
  KEY_0 = 0,
  KEY_1 = 1,
  KEY_2 = 2,
  KEY_3 = 3,
  KEY_4 = 4,
  KEY_5 = 5,
  KEY_6 = 6,
  KEY_7 = 7,
  KEY_8 = 8,
  KEY_9 = 9,
  KEY_MENU = 10,
  KEY_UP = 11,
  KEY_DOWN = 12,
  KEY_EXIT = 13,
  KEY_STAR = 14,
  KEY_F = 15,
  KEY_PTT = 21,
  KEY_SIDE2 = 22,
  KEY_SIDE1 = 23,
  KEY_INVALID = 255,
} KEY_Code_t;

extern KEY_Code_t gKeyReading0;
extern KEY_Code_t gKeyReading1;
extern uint16_t gDebounceCounter;
extern bool gKeyBeingHeld;
extern bool gRepeatHeld;

KEY_Code_t KEYBOARD_Poll();
void KEYBOARD_CheckKeys(void onKey(KEY_Code_t, bool, bool));
void KEYBOARD_CheckKeys2(void onKey(KEY_Code_t, bool, bool));

#endif
