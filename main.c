#include "raylib.h"
#include "tkbc.h"
#include <assert.h>
#include <complex.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {

  const char *title = "TEAM KITE BALLETT CHOREOGRAPHER";

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, title);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(60);
  Vector2 pos = {GetScreenWidth() / 2.f, GetScreenHeight() / 2.f};
  printf("The POS:" VECTOR2_FMT "\n", Vector2_FMT_ARGS(pos));

  Kite *kite = kite_init();
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(SKYBLUE);

    kite_input_handler(kite);

    kite_draw_kite(kite);
    DrawCircle(kite->center.x, kite->center.y, 10, RED);

    EndDrawing();
  };

  kite_destroy(kite);
  CloseWindow();
  return 0;
}

void kite_tip_rotation(Kite *k, Vector2 *position, float tip_deg_rotation,
                       TIP tip) {

  Vector2 *pos = {0};
  if (position != NULL)
    pos = position;
  else
    pos = &k->center;

  float_t length = (k->width / 2.f + k->spread);
  float phi = (PI * (tip_deg_rotation) / 180);

  switch (tip) {
  case LEFT_TIP: {

    // With out it just flies a circle
    // Just this is bicycle

    // pos->x -= ceilf(length);

    // Move the rotation position to the left tip
    pos->x = k->left.v1.x;
    pos->y = k->left.v1.y;

    // Then rotate
    pos->x += ceilf(crealf((length)*cexpf(I * phi)));
    pos->y -= ceilf(cimagf((length)*cexpf(I * phi)));
  } break;
  case RIGHT_TIP: {

    // pos->x += ceilf(length);

    // Move the rotation position to the right tip
    pos->x = k->right.v3.x;
    pos->y = k->right.v3.y;
    // Then rotate
    pos->x -= ceilf(crealf((length)*cexpf(I * phi)));
    pos->y += ceilf(cimagf((length)*cexpf(I * phi)));

  } break;
  default:
    assert(0 && "The chosen TIP is not vallid!");
    break;
  }

  // Just compute a center rotation instead at the new found position
  kite_center_rotation(k, pos, tip_deg_rotation);
}

void kite_center_rotation(Kite *k, Vector2 *position,
                          float center_deg_rotation) {
  Vector2 *pos = {0};
  if (position != NULL)
    pos = position;
  else
    pos = &k->center;

  k->center.x = pos->x;
  k->center.y = pos->y;

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
  k->left.v1.x = pos->x - ceilf(crealf((cw / 2.f) * cexpf(I * phi)));
  k->left.v1.y = pos->y + ceilf(cimagf((cw / 2.f) * cexpf(I * phi)));
  k->left.v2.x = pos->x - ceilf(crealf(is * cexpf(I * (phi - bl_angle))));
  k->left.v2.y = pos->y + ceilf(cimagf(is * cexpf(I * (phi - bl_angle))));
  k->left.v3.x = pos->x + ceilf(crealf(o * cexpf(I * phi)));
  k->left.v3.y = pos->y - ceilf(cimagf(o * cexpf(I * phi)));

  // RIGHT Triangle
  // Correct
  k->right.v1.x = pos->x - ceilf(crealf(o * cexpf(I * phi)));
  k->right.v1.y = pos->y + ceilf(cimagf(o * cexpf(I * phi)));
  k->right.v2.x = pos->x + ceilf(crealf(is * cexpf(I * (phi - br_angle))));
  k->right.v2.y = pos->y - ceilf(cimagf(is * cexpf(I * (phi - br_angle))));
  k->right.v3.x = pos->x + ceilf(crealf((cw / 2.f) * cexpf(I * phi)));
  k->right.v3.y = pos->y - ceilf(cimagf((cw / 2.f) * cexpf(I * phi)));

  // Just an random suitable height and width that fits the scaling and spread.
  k->rec.height = 2 * PI * PI * logf(k->spread * k->spread);
  k->rec.width = 2 * length;
  k->rec.x = pos->x - ceilf(crealf(length * cexpf(I * phi)));
  k->rec.y = pos->y + ceilf(cimagf(length * cexpf(I * phi)));
}

void kite_draw_kite(Kite *k) {
  Vector2 origin = {0};

  // Draw a color-filled triangle (vertex in counter-clockwise order!)
  DrawTriangle(k->left.v1, k->left.v2, k->left.v3, k->body_color);
  DrawTriangle(k->right.v1, k->right.v2, k->right.v3, k->body_color);
  DrawRectanglePro(k->rec, origin, -k->center_rotation, k->top_color);
}

void kite_destroy(Kite *kite) { free(kite); }
Kite *kite_init() {
  Kite *kite = calloc(1, sizeof(Kite));
  kite->center.x = 0;
  kite->center.y = 0;
  kite->center.x = GetScreenWidth() / 2.f;
  kite->center.y = GetScreenHeight() / 2.f;
  kite->speed = 30;

  kite->body_color = TEAL;
  kite->overlap = 8.f;
  kite->inner_space = 20.f;

  kite->top_color = DARKGRAY;
  kite->spread = 0.2f;

  kite->width = 20.0f;
  kite->height = 0.0f;
  kite->scale = 7.f;
  kite->center_rotation = 0;

  kite->overlap *= kite->scale;
  kite->inner_space *= kite->scale;
  kite->spread *= kite->scale;
  kite->width *= kite->scale * 2;

  kite_center_rotation(kite, NULL, kite->center_rotation);

  kite->height = fabsf(kite->left.v1.y - kite->left.v2.y);
  return kite;
}

int kite_check_boundary(Kite *kite, Orientation orientation) {
  float width = GetScreenWidth();
  float height = GetScreenHeight();
  float x = kite->center.x;
  float y = kite->center.y;
  size_t padding = kite->width / 2;

  switch (orientation) {
  case KITE_X:
    return x < width - padding && x > 0 + padding;
  case KITE_Y:
    return y < height - padding && y > 0 + padding;
  default:
    assert(0 && "UNREACHABLE");
  }
  return 0;
}

bool interrupt = false;
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
    kite_center_rotation(k, NULL, k->center_rotation + 45);
  } else if (IsKeyPressed(KEY_R)) {
    kite_center_rotation(k, NULL, k->center_rotation - 45);
  }

  if (IsKeyPressed(KEY_T) &&
      (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {
    interrupt = true;

    if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT)) {
      kite_tip_rotation(k, NULL, k->center_rotation + 45, RIGHT_TIP);
    }
    if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT)) {
      kite_tip_rotation(k, NULL, k->center_rotation + 45, LEFT_TIP);
    }
  } else if (IsKeyPressed(KEY_T)) {
    interrupt = true;

    if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT)) {
      kite_tip_rotation(k, NULL, k->center_rotation - 45, RIGHT_TIP);
    }
    if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT)) {
      kite_tip_rotation(k, NULL, k->center_rotation - 45, LEFT_TIP);
    }
  }

  // if (!kite_check_boundary(k, KITE_Y)) {
  //   return;
  // }
  // if (!kite_check_boundary(k, KITE_Y)) {
  //   return;
  // }

  // TODO: fixed angel rotations with locking like key F for FIX and otherwise
  // angle smothe with like 1 degre or 5 and do maybe IsKeyReleased fot T,
  // because the locked state can be checked.

  // if (IsKeyReleased(KEY_T)) {
  //   interrupt = false;
  // }

  if (IsKeyUp(KEY_T) && IsKeyUp(KEY_H) && IsKeyUp(KEY_L)) {
    interrupt = false;
  }

  if (interrupt) {
    return;
  }

  if (IsKeyDown(KEY_J) || IsKeyDown(KEY_DOWN)) {
    k->center.y += velocity;
    if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT))
      k->center.x += velocity;
    if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT))
      k->center.x -= velocity;
    kite_center_rotation(k, NULL, k->center_rotation);

  } else if (IsKeyDown(KEY_K) || IsKeyDown(KEY_UP)) {
    k->center.y -= velocity;
    if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT))
      k->center.x += velocity;
    if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT))
      k->center.x -= velocity;
    kite_center_rotation(k, NULL, k->center_rotation);

  } else if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT)) {
    k->center.x -= velocity;
    kite_center_rotation(k, NULL, k->center_rotation);
  } else if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT)) {
    k->center.x += velocity;
    kite_center_rotation(k, NULL, k->center_rotation);
  }
}
