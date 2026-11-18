#ifndef TKBC_SOUND_H_
#define TKBC_SOUND_H_

#include "../global/tkbc-types.h"
#include "raylib.h"
#include <stddef.h>

// ===========================================================================
// ========================== Sound Handler ==================================
// ===========================================================================

void tkbc_update_music_stream(Env *env);
void tkbc_music_destroy(Music music);
void tkbc_input_music_handler(Env *env);

#endif  // TKBC_SOUND_H_
