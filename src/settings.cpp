#include "settings.h"
#include "driver/eeprom.h"
#include "scheduler.h"

Settings gSettings;

const uint8_t BL_TIME_VALUES[7] = {0, 5, 10, 20, 60, 120, 255};
