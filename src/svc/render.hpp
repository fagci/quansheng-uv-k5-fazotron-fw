#pragma once

#include "../apps/apps.hpp"
#include "../board.hpp"
#include "../driver/system.hpp"
#include "../ui/statusline.hpp"

class RenderService {
  const uint32_t RENDER_TIME = 40;

public:
  void init() {}

  void update() {
    if (redrawScreen && Now() - lastRender >= RENDER_TIME) {
      APPS_render();
      STATUSLINE_render();

      Board::display.render();
      lastRender = elapsedMilliseconds;
      redrawScreen = false;
    }
  }

  void deinit() {}

  void schedule() { redrawScreen = true; }

protected:
  bool redrawScreen;
  uint32_t lastRender = 0;
};
