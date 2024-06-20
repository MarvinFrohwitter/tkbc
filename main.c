#include <complex.h>
#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>

#include "./tkbc_scripts/first.tkb.c"
#include "tkbc.h"

#define WINDOW_SCALE 120
#define SCREEN_WIDTH 16 * WINDOW_SCALE
#define SCREEN_HEIGHT 9 * WINDOW_SCALE
#define TARGET_FPS 60

/**
 * @brief The main function that handles the event loop and the sound loading.
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
  Vector2 center_pos = {GetScreenWidth() / 2.f, GetScreenHeight() / 2.f};
  fprintf(stdout, "The POS:" VECTOR2_FMT "\n", Vector2_FMT_ARGS(center_pos));

#ifdef LOADIMAGE
  Image background_image =
      LoadImage("/home/marvin/Entwicklung/c/tkbc/src/assets/raw.png");
  Texture2D background_texture = LoadTextureFromImage(background_image);
#endif /* ifdef LOADIMAGE */

  Sound kite_sound = kite_init_sound(40);

  Env *env = kite_init_env();
  kite_kite_array_generate(env, 9);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(SKYBLUE);

    if (!kite_script_finished(env)) {
      kite_script_input(env);
      kite_script_update_frames(env);
    }

#ifdef LOADIMAGE
    float scale_width = (float)GetScreenWidth() / background_texture.width;
    float scale_height = (float)GetScreenHeight() / background_texture.height;
    float scale = fmaxf(scale_width, scale_height);
    DrawTextureEx(background_texture, (Vector2){0, 0}, 0, scale, WHITE);
#endif /* ifdef LOADIMAGE */

    kite_draw_kite_array(env);
    DrawFPS(center_pos.x, 10);
    EndDrawing();

    kite_sound_handler(&kite_sound);
    kite_input_handler_kite_array(env);
  };

  kite_destroy_env(env);
  kite_sound_destroy(kite_sound);
  CloseWindow();
  return 0;
}
