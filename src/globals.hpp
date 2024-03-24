#ifndef GLOBALS_H
#define GLOBALS_H

#include "driver/bk4819.hpp"
#include "driver/keyboard.hpp"
#include "svc/channel.hpp"
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
  OFFSET_NONE,
  OFFSET_PLUS,
  OFFSET_MINUS,
} OffsetDirection;

typedef enum {
  TX_POW_LOW,
  TX_POW_MID,
  TX_POW_HIGH,
} TXOutputPower;

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

typedef struct {
  uint8_t count;
  uint8_t maxCount;
  ChannelService::CH *slots[5];
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
  char name[10];
} __attribute__((packed)) Scanlist;

#endif /* end of include guard: GLOBALS_H */
