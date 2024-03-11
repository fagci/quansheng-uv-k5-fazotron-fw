/* Copyright 2023 Dual Tachyon
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

#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>

#define VHF_UHF_BOUND1 24000000
#define VHF_UHF_BOUND2 28000000

typedef enum {
  FILTER_VHF,
  FILTER_UHF,
  FILTER_OFF,
} Filter;

void BOARD_FLASH_Init();
void BOARD_GPIO_Init();
void BOARD_PORTCON_Init();
void BOARD_ADC_Init();
void BOARD_ADC_GetBatteryInfo(uint16_t *pVoltage, uint16_t *pCurrent);
void BOARD_Init();
void BOARD_EEPROM_Init();
void BOARD_EEPROM_LoadCalibration();
void BOARD_FactoryReset(bool bIsAll);

#endif
