#pragma once

#include "../board.hpp"
#include "../scheduler.hpp"
#include "apps.hpp"
#include "backlight.hpp"
#include "bat_save.hpp"
#include "driver/uart.hpp"
#include "keyboard.hpp"
#include "listening.hpp"
#include "render.hpp"
#include "scan.hpp"
#include "settings.hpp"
#include "sys.hpp"
#include <stdint.h>

class ServiceManager {
public:
  typedef enum {
    SVC_KEYBOARD,
    SVC_LISTEN,
    SVC_SCAN,
    SVC_BAT_SAVE,
    SVC_APPS,
    SVC_SYS,
    SVC_RENDER,
  } Svc;

  typedef struct {
    const char *name;
    void (*init)();
    void (*update)();
    void (*deinit)();
    uint8_t priority;
  } Service;

  Service services[] = {
      {"Keyboard", SVC_KEYBOARD_Init, SVC_KEYBOARD_Update, SVC_KEYBOARD_Deinit,
       0},
      {"Listen", SVC_LISTEN_Init, SVC_LISTEN_Update, SVC_LISTEN_Deinit, 50},
      {"Scan", SVC_SCAN_Init, SVC_SCAN_Update, SVC_SCAN_Deinit, 51},
      {"Bat save", SVC_BAT_SAVE_Init, SVC_BAT_SAVE_Update, SVC_BAT_SAVE_Deinit,
       52},
      {"Apps", SVC_APPS_Init, SVC_APPS_Update, SVC_APPS_Deinit, 100},
      {"Sys", SVC_SYS_Init, SVC_SYS_Update, SVC_SYS_Deinit, 150},
      {"Render", SVC_RENDER_Init, SVC_RENDER_Update, SVC_RENDER_Deinit, 255},
  };

  ServiceManager(Board *b) : board(b) {
    static Scheduler scheduler;
    // TODO: move into main service to make them running from there
    static BacklightService backlightService(&board->display);
    static ListenService listenService(&settings, &board->radio);
    static RenderService renderService(&board->display);
  }

  bool SVC_Running(Svc svc) { return TaskExists(services[svc].update); }

  void SVC_Toggle(Svc svc, bool on, uint16_t interval) {
    Service *s = &services[svc];
    bool exists = scheduler->taskExists(s->update);
    if (on) {
      if (!exists) {
        s->init();
        scheduler->taskAdd(s->name, s->update, interval, true, s->priority);
      }
    } else {
      if (exists) {
        scheduler->taskRemove(s->update);
        s->deinit();
      }
    }
  }

  void update() { scheduler->tasksUpdate(); }

private:
  Scheduler *scheduler;
  SettingsService *settings;
  Board *board;
};
