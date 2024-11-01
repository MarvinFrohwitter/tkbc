// ========================== Sound Handler ==================================

#include "../global/tkbc-types.h"
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
Sound tkbc_init_sound(size_t master_volume) {
  Sound sound = {0};
  InitAudioDevice();
  if (IsAudioDeviceReady()) {
    SetMasterVolume(master_volume);
  }

  return sound;
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
 * @brief The function checks for key presses related to the audio. And if any
 * audio file has been dropped into the application.
 *
 * @param env The environment that holds the current state of the application.
 * @param kite_sound The current loaded sound to play or pause or load another.
 */
void tkbc_sound_handler(Env *env, Sound *kite_sound) {

  // Checks drag and dropped audio files.
  if (IsFileDropped()) {
    FilePathList file_path_list = LoadDroppedFiles();
    if (file_path_list.count == 0) {
      return;
    }
    char *file_path;
    for (size_t i = 0; i < file_path_list.count && i < 1; ++i) {
      file_path = file_path_list.paths[i];
      fprintf(stderr, "INFO: FILE: PATH :MUSIC: %s\n", file_path);
      *kite_sound = LoadSound(file_path);
    }

    int length = strlen(file_path);
    env->sound_file_name =
        realloc(env->sound_file_name, sizeof(char) * length + 1);
    if (env->sound_file_name == NULL) {
      fprintf(stderr, "The allocation has failed in: %s: %d\n", __FILE__,
              __LINE__);
    }
    strncpy(env->sound_file_name, file_path, length);

    UnloadDroppedFiles(file_path_list);
  }

  // Handles current loaded sound file.
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
