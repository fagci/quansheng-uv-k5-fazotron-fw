#pragma once

#include "../apps/apps.hpp"
#include "../board.hpp"
#include "../driver/system.hpp"
#include "../ui/statusline.hpp"
#include "scheduler.hpp"
#include "svc/svc.hpp"

namespace svc::render {
const uint32_t RENDER_TIME = 40;

namespace {
bool redrawScreen;
uint32_t lastRender = 0;
} // namespace

void schedule() { redrawScreen = true; }

void update() {
  if (redrawScreen && Now() - lastRender >= RENDER_TIME) {
    APPS_render();
    STATUSLINE_render();

    Board::display.render();
    lastRender = elapsedMilliseconds;
    redrawScreen = false;
  }
}
void stop() { Scheduler::taskRemove(update); }
void start(uint32_t interval = 25) {
  stop();
  Scheduler::taskAdd("Svc", update, interval, true, 255);
}

}; // namespace svc::render
