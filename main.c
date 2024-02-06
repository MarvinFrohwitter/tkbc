#include "raylib.h"
#include "tkbc.h"
#include <complex.h>
#include <math.h>
#include <stdio.h>

int main(void) {

  const char *title = "TEAM KITE BALLETT CHOREOGRAPHER";

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, title);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(60);
  Vector2 pos_up;
  Vector2 pos = {GetScreenWidth() / 2.f, GetScreenHeight() / 2.f};
  printf("The POS:" VECTOR2_FMT "\n", Vector2_FMT_ARGS(pos));

  Kite kite = {0};
  Kite kite1 = {0};
  kite_init(&kite);
  kite_init(&kite1);
  kite1.body_color = RED;
  kite.center.y += kite.height + 10;

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(SKYBLUE);

    pos_up = input_handler(&kite);

    draw_kite(&kite, pos_up, 0);
    draw_kite(&kite1, pos, 180);
    EndDrawing();
  };

  CloseWindow();
  return 0;
}

void kite_update(Kite *k, Vector2 position, float center_deg_rotation) {
  k->center.x = position.x;
  k->center.y = position.y;

  float cw = k->width;
  float is = k->inner_space;
  float o = k->overlap;

  // The difference between the angle 0 and the default downward interpolation
  float angle = 42;
  float bl_angle = (PI * (360 - (90 - angle)) / 180);
  float br_angle = (PI * (360 + (90 - angle)) / 180);
  float phi = (PI * (center_deg_rotation) / 180);

  // LEFT Triangle
  // Correct
  k->left.v1.x = k->center.x - ceilf(crealf((cw / 2.f) * cexpf(I * phi)));
  k->left.v1.y = k->center.y + ceilf(cimagf((cw / 2.f) * cexpf(I * phi)));
  k->left.v2.x = k->center.x - ceilf(crealf(is * cexpf(I * (phi - bl_angle))));
  k->left.v2.y = k->center.y + ceilf(cimagf(is * cexpf(I * (phi - bl_angle))));
  k->left.v3.x = k->center.x + ceilf(crealf(o * cexpf(I * phi)));
  k->left.v3.y = k->center.y - ceilf(cimagf(o * cexpf(I * phi)));

  // RIGHT Triangle
  // Correct
  k->right.v1.x = k->center.x - ceilf(crealf(o * cexpf(I * phi)));
  k->right.v1.y = k->center.y + ceilf(cimagf(o * cexpf(I * phi)));
  k->right.v2.x = k->center.x + ceilf(crealf(is * cexpf(I * (phi - br_angle))));
  k->right.v2.y = k->center.y - ceilf(cimagf(is * cexpf(I * (phi - br_angle))));
  k->right.v3.x = k->center.x + ceilf(crealf((cw / 2.f) * cexpf(I * phi)));
  k->right.v3.y = k->center.y - ceilf(cimagf((cw / 2.f) * cexpf(I * phi)));

  // Just an random suitable height and width that fits the scaling and spread.
  k->rec.height = 2 * PI * PI * logf(k->spread * k->spread);
  k->rec.width = cw + k->spread * 2;
  k->rec.x =
      k->center.x - ceilf(crealf((cw / 2.f + k->spread) * cexpf(I * phi)));
  k->rec.y =
      k->center.y + ceilf(cimagf((cw / 2.f + k->spread) * cexpf(I * phi)));
}

void draw_kite(Kite *k, Vector2 position, float center_deg_rotation) {

  kite_update(k, position, center_deg_rotation);
  // Draw a color-filled triangle (vertex in counter-clockwise order!)
  DrawTriangle(k->left.v1, k->left.v2, k->left.v3, k->body_color);
  DrawTriangle(k->right.v1, k->right.v2, k->right.v3, k->body_color);

  // Just use origin 0 and compute the new position for the angel.
  Vector2 origin = {0};
  DrawRectanglePro(k->rec, origin, -center_deg_rotation, k->top_color);
}

void kite_init(Kite *k) {
  k->center.x = 0;
  k->center.y = 0;
  k->center.x = GetScreenWidth() / 2.f;
  k->center.y = GetScreenHeight() / 2.f;

  k->body_color = TEAL;
  k->overlap = 8.f;
  k->inner_space = 20.f;

  k->top_color = DARKGRAY;
  k->spread = 0.2f;

  k->width = 20.0f;
  k->height = 0.0f;
  k->scale = 7.f;

  k->overlap *= k->scale;
  k->inner_space *= k->scale;
  k->spread *= k->scale;
  k->width *= k->scale * 2;

  Vector2 position = {.x = k->center.x, .y = k->center.y};
  kite_update(k, position, 0);

  k->height = fabsf(k->left.v1.y - k->left.v2.y);
}

Vector2 input_handler(Kite *k) {

  // TODO: Find a reasonable velocity
  float speed = 5;
  float velocity = 1;
  float friction = 100;

  if (IsKeyDown(KEY_J) || IsKeyDown(KEY_DOWN)) {
    k->center.y += speed * velocity;
    if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT))
      k->center.x += speed * velocity;
    if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT))
      k->center.x -= speed * velocity;

  } else if (IsKeyDown(KEY_K) || IsKeyDown(KEY_UP)) {
    k->center.y -= speed * velocity;
    if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT))
      k->center.x += speed * velocity;
    if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT))
      k->center.x -= speed * velocity;

  } else if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT)) {
    k->center.x -= speed * velocity;
  } else if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT)) {
    k->center.x += speed * velocity;
  }

  // if (velocity <= speed / friction) {
  //   velocity *= velocity;
  // }
  return k->center;
}
