#pragma once
#include "../driver/eeprom.hpp"
#include <stdint.h>

class SettingsService {
public:
  typedef enum {
    UPCONVERTER_OFF,
    UPCONVERTER_50M,
    UPCONVERTER_125M,
  } UpconverterTypes;

  EEPROM::Type eepromType : 3;
  uint8_t checkbyte : 5; // 1
  uint8_t scrambler : 4;
  uint8_t batsave : 4; // 2
  uint8_t vox : 4;
  uint8_t backlight : 4; // 3
  uint8_t txTime : 4;
  uint8_t micGain : 4; // 4
  uint8_t currentScanlist : 4;
  UpconverterTypes upconverter : 2;
  uint8_t roger : 2; // 5
  uint8_t scanmode : 2;
  uint8_t chDisplayMode : 2;
  uint8_t dw : 1;
  uint8_t crossBand : 1;
  uint8_t beep : 1;
  uint8_t keylock : 1; // 6
  uint8_t busyChannelTxLock : 1;
  uint8_t ste : 1;
  uint8_t repeaterSte : 1;
  uint8_t dtmfdecode : 1;
  uint8_t brightness : 4; // 7
  uint8_t mainApp : 8;    // 8

  int8_t bandsCount : 8; // 9
  int8_t activeBand : 8; // 10
  uint16_t batteryCalibration : 12;
  uint8_t contrast : 4; // 12
  BatteryType batteryType : 2;
  BatteryStyle batteryStyle : 2;
  bool bound_240_280 : 1;
  bool noListen : 1;
  BacklightOnSquelchMode backlightOnSquelch : 2; // 13
  uint8_t reserved2 : 5;
  bool skipGarbageFrequencies : 1;
  uint8_t activeCH : 2;
  char nickName[10];
  PowerCalibration powCalib[12];
  AllowTX allowTX;
  SettingsService(EEPROM *e) : eeprom{e} {}

  void save() {
    EEPROM_WriteBuffer(SETTINGS_OFFSET, &gSettings, SETTINGS_SIZE);
  }

  void load() { EEPROM_ReadBuffer(SETTINGS_OFFSET, &gSettings, SETTINGS_SIZE); }

  void delayedSave() {
    TaskRemove(SETTINGS_Save);
    TaskAdd("Settings save", SETTINGS_Save, 5000, false, 0);
  }

  uint32_t getFilterBound() {
    return gSettings.bound_240_280 ? VHF_UHF_BOUND2 : VHF_UHF_BOUND1;
  }

  uint32_t getEEPROMSize() { return EEPROM_SIZES[gSettings.eepromType]; }

  uint8_t getPageSize() { return EEPROM_PAGE_SIZES[gSettings.eepromType]; }

private:
  EEPROM *eeprom;
} __attribute__((packed));
