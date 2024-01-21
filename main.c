#include "raylib.h"
#include <assert.h>
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
  Vector2 center;

  Color body_color;
  Triangle left;
  Triangle right;
  float overlap;
  float inner_space;

  Color top_color;
  Rectangle rec;
  float spread;

  float scale;
} Kite;

void draw_kite(Kite *kite, Vector2 *new__center_pos, float center_rotation) {
  kite->center.x = new__center_pos->x;
  kite->center.y = new__center_pos->y;

  // TODO: This thing is not implemented for the triangles
  // center_rotation = -45;

  float cw = kite->right.v3.x - kite->left.v1.x;
  float is = kite->inner_space;
  float o = kite->overlap;

  if (center_rotation == 0) {
    kite->left.v1.x = new__center_pos->x - (cw / 2.f);
    kite->left.v1.y = new__center_pos->y;
    kite->left.v2.x = new__center_pos->x - is;
    kite->left.v2.y = new__center_pos->y + is;
    kite->left.v3.x = new__center_pos->x + o;
    kite->left.v3.y = new__center_pos->y;

    kite->right.v1.x = new__center_pos->x - o;
    kite->right.v1.y = new__center_pos->y;
    kite->right.v2.x = new__center_pos->x + is;
    kite->right.v2.y = new__center_pos->y + is;
    kite->right.v3.x = new__center_pos->x + (cw / 2.f);
    kite->right.v3.y = new__center_pos->y;
  } else {
    assert(0 && "Rotation not IMPLEMENTED");
    // Vector2 origin = {.x = kite->rec.width / 2.f, .y = 0};
  }

  // Draw a color-filled triangle (vertex in counter-clockwise order!)
  DrawTriangle(kite->left.v1, kite->left.v2, kite->left.v3, kite->body_color);
  DrawTriangle(kite->right.v1, kite->right.v2, kite->right.v3,
               kite->body_color);

  kite->rec.height = kite->spread;
  kite->rec.width = cw + kite->spread;
  kite->rec.x = kite->left.v1.x - kite->spread / 2;
  kite->rec.y = kite->left.v1.y - kite->rec.height;

  Vector2 origin = {0};
  DrawRectanglePro(kite->rec, origin, center_rotation, kite->top_color);
}

void kite_init(Kite *kite, Vector2 center) {
  kite->center.x = center.x;
  kite->center.y = center.y;
  kite->body_color = RED;
  kite->top_color = DARKGRAY;
  kite->scale = 10.f;
  kite->overlap = 4.f;
  kite->inner_space = 20.f;
  kite->spread = 1.f;

  float cw = 20.f * kite->scale * 2;
  kite->inner_space += kite->scale * kite->scale;
  kite->overlap += kite->scale * 4;
  kite->spread += kite->scale;

  kite->left.v1 = (Vector2){.x = center.x - (cw / 2.f), .y = center.y};
  kite->left.v2 = (Vector2){.x = center.x - kite->inner_space,
                            .y = center.y + kite->inner_space};
  kite->left.v3 = (Vector2){.x = center.x + kite->overlap, .y = center.y};

  kite->right.v1 = (Vector2){.x = center.x - kite->overlap, .y = center.y};
  kite->right.v2 = (Vector2){.x = center.x + kite->inner_space,
                             .y = center.y + kite->inner_space};
  kite->right.v3 = (Vector2){.x = center.x + (cw / 2.f), .y = center.y};

  cw = kite->right.v3.x - kite->left.v1.x;

  kite->rec.height = kite->spread;
  kite->rec.width = cw + kite->spread;
  kite->rec.x = kite->left.v1.x - kite->spread / 2;
  kite->rec.y = kite->left.v1.y - kite->rec.height;
}

void input_handler(Vector2 *pos) {

  float speed = 5;
  float velocity = 1;
  float friction = 2;

  if (IsKeyDown(KEY_J) ||IsKeyDown(KEY_DOWN) ) {
    pos->y += speed * velocity;
    if (IsKeyDown(KEY_L)||IsKeyDown(KEY_RIGHT) )
      pos->x += speed * velocity;
    if (IsKeyDown(KEY_H)||IsKeyDown(KEY_LEFT))
      pos->x -= speed * velocity;

  } else if (IsKeyDown(KEY_K)||IsKeyDown(KEY_UP)) {
    pos->y -= speed * velocity;
    if (IsKeyDown(KEY_L)||IsKeyDown(KEY_RIGHT))
      pos->x += speed * velocity;
    if (IsKeyDown(KEY_H)||IsKeyDown(KEY_LEFT))
      pos->x -= speed * velocity;

  } else if (IsKeyDown(KEY_H)||IsKeyDown(KEY_LEFT)) {
    pos->x -= speed * velocity;
  } else if (IsKeyDown(KEY_L)||IsKeyDown(KEY_RIGHT)) {
    pos->x += speed * velocity;
  }

  if (velocity <= speed / friction) {
    velocity *= velocity;
  }
}

int main(void) {
  const char *title = "TEAM KITE BALLETT CHOREOGRAPHER";

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, title);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(120);

  Vector2 pos = {GetScreenWidth() / 2.f, GetScreenHeight() / 2.f};

  Kite kite = {0};
  kite_init(&kite, pos);
  // TODO: Find a reasonable velocity

  printf("The POS:" VECTOR2_FMT "\n", Vector2_FMT_ARGS(pos));
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(SKYBLUE);

    input_handler(&pos);

    draw_kite(&kite, &pos, 0);

    EndDrawing();
  };

  CloseWindow();
  return 0;
}
