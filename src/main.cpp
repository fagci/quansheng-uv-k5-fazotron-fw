#include "board.hpp"
#include "driver/backlight.hpp"
#include "svc/apps.hpp"
#include "svc/backlight.hpp"
#include "svc/batsave.hpp"
#include "svc/channel.hpp"
#include "svc/keyboard.hpp"
#include "svc/listening.hpp"
#include "svc/loot.hpp"
#include "svc/render.hpp"
#include "svc/scan.hpp"
#include "svc/settings.hpp"
#include "svc/sys.hpp"
#include "svc/tune.hpp"

// Required by __libc_init_array in startup code if we are compiling using
// -nostdlib/-nostartfiles.
/* extern "C" void _init(void)
{

} */

extern "C" int Main(void) {
  using namespace svc;

  settings::load();

  keyboard::start();
  listen::start();
  apps::start();
  statusline::start();
  backlight::start();
  render::start();

  while (true) {
    Scheduler::tasksUpdate();
  }
  return 0;
}
