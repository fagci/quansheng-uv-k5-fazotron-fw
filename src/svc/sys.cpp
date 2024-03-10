#include "sys.hpp"
#include "driver/backlight.hpp"
#include "ui/statusline.hpp"

void SVC_SYS_Init() {}
void SVC_SYS_Update() {
  STATUSLINE_update();
  BACKLIGHT_Update();
}
void SVC_SYS_Deinit() {}
