#ifndef SETTINGS_H
#define SETTINGS_H

#include "driver/eeprom.hpp"
#include "globals.hpp"
#include <stdint.h>

// getsize(Settings)

#define SETTINGS_SIZE sizeof(Settings)
#define BAND_SIZE sizeof(CH)
#define SCANLIST_SIZE 10
#define CH_SIZE sizeof(CH)

#define SETTINGS_OFFSET (0)
#define SCANLISTS_OFFSET (SETTINGS_OFFSET + SETTINGS_SIZE)
#define CHANNELS_END_OFFSET (SCANLISTS_OFFSET + SCANLIST_SIZE * 8)

// settings
// CHs
// bands
// ...
// channel 2
// channel 1


void SETTINGS_Save();
void SETTINGS_Load();
void SETTINGS_DelayedSave();
uint32_t SETTINGS_GetFilterBound();
uint32_t SETTINGS_GetEEPROMSize();
uint8_t SETTINGS_GetPageSize();

#endif /* end of include guard: SETTINGS_H */
