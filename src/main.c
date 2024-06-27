#include "tkbc.h"

#include <complex.h>
#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>

#include "first.c"


#ifndef TKBC_TEAM_FIGURES_API_IMPLEMENTATION
#define TKBC_TEAM_FIGURES_API_IMPLEMENTATION
#endif // TKBC_TEAM_FIGURES_API_IMPLEMENTATION
#include "tkbc-team-figures-api.h"

#ifndef TKBC_INPUT_HANDLER_IMPLEMENTATION
#define TKBC_INPUT_HANDLER_IMPLEMENTATION
#endif // TKBC_INPUT_HANDLER_IMPLEMENTATION
#include "tkbc-input-handler.h"

#ifndef TKBC_SCRIPT_HANDLER_IMPLEMENTATION
#define TKBC_SCRIPT_HANDLER_IMPLEMENTATION
#endif // TKBC_SCRIPT_HANDLER_IMPLEMENTATION
#include "tkbc-script-handler.h"

#ifndef TKBC_SOUND_IMPLEMENTATION
#define TKBC_SOUND_IMPLEMENTATION
#endif // TKBC_SOUND_IMPLEMENTATION
#include "tkbc-sound-handler.h"

#ifndef TKBC_FFMPEG_IMPLEMENTATION
#define TKBC_FFMPEG_IMPLEMENTATION
#endif // TKBC_FFMPEG_IMPLEMENTATION
#include "tkbc-ffmpeg.h"

#define WINDOW_SCALE 120
#define SCREEN_WIDTH 16 * WINDOW_SCALE
#define SCREEN_HEIGHT 9 * WINDOW_SCALE

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
  int width = GetScreenWidth();
  int height = GetScreenHeight();
  Vector2 center_pos = {width / 2.f, height / 2.f};
  fprintf(stdout, "The POS:" VECTOR2_FMT "\n", Vector2_FMT_ARGS(center_pos));

#ifdef LOADIMAGE
  Image background_image =
      LoadImage("/home/marvin/Entwicklung/c/tkbc/src/assets/raw.png");
  Texture2D background_texture = LoadTextureFromImage(background_image);
#endif /* ifdef LOADIMAGE */

  Sound kite_sound = tkbc_init_sound(40);

  Env *env = tkbc_init_env();
  tkbc_kite_array_generate(env, 9);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(SKYBLUE);

    if (!tkbc_script_finished(env)) {
      tkbc_script_input(env);
      tkbc_script_update_frames(env);
    }

#ifdef LOADIMAGE
    float scale_width = (float)GetScreenWidth() / background_texture.width;
    float scale_height = (float)GetScreenHeight() / background_texture.height;
    float scale = fmaxf(scale_width, scale_height);
    DrawTextureEx(background_texture, (Vector2){0, 0}, 0, scale, WHITE);
#endif /* ifdef LOADIMAGE */

    tkbc_draw_kite_array(env);
    DrawFPS(center_pos.x, 10);
    EndDrawing();

    tkbc_sound_handler(&kite_sound);
    tkbc_input_handler_kite_array(env);

    if (IsKeyPressed(KEY_V)) {
      tkbc_ffmpeg_create_proc(env);
    }

    tkbc_ffmpeg_write_image(env);

    if ((IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) &&
        IsKeyPressed(KEY_V)) {
      tkbc_ffmpeg_end(env);
    }
  };

  tkbc_destroy_env(env);
  tkbc_sound_destroy(kite_sound);
  CloseWindow();
  return 0;
}
