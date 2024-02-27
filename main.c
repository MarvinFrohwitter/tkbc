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

  Image background_image = LoadImage("./assets/raw.png");
  Texture2D background_texture = LoadTextureFromImage(background_image);

  State *state = kite_init();
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(SKYBLUE);

    float scale_width = (float)GetScreenWidth() / background_texture.width;
    float scale_height = (float)GetScreenHeight() / background_texture.height;
    float scale = fmaxf(scale_width, scale_height);
    DrawTextureEx(background_texture, (Vector2){0, 0}, 0, scale, WHITE);
    // DrawTexture(background_texture, 0, 0, WHITE);
    kite_input_handler(state);

    kite_draw_kite(state->kite);
    // DrawCircle(state->kite->center.x, state->kite->center.y, 10, RED);

    EndDrawing();
  };

  kite_destroy(state);

  CloseWindow();
  return 0;
}
void kite_circle_rotation(Kite *k, Vector2 *position, float deg_rotation,
                          TIP tip, bool below) {
  Vector2 *pos = {0};
  if (position != NULL)
    pos = position;
  else
    pos = &k->center;

  // TODO: Change back to full circle size
  // float_t length = k->height;
  float_t length = k->height / 2;
  float phi = (PI * (deg_rotation) / 180);
  float center_angle = 0;
  if (below) {
    center_angle = (PI * (360 - 270) / 180);
  } else {
    center_angle = (PI * (360 - 90) / 180);
  }

  // center rotation point;
  pos->x += ceilf(crealf((length)*cexpf(I * center_angle)));
  pos->y += ceilf(cimagf((length)*cexpf(I * center_angle)));

  switch (tip) {
  case LEFT_TIP: {

    pos->x -= ceilf(crealf((length)*cexpf(I * phi)));
    pos->y += ceilf(cimagf((length)*cexpf(I * phi)));

    kite_center_rotation(k, pos, deg_rotation);
  } break;
  case RIGHT_TIP: {

    pos->x += ceilf(crealf((length)*cexpf(I * phi)));
    pos->y -= ceilf(cimagf((length)*cexpf(I * phi)));

    kite_center_rotation(k, pos, deg_rotation);
  } break;
  default:
    assert(0 && "The chosen TIP is not vallid!");
  }
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

void kite_destroy(State *state) {
  free(state->kite);
  free(state);
}
State *kite_init() {
  State *state = calloc(1, sizeof(State));

  state->iscenter = false;
  state->fixed = true;
  state->interrupt_movement = false;
  state->interrupt_smoothness = false;
  state->velocity = 10;

  state->kite = calloc(1, sizeof(Kite));

  state->kite->center.x = 0;
  state->kite->center.y = 0;
  state->kite->center.x = GetScreenWidth() / 2.f;
  state->kite->center.y = GetScreenHeight() / 2.f;
  state->kite->speed = 30;

  state->kite->body_color = TEAL;
  state->kite->overlap = 8.f;
  state->kite->inner_space = 20.f;

  state->kite->top_color = DARKGRAY;
  state->kite->spread = 0.2f;

  state->kite->width = 20.0f;
  state->kite->height = 0.0f;
  state->kite->scale = 7.f;
  state->kite->center_rotation = 0;

  state->kite->overlap *= state->kite->scale;
  state->kite->inner_space *= state->kite->scale;
  state->kite->spread *= state->kite->scale;
  state->kite->width *= state->kite->scale * 2;

  kite_center_rotation(state->kite, NULL, state->kite->center_rotation);

  state->kite->height = fabsf(state->kite->left.v1.y - state->kite->left.v2.y);
  return state;
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

void kite_input_handler(State *s) {
  s->iscenter = false;
  s->velocity = 10;
  s->velocity *= GetFrameTime();
  s->velocity *= s->kite->speed;

  // Hard reset to top left corner angel 0, position (0,0)
  if (IsKeyDown(KEY_SPACE))
    kite_center_rotation(s->kite, &(CLITERAL(Vector2){.x = 0, .y = 0}), 0);

  if (IsKeyDown(KEY_N))
    kite_center_rotation(s->kite, NULL, 0);

  if (IsKeyUp(KEY_R) && IsKeyUp(KEY_T)) {
    s->interrupt_smoothness = false;
  }
  if (s->interrupt_smoothness) {
    return;
  }

  kite_input_check_speed(s);
  if (IsKeyPressed(KEY_F)) {
    s->fixed = !s->fixed;
    return;
  }

  kite_input_check_rotation(s);
  kite_input_check_tip_turn(s);
  kite_input_check_circle(s);

  if (!s->iscenter) {
    // NOTE: Currently not check for arrow KEY_RIGHT and KEY_LEFT, so that you
    // can still move the kite with no interrupt but with steps of 45 degrees
    // angle.
    if (IsKeyUp(KEY_T) && IsKeyUp(KEY_H) && IsKeyUp(KEY_L)) {
      s->interrupt_movement = false;
    }
  } else {
    s->iscenter = false;
    if (IsKeyUp(KEY_R)) {
      s->interrupt_movement = false;
    }
  }
  if (s->interrupt_movement) {
    return;
  }

  kite_input_check_movement(s);
}

void kite_input_check_rotation(State *s) {

  if (IsKeyDown(KEY_R) &&
      (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {
    s->iscenter = true;

    if (!s->fixed) {
      kite_center_rotation(s->kite, NULL, s->kite->center_rotation + 1);
    } else {
      if (!s->interrupt_smoothness) {
        s->interrupt_movement = true;
        kite_center_rotation(s->kite, NULL, s->kite->center_rotation + 45);
      }
      s->interrupt_smoothness = true;
    }

  } else if (IsKeyDown(KEY_R)) {
    s->iscenter = true;

    if (!s->fixed) {
      kite_center_rotation(s->kite, NULL, s->kite->center_rotation - 1);
    } else {
      if (!s->interrupt_smoothness) {
        s->interrupt_movement = true;
        kite_center_rotation(s->kite, NULL, s->kite->center_rotation - 45);
      }
      s->interrupt_smoothness = true;
    }
  }
}

void kite_input_check_tip_turn(State *s) {
  // TODO: Think about the clamp in terms of a tip rotation
  if (IsKeyDown(KEY_T) &&
      (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {

    if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT)) {

      if (!s->fixed) {
        kite_tip_rotation(s->kite, NULL, s->kite->center_rotation + 1,
                          RIGHT_TIP);
      } else {
        if (!s->interrupt_smoothness) {
          s->interrupt_movement = true;
          kite_tip_rotation(s->kite, NULL, s->kite->center_rotation + 45,
                            RIGHT_TIP);
        }
        s->interrupt_smoothness = true;
      }
    }

    if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT)) {
      if (!s->fixed) {
        kite_tip_rotation(s->kite, NULL, s->kite->center_rotation + 1,
                          LEFT_TIP);
      } else {
        if (!s->interrupt_smoothness) {
          s->interrupt_movement = true;
          kite_tip_rotation(s->kite, NULL, s->kite->center_rotation + 45,
                            LEFT_TIP);
        }
        s->interrupt_smoothness = true;
      }
    }
  } else if (IsKeyDown(KEY_T)) {

    if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT)) {
      if (!s->fixed) {
        kite_tip_rotation(s->kite, NULL, s->kite->center_rotation - 1,
                          RIGHT_TIP);
      } else {
        if (!s->interrupt_smoothness) {
          s->interrupt_movement = true;
          kite_tip_rotation(s->kite, NULL, s->kite->center_rotation - 45,
                            RIGHT_TIP);
        }
        s->interrupt_smoothness = true;
      }
    }
    if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT)) {
      if (!s->fixed) {
        kite_tip_rotation(s->kite, NULL, s->kite->center_rotation - 1,
                          LEFT_TIP);
      } else {
        if (!s->interrupt_smoothness) {
          s->interrupt_movement = true;
          kite_tip_rotation(s->kite, NULL, s->kite->center_rotation - 45,
                            LEFT_TIP);
        }
        s->interrupt_smoothness = true;
      }
    }
  }
}

void kite_input_check_circle(State *s) {
  if (IsKeyPressed(KEY_C) &&
      (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {

    s->interrupt_movement = true;
    if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT)) {

      if (!s->fixed) {
        kite_circle_rotation(s->kite, NULL, s->kite->center_rotation + 1,
                             RIGHT_TIP, false);
      } else {
        if (!s->interrupt_smoothness) {
          s->interrupt_movement = true;
          kite_circle_rotation(s->kite, NULL, s->kite->center_rotation + 45,
                               RIGHT_TIP, false);
        }
        s->interrupt_smoothness = true;
      }
    }

    if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT)) {
      if (!s->fixed) {
        kite_circle_rotation(s->kite, NULL, s->kite->center_rotation - 1,
                             LEFT_TIP, false);
      } else {
        if (!s->interrupt_smoothness) {
          s->interrupt_movement = true;
          kite_circle_rotation(s->kite, NULL, s->kite->center_rotation - 45,
                               LEFT_TIP, false);
        }
        s->interrupt_smoothness = true;
      }
    }
  } else if (IsKeyPressed(KEY_C)) {
    s->interrupt_movement = true;

    if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT)) {
      if (!s->fixed) {
        kite_circle_rotation(s->kite, NULL, s->kite->center_rotation - 1,
                             RIGHT_TIP, true);
      } else {
        if (!s->interrupt_smoothness) {
          s->interrupt_movement = true;
          kite_circle_rotation(s->kite, NULL, s->kite->center_rotation - 45,
                               RIGHT_TIP, true);
        }
        s->interrupt_smoothness = true;
      }
    }
    if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT)) {
      if (!s->fixed) {
        kite_circle_rotation(s->kite, NULL, s->kite->center_rotation + 1,
                             LEFT_TIP, true);
      } else {
        if (!s->interrupt_smoothness) {
          s->interrupt_movement = true;
          kite_circle_rotation(s->kite, NULL, s->kite->center_rotation + 45,
                               LEFT_TIP, true);
        }
        s->interrupt_smoothness = true;
      }
    }
  }
}

void kite_input_check_movement(State *s) {
  int viewport_padding =
      s->kite->width > s->kite->height ? s->kite->width / 2 : s->kite->height;
  Vector2 window = {GetScreenWidth(), GetScreenHeight()};
  window.x -= viewport_padding;
  window.y -= viewport_padding;

  if (IsKeyDown(KEY_J) || IsKeyDown(KEY_DOWN)) {
    s->kite->center.y =
        kite_clamp(s->kite->center.y + s->velocity, viewport_padding, window.y);
    if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT))
      s->kite->center.x = kite_clamp(s->kite->center.x + s->velocity,
                                     viewport_padding, window.x);
    if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT))
      s->kite->center.x = kite_clamp(s->kite->center.x - s->velocity,
                                     viewport_padding, window.x);

    kite_center_rotation(s->kite, NULL, s->kite->center_rotation);

  } else if (IsKeyDown(KEY_K) || IsKeyDown(KEY_UP)) {
    s->kite->center.y =
        kite_clamp(s->kite->center.y - s->velocity, viewport_padding, window.y);
    if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT))
      s->kite->center.x = kite_clamp(s->kite->center.x + s->velocity,
                                     viewport_padding, window.x);
    if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT))
      s->kite->center.x = kite_clamp(s->kite->center.x - s->velocity,
                                     viewport_padding, window.x);
    kite_center_rotation(s->kite, NULL, s->kite->center_rotation);

  } else if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT)) {
    s->kite->center.x =
        kite_clamp(s->kite->center.x - s->velocity, viewport_padding, window.x);
    kite_center_rotation(s->kite, NULL, s->kite->center_rotation);
  } else if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT)) {
    s->kite->center.x =
        kite_clamp(s->kite->center.x + s->velocity, viewport_padding, window.x);
    kite_center_rotation(s->kite, NULL, s->kite->center_rotation);
  }
}

void kite_input_check_speed(State *s) {
  if (IsKeyDown(KEY_P) &&
      (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {
    if (s->kite->speed > 0) {
      s->kite->speed -= 1;
    }
  } else if (IsKeyDown(KEY_P)) {
    if (s->kite->speed <= 100) {
      s->kite->speed += 1;
    }
  }
}

float kite_clamp(float z, float a, float b) {

  float s = z < a ? a : z;
  return s < b ? s : b;
}
