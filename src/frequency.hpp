#pragma once

#include "misc.hpp"
#include <stdint.h>

struct PowerCalibration {
  uint8_t s : 8;
  uint8_t m : 8;
  uint8_t e : 8;
} __attribute__((packed));

class Frequency {
public:
  class Range {
  public:
    const uint32_t start : 27;
    const uint32_t end : 27;
    bool contains(const uint32_t f) const { return f >= start && f <= end; }
  } __attribute__((packed));

  constexpr static uint16_t StepFrequencyTable[12] = {
      1, 10, 100, 250, 500, 625, 833, 900, 1000, 1250, 2500, 10000,
  };

  static bool inRange(uint32_t f, const Frequency::Range *r) {
    return r->contains(f);
  }
};

class Band {
public:
  constexpr static Frequency::Range LPD = {43307500, 43477500};
  constexpr static Frequency::Range PMR = {44600625, 44609375};
  constexpr static Frequency::Range HAM2M = {14400000, 14799999};
  constexpr static Frequency::Range HAM70CM = {43000000, 43999999};
  constexpr static Frequency::Range SATCOM = {23000000, 31999999};

  constexpr static Frequency::Range STOCK_BANDS[12] = {
      {1500000, 3000000},   {3000000, 5000000},    {5000000, 7600000},
      {10800000, 13500000}, {13600000, 17300000},  {17400000, 34900000},
      {35000000, 39900000}, {40000000, 46900000},  {47000000, 59900000},
      {60000000, 90000000}, {90000000, 120000000}, {120000000, 134000000},
  };

  static uint8_t getIndex(uint32_t f) {
    for (uint8_t i = 0; i < ARRAY_SIZE(Band::STOCK_BANDS); ++i) {
      auto *b = &STOCK_BANDS[i];
      if (f >= b->start && f <= b->end) {
        return i;
      }
    }
    return 6;
  }

  static uint8_t calculateOutputPower(uint32_t f,
                                      PowerCalibration *powerCalibration) {
    const uint8_t bi = getIndex(f);
    const Frequency::Range *range = &STOCK_BANDS[bi];
    const PowerCalibration *pCal = &powerCalibration[bi];
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
};
