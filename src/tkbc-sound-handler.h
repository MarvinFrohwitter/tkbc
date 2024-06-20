#ifndef TKBC_SOUND_H_
#define TKBC_SOUND_H_

#include <raylib.h>
#include <stdio.h>

// ===========================================================================
// ========================== Sound Handler ==================================
// ===========================================================================

Sound kite_init_sound(size_t master_volume);
void kite_sound_destroy(Sound sound);
void kite_sound_handler(Sound *kite_sound);

#endif // TKBC_SOUND_H_

// ===========================================================================

#ifdef TKBC_SOUND_IMPLEMENTATION

// ========================== Sound Handler ==================================

/**
 * @brief [TODO:description]
 *
 * @param master_volume [TODO:parameter]
 * @return [TODO:return]
 */
Sound kite_init_sound(size_t master_volume) {
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
void kite_sound_destroy(Sound sound) {
  StopSound(sound);
  UnloadSound(sound);
  CloseAudioDevice();
}

/**
 * @brief The function kite_sound_handler() checks for key presses related to
 * the audio. And if any audio file has been dropped into the application.
 */
void kite_sound_handler(Sound *kite_sound) {

  if (IsFileDropped()) {
    FilePathList file_path_list = LoadDroppedFiles();
    for (size_t i = 0; i < file_path_list.count && i < 1; ++i) {
      char *file_path = file_path_list.paths[i];
      fprintf(stderr, "ERROR: FILE: PATH :MUSIC: %s\n", file_path);
      *kite_sound = LoadSound(file_path);
    }
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
