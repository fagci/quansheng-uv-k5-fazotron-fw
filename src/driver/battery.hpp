#pragma once

#include "../misc.hpp"
#include "adc.hpp"
#include <stdint.h>

#define BAT_WARN_PERCENT 15

class Battery {
public:
  typedef enum {
    BAT_1600,
    BAT_2200,
    BAT_3500,
  } Type;

  Battery(ADC *adc) : adc{adc} {}

  constexpr static uint16_t Voltage2PercentageTable[][11][2] = {
      [BAT_1600] =
          {
              {840, 100},
              {780, 90},
              {760, 80},
              {740, 70},
              {720, 60},
              {710, 50},
              {700, 40},
              {690, 30},
              {680, 20},
              {672, 10},
              {600, 0},
          },

      [BAT_2200] =
          {
              {840, 100},
              {800, 90},
              {784, 80},
              {768, 70},
              {756, 60},
              {748, 50},
              {742, 40},
              {738, 30},
              {732, 20},
              {720, 10},
              {600, 0},
          },
      [BAT_3500] =
          {
              {840, 100},
              {762, 90},
              {744, 80},
              {726, 70},
              {710, 60},
              {690, 50},
              {674, 40},
              {660, 30},
              {648, 20},
              {628, 10},
              {600, 0},
          },
  };

  void init(Type type) { batteryType = type; }

  void getBatteryInfo(uint16_t *pVoltage, uint16_t *pCurrent) {
    adc->start();

    while (!adc->checkEndOfConversion(ADC::ADC_CH9)) {
    }
    *pVoltage = adc->getValue(ADC::ADC_CH4);
    *pCurrent = adc->getValue(ADC::ADC_CH9);
  }

  void updateBatteryInfo() {
    getBatteryInfo(&batAdcV, &batteryCurrent);
    bool charg = batteryCurrent >= 501;
    if (batAvgV == 0 || charg != chargingWithTypeC) {
      batAvgV = batAdcV;
    } else {
      batAvgV = batAvgV - (batAvgV - batAdcV) / 7;
    }

    batteryVoltage = (batAvgV * 760) / calibration;
    chargingWithTypeC = charg;
    batteryPercent = voltsToPercent(batteryVoltage, batteryType);
  }

  uint8_t percentage() { return batteryPercent; }
  uint16_t voltage() { return batteryVoltage; }
  bool isCharging() { return chargingWithTypeC; }

private:
  ADC *adc;
  uint16_t calibration = 2005;
  uint16_t batAdcV = 0;
  uint16_t batAvgV = 0;

  uint16_t batteryVoltage = 0;
  uint16_t batteryCurrent = 0;
  uint8_t batteryPercent = 0;
  bool chargingWithTypeC = true;
  Type batteryType;

  static uint8_t voltsToPercent(const unsigned int voltage_10mV,
                                Type batteryType) {
    const uint16_t(*crv)[2] = Voltage2PercentageTable[batteryType];
    const int mulipl = 1000;
    for (uint8_t i = 1; i < ARRAY_SIZE(Voltage2PercentageTable[BAT_2200]);
         i++) {
      if (voltage_10mV > crv[i][0]) {
        const int a =
            (crv[i - 1][1] - crv[i][1]) * mulipl / (crv[i - 1][0] - crv[i][0]);
        const int b = crv[i][1] - a * crv[i][0] / mulipl;
        const int p = a * voltage_10mV / mulipl + b;
        return p < 100 ? p : 100;
      }
    }

    return 0;
  }
};
