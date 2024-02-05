#include "raylib.h"
#include <complex.h>
#include <math.h>
#include <stdio.h>

#define TEAL                                                                   \
  CLITERAL(Color) { 0, 128, 128, 255 } // Teal
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
  float width;
  // float height;
} Kite;

void draw_kite(Kite *k, Vector2 *position, float center_deg_rotation) {
  k->center.x = position->x;
  k->center.y = position->y;

  float cw = k->width;
  float is = k->inner_space;
  float o = k->overlap;

  // The difference between the angle 0 and the default downward interpolation
  float bl_angle = (PI * (360 - 42) / 180);
  float br_angle = (PI * (360 + 90 - 42) / 180);
  float phi = (PI * (center_deg_rotation) / 180);

  // LEFT Triangle
  // Correct
  k->left.v1.x = position->x - ceilf(crealf((cw / 2.f) * cexpf(I * phi)));
  k->left.v1.y = position->y + ceilf(cimagf((cw / 2.f) * cexpf(I * phi)));
  k->left.v2.x = position->x - ceilf(crealf(is * cexpf(I * (phi - bl_angle))));
  k->left.v2.y = position->y + ceilf(cimagf(is * cexpf(I * (phi - bl_angle))));
  k->left.v3.x = position->x + ceilf(crealf(o * cexpf(I * phi)));
  k->left.v3.y = position->y - ceilf(cimagf(o * cexpf(I * phi)));

  // RIGHT Triangle
  // Correct
  k->right.v1.x = position->x - ceilf(crealf(o * cexpf(I * phi)));
  k->right.v1.y = position->y + ceilf(cimagf(o * cexpf(I * phi)));
  k->right.v2.x = position->x + ceilf(crealf(is * cexpf(I * (phi - br_angle))));
  k->right.v2.y = position->y - ceilf(cimagf(is * cexpf(I * (phi - br_angle))));
  k->right.v3.x = position->x + ceilf(crealf((cw / 2.f) * cexpf(I * phi)));
  k->right.v3.y = position->y - ceilf(cimagf((cw / 2.f) * cexpf(I * phi)));

  // Draw a color-filled triangle (vertex in counter-clockwise order!)
  DrawTriangle(k->left.v1, k->left.v2, k->left.v3, k->body_color);
  DrawTriangle(k->right.v1, k->right.v2, k->right.v3, k->body_color);

  // Just use origin 0 and compute the new pos for the angel
  Vector2 origin = {0};
  k->rec.height = k->spread;
  k->rec.width = cw + k->spread * 2;

  k->rec.x = position->x - ceilf(crealf((cw / 2.f + k->spread) * cexpf(I * phi)));
  k->rec.y = position->y + ceilf(cimagf((cw / 2.f + k->spread) * cexpf(I * phi)));

  // k->rec.x = k->left.v1.x + k->spread;
  // k->rec.y = k->left.v1.y - k->rec.height;

  DrawRectanglePro(k->rec, origin, -center_deg_rotation, k->top_color);
}

void kite_init(Kite *k, Vector2 position) {
  k->center.x = position.x;
  k->center.y = position.y;
  k->body_color = TEAL;
  k->top_color = DARKGRAY;
  k->scale = 6.f;
  k->overlap = 8.f;
  k->inner_space = 20.f;
  k->spread = 1.f;

  float cw = 20.f * k->scale * 2;
  k->inner_space *= k->scale;
  k->overlap *= k->scale;
  k->spread += k->scale;

  k->left.v1 = (Vector2){.x = position.x - (cw / 2.f), .y = position.y};
  k->left.v2 = (Vector2){.x = position.x - k->inner_space,
                         .y = position.y + k->inner_space};
  k->left.v3 = (Vector2){.x = position.x + k->overlap, .y = position.y};

  k->right.v1 = (Vector2){.x = position.x - k->overlap, .y = position.y};
  k->right.v2 = (Vector2){.x = position.x + k->inner_space,
                          .y = position.y + k->inner_space};
  k->right.v3 = (Vector2){.x = position.x + (cw / 2.f), .y = position.y};

  cw = sqrtf((k->right.v3.y - k->left.v1.y) * (k->right.v3.y - k->left.v1.y) +
             (k->right.v3.x - k->left.v1.x) * (k->right.v3.x - k->left.v1.x));
  k->width = cw;

  k->rec.height = k->spread;
  k->rec.width = k->width + k->spread;
  k->rec.x = k->left.v1.x - k->spread / 2;
  k->rec.y = k->left.v1.y - k->rec.height;
}

void input_handler(Vector2 *position) {

  float speed = 5;
  float velocity = 1;
  float friction = 100;

  if (IsKeyDown(KEY_J) || IsKeyDown(KEY_DOWN)) {
    position->y += speed * velocity;
    if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT))
      position->x += speed * velocity;
    if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT))
      position->x -= speed * velocity;

  } else if (IsKeyDown(KEY_K) || IsKeyDown(KEY_UP)) {
    position->y -= speed * velocity;
    if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT))
      position->x += speed * velocity;
    if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT))
      position->x -= speed * velocity;

  } else if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT)) {
    position->x -= speed * velocity;
  } else if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT)) {
    position->x += speed * velocity;
  }

  if (velocity <= speed / friction) {
    velocity *= velocity;
  }
}

int main(void) {

  const char *title = "TEAM KITE BALLETT CHOREOGRAPHER";

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, title);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  // SetTargetFPS(1);
  SetTargetFPS(60);

  Vector2 pos = {GetScreenWidth() / 2.f, GetScreenHeight() / 2.f};
  Vector2 pos1 = {GetScreenWidth() / 2.f, GetScreenHeight() / 2.f};
  Vector2 pos2 = {GetScreenWidth() / 3.f, GetScreenHeight() / 4.f};

  Kite kite = {0};
  Kite kite1 = {0};
  kite_init(&kite, pos);
  kite_init(&kite1, pos1);
  kite1.body_color = RED;
  // TODO: Find a reasonable velocity

  printf("The POS:" VECTOR2_FMT "\n", Vector2_FMT_ARGS(pos));
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(SKYBLUE);

    input_handler(&pos);

    // draw_kite(&kite1, &pos, 270);
    // draw_kite(&kite, &pos1, 50);
    // draw_kite(&kite, &pos2, 90);

    draw_kite(&kite, &pos1, 0);
    draw_kite(&kite1, &pos, 180);
    EndDrawing();
  };

  CloseWindow();
  return 0;
}
