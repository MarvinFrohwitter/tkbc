#define TKBC_LOGGING
#define TKBC_LOGGING_ERROR
#define TKBC_LOGGING_INFO
#define TKBC_LOGGING_WARNING

#define TKBC_UTILS_IMPLEMENTATION
#include "../global/tkbc-utils.h"
#undef TKBC_UTILS_IMPLEMENTATION

#define SPACE_IMPLEMENTATION
#include "../../external/space/space.h"
#undef SPACE_IMPLEMENTATION

#include "tkbc-asset-handler.h"
#include "tkbc-ffmpeg.h"
#include "tkbc-input-handler.h"
#include "tkbc-keymaps.h"
#include "tkbc-script-api.h"
#include "tkbc-sound-handler.h"
#include "tkbc.h"

#include "raylib.h"
#include <stdbool.h>

#include "../../tkbc_scripts/first.c"

#define WINDOW_SCALE 120
#define SCREEN_WIDTH 16 * WINDOW_SCALE
#define SCREEN_HEIGHT 9 * WINDOW_SCALE
#define LOADIMAGE

Space kite_images_space = {0};
Kite_Images kite_images = {0};
Space kite_textures_space = {0};
Kite_Textures kite_textures = {0};

/**
 * @brief The main function that handles the event loop.
 *
 * @return int Returns 0 if no errors occur.
 */
int main(void) {

  const char *title = "TEAM KITE BALLETT CHOREOGRAPHER";
  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT |
                 FLAG_WINDOW_MINIMIZED | FLAG_WINDOW_MAXIMIZED);
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, title);
  SetWindowMaxSize(SCREEN_WIDTH, SCREEN_HEIGHT);
  SetTargetFPS(TARGET_FPS);

  srand(time(NULL));
  Env *env = tkbc_init_env();
  if (tkbc_load_keymaps_from_file(&env->keymaps, ".tkbc-keymaps")) {
    tkbc_fprintf(stderr, "INFO", "No keympas are load from file.\n");
  }
  SetExitKey(tkbc_hash_to_key(env->keymaps, KMH_QUIT_PROGRAM));
  tkbc_init_sound(40);

#ifdef LOADIMAGE
  tkbc_load_kite_images_and_textures();
#endif /* ifdef LOADIMAGE */

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(SKYBLUE);

    if (env->script_setup) {
      // For detection if the begin and end is called correctly.
      env->script_setup = false;
      tkbc__script_input(env);
      for (size_t i = 0; i < env->block_frames->count; ++i) {
        // tkbc_print_script(stderr, &env->block_frames->elements[i]);

        // char buf[32];
        // sprintf(buf, "Script%zu.kite", i);
        // tkbc_write_script_kite_from_mem(&env->block_frames->elements[i],
        // buf);
      }
    }

    if (!tkbc_script_finished(env)) {
      tkbc_script_update_frames(env);
    }

    tkbc_update_kites_for_resize_window(env);
    tkbc_draw_kite_array(env->kite_array);
    tkbc_draw_ui(env);

    tkbc_file_handler(env);
    if (!env->keymaps_interaction) {
      tkbc_input_sound_handler(env);
      tkbc_input_handler_kite_array(env);
      tkbc_input_handler_script(env);
    }
    EndDrawing();
    // The end of the current frame has to be executed so ffmpeg gets the full
    // executed fame.
    if (!env->keymaps_interaction) {
      tkbc_ffmpeg_handler(env, "output.mp4");
    }
  };

  tkbc_sound_destroy(env->sound);
  tkbc_destroy_env(env);

  space_free_space(&kite_images_space);
  space_free_space(&kite_textures_space);
  CloseWindow();
  return 0;
}
