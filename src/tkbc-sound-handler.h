#ifndef TKBC_SOUND_H_
#define TKBC_SOUND_H_

#include "tkbc-types.h"
#include <raylib.h>
#include <stdio.h>

// ===========================================================================
// ========================== Sound Handler ==================================
// ===========================================================================

Sound tkbc_init_sound(size_t master_volume);
void tkbc_sound_destroy(Sound sound);
void tkbc_sound_handler(Env *env, Sound *kite_sound);

#endif // TKBC_SOUND_H_

// ===========================================================================

#ifdef TKBC_SOUND_IMPLEMENTATION

// ========================== Sound Handler ==================================

#include <string.h>

/**
 * @brief [TODO:description]
 *
 * @param master_volume [TODO:parameter]
 * @return [TODO:return]
 */
Sound tkbc_init_sound(size_t master_volume) {
  Sound s = {0};
  InitAudioDevice();
  if (IsAudioDeviceReady()) {
    SetMasterVolume(master_volume);
  }

  return s;
}

/**
 * @brief [TODO:description]
 *
 * @param sound [TODO:parameter]
 */
void tkbc_sound_destroy(Sound sound) {
  StopSound(sound);
  UnloadSound(sound);
  CloseAudioDevice();
}

/**
 * @brief The function checks for key presses related to the audio. And if any
 * audio file has been dropped into the application.
 *
 * @param env [TODO:parameter]
 * @param kite_sound [TODO:parameter]
 */
void tkbc_sound_handler(Env *env, Sound *kite_sound) {

  if (IsFileDropped()) {
    FilePathList file_path_list = LoadDroppedFiles();
    char *file_path;
    for (size_t i = 0; i < file_path_list.count && i < 1; ++i) {
      file_path = file_path_list.paths[i];
      fprintf(stderr, "INFO: FILE: PATH :MUSIC: %s\n", file_path);
      *kite_sound = LoadSound(file_path);
    }

    env->sound_file_name =
        realloc(env->sound_file_name, sizeof(char) * strlen(file_path) + 1);
    if (env->sound_file_name == NULL) {
      fprintf(stderr, "The allocation has failed in: %s: %d\n", __FILE__,
              __LINE__);
    }
    strncpy(env->sound_file_name, file_path, strlen(file_path));

    UnloadDroppedFiles(file_path_list);
  }

  if (IsKeyPressed(KEY_S) &&
      (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {
    StopSound(*kite_sound);
  } else if (IsKeyPressed(KEY_S)) {
    PlaySound(*kite_sound);
  } else if (IsKeyPressed(KEY_M) &&
             (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {
    ResumeSound(*kite_sound);
  } else if (IsKeyPressed(KEY_M)) {
    PauseSound(*kite_sound);
  }
}

#endif // TKBC_SOUND_IMPLEMENTATION
