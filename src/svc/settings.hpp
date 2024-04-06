#pragma once

#include "../board.hpp"
#include "../driver/eeprom.hpp"
#include "../globals.hpp"
#include "../scheduler.hpp"
#include "../settings.hpp"
#include "../svc/svc.hpp"
#include "driver/abstractradio.hpp"
#include "driver/eeprom.hpp"
#include "svc/settings.hpp"
#include <stdint.h>

struct CH {
  ChannelType type;
  uint16_t groups;
  VFO vfo;
  static constexpr size_t size() { return sizeof(CH); };
};

typedef struct MemoryCell {
  uint32_t number;
  CH ch;
} MemoryCell;

class EEPROMService {
public:
  static void write(Settings *data) {
    // write settings
  }
  static void write(Scanlist *data) {
    // write channel
  }
  static void write(MemoryCell *data) {
    // write channel
    EEPROM::writeBuffer(getChannelAddress(data), data, CH::size());
  }

  void saveSettings() { EEPROM::writeBuffer(0, this, sizeof(*this)); }
  void loadSettings() { EEPROM::readBuffer(0, this, sizeof(*this)); }

  void delayedSave() {
    Scheduler::taskRemove(save);
    Scheduler::taskAdd("Settings save", save, 5000, false, 0);
  }

private:
  static uint32_t getChannelAddress(MemoryCell *data) {
    return getChOffset(data->number);
  }

  static int16_t getMaxChannels() {
    return (EEPROM::getSize() - channelsEndOffset()) / CH::size();
  }

  static int16_t channelsEndOffset() {
    return Settings::size() + Scanlist::size() * 16;
  }

  static uint32_t getChOffset(int16_t num) {
    return EEPROM::getSize() - (num + 1) * CH::size();
  }

  void load(int16_t num, CH *p) {
    if (num >= 0) {
      EEPROM::readBuffer(getChOffset(num), p, CH::size());
    }
  }

  void save(int16_t num, CH *p) {
    if (num >= 0) {
      EEPROM::writeBuffer(getChOffset(num), p, CH::size());
    }
  }

  bool existing(int16_t i) {
    ChannelType type;
    EEPROM::readBuffer(getChOffset(i), &type, 1);
    return type != CH_EMPTY;
  }

  ChannelType getType(int16_t i) {
    ChannelType type;
    EEPROM::readBuffer(getChOffset(i), &type, 1);
    return type;
  }

  uint8_t scanlists(int16_t i) {
    uint8_t groups;
    uint32_t addr = getChOffset(i) + offsetof(CH, groups);
    EEPROM::readBuffer(addr, &groups, 1);
    return groups;
  }

  int16_t next(int16_t base, bool next) {
    int16_t si = base;
    int16_t max = getMaxChannels();
    si = IncDec(si, 0, max, next ? 1 : -1);
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

  void remove(int16_t i) {
    CH v{.type = CH_EMPTY};
    save(i, &v);
  }

  void loadScanlist(uint8_t n) {
    Board::settings.currentScanlist = n;
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
    Svc::settings.save();
  }
};
