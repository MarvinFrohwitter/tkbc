#include "raylib.h"
#include "tkbc.h"
#include <assert.h>
#include <complex.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "./tkbc_scripts/first.tkb.c"
Wave kite_wave;
Sound kite_sound;

int main(void) {

  const char *title = "TEAM KITE BALLETT CHOREOGRAPHER";

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, title);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(TARGET_FPS);
  SetExitKey(KEY_ESCAPE);
  Vector2 pos = {GetScreenWidth() / 2.f, GetScreenHeight() / 2.f};
  printf("The POS:" VECTOR2_FMT "\n", Vector2_FMT_ARGS(pos));

  Image background_image = LoadImage("./assets/raw.png");
  Texture2D background_texture = LoadTextureFromImage(background_image);

  InitAudioDevice();
  if (IsAudioDeviceReady()) {
    SetMasterVolume(40);
  }
  kite_wave = LoadWave("./assets/Quest-of-Power.mp3");
  if (IsWaveReady(kite_wave)) {
    kite_sound = LoadSoundFromWave(kite_wave);
  }

  State *state = kite_init();
  state->interrupt_script = false;
  while (!WindowShouldClose()) {

    if (state->interrupt_script) {
      BeginDrawing();
      ClearBackground(SKYBLUE);
      // Factor out the kite_script_begin and kite_script_end functions to the
      // key-handler.
      kite_script_begin(state);

      kite_script_input(state);
      kite_draw_kite(state->kite);

      if (state->instruction_counter >= state->instruction_count) {
        kite_script_end(state);
      }

      DrawFPS(pos.x, 10);
      EndDrawing();
    } else {
      BeginDrawing();

      float scale_width = (float)GetScreenWidth() / background_texture.width;
      float scale_height = (float)GetScreenHeight() / background_texture.height;
      float scale = fmaxf(scale_width, scale_height);
      DrawTextureEx(background_texture, (Vector2){0, 0}, 0, scale, WHITE);

      kite_draw_kite(state->kite);
      DrawFPS(pos.x, 10);
      EndDrawing();
    }

    kite_input_handler(state);
  };

  kite_destroy(state);
  StopSound(kite_sound);
  UnloadSound(kite_sound);
  UnloadWave(kite_wave);
  CloseAudioDevice();
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
    assert(0 && "The chosen TIP is not valid!");
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
    assert(0 && "The chosen TIP is not valid!");
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
  // k->rec.height = 2 * PI * PI * logf(k->spread * k->spread);
  // k->rec.height = 2 * PI * k->spread;
  k->rec.height = 2 * PI * logf(k->scale);
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

  state->fly_velocity = 10;
  state->turn_velocity = 1;
  state->iscenter = false;
  state->fixed = true;
  state->interrupt_movement = false;
  state->interrupt_smoothness = false;
  state->interrupt_script = true;
  state->instruction_counter = 0;

  state->kite = calloc(1, sizeof(Kite));

  state->kite->center.x = 0;
  state->kite->center.y = 0;
  state->kite->center.x = GetScreenWidth() / 2.f;
  state->kite->center.y = GetScreenHeight() / 2.f;
  state->kite->fly_speed = 30;
  state->kite->turn_speed = 30;

  state->kite->body_color = TEAL;
  state->kite->overlap = 8.f;
  state->kite->inner_space = 20.f;

  state->kite->top_color = DARKGRAY;
  state->kite->spread = 0.2f;

  state->kite->width = 20.0f;
  state->kite->height = 0.0f;
  // state->kite->scale = 7.f;
  state->kite->scale = 4.f;
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
  s->fly_velocity = 10;
  s->turn_velocity = 1;

  s->turn_velocity *= GetFrameTime();
  s->turn_velocity *= s->kite->turn_speed;
  s->fly_velocity *= GetFrameTime();
  s->fly_velocity *= s->kite->fly_speed;

  kite_sound_handler();

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
      kite_center_rotation(s->kite, NULL,
                           s->kite->center_rotation + 1 + s->turn_velocity);
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
      kite_center_rotation(s->kite, NULL,
                           s->kite->center_rotation - 1 - s->turn_velocity);
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
        kite_tip_rotation(s->kite, NULL,
                          s->kite->center_rotation + 1 + s->turn_velocity,
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
        kite_tip_rotation(s->kite, NULL,
                          s->kite->center_rotation + 1 + s->turn_velocity,
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
        kite_tip_rotation(s->kite, NULL,
                          s->kite->center_rotation - 1 - s->turn_velocity,
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
        kite_tip_rotation(s->kite, NULL,
                          s->kite->center_rotation - 1 - s->turn_velocity,
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
        kite_circle_rotation(s->kite, NULL,
                             s->kite->center_rotation + 1 + s->turn_velocity,
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
        kite_circle_rotation(s->kite, NULL,
                             s->kite->center_rotation - 1 - s->turn_velocity,
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
        kite_circle_rotation(s->kite, NULL,
                             s->kite->center_rotation - 1 - s->turn_velocity,
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
        kite_circle_rotation(s->kite, NULL,
                             s->kite->center_rotation + 1 + s->turn_velocity,
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
    s->kite->center.y = kite_clamp(s->kite->center.y + s->fly_velocity,
                                   viewport_padding, window.y);
    if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT))
      s->kite->center.x = kite_clamp(s->kite->center.x + s->fly_velocity,
                                     viewport_padding, window.x);
    if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT))
      s->kite->center.x = kite_clamp(s->kite->center.x - s->fly_velocity,
                                     viewport_padding, window.x);

    kite_center_rotation(s->kite, NULL, s->kite->center_rotation);

  } else if (IsKeyDown(KEY_K) || IsKeyDown(KEY_UP)) {
    s->kite->center.y = kite_clamp(s->kite->center.y - s->fly_velocity,
                                   viewport_padding, window.y);
    if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT))
      s->kite->center.x = kite_clamp(s->kite->center.x + s->fly_velocity,
                                     viewport_padding, window.x);
    if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT))
      s->kite->center.x = kite_clamp(s->kite->center.x - s->fly_velocity,
                                     viewport_padding, window.x);
    kite_center_rotation(s->kite, NULL, s->kite->center_rotation);

  } else if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT)) {
    s->kite->center.x = kite_clamp(s->kite->center.x - s->fly_velocity,
                                   viewport_padding, window.x);
    kite_center_rotation(s->kite, NULL, s->kite->center_rotation);
  } else if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT)) {
    s->kite->center.x = kite_clamp(s->kite->center.x + s->fly_velocity,
                                   viewport_padding, window.x);
    kite_center_rotation(s->kite, NULL, s->kite->center_rotation);
  }
}

void kite_input_check_speed(State *s) {

  if (IsKeyDown(KEY_P) &&
      (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {
    if (s->kite->fly_speed > 0) {
      s->kite->fly_speed -= 1;
    }
  } else if (IsKeyDown(KEY_P)) {
    if (s->kite->fly_speed <= 100) {
      s->kite->fly_speed += 1;
    }
  }

  if (IsKeyDown(KEY_O) &&
      (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {
    if (s->kite->turn_speed > 0) {
      s->kite->turn_speed -= 1;
    }
  } else if (IsKeyDown(KEY_O)) {
    if (s->kite->turn_speed <= 100) {
      s->kite->turn_speed += 1;
    }
  }
}

float kite_clamp(float z, float a, float b) {

  float s = z < a ? a : z;
  return s < b ? s : b;
}

// ===========================================================================
// ========================== Sound Handler ==================================
// ===========================================================================

void kite_sound_handler() {

  if (IsKeyPressed(KEY_S) &&
      (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {
    StopSound(kite_sound);
  } else if (IsKeyPressed(KEY_S)) {
    PlaySound(kite_sound);
  } else if (IsKeyPressed(KEY_M) &&
             (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {
    ResumeSound(kite_sound);
  } else if (IsKeyPressed(KEY_M)) {
    PauseSound(kite_sound);
  }
}

// ===========================================================================
// ========================== Script Handler =================================
// ===========================================================================

void kite_script_begin(State *state) {
  state->interrupt_script = true;
  SetTargetFPS(1);
}
void kite_script_end(State *state) {
  state->interrupt_script = false;
  SetTargetFPS(TARGET_FPS);
}

void kite_script_move(Kite *kite, float steps_x, float steps_y,
                      PARAMETERS parameters) {

  Vector2 pos = {0};

  switch (parameters) {
  case FIXED: {
  final_pos:
    pos.x = steps_x;
    pos.y = steps_y;
    kite_center_rotation(kite, &pos, 0);
    kite_draw_kite(kite);
  } break;
  case SMOOTH: {

    size_t maxiter = fmaxf(fabsf(steps_x), fabsf(steps_y));
    float iter_space_x = fabsf(steps_x) / maxiter;
    float iter_space_y = fabsf(steps_x) / maxiter;

    // TODO: What we do if the x and y are different
    assert(steps_x == steps_y);
    for (size_t i = 0; i < maxiter; ++i) {

      if (floorf(pos.x) != steps_x) {
        pos.x = steps_x > 0 ? pos.x + iter_space_x : pos.x - iter_space_x;
      }

      if (floorf(pos.y) != steps_y) {
        pos.y = steps_y > 0 ? pos.y + iter_space_y : pos.y - iter_space_y;
      }
      if (steps_x == 0)
        pos.x = steps_x;
      if (steps_y == 0)
        pos.y = steps_y;

      kite_center_rotation(kite, &pos, 0);
      kite_draw_kite(kite);
    }

    // Just in case the rounding of the iterations is not enough to reach the
    // final position.
    goto final_pos;

  } break;
  default:
    assert(0 && "ERROR: kite_script_move: UNREACHABLE");
  }
}

void kite_script_rotate(Kite *kite, float angle, PARAMETERS parameters) {

  switch (parameters) {
  case FIXED: {
    kite_center_rotation(kite, NULL, angle);
    kite_draw_kite(kite);
  } break;
  case SMOOTH: {
    if (angle < 0) {
      for (size_t i = 0; i >= angle; --i) {
        kite_center_rotation(kite, NULL, kite->center_rotation + i);
        kite_draw_kite(kite);
      }
    } else {
      for (size_t i = 0; i <= angle; ++i) {
        kite_center_rotation(kite, NULL, kite->center_rotation + i);
        kite_draw_kite(kite);
      }
    }

    // Just in case because we accept floats that could potentially be not an
    // integer. Draw the rest of the rotation.
    kite_center_rotation(kite, NULL, angle);
    kite_draw_kite(kite);

  } break;
  default:
    assert(0 && "ERROR: kite_script_rotate: UNREACHABLE");
  }
}

void kite_script_rotate_tip(Kite *kite, TIP tip, float angle,
                            PARAMETERS parameters) {

  switch (parameters) {
  case FIXED: {
    switch (tip) {
    case LEFT_TIP:
    case RIGHT_TIP:
      kite_tip_rotation(kite, NULL, angle, tip);
      kite_draw_kite(kite);
      break;
    default:
      assert(0 && "ERROR: kite_script_rotate_tip: FIXED: UNREACHABLE");
    }

  } break;
  case SMOOTH: {
    switch (tip) {
    case LEFT_TIP:
    case RIGHT_TIP:
      if (angle < 0) {
        for (size_t i = 0; i >= angle; --i) {
          kite_tip_rotation(kite, NULL, kite->center_rotation + angle, tip);
          kite_draw_kite(kite);
        }
      } else {
        for (size_t i = 0; i <= angle; ++i) {
          kite_tip_rotation(kite, NULL, kite->center_rotation + angle, tip);
          kite_draw_kite(kite);
        }
      }
      break;
    default:
      assert(0 && "ERROR: kite_script_rotate_tip: SMOOTH: UNREACHABLE");
    }

    // Just in case because we accept floats that could potentially be not an
    // integer. Draw the rest of the rotation.
    kite_tip_rotation(kite, NULL, angle, tip);
    kite_draw_kite(kite);

  } break;
  default:
    assert(0 && "ERROR: kite_script_rotate_tip: UNREACHABLE");
  }
}
