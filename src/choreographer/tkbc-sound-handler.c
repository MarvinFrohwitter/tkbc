// ========================== Sound Handler ==================================

#include "../global/tkbc-types.h"
#include "tkbc-keymaps.h"
#include <raylib.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief The function sets up the audiodevice and sets the master volume to the
 * given value.
 *
 * @param master_volume The value the master is set to initially.
 * @return The on the stack allocated sound.
 */
void tkbc_init_sound(size_t master_volume) {
  InitAudioDevice();
  if (IsAudioDeviceReady()) {
    SetMasterVolume(master_volume);
  }
}

/**
 * @brief The function unloads the given sound and closes the audiodevice.
 *
 * @param sound The representation of the current loaded sound.
 */
void tkbc_sound_destroy(Sound sound) {
  StopSound(sound);
  UnloadSound(sound);
  CloseAudioDevice();
}

/**
 * @brief The function checks for key presses related to the audio.
 *
 * @param env The environment that holds the current state of the application.
 */
void tkbc_input_sound_handler(Env *env) {
  // Handles current loaded sound file.
  // KEY_N && KEY_LEFT_SHIFT && KEY_RIGHT_SHIFT
  Key_Map keymap = tkbc_hash_to_keymap(env->keymaps, KMH_STOPS_SOUND);
  if (IsKeyPressed(keymap.key) &&
      (IsKeyDown(keymap.mod_key) || IsKeyDown(keymap.mod_co_key))) {
    StopSound(env->sound);
    // KEY_N
  } else if (IsKeyPressed(tkbc_hash_to_key(env->keymaps, KMH_PLAYS_SOUND))) {
    PlaySound(env->sound);
  }

  keymap = tkbc_hash_to_keymap(env->keymaps, KMH_RESUMES_SOUND);
  // KEY_M && KEY_LEFT_SHIFT && KEY_RIGHT_SHIFT
  if (IsKeyPressed(keymap.key) &&
      (IsKeyDown(keymap.mod_key) || IsKeyDown(keymap.mod_co_key))) {
    ResumeSound(env->sound);
    // KEY_M
  } else if (IsKeyPressed(tkbc_hash_to_key(env->keymaps, KMH_PAUSES_SOUND))) {
    PauseSound(env->sound);
  }
}
