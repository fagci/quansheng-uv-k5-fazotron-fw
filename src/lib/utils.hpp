#pragma once
#include <stdint.h>

template <class R>
constexpr R IncDec(R val, int64_t min, int64_t max, int64_t inc) {
  auto mma = max - min + 1;
  auto vmi = (static_cast<int64_t>(val) - min) + inc;
  return static_cast<R>(((vmi % mma) + mma) % mma + min);
}
