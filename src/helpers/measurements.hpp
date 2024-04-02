#pragma once

#include <stdint.h>

static const uint8_t rssi2s[2][15] = {
    {121, 115, 109, 103, 97, 91, 85, 79, 73, 63, 53, 43, 33, 23, 13},
    {141, 135, 129, 123, 117, 111, 105, 99, 93, 83, 73, 63, 53, 43, 33},
};

template <class T> T Clamp(T v, T min, T max) {
  return v <= min ? min : (v >= max ? max : v);
}

int ConvertDomain(int aValue, int aMin, int aMax, int bMin, int bMax) {
  const int aRange = aMax - aMin;
  const int bRange = bMax - bMin;
  aValue = Clamp(aValue, aMin, aMax);
  return ((aValue - aMin) * bRange + aRange / 2) / aRange + bMin;
}

uint8_t DBm2S(int dbm, bool isVHF) {
  uint8_t i = 0;
  dbm *= -1;
  for (i = 0; i < 15; i++) {
    if (dbm >= rssi2s[isVHF][i]) {
      return i;
    }
  }
  return i;
}

int Rssi2DBm(uint16_t rssi) { return (rssi >> 1) - 160; }

// applied x2 to prevent initial rounding
uint8_t Rssi2PX(uint16_t rssi, uint8_t pxMin, uint8_t pxMax) {
  return ConvertDomain(rssi - 320, -260, -120, pxMin, pxMax);
}

int Mid(uint16_t *array, uint8_t n) {
  int32_t sum = 0;
  for (uint8_t i = 0; i < n; ++i) {
    sum += array[i];
  }
  return sum / n;
}

int Min(uint16_t *array, uint8_t n) {
  uint8_t min = array[0];
  for (uint8_t i = 1; i < n; ++i) {
    if (array[i] < min) {
      min = array[i];
    }
  }
  return min;
}

int Max(uint16_t *array, uint8_t n) {
  uint8_t max = array[0];
  for (uint8_t i = 1; i < n; ++i) {
    if (array[i] > max) {
      max = array[i];
    }
  }
  return max;
}

uint16_t Mean(uint16_t *array, uint8_t n) {
  uint32_t sum = 0;
  for (uint8_t i = 0; i < n; ++i) {
    sum += array[i];
  }
  return sum / n;
}

uint16_t Sqrt(uint32_t v) {
  uint16_t res = 0;
  for (uint32_t i = 0; i < v; ++i) {
    if (i * i <= v) {
      res = i;
    } else {
      break;
    }
  }
  return res;
}

uint16_t Std(uint16_t *data, uint8_t n) {
  uint32_t sumDev = 0;

  for (uint8_t i = 0; i < n; ++i) {
    sumDev += data[i] * data[i];
  }
  return Sqrt(sumDev / n);
}

bool IsReadable(char *name) { return name[0] >= 32 && name[0] < 127; }
