#ifndef TKBC_SOUND_H_
#define TKBC_SOUND_H_

#include "../global/tkbc-types.h"
#include "raylib.h"
#include <stddef.h>

// ===========================================================================
// ========================== Sound Handler ==================================
// ===========================================================================

void tkbc_init_sound(size_t master_volume);
void tkbc_sound_destroy(Sound sound);
void tkbc_input_sound_handler(Env *env);

#endif // TKBC_SOUND_H_
