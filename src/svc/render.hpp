#pragma once

#include "../apps/apps.hpp"
#include "../driver/st7565.hpp"
#include "../driver/system.hpp"
#include "../scheduler.hpp"
#include "../ui/statusline.hpp"

class RenderService {
  const uint32_t RENDER_TIME = 40;

public:
  RenderService(ST7565 *d) : display{d} {}

  void init() {}

  void update() {
    if (redrawScreen && Now() - lastRender >= RENDER_TIME) {
      APPS_render();
      STATUSLINE_render();

      display->render();
      lastRender = elapsedMilliseconds;
      redrawScreen = false;
    }
  }

  void deinit() {}

protected:
  ST7565 *display;
  bool redrawScreen;
  uint32_t lastRender = 0;
};
