#pragma once

#include "svc.hpp"
#include "ui/statusline.hpp"

class SystemService : Svc {
public:
  void init() {}
  void update() {
    STATUSLINE_update();
    backlight.update();
  }
  void deinit() {}
};
