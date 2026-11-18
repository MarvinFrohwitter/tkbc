// ========================== Sound Handler ==================================

#include "../global/tkbc-types.h"
#include "../global/tkbc-utils.h"
#include "raylib.h"
#include "tkbc-keymaps.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief Starts the music stream while keeping the position currently shown
 * by the seek bar.
 *
 * @param env The environment that holds the current state of the application.
 */
static void tkbc_start_music_stream(Env *env) {
    if (IsMusicStreamPlaying(env->music)) {
        return;
    }

    float position = GetMusicTimePlayed(env->music);
    PlayMusicStream(env->music);
    if (position > 0) {
        SeekMusicStream(env->music, position);
    }
}

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
    if (!IsMusicValid(env->music)) {
        env->music_bar_interaction = false;
        return;
    }

    if (IsMouseButtonUp(MOUSE_BUTTON_LEFT)) {
        env->music_bar_interaction = false;
    }

    if (env->music_bar_interaction && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        float length = GetMusicTimeLength(env->music);
        if (length > 0 && env->music_bar_base.width > 0) {
            Vector2 mouse_pos = GetMousePosition();
            float t = (mouse_pos.x - env->music_bar_base.x) / env->music_bar_base.width;
            t = tkbc_clamp(t, 0, 1);
            SeekMusicStream(env->music, t * length);
        }
    }

    // SHIFT+KEY_M: restart from the beginning while playing, otherwise resume.
    if (tkbc_check_keymaps_full(env->keymaps, KMH_RESUMES_SOUND, KEY_MAP_CHECK_KEY_PRESSED_MOD_DOWN)) {
        if (IsMusicStreamPlaying(env->music)) {
            StopMusicStream(env->music);
            PlayMusicStream(env->music);
        } else {
            ResumeMusicStream(env->music);
            StopMusicStream(env->music);
        }
        // KEY_M: pause a playing track, otherwise start it like KEY_N.
    } else if (tkbc_check_keymaps_full(env->keymaps, KMH_PLAY_PAUSES_SOUND, KEY_MAP_CHECK_KEY_PRESSED)) {
        if (IsMusicStreamPlaying(env->music)) {
            PauseMusicStream(env->music);
        } else {
            tkbc_start_music_stream(env);
        }
    }
}
