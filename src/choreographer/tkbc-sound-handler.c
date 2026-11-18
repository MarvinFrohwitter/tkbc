// ========================== Sound Handler ==================================

#include "../global/tkbc-types.h"
#include "raylib.h"
#include "tkbc-keymaps.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief The function updates the music stream so the buffered audio keeps
 * playing. It has to be called once per frame while music is loaded.
 *
 * @param env The environment that holds the current state of the application.
 */
void tkbc_update_music_stream(Env *env) {
    if (IsMusicValid(env->music)) {
        UpdateMusicStream(env->music);
    }
}

/**
 * @brief The function unloads the given music stream and closes the audiodevice.
 *
 * @param music The representation of the current loaded music stream.
 */
void tkbc_music_destroy(Music music) {
    if (IsMusicValid(music)) {
        StopMusicStream(music);
        UnloadMusicStream(music);
    }
    CloseAudioDevice();
}

/**
 * @brief The function checks for key presses related to the audio.
 *
 * @param env The environment that holds the current state of the application.
 */
void tkbc_input_music_handler(Env *env) {
    // Handles current loaded music stream.
    // KEY_N && KEY_LEFT_SHIFT && KEY_RIGHT_SHIFT
    if (tkbc_check_keymaps_full(env->keymaps, KMH_STOPS_SOUND, KEY_MAP_CHECK_KEY_PRESSED_MOD_DOWN)) {
        StopMusicStream(env->music);
        // KEY_N
    } else if (tkbc_check_keymaps_full(env->keymaps, KMH_PLAYS_SOUND, KEY_MAP_CHECK_KEY_PRESSED)) {
        PlayMusicStream(env->music);
    }

    // KEY_M && KEY_LEFT_SHIFT && KEY_RIGHT_SHIFT
    if (tkbc_check_keymaps_full(env->keymaps, KMH_RESUMES_SOUND, KEY_MAP_CHECK_KEY_PRESSED_MOD_DOWN)) {
        ResumeMusicStream(env->music);
        // KEY_M
    } else if (tkbc_check_keymaps_full(env->keymaps, KMH_PAUSES_SOUND, KEY_MAP_CHECK_KEY_PRESSED)) {
        PauseMusicStream(env->music);
    }
}
