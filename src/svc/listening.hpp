#pragma once

#include "../radio.hpp"

typedef Loot *(*ListenFn)();

class ListenService {
public:
  ListenService(Radio *r) : radio{r} {}

  void init() {
    if (!listenFn) {
      setListenFunction(radio->updateMeasurements);
    }
    radio->rxEnable();
  }

  void update() {
    listenFn();
    if (radio->vfo.scan.timeout < 10) {
      radio->resetRSSI();
    }
  }

  void deinit() {
    listenFn = nullptr;
    radio->idle();
  }

  void setListenFunction(ListenFn fn) { listenFn = fn; }

private:
  Radio *radio;
  ListenFn listenFn = nullptr;
};
