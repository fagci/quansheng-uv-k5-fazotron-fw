#pragma once

template <class T, class R> constexpr R IncDec(R val, T min, T max, T inc) {
  T mma = max - min + 1;
  T vmi = (val - min) + inc;
  return (R)(((vmi % mma) + mma) % mma + min);
}
