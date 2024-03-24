#include "board.hpp"
#include "driver/backlight.hpp"
#include "driver/eeprom.hpp"
#include "driver/system.hpp"
#include "svc/backlight.hpp"
#include "svc/listening.hpp"
#include "svc/render.hpp"
#include "svc/settings.hpp"

// Required by __libc_init_array in startup code if we are compiling using
// -nostdlib/-nostartfiles.
/* extern "C" void _init(void)
{

} */

extern "C" int Main(void) {
  static Board board;
  static EEPROM eeprom;
  static Radio radio;

  SYSTICK_Init();

  static SettingsService settingsService(&eeprom);

  board.init();
  eeprom.init(settingsService.eepromType);

  // TODO: move into main service to make them running from there
  static BacklightService backlightService(&board.display);
  static ListenService listenService(&settingsService, &radio);
  static RenderService renderService(&board.display);

  return 0;
}
