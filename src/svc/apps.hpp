#pragma once

#include "../apps/apps.hpp"
#include "svc.hpp"

class AppsService : Svc {
public:
  void init() {}
  void update() { APPS_update(); }
  void deinit() {}
};
