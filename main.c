#include "raylib.h"
#include "tkbc.h"
#include <assert.h>
#include <complex.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./tkbc_scripts/first.tkb.c"
Sound kite_sound;
#define KITE_ARRAY_LEN 4
State kite_array[KITE_ARRAY_LEN];

void kite_array_start_pos() {
  int kite_width = kite_array[0].kite->width;
  int kite_heigt = kite_array[0].kite->height;

  int viewport_padding = kite_width > kite_heigt ? kite_width / 2 : kite_heigt;

  Vector2 start_pos = {.y = GetScreenHeight() - 2 * viewport_padding,
                       .x = GetScreenWidth() / 2.0f -
                            KITE_ARRAY_LEN * kite_width + kite_width / 2.0f};

  for (size_t i = 0; i < KITE_ARRAY_LEN; ++i) {
    kite_set_state_defaults(&kite_array[i]);
    kite_set_kite_defaults(kite_array[i].kite, false);

    kite_center_rotation(kite_array[i].kite, &start_pos, 0);
    start_pos.x += 2 * kite_width;

    kite_array[i].kite_input_handler_active = false;
  }
}

void kite_gen_kites(State *s1, State *s2, State *s3, State *s4) {
  s1 = kite_init();
  s2 = kite_init();
  s3 = kite_init();
  s4 = kite_init();
  kite_array[0] = *s1;
  kite_array[1] = *s2;
  kite_array[2] = *s3;
  kite_array[3] = *s4;

  kite_custom_slide();
  // kite_array_start_pos();

  s1->kite->body_color = BLUE;
  s2->kite->body_color = GREEN;
  s3->kite->body_color = PURPLE;
  s4->kite->body_color = RED;
}

/**
 * @brief The function kite_array_destroy_kites() frees all the kites that are
 * currently in the global kite_array.
 */
void kite_array_destroy_kites() {
  for (size_t i = 0; i < KITE_ARRAY_LEN; ++i) {
    kite_destroy(&kite_array[i]);
  }
}

/**
 * @brief The function kite_draw_kite_array() draws every kite with its
 * corresponding position on the canvas.
 */
void kite_draw_kite_array() {
  for (size_t i = 0; i < KITE_ARRAY_LEN; ++i) {
    kite_draw_kite(kite_array[i].kite);
  }
}

/**
 * @brief The function kite_array_check_interrupt_script() checks if one of the
 * kites that are currently in the kite_array has interrupt_script set to true.
 *
 * @return boolean Returns true if one of the kites of the global kite_array has
 * set the value interrupt_script, otherwise false.
 */
bool kite_array_check_interrupt_script() {
  for (size_t i = 0; i < KITE_ARRAY_LEN; ++i) {
    if (kite_array[i].interrupt_script) {
      return true;
    }
  }
  return false;
}

void kite_array_input_handler() {

  if (IsKeyPressed(KEY_ONE)) {
    kite_array[0].kite_input_handler_active =
        !kite_array[0].kite_input_handler_active;
  } else if (IsKeyPressed(KEY_TWO)) {
    kite_array[1].kite_input_handler_active =
        !kite_array[1].kite_input_handler_active;
  } else if (IsKeyPressed(KEY_THREE)) {
    kite_array[2].kite_input_handler_active =
        !kite_array[2].kite_input_handler_active;
  } else if (IsKeyPressed(KEY_FOUR)) {
    kite_array[3].kite_input_handler_active =
        !kite_array[3].kite_input_handler_active;
  }

  for (size_t i = 0; i < KITE_ARRAY_LEN; ++i) {
    kite_input_handler(&kite_array[i]);
  }
}

int main(void) {

  const char *title = "TEAM KITE BALLETT CHOREOGRAPHER";
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, title);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(TARGET_FPS);
  SetExitKey(KEY_ESCAPE);
  Vector2 pos = {GetScreenWidth() / 2.f, GetScreenHeight() / 2.f};
  printf("The POS:" VECTOR2_FMT "\n", Vector2_FMT_ARGS(pos));

  Image background_image =
      LoadImage("/home/marvin/Entwicklung/c/tkbc/src/assets/raw.png");
  Texture2D background_texture = LoadTextureFromImage(background_image);

  InitAudioDevice();
  if (IsAudioDeviceReady()) {
    SetMasterVolume(40);
  }

  State kite_criss = {0};
  State kite_tim = {0};
  State kite_heiner = {0};
  State kite_marvin = {0};
  kite_gen_kites(&kite_criss, &kite_tim, &kite_heiner, &kite_marvin);

  while (!WindowShouldClose()) {

    if (kite_array_check_interrupt_script()) {
      BeginDrawing();
      ClearBackground(SKYBLUE);
      // Factor out the kite_script_begin and kite_script_end functions to the
      // key-handler.
      kite_script_begin(&kite_marvin);

      kite_script_input(&kite_marvin);
      kite_draw_kite(kite_marvin.kite);

      if (kite_marvin.instruction_counter >= kite_marvin.instruction_count) {
        kite_script_end(&kite_marvin);
      }

      DrawFPS(pos.x, 10);
      EndDrawing();
    } else {
      BeginDrawing();
      float scale_width = (float)GetScreenWidth() / background_texture.width;
      float scale_height = (float)GetScreenHeight() / background_texture.height;
      float scale = fmaxf(scale_width, scale_height);
      DrawTextureEx(background_texture, (Vector2){0, 0}, 0, scale, WHITE);

      kite_draw_kite_array();
      DrawFPS(pos.x, 10);
      EndDrawing();
    }

    if (IsFileDropped()) {
      FilePathList file_path_list = LoadDroppedFiles();
      for (size_t i = 0; i < file_path_list.count && i < 1; ++i) {
        char *file_path = file_path_list.paths[i];
        fprintf(stderr, "ERROR: FILE: PATH :MUSIC: %s\n", file_path);
        kite_sound = LoadSound(file_path);
      }
      UnloadDroppedFiles(file_path_list);
    }
    kite_sound_handler();
    kite_array_input_handler();
  };

  kite_array_destroy_kites();
  StopSound(kite_sound);
  UnloadSound(kite_sound);
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

void kite_set_state_defaults(State *state) {

  state->kite_input_handler_active = false;
  state->fly_velocity = 10;
  state->turn_velocity = 10;
  state->iscenter = false;
  state->fixed = true;
  state->interrupt_movement = false;
  state->interrupt_smoothness = false;
  state->interrupt_script = false;
  state->instruction_counter = 0;
  state->instruction_count = 0;
}

void kite_set_kite_defaults(Kite *kite, bool is_generated) {
  kite->center.x = 0;
  kite->center.y = 0;

  kite->center.x = GetScreenWidth() / 2.f;
  kite->center.y = GetScreenHeight() / 2.f;

  kite->fly_speed = 30;
  kite->turn_speed = 30;

  if (is_generated) {
    kite->body_color = TEAL;
  }
  kite->overlap = 8.f;
  kite->inner_space = 20.f;

  kite->top_color = DARKGRAY;
  kite->spread = 0.2f;

  kite->width = 20.0f;
  kite->height = 0.0f;
  kite->scale = 4.f;
  kite->center_rotation = 0;

  kite->overlap *= kite->scale;
  kite->inner_space *= kite->scale;
  kite->spread *= kite->scale;
  kite->width *= kite->scale * 2;

  kite_center_rotation(kite, NULL, kite->center_rotation);

  kite->height = fabsf(kite->left.v1.y - kite->left.v2.y);
}

State *kite_init() {
  State *state = calloc(1, sizeof(State));

  kite_set_state_defaults(state);
  state->kite = calloc(1, sizeof(Kite));
  kite_set_kite_defaults(state->kite, true);

  int viewport_padding = state->kite->width > state->kite->height
                             ? state->kite->width / 2
                             : state->kite->height;

  Vector2 start_pos = {.y = GetScreenHeight() - 2 * viewport_padding,
                       .x = state->kite->center.x};
  kite_center_rotation(state->kite, &start_pos, state->kite->center_rotation);
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
  if (!s->kite_input_handler_active) {
    return;
  }
  s->iscenter = false;
  s->fly_velocity = 10;
  s->turn_velocity = 1;

  s->turn_velocity *= GetFrameTime();
  s->turn_velocity *= s->kite->turn_speed;
  s->fly_velocity *= GetFrameTime();
  s->fly_velocity *= s->kite->fly_speed;

  // Hard reset to top left corner angel 0, position (0,0)
  if (IsKeyDown(KEY_SPACE))
    kite_array_start_pos();

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

  kite_input_check_mouse(s);

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

void kite_input_check_mouse(State *s) {
  Vector2 mouse_pos = GetMousePosition();

  if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
    kite_center_rotation(s->kite, &mouse_pos, s->kite->center_rotation);
  } else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    kite_center_rotation(s->kite, &mouse_pos, s->kite->center_rotation);
  }
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
  } break;
  case SMOOTH: {
    if (angle < 0) {
      for (size_t i = 0; i >= angle; --i) {
        kite_center_rotation(kite, NULL, kite->center_rotation + i);
      }
    } else {
      for (size_t i = 0; i <= angle; ++i) {
        kite_center_rotation(kite, NULL, kite->center_rotation + i);
      }
    }

    // Just in case because we accept floats that could potentially be not an
    // integer. Draw the rest of the rotation.
    kite_center_rotation(kite, NULL, angle);

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
        }
      } else {
        for (size_t i = 0; i <= angle; ++i) {
          kite_tip_rotation(kite, NULL, kite->center_rotation + angle, tip);
        }
      }
      break;
    default:
      assert(0 && "ERROR: kite_script_rotate_tip: SMOOTH: UNREACHABLE");
    }

    // Just in case because we accept floats that could potentially be not an
    // integer. Draw the rest of the rotation.
    kite_tip_rotation(kite, NULL, angle, tip);

  } break;
  default:
    assert(0 && "ERROR: kite_script_rotate_tip: UNREACHABLE");
  }
}
