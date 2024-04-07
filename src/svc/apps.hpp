#pragma once

#include "../apps/apps.hpp"
#include "globals.hpp"
#include "scheduler.hpp"
#include "svc.hpp"

namespace svc::apps {
AppType_t currentAppId() { return gCurrentApp->id; }
void update() { APPS_update(); }
void stop() { Scheduler::taskRemove(update); }
void start(uint32_t interval = 1) {
  stop();
  Scheduler::taskAdd("Svc", update, interval, true, 100);
}
}; // namespace svc::apps
