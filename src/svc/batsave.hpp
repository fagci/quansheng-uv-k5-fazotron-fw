#pragma once

#include "scheduler.hpp"
#include "svc.hpp"

namespace svc::batsave {
void update() {}

void stop() { Scheduler::taskRemove(update); }
void start(uint32_t interval) {
  stop();
  Scheduler::taskAdd("Svc", update, interval, true, 52);
}
}; // namespace svc::batsave
