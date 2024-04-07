#pragma once

#include "scheduler.hpp"
#include "svc.hpp"
#include "ui/statusline.hpp"

namespace svc::statusline {
void update() { STATUSLINE_update(); }
void stop() { Scheduler::taskRemove(update); }
void start(uint32_t interval = 1000) {
  stop();
  Scheduler::taskAdd("Svc", update, interval, true, 150);
}
} // namespace svc::statusline
