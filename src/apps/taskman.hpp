#ifndef TASKMAN_H
#define TASKMAN_H

#include "../driver/keyboard.hpp"
#include "../helpers/appsregistry.hpp"
#include <stdint.h>

void TASKMAN_Init();
void TASKMAN_Render();
bool TASKMAN_Key(KEY_Code_t key, bool bKeyPressed, bool bKeyHeld);
App *TASKMAN_Meta();

#endif /* end of include guard: TASKMAN_H */
