#pragma once

#include "../scheduler.hpp"
#include <stdint.h>

class Svc {
public:
  virtual void update(void) = 0;

  void setPrio(uint8_t prio) { priority = prio; }

  void start(uint32_t interval) {
    stop();
    Scheduler::taskAdd("Svc", update, interval, true, priority);
  }

  void stop() { Scheduler::taskRemove(update); }

private:
  uint8_t priority = 100;
};
