#ifndef SPECTRUM_H
#define SPECTRUM_H

#include "../driver/keyboard.hpp"
#include "../helpers/appsregistry.hpp"
#include "../radio.hpp"
#include <stdint.h>
#include <string.h>

bool SPECTRUM_key(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld);
void SPECTRUM_init();
void SPECTRUM_deinit();
void SPECTRUM_update();
void SPECTRUM_render();
App *SPECTRUM_Meta();

#endif /* ifndef SPECTRUM_H */
