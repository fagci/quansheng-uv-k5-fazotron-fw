#pragma once

#include "../radio.hpp"

typedef Loot *(*ListenFn)();

class ListenService {
public:
  ListenService(Radio *r) : radio{r} {}

  void init() { radio->rxEnable(); }

  void update() {
    radio->updateMeasurements();
    if (radio->vfo.scan.timeout < 10) {
      radio->resetRSSI();
    }
  }

  void deinit() { radio->idle(); }

private:
  Radio *radio;
};
