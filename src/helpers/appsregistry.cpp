#include "appsregistry.hpp"

#include "../apps/about.hpp"
#include "../apps/analyzer.hpp"
#include "../apps/antenna.hpp"
#include "../apps/appslist.hpp"
#include "../apps/bandcfg.hpp"
#include "../apps/bandlist.hpp"
#include "../apps/channelscanner.hpp"
#include "../apps/fastscan.hpp"
#include "../apps/finput.hpp"
#include "../apps/lootlist.hpp"
#include "../apps/multivfo.hpp"
#include "../apps/reset.hpp"
#include "../apps/savech.hpp"
#include "../apps/scanlists.hpp"
#include "../apps/settings.hpp"
#include "../apps/spectrumreborn.hpp"
#include "../apps/still.hpp"
#include "../apps/taskman.hpp"
#include "../apps/test.hpp"
#include "../apps/textinput.hpp"
#include "../apps/vfocfg.hpp"

#include "../driver/uart.hpp"

uint8_t appsCount = 0;
uint8_t appsToRunCount = 0;

App *apps[256];
App *appsAvailableToRun[256];

void APPS_Register(App *app) {
  Log("[+] APPSREGISTRY %s run=%u", app->name, app->runnable);
  apps[appsCount++] = app;
  if (app->runnable) {
    appsAvailableToRun[appsToRunCount++] = app;
  }
}

void APPS_RegisterAll() {
  APPS_Register(FINPUT_Meta());
  APPS_Register(TEXTINPUT_Meta());

  APPS_Register(BANDCFG_Meta());
  APPS_Register(CHCFG_Meta());

  APPS_Register(RESET_Meta());
  APPS_Register(SAVECH_Meta());
  APPS_Register(SETTINGS_Meta());
  APPS_Register(APPSLIST_Meta());

  APPS_Register(STILL_Meta());
  APPS_Register(MULTIVFO_Meta());
  APPS_Register(CHSCANNER_Meta());
  APPS_Register(SPECTRUM_Meta());
  APPS_Register(ANALYZER_Meta());
  APPS_Register(FASTSCAN_Meta());
  APPS_Register(LOOTLIST_Meta());
  APPS_Register(SCANLISTS_Meta());
  APPS_Register(BANDLIST_Meta());
  APPS_Register(ANTENNA_Meta());
  APPS_Register(TASKMAN_Meta());
  APPS_Register(TEST_Meta());
  APPS_Register(ABOUT_Meta());
}

App *APPS_GetById(AppType_t id) {
  for (uint8_t i = 0; i < appsCount; i++) {
    if (apps[i]->id == id) {
      return apps[i];
    }
  }
  return nullptr;
}
