#include "raylib.h"
#include <stdio.h>

#define WINDOW_SCALE 120
#define SCREEN_WIDTH 16 * WINDOW_SCALE
#define SCREEN_HEIGHT 9 * WINDOW_SCALE

#define VECTOR2_FMT "(%f,%f)"
#define Vector2_FMT_ARGS(arg) (float)(arg).x, (float)(arg).y

int main(void) {
  const char *title = "TEAM KITE BALLETT CHOREOGRAPHER";

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, title);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(60);

  Vector2 center = {GetScreenWidth() / 2.f, GetScreenHeight() / 2.f};

  printf("The POS:" VECTOR2_FMT "\n", Vector2_FMT_ARGS(center));
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RED);

    center.x = GetScreenWidth() / 2.f;
    center.y = GetScreenHeight() / 2.f;
    DrawCircleV(center, 50.0f, SKYBLUE);

    EndDrawing();
  };

  CloseWindow();
  return 0;
}
