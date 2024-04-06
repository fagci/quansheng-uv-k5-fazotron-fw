#pragma once

#include "svc.hpp"
#include "svc/manager.hpp"
#include "ui/statusline.hpp"

class SystemService : public Svc {
public:
  void init() {}
  void update() {
    STATUSLINE_update();
    S::backlight.update();
  }
  void deinit() {}
};
