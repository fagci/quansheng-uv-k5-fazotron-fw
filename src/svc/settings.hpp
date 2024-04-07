#pragma once

#include "../board.hpp"
#include "../driver/eeprom.hpp"
#include "../globals.hpp"
#include "../scheduler.hpp"
#include "../settings.hpp"
#include "driver/abstractradio.hpp"
#include "driver/eeprom.hpp"
#include <stdint.h>

struct CH {
  ChannelType type;
  char *name[10];
  uint16_t groups;
  VFO vfo;
  static constexpr size_t size() { return sizeof(CH); };
} __attribute__((packed));

typedef struct MemoryCell {
  uint32_t number;
  CH ch;
} MemoryCell;

namespace eeprom_layout {
static int16_t channelsEndOffset() {
  return Settings::size() + Scanlist::size() * 16;
}

static uint32_t getChOffset(int16_t num) {
  return EEPROM::getSize() - (num + 1) * CH::size();
}

static uint32_t getChannelAddress(MemoryCell *data) {
  return getChOffset(data->number);
}

static int16_t getMaxChannels() {
  return (EEPROM::getSize() - channelsEndOffset()) / CH::size();
}
} // namespace eeprom_layout

namespace svc::settings {
static void write(Settings *data) {
  // write settings
}
static void write(Scanlist *data) {
  // write channel
}
static void write(MemoryCell *data) {
  // write channel
  EEPROM::writeBuffer(eeprom_layout::getChannelAddress(data), data, CH::size());
}

void save() { EEPROM::writeBuffer(0, &Board::settings, Settings::size()); }
void load() { EEPROM::readBuffer(0, &Board::settings, Settings::size()); }

void delayedSave() {
  Scheduler::taskRemove(save);
  Scheduler::taskAdd("Settings save", save, 5000, false, 0);
}
} // namespace svc::settings
