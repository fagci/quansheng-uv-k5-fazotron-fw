#pragma once

#include "../driver/eeprom.hpp"
#include "../scheduler.hpp"
#include <stdint.h>

class SettingsService {
public:
  void save() { EEPROM::writeBuffer(0, this, sizeof(*this)); }
  void load() { EEPROM::readBuffer(0, this, sizeof(*this)); }

  void delayedSave() {
    Scheduler::taskRemove(save);
    Scheduler::taskAdd("Settings save", save, 5000, false, 0);
  }

private:
} __attribute__((packed));
