extern "C" {
/* #include "board.h"
#include "driver/backlight.h"
#include "driver/system.h" */
#include "driver/system.hpp"
}

extern "C" int Main(void) {
  SYSTICK_Init();
  SYSTEM_ConfigureSysCon();

  // BOARD_Init();
  // BACKLIGHT_Toggle(true);
  return 0;
}
