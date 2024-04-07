#pragma once

#include "../apps/apps.hpp"
#include "scheduler.hpp"
#include "svc.hpp"

namespace svc::apps {
void update() { APPS_update(); }
void stop() { Scheduler::taskRemove(update); }
void start(uint32_t interval = 1) {
  stop();
  Scheduler::taskAdd("Svc", update, interval, true, 100);
}
}; // namespace svc::apps
