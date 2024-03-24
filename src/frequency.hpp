#pragma once

#include "settings.hpp"
#include "svc/settings.hpp"
#include <stdint.h>

class Frequency {
public:
  typedef struct {
    uint32_t start : 27;
    uint32_t end : 27;
  } __attribute__((packed)) Range;

  constexpr static Range BAND_LPD = {43307500, 43477500};
  constexpr static Range BAND_PMR = {44600625, 44609375};
  constexpr static Range BAND_HAM2M = {14400000, 14799999};
  constexpr static Range BAND_HAM70CM = {43000000, 43999999};
  constexpr static Range BAND_SATCOM = {23000000, 31999999};

  constexpr static Range STOCK_BANDS[12] = {
      {1500000, 3000000},   {3000000, 5000000},    {5000000, 7600000},
      {10800000, 13500000}, {13600000, 17300000},  {17400000, 34900000},
      {35000000, 39900000}, {40000000, 46900000},  {47000000, 59900000},
      {60000000, 90000000}, {90000000, 120000000}, {120000000, 134000000},
  };

  constexpr static uint16_t StepFrequencyTable[12] = {
      1, 10, 100, 250, 500, 625, 833, 900, 1000, 1250, 2500, 10000,
  };

  constexpr static uint32_t upConverterValues[3] = {0, 5000000, 12500000};

  uint8_t getBandIndex(uint32_t f) {
    for (uint8_t i = 0; i < ARRAY_SIZE(STOCK_BANDS); ++i) {
      const Range *b = &STOCK_BANDS[i];
      if (f >= b->start && f <= b->end) {
        return i;
      }
    }
    return 6;
  }

  uint8_t
  calculateOutputPower(uint32_t f,
                       SettingsService::PowerCalibration *powerCalibration) {
    const uint8_t bi = getBandIndex(f);
    const Range *range = &STOCK_BANDS[bi];
    const SettingsService::PowerCalibration *pCal = &powerCalibration[bi];
    const uint32_t Middle = range->start + (range->end - range->start) / 2;

    if (f <= range->start) {
      return pCal->s;
    }

    if (f >= range->end) {
      return pCal->e;
    }

    if (f <= Middle) {
      return (uint8_t)(pCal->m + (((pCal->m - pCal->s) * (f - range->start)) /
                                  (Middle - range->start)));
    }

    return (uint8_t)(pCal->m + (((pCal->e - pCal->m) * (f - Middle)) /
                                (range->end - Middle)));
  }

  uint32_t GetScreenF(uint32_t f) {
    return f - upConverterValues[gSettings.upconverter];
  }

  uint32_t GetTuneF(uint32_t f) {
    return f + upConverterValues[gSettings.upconverter];
  }

  bool FreqInRange(uint32_t f, const Range *r) {
    return f >= r->start && f <= r->end;
  }
};
