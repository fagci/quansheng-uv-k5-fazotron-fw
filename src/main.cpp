#include "board.hpp"
#include "driver/backlight.hpp"
#include "driver/system.hpp"

extern "C" int Main(void) {
  Board board;
  SYSTICK_Init();
  SYSTEM_ConfigureSysCon();

  board.init();
  board.backlight.on();
  return 0;
}
