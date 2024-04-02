#pragma once

#include "../driver/eeprom.hpp"
#include "../lib/struct/settings.hpp"
#include "../scheduler.hpp"
#include <stdint.h>

class SettingsService {
public:
  SettingsService(EEPROM *e) : eeprom{e} {}

  void save() { eeprom->writeBuffer(0, this, sizeof(*this)); }
  void load() { eeprom->readBuffer(0, this, sizeof(*this)); }

  void delayedSave() {
    scheduler->taskRemove(save);
    scheduler->taskAdd("Settings save", save, 5000, false, 0);
  }

  uint32_t getFilterBound() {
    return settings.bound_240_280 ? 28000000 : 24000000;
  }

  uint32_t getEEPROMSize() { return EEPROM::SIZES[settings.eepromType]; }

  uint8_t getPageSize() { return EEPROM::PAGE_SIZES[settings.eepromType]; }

private:
  Settings settings;
  EEPROM *eeprom;
  Scheduler *scheduler;
} __attribute__((packed));
