#pragma once

#include "../apps/apps.hpp"
#include "svc.hpp"

class AppsService : public Svc {
public:
  void init() {}
  void update() { APPS_update(); }
  void deinit() {}
};
