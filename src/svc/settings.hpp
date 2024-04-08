#pragma once

#include "../board.hpp"
#include "../driver/eeprom.hpp"
#include "../eeprom_layout.hpp"
#include "../scheduler.hpp"
#include "../settings.hpp"
#include "driver/abstractradio.hpp"
#include "driver/eeprom.hpp"
#include <stdint.h>

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
