#include "raylib.h"
#include "tkbc.h"
#include <assert.h>
#include <complex.h>
#include <math.h>
#include <stdio.h>

int main(void) {

  const char *title = "TEAM KITE BALLETT CHOREOGRAPHER";

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, title);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(60);
  Vector2 pos = {GetScreenWidth() / 2.f, GetScreenHeight() / 2.f};
  printf("The POS:" VECTOR2_FMT "\n", Vector2_FMT_ARGS(pos));

  Kite kite = {0};
  kite_init(&kite);
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(SKYBLUE);

    // kite_input_handler(&kite);
    DrawCircle(kite.center.x, kite.center.y, 10, RED);
    // TODO: The tip rotation function dose not update the center pos.
    kite_tip_rotation(&kite, pos, 70, LEFT_TIP);
    kite_draw_kite(&kite);

    EndDrawing();
  };

  CloseWindow();
  return 0;
}

void kite_tip_rotation(Kite *k, Vector2 position, float tip_deg_rotation,
                       TIP tip) {
  float_t length = (k->width / 2.f + k->spread);
  float phi = (PI * (tip_deg_rotation) / 180);

  switch (tip) {
  case LEFT_TIP: {

    // Move the rotation position to the left tip
    position.x -= ceilf(length);
    // // Then rotate
    position.x += ceilf(crealf((length)*cexpf(I * phi)));
    position.y -= ceilf(cimagf((length)*cexpf(I * phi)));
  } break;
  case RIGHT_TIP: {

    // Move the rotation position to the right tip
    position.x += ceilf(length);
    // // Then rotate
    position.x -= ceilf(crealf((length)*cexpf(I * phi)));
    position.y += ceilf(cimagf((length)*cexpf(I * phi)));

  } break;
  default:
    assert(0 && "The chosen TIP is not vallid!");
    break;
  }

  // Just compute a center rotation instead at the new found position
  kite_center_rotation(k, position, tip_deg_rotation);
}

void kite_center_rotation(Kite *k, Vector2 position,
                          float center_deg_rotation) {
  // TODO: update is needed.
  // k->center.x = position.x;
  // k->center.y = position.y;

  k->center_rotation = center_deg_rotation;
  float cw = k->width;
  float is = k->inner_space;
  float o = k->overlap;
  float_t length = (k->width / 2.f + k->spread);

  // The difference between the angle 0 and the default downward interpolation
  float angle = 42;
  float bl_angle = (PI * (360 - (90 - angle)) / 180);
  float br_angle = (PI * (360 + (90 - angle)) / 180);
  float phi = (PI * (k->center_rotation) / 180);

  // LEFT Triangle
  // Correct
  k->left.v1.x = position.x - ceilf(crealf((cw / 2.f) * cexpf(I * phi)));
  k->left.v1.y = position.y + ceilf(cimagf((cw / 2.f) * cexpf(I * phi)));
  k->left.v2.x = position.x - ceilf(crealf(is * cexpf(I * (phi - bl_angle))));
  k->left.v2.y = position.y + ceilf(cimagf(is * cexpf(I * (phi - bl_angle))));
  k->left.v3.x = position.x + ceilf(crealf(o * cexpf(I * phi)));
  k->left.v3.y = position.y - ceilf(cimagf(o * cexpf(I * phi)));

  // RIGHT Triangle
  // Correct
  k->right.v1.x = position.x - ceilf(crealf(o * cexpf(I * phi)));
  k->right.v1.y = position.y + ceilf(cimagf(o * cexpf(I * phi)));
  k->right.v2.x = position.x + ceilf(crealf(is * cexpf(I * (phi - br_angle))));
  k->right.v2.y = position.y - ceilf(cimagf(is * cexpf(I * (phi - br_angle))));
  k->right.v3.x = position.x + ceilf(crealf((cw / 2.f) * cexpf(I * phi)));
  k->right.v3.y = position.y - ceilf(cimagf((cw / 2.f) * cexpf(I * phi)));

  // Just an random suitable height and width that fits the scaling and spread.
  k->rec.height = 2 * PI * PI * logf(k->spread * k->spread);
  k->rec.width = 2 * length;
  k->rec.x = position.x - ceilf(crealf(length * cexpf(I * phi)));
  k->rec.y = position.y + ceilf(cimagf(length * cexpf(I * phi)));
}

void kite_draw_kite(Kite *k) {
  Vector2 origin = {0};

  // Draw a color-filled triangle (vertex in counter-clockwise order!)
  DrawTriangle(k->left.v1, k->left.v2, k->left.v3, k->body_color);
  DrawTriangle(k->right.v1, k->right.v2, k->right.v3, k->body_color);
  DrawRectanglePro(k->rec, origin, -k->center_rotation, k->top_color);
}

void kite_init(Kite *k) {
  k->center.x = 0;
  k->center.y = 0;
  k->center.x = GetScreenWidth() / 2.f;
  k->center.y = GetScreenHeight() / 2.f;
  k->speed = 30;

  k->body_color = TEAL;
  k->overlap = 8.f;
  k->inner_space = 20.f;

  k->top_color = DARKGRAY;
  k->spread = 0.2f;

  k->width = 20.0f;
  k->height = 0.0f;
  k->scale = 7.f;
  k->center_rotation = 0;

  k->overlap *= k->scale;
  k->inner_space *= k->scale;
  k->spread *= k->scale;
  k->width *= k->scale * 2;

  kite_center_rotation(k, k->center, k->center_rotation);

  k->height = fabsf(k->left.v1.y - k->left.v2.y);
}

void kite_input_handler(Kite *k) {
  float velocity = 10;
  velocity *= GetFrameTime();
  velocity *= k->speed;

  if (IsKeyDown(KEY_P) &&
      (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {
    if (k->speed > 0) {
      k->speed -= 1;
    }
  } else if (IsKeyDown(KEY_P)) {
    if (k->speed <= 100) {
      k->speed += 1;
    }
  }

  if (IsKeyPressed(KEY_R) &&
      (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {
    kite_center_rotation(k, k->center, k->center_rotation + 45);
    return;
  } else if (IsKeyPressed(KEY_R)) {
    kite_center_rotation(k, k->center, k->center_rotation - 45);
    return;
  }

  if (IsKeyPressed(KEY_T) &&
      (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {
    kite_tip_rotation(k, k->center, k->center_rotation + 45, LEFT_TIP);
    return;
  } else if (IsKeyPressed(KEY_T)) {
    kite_tip_rotation(k, k->center, k->center_rotation - 45, LEFT_TIP);
    return;
  }

  if (IsKeyDown(KEY_J) || IsKeyDown(KEY_DOWN)) {
    k->center.y += velocity;
    if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT))
      k->center.x += velocity;
    if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT))
      k->center.x -= velocity;
    kite_center_rotation(k, k->center, k->center_rotation);

  } else if (IsKeyDown(KEY_K) || IsKeyDown(KEY_UP)) {
    k->center.y -= velocity;
    if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT))
      k->center.x += velocity;
    if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT))
      k->center.x -= velocity;
    kite_center_rotation(k, k->center, k->center_rotation);

  } else if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT)) {
    k->center.x -= velocity;
    kite_center_rotation(k, k->center, k->center_rotation);
  } else if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT)) {
    k->center.x += velocity;
    kite_center_rotation(k, k->center, k->center_rotation);
  }
}
