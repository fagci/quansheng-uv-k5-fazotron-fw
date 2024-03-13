#include "board.hpp"
#include "driver/backlight.hpp"
#include "driver/system.hpp"
#include "svc/backlight.hpp"

extern "C" int Main(void) {
  Board board;
  SYSTICK_Init();
  SYSTEM_ConfigureSysCon();

  board.init();

  BacklightService backlightService(&board.display);

  return 0;
}
