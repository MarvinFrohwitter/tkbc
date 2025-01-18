#define TKBC_UTILS_IMPLEMENTATION
#include "../global/tkbc-utils.h"
#undef TKBC_UTILS_IMPLEMENTAION

#include "tkbc-ffmpeg.h"
#include "tkbc-input-handler.h"
#include "tkbc-script-api.h"
#include "tkbc-script-converter.h"
#include "tkbc-sound-handler.h"
#include "tkbc.h"

#include "raylib.h"
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

  tkbc_init_sound(40);

  Env *env = tkbc_init_env();

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
      // For detection if the begin and end is called correctly.
      env->script_setup = false;
      tkbc__script_input(env);
      for (size_t i = 0; i < env->block_frames->count; ++i) {
        // tkbc_print_script(stderr, &env->block_frames->elements[i]);
        char buf[32];
        sprintf(buf, "Script%zu.kite", i);
        tkbc_write_script_kite_from_mem(&env->block_frames->elements[i], buf);
      }
    }

    if (!tkbc_script_finished(env)) {
      tkbc_script_update_frames(env);
    }

    tkbc_draw_kite_array(env->kite_array);
    tkbc_draw_ui(env);
    EndDrawing();

    tkbc_file_handler(env);
    tkbc_input_sound_handler(env);
    tkbc_input_handler_kite_array(env);
    tkbc_input_handler_script(env);
    // The end of the current frame has to be executed so ffmpeg gets the full
    // executed fame.
    tkbc_ffmpeg_handler(env, "output.mp4");
  };

  tkbc_destroy_env(env);
  tkbc_sound_destroy(env->sound);
  CloseWindow();
  return 0;
}
