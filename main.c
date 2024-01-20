#include "raylib.h"
#include <stdio.h>

#define WINDOW_SCALE 120
#define SCREEN_WIDTH 16 * WINDOW_SCALE
#define SCREEN_HEIGHT 9 * WINDOW_SCALE

#define VECTOR2_FMT "(%f,%f)"
#define Vector2_FMT_ARGS(arg) (float)(arg).x, (float)(arg).y

typedef struct Triangle {
  Vector2 v1;
  Vector2 v2;
  Vector2 v3;
} Triangle;

typedef struct Kite {
  Triangle left;
  Triangle right;

  Vector2 center;
  float overlap;
  float scale;

  Color body_color;
  Color top_color;
} Kite;

typedef struct Position {
  Vector2 top_left;
  Vector2 top_right;
  Vector2 top_center;
  Vector2 bottom_left;
  Vector2 bottom_right;
} Position;

void draw_kite(Kite *kite, Position *pos, float center_rotation) {

  // TODO: This thing is not implemented for the triangles
  // center_rotation = -45;

  Vector2 vl1 = {.x = kite->left.v1.x + pos->top_left.x,
                 .y = kite->left.v1.y + pos->top_left.y};
  Vector2 vl2 = {.x = kite->left.v2.x + pos->bottom_left.x,
                 .y = kite->left.v2.y + pos->bottom_left.y};
  Vector2 vl3 = {.x = kite->left.v3.x + pos->top_center.x,
                 .y = kite->left.v3.y + pos->top_center.y};

  Vector2 vr1 = {.x = kite->right.v1.x + pos->top_center.x,
                 .y = kite->right.v1.y + pos->top_center.y};
  Vector2 vr2 = {.x = kite->right.v2.x + pos->bottom_right.x,
                 .y = kite->right.v2.y + pos->bottom_right.y};
  Vector2 vr3 = {.x = kite->right.v3.x + pos->top_right.x,
                 .y = kite->right.v3.y + pos->top_right.y};


  // Draw a color-filled triangle (vertex in counter-clockwise order!)
  DrawTriangle(vl1, vl2, vl3, kite->body_color);
  DrawTriangle(vr1, vr2, vr3, kite->body_color);

    // TODO: Move the Rectangle to the Kite struct and to the init function
  float spread = 3;
  Rectangle rec = {0};
  rec.height = spread * 4;
  rec.width = vr3.x - vl1.x + 2 * spread;
  rec.x = vl1.x - spread + rec.width / 2;
  rec.y = vl1.y - rec.height;
  Vector2 origin = {.x = rec.width / 2, .y = 0};
  DrawRectanglePro(rec, origin, center_rotation, kite->top_color);
}

void kite_init(Kite *kite, Vector2 center) {
  kite->body_color = RED;
  kite->top_color = DARKGRAY;
  kite->overlap = 4;
  kite->scale = 8;
  kite->center.x = center.x;
  kite->center.y = center.y;

  float scale = kite->scale;
  float overlap = kite->overlap;
  float inner_space = 13;
  kite->left.v1 =
      (Vector2){.x = center.x - scale * 20, .y = center.y + scale * 0};
  kite->left.v2 = (Vector2){.x = center.x - scale * inner_space,
                            .y = center.y + scale * inner_space};
  kite->left.v3 =
      (Vector2){.x = center.x + scale * overlap, .y = center.y + scale * 0};
  kite->right.v1 =
      (Vector2){.x = center.x - scale * overlap, .y = center.y + scale * 0};
  kite->right.v2 = (Vector2){.x = center.x + scale * inner_space,
                             .y = center.y + scale * inner_space};
  kite->right.v3 =
      (Vector2){.x = center.x + scale * 20, .y = center.y + scale * 0};
}

int main(void) {
  const char *title = "TEAM KITE BALLETT CHOREOGRAPHER";

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, title);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(60);

  Vector2 center = {GetScreenWidth() / 2.f, GetScreenHeight() / 2.f};

  printf("The POS:" VECTOR2_FMT "\n", Vector2_FMT_ARGS(center));
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(SKYBLUE);
    center.x = GetScreenWidth() / 2.f;
    center.y = GetScreenHeight() / 2.f;
    Kite kite = {0};
    Position pos = {0};

    kite_init(&kite, center);
    draw_kite(&kite, &pos, 0);

    EndDrawing();
  };

  CloseWindow();
  return 0;
}
