#include "tkbc-input-handler.h"
#include "../global/tkbc-utils.h"
#include "tkbc-keymaps.h"

#include "tkbc.h"
#include <math.h>
#include <raylib.h>
#include <raymath.h>

// ========================== KEYBOARD INPUT =================================

/**
 * @brief The function handles all the keyboard input that is provided to
 * control the given state.
 *
 * @param keymaps The current set keymaps.
 * @param state The current state of a kite that should be handled.
 */
void tkbc_input_handler(Key_Maps keymaps, Kite_State *state) {
  if (!state->is_kite_input_handler_active) {
    return;
  }
  state->is_center_rotation = false;
  state->fly_velocity = 10;
  state->turn_velocity = 5;

  float dt = tkbc_get_frame_time();
  state->turn_velocity *= dt;
  state->turn_velocity *= state->kite->turn_speed;
  state->fly_velocity *= dt;
  state->fly_velocity *= state->kite->fly_speed;

  if (IsKeyDown(tkbc_hash_to_key(keymaps, KMH_KEY_KP_8)))
    tkbc_kite_update_angle(state->kite, 0);
  if (IsKeyDown(tkbc_hash_to_key(keymaps, KMH_KEY_KP_9)))
    tkbc_kite_update_angle(state->kite, -45);
  if (IsKeyDown(tkbc_hash_to_key(keymaps, KMH_KEY_KP_6)))
    tkbc_kite_update_angle(state->kite, -90);
  if (IsKeyDown(tkbc_hash_to_key(keymaps, KMH_KEY_KP_3)))
    tkbc_kite_update_angle(state->kite, -135);
  if (IsKeyDown(tkbc_hash_to_key(keymaps, KMH_KEY_KP_2)))
    tkbc_kite_update_angle(state->kite, 180);
  if (IsKeyDown(tkbc_hash_to_key(keymaps, KMH_KEY_KP_7)))
    tkbc_kite_update_angle(state->kite, 45);
  if (IsKeyDown(tkbc_hash_to_key(keymaps, KMH_KEY_KP_4)))
    tkbc_kite_update_angle(state->kite, 90);
  if (IsKeyDown(tkbc_hash_to_key(keymaps, KMH_KEY_KP_1)))
    tkbc_kite_update_angle(state->kite, 135);

  if (IsKeyDown(tkbc_hash_to_key(keymaps, KMH_KEY_KP_5)))
    tkbc_kite_update_angle(state->kite, 42);

  // KEY_R && KEY_T
  if (IsKeyUp(tkbc_hash_to_key(keymaps, KMH_ROTATE_KITES_CENTER_CLOCKWISE)) &&
      IsKeyUp(
          tkbc_hash_to_key(keymaps, KMH_ROTATE_KITES_CENTER_ANTICLOCKWISE)) &&
      IsKeyUp(tkbc_hash_to_key(keymaps, KMH_ROTATE_KITES_TIP_CLOCKWISE)) &&
      IsKeyUp(tkbc_hash_to_key(keymaps, KMH_ROTATE_KITES_TIP_ANTICLOCKWISE))) {
    state->interrupt_smoothness = false;
  }
  if (state->interrupt_smoothness) {
    return;
  }

  tkbc_input_check_speed(keymaps, state);
  // KEY_F
  if (IsKeyPressed(tkbc_hash_to_key(keymaps, KMH_TOGGLE_FIXED))) {
    state->is_fixed_rotation = !state->is_fixed_rotation;
    return;
  }

  tkbc_mouse_control(keymaps, state);
  if (!state->is_mouse_control) {
    tkbc_input_check_mouse(state);
    tkbc_input_check_rotation(keymaps, state);
    tkbc_input_check_tip_turn(keymaps, state);
  }

  if (!state->is_center_rotation) {
    // NOTE: Currently not check for arrow KEY_RIGHT and KEY_LEFT, so that you
    // can still move the kite with no interrupt but with steps of 45 degrees
    // angle.
    // KEY_T && KEY_H && KEY_L
    Key_Map keymap =
        tkbc_hash_to_keymap(keymaps, KMH_ROTATE_KITES_TIP_CLOCKWISE);
    if (IsKeyUp(keymap.key) && IsKeyUp(keymap.selection_key1) &&
        IsKeyUp(keymap.selection_key2)) {
      state->interrupt_movement = false;
    }
  } else {
    state->is_center_rotation = false;
    // KEY_R
    if (IsKeyUp(tkbc_hash_to_keymap(keymaps, KMH_ROTATE_KITES_CENTER_CLOCKWISE)
                    .key) &&
        IsKeyUp(
            tkbc_hash_to_keymap(keymaps, KMH_ROTATE_KITES_CENTER_ANTICLOCKWISE)
                .key)) {
      state->interrupt_movement = false;
    }
  }
  if (state->interrupt_movement) {
    return;
  }

  if (!state->is_mouse_control) {
    tkbc_input_check_movement(keymaps, state);
  }
}

/**
 * @brief The function handles the kite switching and calls the input handler
 * for each kite in the global kite_array.
 *
 * @param env The global state of the application.
 */
void tkbc_input_handler_kite_array(Env *env) {
  // To only handle 9 kites controllable by the keyboard.
  assert(env->kite_array != NULL);
  for (size_t i = 1; i <= 9; ++i) {
    if (!IsKeyPressed(i + 48) || env->kite_array->count < i) {
      continue;
    }

    env->kite_array->elements[i - 1].is_kite_input_handler_active =
        !env->kite_array->elements[i - 1].is_kite_input_handler_active;

    if (env->frames == NULL) {
      continue;
    }

    for (size_t j = 0; j < env->frames->count; ++j) {
      Frame *frame = &env->frames->elements[j];
      if (!frame->kite_id_array.count) {
        continue;
      }

      bool contains = false;
      size_t k = 0;
      for (; k < frame->kite_id_array.count; ++k) {
        if (i - 1 == frame->kite_id_array.elements[k]) {
          contains = true;
          break;
        }
      }

      if (!contains) {
        tkbc_dap(&frame->kite_id_array, i - 1);
        continue;
      }

      Kite_Ids new_kite_index_array = {0};
      tkbc_dapc(&new_kite_index_array, frame->kite_id_array.elements, k);
      if (k + 1 < frame->kite_id_array.count) {
        tkbc_dapc(&new_kite_index_array, &frame->kite_id_array.elements[k + 1],
                  frame->kite_id_array.count - 1 - k);
      }

      if (new_kite_index_array.count == 0) {
        // If there are no kites left in the frame
        // for the cases KITE_MOVE, KITE_ROTATION, KITE_TIP_ROTATION
        frame->finished = true;
        continue;
      }

      // If there are kites left in the frame
      frame->kite_id_array.count = 0;
      tkbc_dapc(&frame->kite_id_array, new_kite_index_array.elements,
                new_kite_index_array.count);
      free(new_kite_index_array.elements);
      new_kite_index_array.elements = NULL;
    }
  }

  // To handle all of the kites currently registered in the kite array.
  for (size_t i = 0; i < env->kite_array->count; ++i) {
    tkbc_input_handler(env->keymaps, &env->kite_array->elements[i]);
  }
}

/**
 * @brief The function handles the corresponding rotation invoked by the key
 * input.
 *
 * @param keymaps The current set keymaps.
 * @param s The current state of a kite that should be handled.
 */
void tkbc_input_check_rotation(Key_Maps keymaps, Kite_State *s) {
  Key_Map keymap =
      tkbc_hash_to_keymap(keymaps, KMH_ROTATE_KITES_CENTER_ANTICLOCKWISE);
  // KEY_R && KEY_LEFT_SHIFT && KEY_RIGHT_SHIFT
  if (IsKeyDown(keymap.key)) {
    s->is_center_rotation = true;

    if (!s->is_fixed_rotation) {
      tkbc_kite_update_angle(s->kite, s->kite->angle + 1 + s->turn_velocity);
      return;
    }

    if (!s->interrupt_smoothness) {
      s->interrupt_movement = true;
      tkbc_kite_update_angle(s->kite, s->kite->angle + 45);
    }
    s->interrupt_smoothness = true;

  } else if (IsKeyDown(
                 tkbc_hash_to_keymap(keymaps, KMH_ROTATE_KITES_CENTER_CLOCKWISE)
                     .key)) {
    s->is_center_rotation = true;

    if (!s->is_fixed_rotation) {
      tkbc_kite_update_angle(s->kite, s->kite->angle - 1 - s->turn_velocity);
      return;
    }

    if (!s->interrupt_smoothness) {
      s->interrupt_movement = true;
      tkbc_kite_update_angle(s->kite, s->kite->angle - 45);
    }
    s->interrupt_smoothness = true;
  }
}

/**
 * @brief The function handles the corresponding tip turn rotation invoked by
 * the key input.
 *
 * @param keymaps The current set keymaps.
 * @param s The current state of a kite that should be handled.
 */
void tkbc_input_check_tip_turn(Key_Maps keymaps, Kite_State *s) {
  // TODO: Think about the clamp in terms of a tip rotation
  // KEY_T
  if (!IsKeyDown(
          tkbc_hash_to_keymap(keymaps, KMH_ROTATE_KITES_TIP_CLOCKWISE).key)) {
    return;
  }

  Key_Map keymap =
      tkbc_hash_to_keymap(keymaps, KMH_ROTATE_KITES_TIP_ANTICLOCKWISE);
  // KEY_T && KEY_LEFT_SHIFT && KEY_RIGHT_SHIFT
  if (IsKeyDown(keymap.key) &&
      (IsKeyDown(keymap.mod_key) || IsKeyDown(keymap.mod_co_key))) {

    // KEY_H
    if (IsKeyDown(keymap.selection_key1) || IsKeyDown(KEY_LEFT)) {
      if (!s->is_fixed_rotation) {
        tkbc_tip_rotation(s->kite, NULL, s->kite->angle + 1 + s->turn_velocity,
                          LEFT_TIP);
        return;
      }
      if (!s->interrupt_smoothness) {
        s->interrupt_movement = true;
        tkbc_tip_rotation(s->kite, NULL, s->kite->angle + 45, LEFT_TIP);
      }
      s->interrupt_smoothness = true;
    }

    // KEY_L && KEY_RIGHT
    if (IsKeyDown(keymap.selection_key2) || IsKeyDown(KEY_RIGHT)) {
      if (!s->is_fixed_rotation) {
        tkbc_tip_rotation(s->kite, NULL, s->kite->angle + 1 + s->turn_velocity,
                          RIGHT_TIP);
        return;
      }
      if (!s->interrupt_smoothness) {
        s->interrupt_movement = true;
        tkbc_tip_rotation(s->kite, NULL, s->kite->angle + 45, RIGHT_TIP);
      }
      s->interrupt_smoothness = true;
    }

    return;
  }

  if (IsKeyDown(tkbc_hash_to_keymap(keymaps, KMH_ROTATE_KITES_TIP_CLOCKWISE)
                    .selection_key1) ||
      IsKeyDown(KEY_LEFT)) {
    if (!s->is_fixed_rotation) {
      tkbc_tip_rotation(s->kite, NULL, s->kite->angle - 1 - s->turn_velocity,
                        LEFT_TIP);
      return;
    }
    if (!s->interrupt_smoothness) {
      s->interrupt_movement = true;
      tkbc_tip_rotation(s->kite, NULL, s->kite->angle - 45, LEFT_TIP);
    }
    s->interrupt_smoothness = true;
  }
  if (IsKeyDown(tkbc_hash_to_keymap(keymaps, KMH_ROTATE_KITES_TIP_CLOCKWISE)
                    .selection_key2) ||
      IsKeyDown(KEY_RIGHT)) {
    if (!s->is_fixed_rotation) {
      tkbc_tip_rotation(s->kite, NULL, s->kite->angle - 1 - s->turn_velocity,
                        RIGHT_TIP);
      return;
    }
    if (!s->interrupt_smoothness) {
      s->interrupt_movement = true;
      tkbc_tip_rotation(s->kite, NULL, s->kite->angle - 45, RIGHT_TIP);
    }
    s->interrupt_smoothness = true;
  }
}

/**
 * @brief The function handles the key presses invoked by the caller related to
 * basic movement of the kite.
 *
 * @param keymaps The current set keymaps.
 * @param state The current state of a kite that should be handled.
 */
void tkbc_input_check_movement(Key_Maps keymaps, Kite_State *state) {
  int viewport_padding = state->kite->width > state->kite->height
                             ? state->kite->width / 2
                             : state->kite->height;
  Vector2 window = {tkbc_get_screen_width(), tkbc_get_screen_height()};
  window.x -= viewport_padding;
  window.y -= viewport_padding;

  // KEY_H
  if (IsKeyDown(tkbc_hash_to_key(keymaps, KMH_MOVES_KITES_LEFT)) ||
      IsKeyDown(KEY_LEFT)) {
    state->kite->center.x =
        tkbc_clamp(state->kite->center.x - state->fly_velocity,
                   viewport_padding, window.x);
  }
  // KEY_J
  if (IsKeyDown(tkbc_hash_to_key(keymaps, KMH_MOVES_KITES_DOWN)) ||
      IsKeyDown(KEY_DOWN)) {
    state->kite->center.y =
        tkbc_clamp(state->kite->center.y + state->fly_velocity,
                   viewport_padding, window.y);
  }
  // KEY_K
  if (IsKeyDown(tkbc_hash_to_key(keymaps, KMH_MOVES_KITES_UP)) ||
      IsKeyDown(KEY_UP)) {
    state->kite->center.y =
        tkbc_clamp(state->kite->center.y - state->fly_velocity,
                   viewport_padding, window.y);
  }
  // KEY_L
  if (IsKeyDown(tkbc_hash_to_key(keymaps, KMH_MOVES_KITES_RIGHT)) ||
      IsKeyDown(KEY_RIGHT)) {
    state->kite->center.x =
        tkbc_clamp(state->kite->center.x + state->fly_velocity,
                   viewport_padding, window.x);
  }

  tkbc_kite_update_internal(state->kite);
}

/**
 * @brief The function controls the turn speed and the fly speed of the kite
 * corresponding to the key presses.
 *
 * @param keymaps The current set keymaps.
 * @param state The current state of a kite that should be handled.
 */
void tkbc_input_check_speed(Key_Maps keymaps, Kite_State *state) {
  int max = 100;
  int min = 0;

  Key_Map keymap = tkbc_hash_to_keymap(keymaps, KMH_REDUCE_FLY_SPEED);
  // KEY_P && KEY_LEFT_SHIFT && KEY_RIGHT_SHIFT
  if (IsKeyDown(keymap.key) &&
      (IsKeyDown(keymap.mod_key) || IsKeyDown(keymap.mod_co_key))) {
    state->kite->fly_speed = tkbc_clamp(state->kite->fly_speed - 1, min, max);
    // KEY_P
  } else if (IsKeyDown(tkbc_hash_to_key(keymaps, KMH_INCREASE_FLY_SPEED))) {
    state->kite->fly_speed = tkbc_clamp(state->kite->fly_speed + 1, min, max);
  }

  Vector2 mouse_wheel_move = GetMouseWheelMoveV();
  if (mouse_wheel_move.y < 0) {
    state->kite->fly_speed = tkbc_clamp(state->kite->fly_speed - 5, min, max);
  } else if (mouse_wheel_move.y > 0) {
    state->kite->fly_speed = tkbc_clamp(state->kite->fly_speed + 5, min, max);
  }

  if (mouse_wheel_move.x < 0) {
    state->kite->turn_speed = tkbc_clamp(state->kite->turn_speed + 5, min, max);
  } else if (mouse_wheel_move.x > 0) {
    state->kite->turn_speed = tkbc_clamp(state->kite->turn_speed - 5, min, max);
  }

  keymap = tkbc_hash_to_keymap(keymaps, KMH_REDUCE_TURN_SPEED);
  // KEY_O && KEY_LEFT_SHIFT && KEY_RIGHT_SHIFT
  if (IsKeyDown(keymap.key) &&
      (IsKeyDown(keymap.mod_key) || IsKeyDown(keymap.mod_co_key))) {
    state->kite->turn_speed = tkbc_clamp(state->kite->turn_speed - 1, min, max);
    // KEY_O
  } else if (IsKeyDown(tkbc_hash_to_key(keymaps, KMH_INCREASE_TURN_SPEED))) {
    state->kite->turn_speed = tkbc_clamp(state->kite->turn_speed + 1, min, max);
  }
}

/**
 * @brief The function sets the new position of the kite corresponding to the
 * current mouse position and action.
 *
 * @param state The current state of a kite that should be handled.
 */
void tkbc_input_check_mouse(Kite_State *state) {
  Vector2 mouse_pos = GetMousePosition();

  if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
    tkbc_kite_update_position(state->kite, &mouse_pos);
  } else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    tkbc_kite_update_position(state->kite, &mouse_pos);
  }
}

/**
 * @brief The function sets the new position of the kite corresponding to the
 * current mouse position. If faces the kites leading edge towards the mouse and
 * can move the kite towards, away and left and right around the mouse, with the
 * keyboard input keys [w,a,s,d] or alternative with [up, left, down, right].
 *
 * @param keymaps The current set keymaps.
 * @param state The current state of a kite that should be handled.
 */
void tkbc_mouse_control(Key_Maps keymaps, Kite_State *state) {
  // KEY_ZERO
  if (IsKeyPressed(
          tkbc_hash_to_key(keymaps, KMH_SWITCH_MOUSE_CONTOL_MOVEMENT))) {
    state->is_mouse_control = !state->is_mouse_control;
  }
  if (IsKeyPressed(tkbc_hash_to_key(keymaps, KMH_KEY_161))) {
    state->is_kite_reversed = !state->is_kite_reversed;
  }

  if (!state->is_mouse_control) {
    return;
  }

  // Reset to the defaults for state values that are set in each iteration.
  // NOTE: This is especially important to ensure combinations of functionality
  // is not blocked or gets in a unexpected state.
  state->is_angle_locked = false;
  state->is_tip_locked = false;
  state->is_rotating = false;
  state->selected_tips = 0;

  Kite *kite = state->kite;
  tkbc_check_is_mouse_in_dead_zone(state, kite->height / 2);
  tkbc_check_is_angle_locked(keymaps, state);
  tkbc_calcluate_and_update_angle(keymaps, state);
  tkbc_calculate_new_kite_position(keymaps, state);

  size_t window_padding = state->kite->width > state->kite->height
                              ? state->kite->width / 2
                              : state->kite->height;
  Vector2 window = {
      tkbc_get_screen_width() - window_padding,
      tkbc_get_screen_height() - window_padding,
  };
  kite->center.y = tkbc_clamp(kite->center.y, window_padding, window.y);
  kite->center.x = tkbc_clamp(kite->center.x, window_padding, window.x);
  tkbc_kite_update_internal(state->kite);
}

/**
 * @brief The function handles the corresponding rotation invoked by the key
 * input.
 *
 * @param keymaps The current set keymaps.
 * @param s The current state of a kite that should be handled.
 */
void tkbc_input_check_rotation_mouse_control(Key_Maps keymaps, Kite_State *s) {
  // KEY_R
  if (IsKeyDown(
          tkbc_hash_to_key(keymaps, KMH_ROTATE_KITES_CENTER_ANTICLOCKWISE)) ||
      IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    tkbc_kite_update_angle(s->kite, s->kite->angle + s->turn_velocity);
    s->is_rotating = true;

  } else if (IsKeyDown(tkbc_hash_to_key(keymaps,
                                        KMH_ROTATE_KITES_CENTER_CLOCKWISE)) ||
             IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
    tkbc_kite_update_angle(s->kite, s->kite->angle - s->turn_velocity);
    s->is_rotating = true;
  }
}

/**
 * @brief The function handles the corresponding rotation invoked by the key
 * input.
 *
 * @param keymaps The current set keymaps.
 * @param s The current state of a kite that should be handled.
 */
void tkbc_input_check_tip_rotation_mouse_control(Key_Maps keymaps,
                                                 Kite_State *s) {
  bool is_left = IsKeyDown(tkbc_hash_to_key(
                     keymaps, KMH_ROTATE_KITES_CENTER_ANTICLOCKWISE)) ||
                 IsMouseButtonDown(MOUSE_BUTTON_LEFT);
  bool is_right =
      IsKeyDown(tkbc_hash_to_key(keymaps, KMH_ROTATE_KITES_CENTER_CLOCKWISE)) ||
      IsMouseButtonDown(MOUSE_BUTTON_RIGHT);

  if (!is_left && !is_right) {
    return;
  }

  // Q //
  if (is_left) {
    s->selected_tips |= LEFT_TIP;
    // W
    if (IsKeyDown(tkbc_hash_to_key(keymaps, KMH_MOVES_KITES_TOWARDS_MOUSE))) {
      tkbc_tip_rotation(s->kite, NULL, s->kite->angle + s->turn_velocity,
                        LEFT_TIP);
      s->is_rotating = true;
    }

    // S
    if (IsKeyDown(tkbc_hash_to_key(keymaps, KMH_MOVES_KITES_AWAY_MOUSE))) {
      tkbc_tip_rotation(s->kite, NULL, s->kite->angle - s->turn_velocity,
                        LEFT_TIP);
      s->is_rotating = true;
    }
  }

  // E //
  if (is_right) {
    s->selected_tips |= RIGHT_TIP;
    // W
    if (IsKeyDown(tkbc_hash_to_key(keymaps, KMH_MOVES_KITES_TOWARDS_MOUSE))) {
      tkbc_tip_rotation(s->kite, NULL, s->kite->angle - s->turn_velocity,
                        RIGHT_TIP);
      s->is_rotating = true;
    }

    // S
    if (IsKeyDown(tkbc_hash_to_key(keymaps, KMH_MOVES_KITES_AWAY_MOUSE))) {
      tkbc_tip_rotation(s->kite, NULL, s->kite->angle + s->turn_velocity,
                        RIGHT_TIP);
      s->is_rotating = true;
    }
  }
}

/**
 * @brief The function computes the new position of the kite corresponding to
 * the current mouse position and the keyboard input. If faces the kites leading
 * edge towards the mouse and can move the kite towards, away and left and right
 * around the mouse, with the keyboard input keys [w,a,s,d].
 *
 * @param keymaps The current set keymaps.
 * @param state The current state of a kite that should be handled.
 */
void tkbc_calculate_new_kite_position(Key_Maps keymaps, Kite_State *state) {
  // Movement corresponding to the mouse position.
  Kite *kite = state->kite;
  Vector2 mouse_pos = GetMousePosition();
  Vector2 face = {
      .x = kite->right.v3.x - kite->left.v1.x,
      .y = kite->right.v3.y - kite->left.v1.y,
  };
  face = Vector2Normalize(face);

  Vector2 distance_to_mouse = {
      .x = mouse_pos.x - kite->center.x,
      .y = mouse_pos.y - kite->center.y,
  };
  distance_to_mouse = Vector2Normalize(distance_to_mouse);

  Vector2 around = {
      .x = -distance_to_mouse.y,
      .y = distance_to_mouse.x,
  };

  Vector2 face_orthogonal_norm = {
      .x = face.y,
      .y = -face.x,
  };

  if (state->is_angle_locked) {

    if (state->is_kite_reversed) {

      if (state->is_rotating) {
        // KEY_W
        if (IsKeyDown(tkbc_hash_to_key(keymaps, KMH_MOVES_KITES_UP))) {
          kite->center.y -= state->fly_velocity;
        }

        // KEY_S
        if (IsKeyDown(tkbc_hash_to_key(keymaps, KMH_MOVES_KITES_DOWN))) {
          kite->center.y += state->fly_velocity;
        }

      } else {
        // KEY_W
        if (IsKeyDown(
                tkbc_hash_to_key(keymaps, KMH_MOVES_KITES_TOWARDS_MOUSE))) {
          kite->center.x -= state->fly_velocity * face_orthogonal_norm.x;
          kite->center.y -= state->fly_velocity * face_orthogonal_norm.y;
        }

        // KEY_S
        if (IsKeyDown(tkbc_hash_to_key(keymaps, KMH_MOVES_KITES_AWAY_MOUSE))) {
          kite->center.x += state->fly_velocity * face_orthogonal_norm.x;
          kite->center.y += state->fly_velocity * face_orthogonal_norm.y;
        }
      }

    } else {
      if (state->is_rotating) {
        // KEY_W
        if (IsKeyDown(tkbc_hash_to_key(keymaps, KMH_MOVES_KITES_UP))) {
          kite->center.y -= state->fly_velocity;
        }

        // KEY_S
        if (IsKeyDown(tkbc_hash_to_key(keymaps, KMH_MOVES_KITES_DOWN))) {
          kite->center.y += state->fly_velocity;
        }

      } else {
        // KEY_W
        if (IsKeyDown(
                tkbc_hash_to_key(keymaps, KMH_MOVES_KITES_TOWARDS_MOUSE))) {
          kite->center.x += state->fly_velocity * face_orthogonal_norm.x;
          kite->center.y += state->fly_velocity * face_orthogonal_norm.y;
        }

        // KEY_S
        if (IsKeyDown(tkbc_hash_to_key(keymaps, KMH_MOVES_KITES_AWAY_MOUSE))) {
          kite->center.x -= state->fly_velocity * face_orthogonal_norm.x;
          kite->center.y -= state->fly_velocity * face_orthogonal_norm.y;
        }
      }
    }

  } else {
    // KEY_W
    if (IsKeyDown(tkbc_hash_to_key(keymaps, KMH_MOVES_KITES_TOWARDS_MOUSE))) {
      if (!state->is_mouse_in_dead_zone) {
        kite->center.x += state->fly_velocity * distance_to_mouse.x;
        kite->center.y += state->fly_velocity * distance_to_mouse.y;
      }
    }

    // KEY_S
    if (IsKeyDown(tkbc_hash_to_key(keymaps, KMH_MOVES_KITES_AWAY_MOUSE))) {
      if (!state->is_tip_locked) {
        kite->center.x -= state->fly_velocity * distance_to_mouse.x;
        kite->center.y -= state->fly_velocity * distance_to_mouse.y;
      }
    }
  }

  if (state->is_rotating) {
    if (state->is_angle_locked) {
      // KEY_D
      if (IsKeyDown(tkbc_hash_to_key(keymaps, KMH_MOVES_KITES_RIGHT))) {
        kite->center.x += state->fly_velocity;
      }
      // KEY_A
      if (IsKeyDown(tkbc_hash_to_key(keymaps, KMH_MOVES_KITES_LEFT))) {
        kite->center.x -= state->fly_velocity;
      }
    } else {

      // KEY_D
      if (IsKeyDown(
              tkbc_hash_to_key(keymaps, KMH_MOVES_KITES_RIGHT_AROUND_MOUSE))) {
        kite->center.x -= state->fly_velocity * around.x;
        kite->center.y -= state->fly_velocity * around.y;
      }
      // KEY_A
      if (IsKeyDown(
              tkbc_hash_to_key(keymaps, KMH_MOVES_KITES_LEFT_AROUND_MOUSE))) {
        kite->center.x += state->fly_velocity * around.x;
        kite->center.y += state->fly_velocity * around.y;
      }
    }

  } else {
    // KEY_D
    if (IsKeyDown(
            tkbc_hash_to_key(keymaps, KMH_MOVES_KITES_RIGHT_AROUND_MOUSE))) {
      kite->center.x -= state->fly_velocity * face.x;
      kite->center.y -= state->fly_velocity * face.y;
    }
    // KEY_A
    if (IsKeyDown(
            tkbc_hash_to_key(keymaps, KMH_MOVES_KITES_LEFT_AROUND_MOUSE))) {
      kite->center.x += state->fly_velocity * face.x;
      kite->center.y += state->fly_velocity * face.y;
    }
  }
}

/**
 * @brief The function calculates the snapping angle in 45 degrees steps.
 * This is only activated if the kite control is in precision mode.
 *
 * @param keymaps The current set keymaps.
 * @param state The current state of a kite that should be handled.
 */
void tkbc_calculate_and_update_snapping_angle(Key_Maps keymaps,
                                              Kite_State *state) {
  if (IsKeyPressed(tkbc_hash_to_key(keymaps, KMH_SNAP_KITE_ANGLE))) {
    state->is_snapping_to_angle = !state->is_snapping_to_angle;
  }

  if (!state->is_snapping_to_angle) {
    return;
  }

  if (state->is_angle_locked || state->is_tip_locked) {
    float remainder = fmodf(state->kite->angle, 45);
    float result_angle = fmodf(state->kite->angle, 360) - remainder;
    if (fabsf(remainder) > 22.5) {
      result_angle += remainder < 0 ? -45 : 45;
    }

    if (state->is_angle_locked) {
      tkbc_kite_update_angle(state->kite, result_angle);
    }

    if (state->is_tip_locked) {

      if (!state->selected_tips) {
        tkbc_kite_update_angle(state->kite, result_angle);
      }

      if (state->selected_tips & LEFT_TIP && state->selected_tips & RIGHT_TIP) {
        // This is needed to prevent floating point precision errors.
        // Normally both cases down below should result in this behavior anyway.
        tkbc_kite_update_angle(state->kite, result_angle);
      }
      if (state->selected_tips & LEFT_TIP) {
        tkbc_tip_rotation(state->kite, NULL, result_angle, LEFT_TIP);
      }
      if (state->selected_tips & RIGHT_TIP) {
        tkbc_tip_rotation(state->kite, NULL, result_angle, RIGHT_TIP);
      }
    }
  }
}

/**
 * @brief The function checks if the mouse is to close to the kite. To close is
 * defined by the dead_zone_radius.
 *
 * @param state The current state of a kite that should be handled.
 * @param dead_zone_radius Represents the radius where the kite can't move any
 * closer to the mouse.
 * @return True if the mouse is to close to the kite, otherwise false.
 */
bool tkbc_check_is_mouse_in_dead_zone(Kite_State *state,
                                      size_t dead_zone_radius) {
  Vector2 mouse_position = GetMousePosition();
  if (fabsf(mouse_position.x - state->kite->center.x) <= dead_zone_radius &&
      fabsf(mouse_position.y - state->kite->center.y) <= dead_zone_radius) {
    state->is_mouse_in_dead_zone = true;
    return true;
  }
  state->is_mouse_in_dead_zone = false;
  return false;
}

/**
 * @brief The function checks if the precision mode is activated and the current
 * kite angle is locked in and checks for the new rotation.
 *
 * @param keymaps The current set keymaps.
 * @param state The current state of a kite that should be handled.
 * @return True if the kite angle is locked to a fixed angle, otherwise false.
 */
bool tkbc_check_is_angle_locked(Key_Maps keymaps, Kite_State *state) {
  if (IsKeyDown(tkbc_hash_to_keymap(keymaps, KMH_LOCK_KITE_ANGLE).mod_key) ||
      IsKeyDown(tkbc_hash_to_keymap(keymaps, KMH_LOCK_KITE_ANGLE).mod_co_key)) {
    state->is_angle_locked = true;
  }

  if (IsKeyDown(tkbc_hash_to_keymap(keymaps, KMH_LOCK_KITE_TIP).mod_key) ||
      IsKeyDown(tkbc_hash_to_keymap(keymaps, KMH_LOCK_KITE_TIP).mod_co_key)) {
    state->is_tip_locked = true;
  }

  if (state->is_tip_locked) {
    state->is_mouse_in_dead_zone = true;
    tkbc_input_check_tip_rotation_mouse_control(keymaps, state);
    return true;
  } else {
    tkbc_input_check_rotation_mouse_control(keymaps, state);
  }

  if (state->is_angle_locked) {
    state->is_mouse_in_dead_zone = true;
    return true;
  }
  return false;
}

/**
 * @brief The function computes the new angle in respect to the mouse position
 * and if snapping is activated it checks for new snapping angle.
 *
 * @param keymaps The current set keymaps.
 * @param state The current state of a kite that should be handled.
 */
void tkbc_calcluate_and_update_angle(Key_Maps keymaps, Kite_State *state) {
  if (state->is_rotating) {
    return;
  }

  Kite *kite = state->kite;
  Vector2 face = {
      .x = kite->right.v3.x - kite->left.v1.x,
      .y = kite->right.v3.y - kite->left.v1.y,
  };
  Vector2 mouse_pos = GetMousePosition();
  Vector2 distance_to_mouse = {
      .x = mouse_pos.x - kite->center.x,
      .y = mouse_pos.y - kite->center.y,
  };
  distance_to_mouse = Vector2Normalize(distance_to_mouse);
  face = Vector2Normalize(face);

  if ((!state->is_mouse_in_dead_zone || !state->is_angle_locked) &&
      !state->is_tip_locked) {
    float angle = Vector2Angle(face, distance_to_mouse) * 180 / PI;
    float result_angle = state->kite->angle - angle - 90;

    if (state->is_kite_reversed) {
      tkbc_kite_update_angle(state->kite, result_angle + 180);
    } else {
      tkbc_kite_update_angle(state->kite, result_angle);
    }
  }

  tkbc_calculate_and_update_snapping_angle(keymaps, state);
}
