#include "utils.hpp"
#include <stdint.h>
#include <type_traits>

class Some {
public:
  Some() = default;
  enum Value : uint8_t { A, B, C };
  constexpr Some(Value v) : value(v) {}

  constexpr Some inc(int16_t inc) {
    uint8_t min = 0;
    int16_t mma = C - min + 1;
    int16_t vmi = (value - min) + inc;
    return (Some)(((vmi % mma) + mma) % mma + min);
  }

private:
  Value value;
};

typedef struct {
  Some some : 2;
} Settings;

Settings settings;
settings.some.inc(1);
