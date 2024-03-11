#pragma once

#include "../globals.hpp"
#include "../settings.hpp"
#include <stdint.h>

typedef struct VFO : CH {
  Step step;
  SquelchSettings sq;
  VFO() { type = CH_VFO; }
} __attribute__((packed)) VFO;

typedef struct MemoryCell {
  uint32_t number;
  CH ch;
} MemoryCell;

class EEPROMManager {
public:
  static void write(Settings *data) {
    // write settings
  }
  static void write(Scanlist *data) {
    // write channel
  }
  static void write(MemoryCell *data) {
    // write channel
    EEPROM_WriteBuffer(getChannelAddress(data), data, sizeof(CH));
  }
  EEPROMManager() = delete;

private:
  static uint32_t getChannelAddress(MemoryCell *data) {
    return getChOffset(data->number);
  }

  static int16_t getMaxChannels() {
    return (SETTINGS_GetEEPROMSize() - CHANNELS_END_OFFSET) / CH_SIZE;
  }

  static uint32_t getChOffset(int16_t num) {
    return SETTINGS_GetEEPROMSize() - (num + 1) * CH_SIZE;
  }

  static void load(int16_t num, CH *p) {
    if (num >= 0) {
      EEPROM_ReadBuffer(getChOffset(num), p, CH_SIZE);
    }
  }

  static void save(int16_t num, CH *p) {
    if (num >= 0) {
      EEPROM_WriteBuffer(getChOffset(num), p, CH_SIZE);
    }
  }

  static bool existing(int16_t i) {
    ChannelType type;
    EEPROM_ReadBuffer(getChOffset(i), &type, 1);
    return type != CH_EMPTY;
  }

  static ChannelType getType(int16_t i) {
    ChannelType type;
    EEPROM_ReadBuffer(getChOffset(i), &type, 1);
    return type;
  }

  static uint8_t scanlists(int16_t i) {
    uint8_t groups;
    uint32_t addr = getChOffset(i) + offsetof(CH, groups);
    EEPROM_ReadBuffer(addr, &groups, 1);
    return groups;
  }

  static int16_t next(int16_t base, bool next) {
    int16_t si = base;
    int16_t max = getMaxChannels();
    IncDecI16(&si, 0, max, next ? 1 : -1);
    int16_t i = si;
    if (next) {
      for (; i < max; ++i) {
        if (existing(i)) {
          return i;
        }
      }
      for (i = 0; i < base; ++i) {
        if (existing(i)) {
          return i;
        }
      }
    } else {
      for (; i >= 0; --i) {
        if (existing(i)) {
          return i;
        }
      }
      for (i = max - 1; i > base; --i) {
        if (existing(i)) {
          return i;
        }
      }
    }
    return -1;
  }

  static void remove(int16_t i) {
    CH v{.type = CH_EMPTY};
    save(i, &v);
  }

  static void loadScanlist(uint8_t n) {
    gSettings.currentScanlist = n;
    int32_t max = getMaxChannels();
    uint8_t scanlistMask = 1 << n;
    gScanlistSize = 0;
    for (int32_t i = 0; i < max; ++i) {
      if ((n == 15 && existing(i)) ||
          (scanlists(i) & scanlistMask) == scanlistMask) {
        gScanlist[gScanlistSize] = i;
        gScanlistSize++;
      }
    }
    SETTINGS_Save();
  }
};
