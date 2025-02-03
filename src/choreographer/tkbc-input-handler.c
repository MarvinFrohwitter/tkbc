#include "tkbc-input-handler.h"
#include "../global/tkbc-utils.h"
#include "tkbc-keymaps.h"

#include "tkbc.h"
#include <raymath.h>

// ========================== KEYBOARD INPUT =================================

/**
 * @brief The function handles all the keyboard input that is provided to
 * control the given state.
 *
 * @param keymaps The current set keymaps.
 * @param state The current state of a kite that should be handled.
 */
void tkbc_input_handler(KeyMaps keymaps, Kite_State *state) {
  if (!state->kite_input_handler_active) {
    return;
  }
  state->iscenter = false;
  state->fly_velocity = 10;
  state->turn_velocity = 1;

  float dt = tkbc_get_frame_time();
  state->turn_velocity *= dt;
  state->turn_velocity *= state->kite->turn_speed;
  state->fly_velocity *= dt;
  state->fly_velocity *= state->kite->fly_speed;

  // KEY_X
  if (IsKeyDown(tkbc_hash_to_key(keymaps, 1011)))
    tkbc_kite_update_angle(state->kite, 0);

  // KEY_R && KEY_T
  if (IsKeyUp(tkbc_hash_to_key(keymaps, 1012)) &&
      IsKeyUp(tkbc_hash_to_key(keymaps, 1014))) {
    state->interrupt_smoothness = false;
  }
  if (state->interrupt_smoothness) {
    return;
  }

  tkbc_input_check_speed(keymaps, state);
  // KEY_F
  if (IsKeyPressed(tkbc_hash_to_key(keymaps, 1016))) {
    state->fixed = !state->fixed;
    return;
  }

  tkbc_mouse_control(keymaps, state);
  if (!state->mouse_control) {
    tkbc_input_check_mouse(state);
    tkbc_input_check_rotation(keymaps, state);
    tkbc_input_check_tip_turn(keymaps, state);
    tkbc_input_check_circle(keymaps, state);
  }

  if (!state->iscenter) {
    // NOTE: Currently not check for arrow KEY_RIGHT and KEY_LEFT, so that you
    // can still move the kite with no interrupt but with steps of 45 degrees
    // angle.
    // KEY_T && KEY_H && KEY_L
    KeyMap keymap = tkbc_hash_to_keymap(keymaps, 1014);
    if (IsKeyUp(keymap.key) && IsKeyUp(keymap.selection_key1) &&
        IsKeyUp(keymap.selection_key2)) {
      state->interrupt_movement = false;
    }
  } else {
    state->iscenter = false;
    // KEY_R
    if (IsKeyUp(tkbc_hash_to_keymap(keymaps, 1012).key)) {
      state->interrupt_movement = false;
    }
  }
  if (state->interrupt_movement) {
    return;
  }

  if (!state->mouse_control) {
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
  for (size_t i = 1; i <= 9; ++i) {
    if (IsKeyPressed(i + 48) && env->kite_array->count >= i) {

      for (size_t j = 0; j < env->frames->count; ++j) {

        Kite_Ids new_kite_index_array = {0};
        Frame *frame = &env->frames->elements[j];

        if (frame->kite_id_array == NULL) {
          continue;
        }

        for (size_t k = 0; k < frame->kite_id_array->count; ++k) {
          if (i - 1 != frame->kite_id_array->elements[k]) {
            tkbc_dap(&new_kite_index_array, frame->kite_id_array->elements[k]);
          }
        }

        if (new_kite_index_array.count != 0) {
          // If there are kites left in the frame

          frame->kite_id_array->count = 0;
          tkbc_dapc(frame->kite_id_array, new_kite_index_array.elements,
                    new_kite_index_array.count);

          free(new_kite_index_array.elements);
        } else {
          // If there are no kites left in the frame
          // for the cases KITE_MOVE, KITE_ROTATION, KITE_TIP_ROTATION
          frame->finished = true;
          frame->kite_id_array = NULL;
        }
      }

      env->kite_array->elements[i - 1].kite_input_handler_active =
          !env->kite_array->elements[i - 1].kite_input_handler_active;
    }
  }

  // To handle all of the kites currently registered in the kite array.
  for (size_t i = 0; i < env->kite_array->count; ++i) {
    tkbc_input_handler(*env->keymaps, &env->kite_array->elements[i]);
  }
}

/**
 * @brief The function handles the corresponding rotation invoked by the key
 * input.
 *
 * @param keymaps The current set keymaps.
 * @param s The current state of a kite that should be handled.
 */
void tkbc_input_check_rotation(KeyMaps keymaps, Kite_State *s) {
  KeyMap keymap = tkbc_hash_to_keymap(keymaps, 1013);
  // KEY_R && KEY_LEFT_SHIFT && KEY_RIGHT_SHIFT
  if (IsKeyDown(keymap.key) &&
      (IsKeyDown(keymap.mod_key) || IsKeyDown(keymap.mod_co_key))) {
    s->iscenter = true;

    if (!s->fixed) {
      tkbc_kite_update_angle(s->kite, s->kite->angle + 1 + s->turn_velocity);
      return;
    }

    if (!s->interrupt_smoothness) {
      s->interrupt_movement = true;
      tkbc_kite_update_angle(s->kite, s->kite->angle + 45);
    }
    s->interrupt_smoothness = true;

  } else if (IsKeyDown(tkbc_hash_to_keymap(keymaps, 1012).key)) {
    s->iscenter = true;

    if (!s->fixed) {
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
void tkbc_input_check_tip_turn(KeyMaps keymaps, Kite_State *s) {
  // TODO: Think about the clamp in terms of a tip rotation
  // KEY_T
  if (!IsKeyDown(tkbc_hash_to_keymap(keymaps, 1014).key)) {
    return;
  }

  KeyMap keymap = tkbc_hash_to_keymap(keymaps, 1015);
  // KEY_T && KEY_LEFT_SHIFT && KEY_RIGHT_SHIFT
  if (IsKeyDown(keymap.key) &&
      (IsKeyDown(keymap.mod_key) || IsKeyDown(keymap.mod_co_key))) {

    // KEY_H
    if (IsKeyDown(keymap.selection_key1) || IsKeyDown(KEY_LEFT)) {
      if (!s->fixed) {
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
      if (!s->fixed) {
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

  if (IsKeyDown(tkbc_hash_to_keymap(keymaps, 1014).selection_key1) ||
      IsKeyDown(KEY_LEFT)) {
    if (!s->fixed) {
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
  if (IsKeyDown(tkbc_hash_to_keymap(keymaps, 1014).selection_key2) ||
      IsKeyDown(KEY_RIGHT)) {
    if (!s->fixed) {
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
 * @brief [TODO:description] currently not working!
 *
 * @param keymaps The current set keymaps.
 * @param s The current state of a kite that should be handled.
 */
void tkbc_input_check_circle(KeyMaps keymaps, Kite_State *s) {
  // KEY_C
  if (!IsKeyPressed(tkbc_hash_to_keymap(keymaps, 1031).key)) {
    return;
  }

  KeyMap keymap = tkbc_hash_to_keymap(keymaps, 1032);
  // KEY_C
  if (IsKeyPressed(keymap.key) &&
      (IsKeyDown(keymap.mod_key) || IsKeyDown(keymap.mod_co_key))) {

    // state->interrupt_movement = true;
    // KEY_H
    if (IsKeyDown(keymap.selection_key1) || IsKeyDown(KEY_LEFT)) {
      if (!s->fixed) {
        tkbc_circle_rotation(s->kite, NULL,
                             s->kite->angle - 1 - s->turn_velocity, -1,
                             LEFT_TIP, false);
        return;
      }
      if (!s->interrupt_smoothness) {
        s->interrupt_movement = true;
        tkbc_circle_rotation(s->kite, NULL, s->kite->angle - 45, -1, LEFT_TIP,
                             false);
      }
      s->interrupt_smoothness = true;
    }

    // KEY_L
    if (IsKeyDown(keymap.selection_key2) || IsKeyDown(KEY_RIGHT)) {

      if (!s->fixed) {
        tkbc_circle_rotation(s->kite, NULL,
                             s->kite->angle + 1 + s->turn_velocity, -1,
                             RIGHT_TIP, false);
        return;
      }
      if (!s->interrupt_smoothness) {
        s->interrupt_movement = true;
        tkbc_circle_rotation(s->kite, NULL, s->kite->angle + 45, -1, RIGHT_TIP,
                             false);
      }
      s->interrupt_smoothness = true;
    }
    return;
  }

  // state->interrupt_movement = true;
  // KEY_H
  if (IsKeyDown(tkbc_hash_to_keymap(keymaps, 1031).selection_key1) ||
      IsKeyDown(KEY_LEFT)) {
    if (!s->fixed) {
      tkbc_circle_rotation(s->kite, NULL, s->kite->angle + 1 + s->turn_velocity,
                           -1, LEFT_TIP, true);
      return;
    }
    if (!s->interrupt_smoothness) {
      s->interrupt_movement = true;
      tkbc_circle_rotation(s->kite, NULL, s->kite->angle + 45, -1, LEFT_TIP,
                           true);
    }
    s->interrupt_smoothness = true;
  }
  // KEY_L
  if (IsKeyDown(tkbc_hash_to_keymap(keymaps, 1031).selection_key1) ||
      IsKeyDown(KEY_RIGHT)) {
    if (!s->fixed) {
      tkbc_circle_rotation(s->kite, NULL, s->kite->angle - 1 - s->turn_velocity,
                           -1, RIGHT_TIP, true);
      return;
    }
    if (!s->interrupt_smoothness) {
      s->interrupt_movement = true;
      tkbc_circle_rotation(s->kite, NULL, s->kite->angle - 45, -1, RIGHT_TIP,
                           true);
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
void tkbc_input_check_movement(KeyMaps keymaps, Kite_State *state) {
  int viewport_padding = state->kite->width > state->kite->height
                             ? state->kite->width / 2
                             : state->kite->height;
  Vector2 window = {tkbc_get_screen_width(), tkbc_get_screen_height()};
  window.x -= viewport_padding;
  window.y -= viewport_padding;

  // KEY_H
  if (IsKeyDown(tkbc_hash_to_key(keymaps, 1017)) || IsKeyDown(KEY_LEFT)) {
    state->kite->center.x =
        tkbc_clamp(state->kite->center.x - state->fly_velocity,
                   viewport_padding, window.x);
  }
  // KEY_J
  if (IsKeyDown(tkbc_hash_to_key(keymaps, 1018)) || IsKeyDown(KEY_DOWN)) {
    state->kite->center.y =
        tkbc_clamp(state->kite->center.y + state->fly_velocity,
                   viewport_padding, window.y);
  }
  // KEY_K
  if (IsKeyDown(tkbc_hash_to_key(keymaps, 1019)) || IsKeyDown(KEY_UP)) {
    state->kite->center.y =
        tkbc_clamp(state->kite->center.y - state->fly_velocity,
                   viewport_padding, window.y);
  }
  // KEY_L
  if (IsKeyDown(tkbc_hash_to_key(keymaps, 1020)) || IsKeyDown(KEY_RIGHT)) {
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
void tkbc_input_check_speed(KeyMaps keymaps, Kite_State *state) {

  KeyMap keymap = tkbc_hash_to_keymap(keymaps, 1022);
  // KEY_P && KEY_LEFT_SHIFT && KEY_RIGHT_SHIFT
  if (IsKeyDown(keymap.key) &&
      (IsKeyDown(keymap.mod_key) || IsKeyDown(keymap.mod_co_key))) {
    if (state->kite->fly_speed > 0) {
      state->kite->fly_speed -= 1;
    }
    // KEY_P
  } else if (IsKeyDown(tkbc_hash_to_key(keymaps, 1021))) {
    if (state->kite->fly_speed <= 100) {
      state->kite->fly_speed += 1;
    }
  }

  keymap = tkbc_hash_to_keymap(keymaps, 1024);
  // KEY_O && KEY_LEFT_SHIFT && KEY_RIGHT_SHIFT
  if (IsKeyDown(keymap.key) &&
      (IsKeyDown(keymap.mod_key) || IsKeyDown(keymap.mod_co_key))) {
    if (state->kite->turn_speed > 0) {
      state->kite->turn_speed -= 1;
    }
    // KEY_O
  } else if (IsKeyDown(tkbc_hash_to_key(keymaps, 1023))) {
    if (state->kite->turn_speed <= 100) {
      state->kite->turn_speed += 1;
    }
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
void tkbc_mouse_control(KeyMaps keymaps, Kite_State *state) {
  // KEY_ZERO
  if (IsKeyPressed(tkbc_hash_to_key(keymaps, 1033))) {
    state->mouse_control = !state->mouse_control;
  }
  if (!state->mouse_control) {
    return;
  }

  // Angle to face to the current mouse position.
  Kite *kite = state->kite;
  Vector2 mouse_pos = GetMousePosition();
  Vector2 face = {
      .x = kite->right.v3.x - kite->left.v1.x,
      .y = kite->right.v3.y - kite->left.v1.y,
  };

  Vector2 d = {
      .x = mouse_pos.x - kite->center.x,
      .y = mouse_pos.y - kite->center.y,
  };

  float angle = Vector2Angle(face, d);
  angle = angle * 180 / PI;
  int angle_from_face_to_orth = 90;
  tkbc_kite_update_angle(kite, kite->angle - angle - angle_from_face_to_orth);

  // Movement corresponding to the mouse position.
  int padding = kite->width > kite->height ? kite->width / 2 : kite->height;
  Vector2 window = {tkbc_get_screen_width(), tkbc_get_screen_height()};
  window.x -= padding;
  window.y -= padding;
  float t = state->fly_velocity;
  d = Vector2Normalize(d);
  face = Vector2Normalize(face);

  // KEY_W
  if (IsKeyDown(tkbc_hash_to_key(keymaps, 1034)) || IsKeyDown(KEY_UP)) {
    kite->center.x = tkbc_clamp(kite->center.x + t * d.x, padding, window.x);
    kite->center.y = tkbc_clamp(kite->center.y + t * d.y, padding, window.y);
  }
  // KEY_A
  if (IsKeyDown(tkbc_hash_to_key(keymaps, 1035)) || IsKeyDown(KEY_RIGHT)) {
    kite->center.x = tkbc_clamp(kite->center.x + t * face.x, padding, window.x);
    kite->center.y = tkbc_clamp(kite->center.y + t * face.y, padding, window.y);
  }
  // KEY_S
  if (IsKeyDown(tkbc_hash_to_key(keymaps, 1036)) || IsKeyDown(KEY_DOWN)) {
    kite->center.x = tkbc_clamp(kite->center.x - t * d.x, padding, window.x);
    kite->center.y = tkbc_clamp(kite->center.y - t * d.y, padding, window.y);
  }
  // KEY_D
  if (IsKeyDown(tkbc_hash_to_key(keymaps, 1037)) || IsKeyDown(KEY_LEFT)) {
    kite->center.x = tkbc_clamp(kite->center.x - t * face.x, padding, window.x);
    kite->center.y = tkbc_clamp(kite->center.y - t * face.y, padding, window.y);
  }

  tkbc_kite_update_internal(state->kite);
}
