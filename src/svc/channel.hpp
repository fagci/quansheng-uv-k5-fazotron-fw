#pragma once

#include "driver/eeprom.hpp"
#include "settings.hpp"

namespace svc::channel {


CH currentCh;
uint32_t gScanlist[128] = {0};
uint16_t gScanlistSize = 0;

void load(int16_t num, CH *p) {
  if (num >= 0) {
    EEPROM::readBuffer(eeprom_layout::getChOffset(num), p, CH::size());
  }
}

void save(int16_t num, CH *p) {
  if (num >= 0) {
    EEPROM::writeBuffer(eeprom_layout::getChOffset(num), p, CH::size());
  }
}

bool existing(int16_t i) {
  ChannelType type;
  EEPROM::readBuffer(eeprom_layout::getChOffset(i), &type, 1);
  return type != CH_EMPTY;
}

ChannelType getType(int16_t i) {
  ChannelType type;
  EEPROM::readBuffer(eeprom_layout::getChOffset(i), &type, 1);
  return type;
}

uint8_t scanlists(int16_t i) {
  uint8_t groups;
  uint32_t addr = eeprom_layout::getChOffset(i) + offsetof(CH, groups);
  EEPROM::readBuffer(addr, &groups, 1);
  return groups;
}

int16_t next(int16_t base, bool next) {
  int16_t si = base;
  int16_t max = eeprom_layout::getMaxChannels();
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
  int32_t max = eeprom_layout::getMaxChannels();
  uint8_t scanlistMask = 1 << n;
  gScanlistSize = 0;
  for (int32_t i = 0; i < max; ++i) {
    if ((n == 15 && existing(i)) ||
        (scanlists(i) & scanlistMask) == scanlistMask) {
      gScanlist[gScanlistSize] = i;
      gScanlistSize++;
    }
  }
  svc::settings::save();
}

CH *current() { return &currentCh; }

void saveCurrent() { save(Board::settings.activeCH, current()); }

}; // namespace svc::channel
