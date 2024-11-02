#define TKBC_UTILS_IMPLEMENTATION
#include "../global/tkbc-utils.h"
#undef TKBC_UTILS_IMPLEMENTAION

#include "tkbc-ffmpeg.h"
#include "tkbc-input-handler.h"
#include "tkbc-script-api.h"
#include "tkbc-sound-handler.h"
#include "tkbc.h"

#include "raylib.h"
#include <complex.h>
#include <stdbool.h>

#include "../../tkbc_scripts/first.c"

#define WINDOW_SCALE 120
#define SCREEN_WIDTH 16 * WINDOW_SCALE
#define SCREEN_HEIGHT 9 * WINDOW_SCALE

/**
 * @brief The main function that handles the event loop.
 *
 * @return int Returns 0 if no errors occur.
 */
int main(void) {

  const char *title = "TEAM KITE BALLETT CHOREOGRAPHER";
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, title);
  // SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(TARGET_FPS);
  SetExitKey(KEY_ESCAPE);

#ifdef LOADIMAGE
  Image background_image =
      LoadImage("/home/marvin/Entwicklung/c/tkbc/assets/background.png");
  Texture2D background_texture = LoadTextureFromImage(background_image);
#endif /* ifdef LOADIMAGE */

  Sound kite_sound = tkbc_init_sound(40);

  Env *env = tkbc_init_env();
  tkbc_kite_array_generate(env, 9);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(SKYBLUE);

#ifdef LOADIMAGE
    float scale_width = (float)env->window_width / background_texture.width;
    float scale_height = (float)env->window_height / background_texture.height;
    float scale = fmaxf(scale_width, scale_height);
    DrawTextureEx(background_texture, (Vector2){0, 0}, 0, scale, WHITE);
#endif /* ifdef LOADIMAGE */

    if (env->script_setup) {
      tkbc_script_input(env);
    }

    if (!tkbc_script_finished(env)) {
      tkbc_script_update_frames(env);
    }

    tkbc_draw_kite_array(env->kite_array);
    tkbc_draw_ui(env);
    EndDrawing();

    tkbc_sound_handler(env, &kite_sound);
    tkbc_input_handler_kite_array(env);
    tkbc_input_handler_script(env);
    // The end of the current frame has to be executed so ffmpeg gets the full
    // executed fame.
    tkbc_ffmpeg_handler(env, "output.mp4");
  };

  tkbc_destroy_env(env);
  tkbc_sound_destroy(kite_sound);
  CloseWindow();
  return 0;
}