#ifndef GLOBALS_H
#define GLOBALS_H

#include "driver/bk4819.hpp"
#include "driver/keyboard.hpp"
#include <stdint.h>

#define getsize(V) char (*__ #V)()[sizeof(V)] = 1;

extern const uint16_t StepFrequencyTable[12];
extern const char *modulationTypeOptions[6];
extern const SquelchType sqTypeValues[4];

typedef enum {
  APP_NONE,
  APP_TEST,
  APP_SPECTRUM,
  APP_ANALYZER,
  APP_CH_SCANNER,
  APP_FASTSCAN,
  APP_STILL,
  APP_FINPUT,
  APP_APPSLIST,
  APP_LOOT_LIST,
  APP_BANDS_LIST,
  APP_RESET,
  APP_TEXTINPUT,
  APP_CH_CFG,
  APP_BAND_CFG,
  APP_SCANLISTS,
  APP_SAVECH,
  APP_SETTINGS,
  APP_MULTIVFO,
  APP_ABOUT,
  APP_ANT,
  APP_TASKMAN,
  APP_MESSENGER,
} AppType_t;

typedef enum {
  STEP_0_01kHz,
  STEP_0_1kHz,
  STEP_1_0kHz,
  STEP_2_5kHz,
  STEP_5_0kHz,
  STEP_6_25kHz,
  STEP_8_33kHz,
  STEP_9kHz,
  STEP_10_0kHz,
  STEP_12_5kHz,
  STEP_25_0kHz,
  STEP_100_0kHz,
} Step;

typedef enum {
  UPCONVERTER_OFF,
  UPCONVERTER_50M,
  UPCONVERTER_125M,
} UpconverterTypes;

typedef enum {
  OFFSET_NONE,
  OFFSET_PLUS,
  OFFSET_MINUS,
} OffsetDirection;

typedef enum {
  BL_SQL_OFF,
  BL_SQL_ON,
  BL_SQL_OPEN,
} BacklightOnSquelchMode;

typedef enum {
  BAT_1600,
  BAT_2200,
  BAT_3500,
} BatteryType;

typedef enum {
  BAT_CLEAN,
  BAT_PERCENT,
  BAT_VOLTAGE,
} BatteryStyle;

typedef enum {
  TX_POW_LOW,
  TX_POW_MID,
  TX_POW_HIGH,
} TXOutputPower;

/* typedef enum {
  EEPROM_A,         // 000
  EEPROM_B,         // 001
  EEPROM_BL24C64,   // 010 checkbyte default
  EEPROM_BL24C128,  // 011
  EEPROM_BL24C256,  // 100
  EEPROM_BL24C512,  // 101
  EEPROM_BL24C1024, // 110
  EEPROM_M24M02,    // 111
} EEPROMType; */

typedef enum {
  TX_DISALLOW,
  TX_ALLOW_LPD_PMR,
  TX_ALLOW_LPD_PMR_SATCOM,
  TX_ALLOW_HAM,
  TX_ALLOW_ALL,
} AllowTX;

typedef enum {
  TX_UNKNOWN,
  TX_ON,
  TX_VOL_HIGH,
  TX_BAT_LOW,
  TX_DISABLED,
  TX_DISABLED_UPCONVERTER,
  TX_POW_OVERDRIVE,
} TXState;

typedef enum {
  CH_CHANNEL,
  CH_VFO,
  CH_BAND,
  CH_EMPTY = 255,
} ChannelType;

// getsize(CH)

typedef struct {
  uint8_t count;
  uint8_t maxCount;
  CH *slots[5];
} AppVFOSlots;

typedef struct {
  const char *name;
  void (*init)();
  void (*update)();
  void (*render)();
  bool (*key)(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld);
  void (*deinit)();
  void *cfg;
  bool runnable;
  AppType_t id;
  AppVFOSlots *vfoSlots;
} App;

typedef struct {
  uint8_t s : 8;
  uint8_t m : 8;
  uint8_t e : 8;
} __attribute__((packed)) PowerCalibration;

typedef struct {
  char name[10];
} __attribute__((packed)) Scanlist;

#endif /* end of include guard: GLOBALS_H */
