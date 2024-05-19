#include <assert.h>
#include <complex.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"
#include "tkbc.h"

/**
 * @brief The function kite_circle_rotation() computes the rotation as a ball
 * below and above.
 *
 * @param kite The kite for which the calculation will happen.
 * @param position The new position for the kite at the center of the leading
 * edge or NULL for internal center position of the kite structure.
 * @param deg_rotation The kite rotation in degrees.
 * @param tip The tip that will be chosen to calculate the beginning of the
 * circle.
 * @param below The area where the rotation will happen.
 */
void kite_circle_rotation(Kite *kite, Vector2 *position, float deg_rotation,
                          TIP tip, bool below) {
  Vector2 *pos = {0};

  if (position != NULL)
    pos = position;
  else
    pos = &kite->center;

  // TODO: Change back to full circle size
  // float_t length = k->height;
  float_t length = kite->height / 2;
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

    kite_center_rotation(kite, pos, deg_rotation);
  } break;
  case RIGHT_TIP: {

    pos->x += ceilf(crealf((length)*cexpf(I * phi)));
    pos->y -= ceilf(cimagf((length)*cexpf(I * phi)));

    kite_center_rotation(kite, pos, deg_rotation);
  } break;
  default:
    assert(0 && "The chosen TIP is not valid!");
  }
}

/**
 * @brief The function kite_tip_rotation() computes the new position of the kite
 * and its corresponding structure values with a tip rotation.
 *
 * @param kite The kite that is going to be modified.
 * @param position The new position for the kite at the center of the leading
 * edge or NULL for internal center position of the kite structure.
 * @param tip_deg_rotation The angle in degrees.
 * angle.
 * @param tip The tip chosen left or right around where the kite is turning.
 */
void kite_tip_rotation(Kite *kite, Vector2 *position, float tip_deg_rotation,
                       TIP tip) {

  Vector2 *pos = {0};
  if (position != NULL)
    pos = position;
  else
    pos = &kite->center;

  float_t length = (kite->width / 2.f + kite->spread);
  float phi = (PI * (tip_deg_rotation) / 180);

  switch (tip) {
  case LEFT_TIP: {

    // With out it just flies a circle
    // pos->x -= ceilf(length);

    // Move the rotation position to the left tip
    pos->x = kite->left.v1.x;
    pos->y = kite->left.v1.y;

    // Then rotate
    pos->x += ceilf(crealf((length)*cexpf(I * phi)));
    pos->y -= ceilf(cimagf((length)*cexpf(I * phi)));
  } break;
  case RIGHT_TIP: {

    // pos->x += ceilf(length);

    // Move the rotation position to the right tip
    pos->x = kite->right.v3.x;
    pos->y = kite->right.v3.y;
    // Then rotate
    pos->x -= ceilf(crealf((length)*cexpf(I * phi)));
    pos->y += ceilf(cimagf((length)*cexpf(I * phi)));

  } break;
  default:
    assert(0 && "The chosen TIP is not valid!");
    break;
  }

  // Just compute a center rotation instead at the new found position
  kite_center_rotation(kite, pos, tip_deg_rotation);
}

/**
 * @brief The function kite_center_rotation() computes all the internal points
 * for the kite and it's new position as well as the angle.
 *
 * @param kite The kite that is going to be modified.
 * @param position The new position for the kite at the center of the leading
 * edge or NULL for internal center position of the kite structure.
 * @param center_deg_rotation The rotation of the kite.
 */
void kite_center_rotation(Kite *kite, Vector2 *position,
                          float center_deg_rotation) {
  Vector2 *pos = {0};
  if (position != NULL)
    pos = position;
  else
    pos = &kite->center;

  kite->center.x = pos->x;
  kite->center.y = pos->y;

  kite->center_rotation = center_deg_rotation;
  float cw = kite->width;
  float is = kite->inner_space;
  float o = kite->overlap;
  float_t length = (kite->width / 2.f + kite->spread);

  // The difference between the angle 0 and the default downward interpolation
  float angle = 42;
  float bl_angle = (PI * (360 - (90 - angle)) / 180);
  float br_angle = (PI * (360 + (90 - angle)) / 180);
  float phi = (PI * (kite->center_rotation) / 180);

  // LEFT Triangle
  // Correct
  kite->left.v1.x = pos->x - ceilf(crealf((cw / 2.f) * cexpf(I * phi)));
  kite->left.v1.y = pos->y + ceilf(cimagf((cw / 2.f) * cexpf(I * phi)));
  kite->left.v2.x = pos->x - ceilf(crealf(is * cexpf(I * (phi - bl_angle))));
  kite->left.v2.y = pos->y + ceilf(cimagf(is * cexpf(I * (phi - bl_angle))));
  kite->left.v3.x = pos->x + ceilf(crealf(o * cexpf(I * phi)));
  kite->left.v3.y = pos->y - ceilf(cimagf(o * cexpf(I * phi)));

  // RIGHT Triangle
  // Correct
  kite->right.v1.x = pos->x - ceilf(crealf(o * cexpf(I * phi)));
  kite->right.v1.y = pos->y + ceilf(cimagf(o * cexpf(I * phi)));
  kite->right.v2.x = pos->x + ceilf(crealf(is * cexpf(I * (phi - br_angle))));
  kite->right.v2.y = pos->y - ceilf(cimagf(is * cexpf(I * (phi - br_angle))));
  kite->right.v3.x = pos->x + ceilf(crealf((cw / 2.f) * cexpf(I * phi)));
  kite->right.v3.y = pos->y - ceilf(cimagf((cw / 2.f) * cexpf(I * phi)));

  // Just an random suitable height and width that fits the scaling and spread.
  // k->rec.height = 2 * PI * PI * logf(k->spread * k->spread);
  // k->rec.height = 2 * PI * k->spread;
  kite->rec.height = 2 * PI * logf(kite->scale);
  kite->rec.width = 2 * length;
  kite->rec.x = pos->x - ceilf(crealf(length * cexpf(I * phi)));
  kite->rec.y = pos->y + ceilf(cimagf(length * cexpf(I * phi)));
}

/**
 * @brief The function kite_draw_kite() draws all the components of the kite.
 *
 * @param kite The kite that is going to be modified.
 */
void kite_draw_kite(Kite *kite) {
  Vector2 origin = {0};

  // Draw a color-filled triangle (vertex in counter-clockwise order!)
  DrawTriangle(kite->left.v1, kite->left.v2, kite->left.v3, kite->body_color);
  DrawTriangle(kite->right.v1, kite->right.v2, kite->right.v3,
               kite->body_color);
  DrawRectanglePro(kite->rec, origin, -kite->center_rotation, kite->top_color);
}

/**
 * @brief The function kite_destroy() frees the memory for the given state.
 *
 * @param state The current state of a kite.
 */
void kite_destroy(State *state) {
  free(state->kite);
  free(state);
}

/**
 * @brief The function kite_set_state_defaults() sets all the default settings
 * for a kite.
 *
 * @param state The state for which the values will be changed to defaults.
 */
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

/**
 * @brief The function kite_set_kite_defaults() sets all the internal default of
 * the kite and computes the internal corner points of the kite.
 *
 * @param kite The kite that is going to be modified.
 * @param is_generated Chooses the information if the function is called by a
 * generator or as a reset of the values.
 */
void kite_set_kite_defaults(Kite *kite, bool is_generated) {
  if (is_generated) {
    kite->center.x = 0;
    kite->center.y = 0;

    kite->center.x = GetScreenWidth() / 2.f;
    kite->center.y = GetScreenHeight() / 2.f;
  }

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

/**
 * @brief The function kite_init() allocates the memory for a kite and it's
 * corresponding state and gives back the state structure.
 *
 * @return state The new allocated state.
 */
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

/**
 * @brief The function kite_check_boundary() checks if the kite is still in the
 * displayed window in the given orientation of the kite.
 *
 * @param kite The kite that is going to be modified.
 * @param orientation The orientation of the kite, to determine where the tips
 * are.
 * @return True if the kite is in the window, otherwise false.
 */
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

/**
 * @brief The function kite_input_handler() handles all the keyboard input that
 * is provided to control the given state.
 *
 * @param state The current state of a kite that should be handled.
 */
void kite_input_handler(State *state) {
  if (!state->kite_input_handler_active) {
    return;
  }
  state->iscenter = false;
  state->fly_velocity = 10;
  state->turn_velocity = 1;

  state->turn_velocity *= GetFrameTime();
  state->turn_velocity *= state->kite->turn_speed;
  state->fly_velocity *= GetFrameTime();
  state->fly_velocity *= state->kite->fly_speed;

  // Hard reset to top left corner angel 0, position (0,0)
  if (IsKeyDown(KEY_SPACE))
    kite_array_start_pos();

  if (IsKeyDown(KEY_N))
    kite_center_rotation(state->kite, NULL, 0);

  if (IsKeyUp(KEY_R) && IsKeyUp(KEY_T)) {
    state->interrupt_smoothness = false;
  }
  if (state->interrupt_smoothness) {
    return;
  }

  kite_input_check_speed(state);
  if (IsKeyPressed(KEY_F)) {
    state->fixed = !state->fixed;
    return;
  }

  kite_input_check_mouse(state);

  kite_input_check_rotation(state);
  kite_input_check_tip_turn(state);
  kite_input_check_circle(state);

  if (!state->iscenter) {
    // NOTE: Currently not check for arrow KEY_RIGHT and KEY_LEFT, so that you
    // can still move the kite with no interrupt but with steps of 45 degrees
    // angle.
    if (IsKeyUp(KEY_T) && IsKeyUp(KEY_H) && IsKeyUp(KEY_L)) {
      state->interrupt_movement = false;
    }
  } else {
    state->iscenter = false;
    if (IsKeyUp(KEY_R)) {
      state->interrupt_movement = false;
    }
  }
  if (state->interrupt_movement) {
    return;
  }

  kite_input_check_movement(state);
}

/**
 * @brief The function kite_input_check_mouse() sets the new position of the
 * kite corresponding to the current mouse position and action.
 *
 * @param state The current state of a kite that should be handled.
 */
void kite_input_check_mouse(State *state) {
  Vector2 mouse_pos = GetMousePosition();

  if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
    kite_center_rotation(state->kite, &mouse_pos, state->kite->center_rotation);
  } else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    kite_center_rotation(state->kite, &mouse_pos, state->kite->center_rotation);
  }
}

/**
 * @brief The function kite_input_check_rotation() handles the corresponding
 * rotation invoked by the key in put.
 *
 * @param state The current state of a kite that should be handled.
 */
void kite_input_check_rotation(State *state) {

  if (IsKeyDown(KEY_R) &&
      (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {
    state->iscenter = true;

    if (!state->fixed) {
      kite_center_rotation(state->kite, NULL,
                           state->kite->center_rotation + 1 +
                               state->turn_velocity);
    } else {
      if (!state->interrupt_smoothness) {
        state->interrupt_movement = true;
        kite_center_rotation(state->kite, NULL,
                             state->kite->center_rotation + 45);
      }
      state->interrupt_smoothness = true;
    }

  } else if (IsKeyDown(KEY_R)) {
    state->iscenter = true;

    if (!state->fixed) {
      kite_center_rotation(state->kite, NULL,
                           state->kite->center_rotation - 1 -
                               state->turn_velocity);
    } else {
      if (!state->interrupt_smoothness) {
        state->interrupt_movement = true;
        kite_center_rotation(state->kite, NULL,
                             state->kite->center_rotation - 45);
      }
      state->interrupt_smoothness = true;
    }
  }
}

/**
 * @brief The function kite_input_check_tip_turn() handles the corresponding tip
 * turn rotation invoked by the key input.
 *
 * @param state The current state of a kite that should be handled.
 */
void kite_input_check_tip_turn(State *state) {
  // TODO: Think about the clamp in terms of a tip rotation
  if (IsKeyDown(KEY_T) &&
      (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {

    if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT)) {

      if (!state->fixed) {
        kite_tip_rotation(
            state->kite, NULL,
            state->kite->center_rotation + 1 + state->turn_velocity, RIGHT_TIP);
      } else {
        if (!state->interrupt_smoothness) {
          state->interrupt_movement = true;
          kite_tip_rotation(state->kite, NULL,
                            state->kite->center_rotation + 45, RIGHT_TIP);
        }
        state->interrupt_smoothness = true;
      }
    }

    if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT)) {
      if (!state->fixed) {
        kite_tip_rotation(
            state->kite, NULL,
            state->kite->center_rotation + 1 + state->turn_velocity, LEFT_TIP);
      } else {
        if (!state->interrupt_smoothness) {
          state->interrupt_movement = true;
          kite_tip_rotation(state->kite, NULL,
                            state->kite->center_rotation + 45, LEFT_TIP);
        }
        state->interrupt_smoothness = true;
      }
    }
  } else if (IsKeyDown(KEY_T)) {

    if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT)) {
      if (!state->fixed) {
        kite_tip_rotation(
            state->kite, NULL,
            state->kite->center_rotation - 1 - state->turn_velocity, RIGHT_TIP);
      } else {
        if (!state->interrupt_smoothness) {
          state->interrupt_movement = true;
          kite_tip_rotation(state->kite, NULL,
                            state->kite->center_rotation - 45, RIGHT_TIP);
        }
        state->interrupt_smoothness = true;
      }
    }
    if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT)) {
      if (!state->fixed) {
        kite_tip_rotation(
            state->kite, NULL,
            state->kite->center_rotation - 1 - state->turn_velocity, LEFT_TIP);
      } else {
        if (!state->interrupt_smoothness) {
          state->interrupt_movement = true;
          kite_tip_rotation(state->kite, NULL,
                            state->kite->center_rotation - 45, LEFT_TIP);
        }
        state->interrupt_smoothness = true;
      }
    }
  }
}

/**
 * @brief [TODO:description] currently not working!
 *
 * @param state The current state of a kite that should be handled.
 */
void kite_input_check_circle(State *state) {
  if (IsKeyPressed(KEY_C) &&
      (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {

    state->interrupt_movement = true;
    if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT)) {

      if (!state->fixed) {
        kite_circle_rotation(state->kite, NULL,
                             state->kite->center_rotation + 1 +
                                 state->turn_velocity,
                             RIGHT_TIP, false);
      } else {
        if (!state->interrupt_smoothness) {
          state->interrupt_movement = true;
          kite_circle_rotation(state->kite, NULL,
                               state->kite->center_rotation + 45, RIGHT_TIP,
                               false);
        }
        state->interrupt_smoothness = true;
      }
    }

    if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT)) {
      if (!state->fixed) {
        kite_circle_rotation(state->kite, NULL,
                             state->kite->center_rotation - 1 -
                                 state->turn_velocity,
                             LEFT_TIP, false);
      } else {
        if (!state->interrupt_smoothness) {
          state->interrupt_movement = true;
          kite_circle_rotation(state->kite, NULL,
                               state->kite->center_rotation - 45, LEFT_TIP,
                               false);
        }
        state->interrupt_smoothness = true;
      }
    }
  } else if (IsKeyPressed(KEY_C)) {
    state->interrupt_movement = true;

    if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT)) {
      if (!state->fixed) {
        kite_circle_rotation(state->kite, NULL,
                             state->kite->center_rotation - 1 -
                                 state->turn_velocity,
                             RIGHT_TIP, true);
      } else {
        if (!state->interrupt_smoothness) {
          state->interrupt_movement = true;
          kite_circle_rotation(state->kite, NULL,
                               state->kite->center_rotation - 45, RIGHT_TIP,
                               true);
        }
        state->interrupt_smoothness = true;
      }
    }
    if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT)) {
      if (!state->fixed) {
        kite_circle_rotation(state->kite, NULL,
                             state->kite->center_rotation + 1 +
                                 state->turn_velocity,
                             LEFT_TIP, true);
      } else {
        if (!state->interrupt_smoothness) {
          state->interrupt_movement = true;
          kite_circle_rotation(state->kite, NULL,
                               state->kite->center_rotation + 45, LEFT_TIP,
                               true);
        }
        state->interrupt_smoothness = true;
      }
    }
  }
}

/**
 * @brief The function kite_input_check_movement() handles the key presses
 * invoked by the caller related to basic movement of the kite.
 *
 * @param state The current state of a kite that should be handled.
 */
void kite_input_check_movement(State *state) {
  int viewport_padding = state->kite->width > state->kite->height
                             ? state->kite->width / 2
                             : state->kite->height;
  Vector2 window = {GetScreenWidth(), GetScreenHeight()};
  window.x -= viewport_padding;
  window.y -= viewport_padding;

  if (IsKeyDown(KEY_J) || IsKeyDown(KEY_DOWN)) {
    state->kite->center.y =
        kite_clamp(state->kite->center.y + state->fly_velocity,
                   viewport_padding, window.y);
    if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT))
      state->kite->center.x =
          kite_clamp(state->kite->center.x + state->fly_velocity,
                     viewport_padding, window.x);
    if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT))
      state->kite->center.x =
          kite_clamp(state->kite->center.x - state->fly_velocity,
                     viewport_padding, window.x);

    kite_center_rotation(state->kite, NULL, state->kite->center_rotation);

  } else if (IsKeyDown(KEY_K) || IsKeyDown(KEY_UP)) {
    state->kite->center.y =
        kite_clamp(state->kite->center.y - state->fly_velocity,
                   viewport_padding, window.y);
    if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT))
      state->kite->center.x =
          kite_clamp(state->kite->center.x + state->fly_velocity,
                     viewport_padding, window.x);
    if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT))
      state->kite->center.x =
          kite_clamp(state->kite->center.x - state->fly_velocity,
                     viewport_padding, window.x);
    kite_center_rotation(state->kite, NULL, state->kite->center_rotation);

  } else if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT)) {
    state->kite->center.x =
        kite_clamp(state->kite->center.x - state->fly_velocity,
                   viewport_padding, window.x);
    kite_center_rotation(state->kite, NULL, state->kite->center_rotation);
  } else if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT)) {
    state->kite->center.x =
        kite_clamp(state->kite->center.x + state->fly_velocity,
                   viewport_padding, window.x);
    kite_center_rotation(state->kite, NULL, state->kite->center_rotation);
  }
}

/**
 * @brief The function kite_input_check_speed() controls the turn speed and the
 * fly speed of the kite corresponding to the key presses.
 *
 * @param state The current state of a kite that should be handled.
 */
void kite_input_check_speed(State *state) {

  if (IsKeyDown(KEY_P) &&
      (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {
    if (state->kite->fly_speed > 0) {
      state->kite->fly_speed -= 1;
    }
  } else if (IsKeyDown(KEY_P)) {
    if (state->kite->fly_speed <= 100) {
      state->kite->fly_speed += 1;
    }
  }

  if (IsKeyDown(KEY_O) &&
      (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {
    if (state->kite->turn_speed > 0) {
      state->kite->turn_speed -= 1;
    }
  } else if (IsKeyDown(KEY_O)) {
    if (state->kite->turn_speed <= 100) {
      state->kite->turn_speed += 1;
    }
  }
}

/**
 * @brief The function kite_clamp() checks if the point z is between the range a
 * and b and if not returns the closer point of thous.
 *
 * @param z The value to check for the range a and b.
 * @param a The minimum range value.
 * @param b The maximum range value.
 * @return float The given value z if is in between the range a and b, otherwise
 * if the value of z is less than a, the value of a will be returned or b if the
 * value is lager than to b.
 */
float kite_clamp(float z, float a, float b) {

  float s = z < a ? a : z;
  return s < b ? s : b;
}

// ===========================================================================
// ========================== Sound Handler ==================================
// ===========================================================================

/**
 * @brief [TODO:description]
 *
 * @param master_volume [TODO:parameter]
 * @return [TODO:return]
 */
Sound kite_sound_init(size_t master_volume) {
  Sound s = {0};
  InitAudioDevice();
  if (IsAudioDeviceReady()) {
    SetMasterVolume(master_volume);
  }

  return s;
}

/**
 * @brief [TODO:description]
 *
 * @param sound [TODO:parameter]
 */
void kite_defer_sound(Sound sound) {
  StopSound(sound);
  UnloadSound(sound);
  CloseAudioDevice();
}

/**
 * @brief The function kite_sound_handler() checks for key presses related to
 * the audio. And if any audio file has been dropped into the application.
 */
void kite_sound_handler(Sound *kite_sound) {

  if (IsFileDropped()) {
    FilePathList file_path_list = LoadDroppedFiles();
    for (size_t i = 0; i < file_path_list.count && i < 1; ++i) {
      char *file_path = file_path_list.paths[i];
      fprintf(stderr, "ERROR: FILE: PATH :MUSIC: %s\n", file_path);
      *kite_sound = LoadSound(file_path);
    }
    UnloadDroppedFiles(file_path_list);
  }

  if (IsKeyPressed(KEY_S) &&
      (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {
    StopSound(*kite_sound);
  } else if (IsKeyPressed(KEY_S)) {
    PlaySound(*kite_sound);
  } else if (IsKeyPressed(KEY_M) &&
             (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {
    ResumeSound(*kite_sound);
  } else if (IsKeyPressed(KEY_M)) {
    PauseSound(*kite_sound);
  }
}
