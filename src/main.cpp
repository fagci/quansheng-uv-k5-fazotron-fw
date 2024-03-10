#include "board.hpp"
#include "driver/backlight.hpp"
#include "driver/system.hpp"

extern "C" int Main(void) {
  SYSTICK_Init();
  SYSTEM_ConfigureSysCon();

  BOARD_Init();
  BACKLIGHT_Toggle(true);
  return 0;
}
