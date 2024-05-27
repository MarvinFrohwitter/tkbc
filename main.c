#include <complex.h>
#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>

#include "tkbc.h"

#define WINDOW_SCALE 120
#define SCREEN_WIDTH 16 * WINDOW_SCALE
#define SCREEN_HEIGHT 9 * WINDOW_SCALE
#define TARGET_FPS 60

/**
 * @brief The main function that handles the event loop and the sound loading.
 *
 * @return int Returns 1 if no errors occur, otherwise none zero.
 */
int main(void) {

  const char *title = "TEAM KITE BALLETT CHOREOGRAPHER";
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, title);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(TARGET_FPS);
  SetExitKey(KEY_ESCAPE);
  Vector2 center_pos = {GetScreenWidth() / 2.f, GetScreenHeight() / 2.f};
  printf("The POS:" VECTOR2_FMT "\n", Vector2_FMT_ARGS(center_pos));

#ifdef LOADIMAGE
  Image background_image =
      LoadImage("/home/marvin/Entwicklung/c/tkbc/src/assets/raw.png");
  Texture2D background_texture = LoadTextureFromImage(background_image);
#endif /* ifdef LOADIMAGE */

  Sound kite_sound = kite_sound_init(40);

  Env *env = kite_env_init();

  kite_gen_kites(env, 4);

  // TODO: The next overides the old kite array
  kite_register_frames(
      env, 5,
      kite_gen_frame(KITE_ROTATION, kite_indexs_append(1, 1),
                     &(CLITERAL(Rotation_Action){.angle = 90}), 5),
      kite_gen_frame(KITE_ROTATION, kite_indexs_append(2, 1, 2),
                     &(CLITERAL(Rotation_Action){.angle = 90}), 10),
      kite_gen_frame(KITE_MOVE, kite_indexs_append(1, 0),
                     &(CLITERAL(Move_Action){
                         .position.x = 10,
                         .position.y = 10,
                     }),
                     20),
      kite_script_wait(3),
      kite_script_frames_quit(5)
    );

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(SKYBLUE);
    kite_update_frames(env);

    // TODO: Append a default frame for the start and call the registration for
    // that frame in the kite_script_begin() function and set it to finished.

    // kite_register_frames(env, 1, kite_script_wait(3));

    kite_register_frames(
        env, 2,
        kite_gen_frame(KITE_ROTATION, kite_indexs_append(1, 3),
                       &(CLITERAL(Rotation_Action){.angle = 40}), 0),
        kite_gen_frame(KITE_MOVE, kite_indexs_append(1, 0),
                       &(CLITERAL(Move_Action){
                           .position.x = 300,
                           .position.y = 500,
                       }),
                       0));

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
    kite_array_input_handler(env);
  };

  kite_env_destroy(env);
  kite_defer_sound(kite_sound);
  CloseWindow();
  return 0;
}
