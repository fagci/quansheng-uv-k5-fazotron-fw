#include "board.hpp"
#include "driver/backlight.hpp"
#include "svc/svc.hpp"

// Required by __libc_init_array in startup code if we are compiling using
// -nostdlib/-nostartfiles.
/* extern "C" void _init(void)
{

} */

extern "C" int Main(void) {
  static Board board;
  static ServiceManager serviceManager(&board);
  while (true) {
    serviceManager.update();
  }
  return 0;
}
