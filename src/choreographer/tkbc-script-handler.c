#include <assert.h>
#include <math.h>
#include <raylib.h>
#include <stdlib.h>
#include <string.h>

#include "../global/tkbc-types.h"
#include "../global/tkbc-utils.h"
#include "tkbc-keymaps.h"
#include "tkbc-script-handler.h"
#include "tkbc.h"

#include "raymath.h"

// ========================== Script Handler =================================

/**
 * @brief The function can be used to get a new allocated zero initialized
 * frame.
 *
 * @return A new on the heap allocated frame region is given back.
 */
Frame *tkbc_init_frame(void) {
  Frame *frame = malloc(sizeof(*frame));
  if (frame == NULL) {
    tkbc_fprintf(stderr, "ERROR", "No more memory can be allocated.\n");
    return NULL;
  }
  memset(frame, 0, sizeof(*frame));
  return frame;
}

/**
 * @brief The function can be used to get a direct view pointer to the kite in
 * the kite_array stored in the env by providing its id.
 *
 * @param env The global state of the application.
 * @param id THe id that identifies the kite.
 * @return A pointer to the requested kite or NULL if the kite doesn't exist.
 */
Kite *tkbc_get_kite_by_id(Env *env, size_t id) {
  for (size_t i = 0; i < env->kite_array->count; ++i) {
    if (env->kite_array->elements[i].kite_id == id) {
      return env->kite_array->elements[i].kite;
    }
  }
  return NULL;
}

/**
 * @brief The function can be used to get a direct view pointer to the kite
 * state in the kite_array stored in the env by providing its id.
 *
 * @param env The global state of the application.
 * @param id THe id that identifies the kite.
 * @return A pointer to the requested kite or NULL if the kite doesn't exist.
 */
Kite_State *tkbc_get_kite_state_by_id(Env *env, size_t id) {
  for (size_t i = 0; i < env->kite_array->count; ++i) {
    if (env->kite_array->elements[i].kite_id == id) {
      return &env->kite_array->elements[i];
    }
  }
  return NULL;
}

/**
 * @brief The function check for the error and crashes if the kite doesn't
 * exist it will report the error.
 *
 * @param env The global state of the application.
 * @param id THe id that identifies the kite.
 * @return A pointer to the requested kite.
 */
Kite *tkbc_get_kite_by_id_unwrap(Env *env, size_t id) {
  Kite *kite = tkbc_get_kite_by_id(env, id);
  if (!kite) {
    tkbc_fprintf(stderr, "ERROR", "The kite index array is invalid.\n");
    tkbc_fprintf(stderr, "ERROR", "The id: %zu was not found.\n", id);
  }
  assert(kite != NULL);
  return kite;
}

/**
 * @brief The function copies every single value even the values that are just
 * represented by a pointer of the struct Frames to a new instance. Every
 * internal pointer is a new one in the created representation and points to the
 * new copied values. The result is a complete copy of the given frames. It can
 * be used to move a creation of a temporary struct of type Frames to a
 * permanently stored one.
 *
 * @param frames The pointer that holds the values that should be copied.
 * @return The value ready copy of the frames.
 */
Frames tkbc_deep_copy_frames(Frames *frames) {
  Frames new_frames = {0};
  if (frames == NULL) {
    return new_frames;
  }
  new_frames.block_index = frames->block_index;

  if (frames->kite_frame_positions.count) {
    tkbc_dapc(&new_frames.kite_frame_positions,
              frames->kite_frame_positions.elements,
              frames->kite_frame_positions.count);
  }

  if (frames->elements == NULL) {
    return new_frames;
  }

  for (size_t i = 0; i < frames->count; ++i) {
    Frame frame = tkbc_deep_copy_frame(&frames->elements[i]);
    tkbc_dap(&new_frames, frame);
  }

  return new_frames;
}

/**
 * @brief The function can be used to copy a frame struct.
 *
 * @param frame The frame that should be copied.
 * @return The copy of the original fames provided in the argument.
 */
Frame tkbc_deep_copy_frame(Frame *frame) {
  Frame f = {0};
  f.duration = frame->duration;
  f.finished = frame->finished;
  f.kind = frame->kind;
  f.index = frame->index;
  f.action = frame->action;
  if (frame->kite_id_array.count) {
    tkbc_dapc(&f.kite_id_array, frame->kite_id_array.elements,
              frame->kite_id_array.count);
  }

  return f;
}

/**
 * @brief The function copies every single value even the values that are just
 * represented by a pointer of the struct Block_Frame to a new instance. It can
 * be used to move a creation of a temporary struct of type Block_Frame to a
 * permanently stored one.
 *
 * @param block_frame The pointer that holds the values that should be copied.
 * @return The value ready copy of the block_frame.
 */
Block_Frame tkbc_deep_copy_block_frame(Block_Frame *block_frame) {
  Block_Frame new_block_frame = {0};
  if (!block_frame) {
    return new_block_frame;
  }
  new_block_frame.script_id = block_frame->script_id;

  for (size_t i = 0; i < block_frame->count; ++i) {
    tkbc_dap(&new_block_frame,
             tkbc_deep_copy_frames(&block_frame->elements[i]));
  }
  return new_block_frame;
}

/**
 * @brief The function can be used to free all the elements and related memory
 * of the given frames. It recursevly handles all the internal saved values.
 *
 * @param frames The frames the memory should be free.
 */
void tkbc_destroy_frames_internal_data(Frames *frames) {
  if (!frames) {
    return;
  }

  tkbc_reset_frames_internal_data(frames);

  if (frames->kite_frame_positions.elements) {
    free(frames->kite_frame_positions.elements);
    frames->kite_frame_positions.elements = NULL;
    frames->kite_frame_positions.count = 0;
    frames->kite_frame_positions.capacity = 0;
  }

  if (frames->elements) {
    free(frames->elements);
    frames->elements = NULL;
    frames->capacity = 0;
  }
  frames->count = 0;
}

/**
 * @brief The function can be used to free all the related memory inside of the
 * elements given frames but not the elements and the kite_frame_positions. The
 * count is just reset in the frames and kite_frame_positions. It recursevly
 * handles all the internal saved values.
 *
 * @param frames The frames that should be reset.
 */
void tkbc_reset_frames_internal_data(Frames *frames) {
  if (!frames) {
    return;
  }

  for (size_t i = 0; i < frames->count; ++i) {
    if (frames->elements[i].kite_id_array.elements) {
      free(frames->elements[i].kite_id_array.elements);
      frames->elements[i].kite_id_array.elements = NULL;
      frames->elements[i].kite_id_array.count = 0;
      frames->elements[i].kite_id_array.capacity = 0;
    }
  }

  frames->kite_frame_positions.count = 0;
  frames->count = 0;
}

/**
 * @brief The function supports all the action kinds that are defined. It can be
 * used to calculate the given frame and its action. For kite actions the new
 * state of the kite results and the internal action values related to time and
 * intermediate positioning is saved and ready to use in a nest update call.
 *
 * @param env The global state of the application.
 * @param frame The frame the action should the handled for. It also can hold
 * intermediate values.
 */
void tkbc_render_frame(Env *env, Frame *frame) {
  Kite *kite = NULL;
  Frame *env_frame = &env->frames->elements[frame->index];

  assert(ACTION_KIND_COUNT == 9 && "NOT ALL THE Action_Kinds ARE IMPLEMENTED");
  switch (frame->kind) {
  case KITE_QUIT: {
    if (tkbc_check_finished_frames_count(env) == env->frames->count - 1) {
      frame->finished = true;
      break;
    }
  } // FALLTHROUGH;
  case KITE_WAIT: {
    Wait_Action *action = &frame->action.as_wait;
    if (frame->duration <= 0) {
      frame->finished = true;
      frame->duration = 0;
    } else {
      double current_time = tkbc_get_time();
      frame->duration -= current_time - action->starttime;
      action->starttime = current_time;
    }
  } break;

  case KITE_MOVE_ADD: {
    Move_Add_Action *action = &frame->action.as_move_add;

    for (size_t i = 0; i < env_frame->kite_id_array.count; ++i) {
      Id id = env_frame->kite_id_array.elements[i];
      kite = tkbc_get_kite_by_id_unwrap(env, id);

      Vector2 dest_position = Vector2Add(kite->old_center, action->position);
      Vector2 d = tkbc_script_move(kite, dest_position, frame->duration);

      if (Vector2Equals(action->position, Vector2Zero())) {
        frame->duration -= tkbc_get_frame_time();
        if (frame->duration <= 0) {
          frame->finished = true;
        }
        continue;
      }

      int res = fabsf(Vector2Add(kite->old_center, action->position).x -
                      kite->center.x) <= d.x &&
                fabsf(Vector2Add(kite->old_center, action->position).y -
                      kite->center.y) <= d.y;

      if (res) {
        frame->finished = true;
        tkbc_script_move(kite, dest_position, 0);
      }
    }

  } break;

  case KITE_MOVE: {
    Move_Action *action = &frame->action.as_move;

    for (size_t i = 0; i < env_frame->kite_id_array.count; ++i) {
      Id id = env_frame->kite_id_array.elements[i];
      kite = tkbc_get_kite_by_id_unwrap(env, id);

      Vector2 d = tkbc_script_move(kite, action->position, frame->duration);

      if (Vector2Equals(action->position, Vector2Zero())) {
        if (frame->duration > 0) {
          frame->duration -= tkbc_get_frame_time();
          continue;
        }
        bool result = fabsf(action->position.x - kite->center.x) <= d.x &&
                      fabsf(action->position.y - kite->center.y) <= d.y;
        if (result) {
          frame->finished = true;
        }
        continue;
      }

      int res = fabsf(action->position.x - kite->center.x) <= d.x &&
                fabsf(action->position.y - kite->center.y) <= d.y;

      if (res) {
        frame->finished = true;
        tkbc_script_move(kite, action->position, 0);
      }
    }
  } break;

  case KITE_ROTATION_ADD: {
    Rotation_Action *action = &frame->action.as_rotation_add;

    for (size_t i = 0; i < env_frame->kite_id_array.count; ++i) {
      Id id = env_frame->kite_id_array.elements[i];
      kite = tkbc_get_kite_by_id_unwrap(env, id);

      float d = tkbc_script_rotate(kite, action->angle, frame->duration, true);
      if (action->angle == 0) {
        frame->duration -= tkbc_get_frame_time();
        if (frame->duration <= 0) {
          frame->finished = true;
        }
        continue;
      }

      int result = fabsf((kite->old_angle + action->angle) - kite->angle) <= d;
      if (result) {
        frame->finished = true;
        // Enable for setting the correct angle precision.
        tkbc_script_rotate(kite, action->angle, 0, true);
      }
    }
  } break;

  case KITE_ROTATION: {
    Rotation_Action *action = &frame->action.as_rotation;

    for (size_t i = 0; i < env_frame->kite_id_array.count; ++i) {
      Id id = env_frame->kite_id_array.elements[i];
      kite = tkbc_get_kite_by_id_unwrap(env, id);

      float intermediate_angle = tkbc_check_angle_zero(
          kite, frame->kind, *(Action *)action, frame->duration);
      float d =
          tkbc_script_rotate(kite, intermediate_angle, frame->duration, false);

      if (action->angle == 0) {
        if (frame->duration > 0) {
          frame->duration -= tkbc_get_frame_time();
          continue;
        }
        bool result = fabsf(kite->angle) <= d * fmaxf(1.0f, fabsf(kite->angle));
        if (result) {
          frame->finished = true;
        }
        continue;
      }

      // NOTE: Different from the ADDing version.
      int result = fabsf(action->angle - kite->angle) <= d;
      if (result) {
        frame->finished = true;
        // Enable for setting the correct angle precision.
        tkbc_script_rotate(kite, intermediate_angle, 0, false);
      }
    }
  } break;

  case KITE_TIP_ROTATION_ADD: {
    Tip_Rotation_Action *action = &frame->action.as_tip_rotation_add;

    for (size_t i = 0; i < env_frame->kite_id_array.count; ++i) {
      Id id = env_frame->kite_id_array.elements[i];
      kite = tkbc_get_kite_by_id_unwrap(env, id);

      float d = tkbc_script_rotate_tip(kite, action->tip, action->angle,
                                       frame->duration, true);
      if (action->angle == 0) {
        frame->duration -= tkbc_get_frame_time();
        if (frame->duration <= 0) {
          frame->finished = true;
        }
        continue;
      }

      int result = fabsf((kite->old_angle + action->angle) - kite->angle) <= d;
      if (result) {
        frame->finished = true;
        // Enable for setting the correct angle precision.
        tkbc_script_rotate_tip(kite, action->tip, action->angle, 0, true);
      }
    }
  } break;

  case KITE_TIP_ROTATION: {
    Tip_Rotation_Action *action = &frame->action.as_tip_rotation;

    for (size_t i = 0; i < env_frame->kite_id_array.count; ++i) {
      Id id = env_frame->kite_id_array.elements[i];
      kite = tkbc_get_kite_by_id_unwrap(env, id);

      float intermediate_angle = tkbc_check_angle_zero(
          kite, frame->kind, *(Action *)action, frame->duration);
      float d = tkbc_script_rotate_tip(kite, action->tip, intermediate_angle,
                                       frame->duration, false);

      if (action->angle == 0) {
        if (frame->duration > 0) {
          frame->duration -= tkbc_get_frame_time();
          continue;
        }
        bool result = fabsf(kite->angle) <= d * fmaxf(1.0f, fabsf(kite->angle));
        if (result) {
          frame->finished = true;
        }
        continue;
      }

      // NOTE: Different from the ADDing version.
      int result = fabsf(action->angle - kite->angle) <= d;
      if (result) {
        frame->finished = true;
        // Enable for setting the correct angle precision.
        tkbc_script_rotate_tip(kite, action->tip, intermediate_angle, 0, false);
      }
    }
  } break;

  default:
    assert(0 && "UNREACHABLE tkbc_render_frame()");
  }
}

/**
 * @brief The function can be used to refresh the time stamp saved in a frame
 * action.
 *
 * @param frames The frames for which the saved time should be updated.
 */
void tkbc_patch_frames_current_time(Frames *frames) {

  for (size_t i = 0; i < frames->count; ++i) {
    Frame *frame = &frames->elements[i];
    assert(ACTION_KIND_COUNT == 9 &&
           "NOT ALL THE Action_Kinds ARE IMPLEMENTED");
    switch (frame->kind) {
    case KITE_QUIT:
    case KITE_WAIT: {
      frame->action.as_wait.starttime = tkbc_get_time();
    } break;
    default: {
    }
    }
  }
}

/**
 * @brief The function can be used to backpatch current kite positions in the
 * frames array to be used later in the redrawing and calculation of a script
 * frame after the script has executed successfully.
 *
 * @param env The global state of the application.
 * @param frames The frames where the kite positions should be updated to the
 * current kite values.
 */
void tkbc_patch_block_frame_kite_positions(Env *env, Frames *frames) {
  for (size_t i = 0; i < frames->count; ++i) {
    if (!frames->elements[i].kite_id_array.count) {
      continue;
    }

    for (size_t j = 0; j < frames->elements[i].kite_id_array.count; ++j) {
      Index kite_id = frames->elements[i].kite_id_array.elements[j];
      Kite *kite = tkbc_get_kite_by_id(env, kite_id);
      assert(kite != NULL);

      Kite_Position kite_position = {
          .kite_id = kite_id,
          .position = kite->center,
          .angle = kite->angle,
      };

      bool contains = false;
      for (size_t k = 0; k < frames->kite_frame_positions.count; ++k) {
        if (frames->kite_frame_positions.elements[k].kite_id == kite_id) {
          contains = true;
          // NOTE: Patching angle in case the kite_position was
          // already added by just a move action, but later the corresponding
          // angle action is handled.
          frames->kite_frame_positions.elements[k].position =
              kite_position.position;
          frames->kite_frame_positions.elements[k].angle = kite_position.angle;
          break;
        }
      }

      if (!contains) {
        tkbc_dap(&frames->kite_frame_positions, kite_position);
      }
    }
  }
}

/**
 * @brief The function can be used to get the state of the current executed
 * frame.
 *
 * @param env The global state of the application.
 * @return True if the current frame has finished its execution, otherwise
 * false.
 */
bool tkbc_check_finished_frames(Env *env) {
  for (size_t i = 0; i < env->frames->count; ++i) {
    if (!env->frames->elements[i].finished) {
      return false;
    }
  }
  return true;
}

/**
 * @brief The function can collect the amount of finished frames in the
 * current block frame execution.
 *
 * @param env The global state of the application.
 * @return It returns the amount of finished frames and 0 if no frames has
 * finished yet.
 */
size_t tkbc_check_finished_frames_count(Env *env) {
  int count = 0;
  for (size_t i = 0; i < env->frames->count; ++i) {
    if (env->frames->elements[i].finished) {
      count++;
    }
  }

  return count;
}

/**
 * @brief The function switches to the next available script. It loads the
 * views block_frame and frames in the env. And sets the kite_frame_positions.
 *
 * @param env The global state of the application.
 */
void tkbc_load_next_script(Env *env) {
  if (env->script_counter == 0) {
    return;
  }
  // Switch to next script.
  // NOTE: The first iteration has no loaded value jet so 0 is default.
  size_t id = env->block_frame == NULL ? 0 : env->block_frame->script_id;
  size_t script_index = id % env->block_frames->count;
  size_t script_id = env->block_frames->elements[script_index].script_id;
  tkbc_load_script_id(env, script_id);
}

/**
 * @brief The function sets the views of the script in the env and loads the
 * corresponding kite_frame_positions of the first frame.
 *
 * @param env The global state of the application.
 * @param script_id The id of the script that should be loaded into the
 * current execution.
 */
void tkbc_load_script_id(Env *env, size_t script_id) {
  for (size_t i = 0; i < env->block_frames->count; ++i) {
    if (env->block_frames->elements[i].script_id == script_id) {
      env->block_frame = &env->block_frames->elements[i];
      break;
    }
  }

  env->frames = &env->block_frame->elements[0];
  tkbc_set_kite_positions_from_kite_frames_positions(env);
  env->script_finished = false;
}

/**
 * @brief The function checks for the user input that is related to a script
 * execution. It can control the timeline and stop and start the execution.
 *
 * @param env The global state of the application.
 */
void tkbc_input_handler_script(Env *env) {
  // Hard reset to startposition angel 0
  // KEY_ENTER
  if (IsKeyDown(
          tkbc_hash_to_key(*env->keymaps, KMH_SET_KITES_TO_START_POSITION))) {
    tkbc_kite_array_start_position(env->kite_array, env->window_width,
                                   env->window_height);
  }
  // KEY_SPACE
  if (IsKeyPressed(
          tkbc_hash_to_key(*env->keymaps, KMH_TOGGLE_SCRIPT_EXECUTION))) {
    if (env->frames) {
      env->script_finished = !env->script_finished;
    }
  }
  // KEY_TAB
  if (IsKeyPressed(tkbc_hash_to_key(*env->keymaps, KMH_SWITCHES_NEXT_SCRIPT))) {
    assert(env->script_counter <= env->block_frames->count);
    tkbc_load_next_script(env);
  }

  tkbc_scrub_frames(env);
}

/**
 * @brief The function can be used to update all the kites positions and angle
 * that are registered in the currently loaded frame of the script before.
 *
 * @param env The global state of the application.
 */
void tkbc_set_kite_positions_from_kite_frames_positions(Env *env) {
  // TODO: Think about kites that are move in the previous frame but not in
  // the current one. The kites can end up in wired locations, because the
  // state is not exactly as if the script has executed from the beginning.
  assert(env->frames);
  for (size_t i = 0; i < env->frames->kite_frame_positions.count; ++i) {
    Id id = env->frames->kite_frame_positions.elements[i].kite_id;
    Kite *kite = tkbc_get_kite_by_id(env, id);
    assert(kite != NULL && "Unexpected data lose.");

    Vector2 position = env->frames->kite_frame_positions.elements[i].position;
    float angle = env->frames->kite_frame_positions.elements[i].angle;

    tkbc_center_rotation(kite, &position, angle);

    // For the correct recomputation of the action where the slider is set to.
    kite->old_angle = kite->angle;
    kite->old_center = kite->center;
  }
}

/**
 * @brief The function computes the frame state of the timeline and syncs up
 * the currently loaded frame. It computes mouse control of the timeline.
 *
 * @param env The global state of the application.
 */
void tkbc_scrub_frames(Env *env) {
  if (env->block_frame == NULL) {
    return;
  }

  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && env->timeline_interaction) {
    int mouse_x = GetMouseX();
    float slider = env->timeline_front.x + env->timeline_front.width;
    float c = mouse_x - slider;
    bool drag_left = c <= 0;

    env->script_finished = true;

    // The block indexes are assumed in order and at the corresponding index.
    int index =
        drag_left ? env->frames->block_index - 1 : env->frames->block_index + 1;

    if (index >= 0 && index < (int)env->block_frame->count) {
      env->frames = &env->block_frame->elements[index];
    }
    tkbc_set_kite_positions_from_kite_frames_positions(env);
  }
}

// ========================== SCRIPT HANDLER INTERNAL ========================

/**
 * @brief The function handles the computation of the new position of the kite
 * corresponding to the called move action.
 *
 * @param kite The kite where the new position is calculated for.
 * @param position The new position of the kite.
 * @param duration The time it should take to interpolate the kite to the new
 * position.
 * @return The amount that the center position has moved to the final
 * position.
 */
Vector2 tkbc_script_move(Kite *kite, Vector2 position, float duration) {
  if (duration <= 0) {
    Vector2 result = Vector2Subtract(position, kite->center);
    tkbc_kite_update_position(kite, &position);
    return result;
  }

  if (Vector2Equals(kite->center, position)) {
    // NOTE:This might be just (0,0), because the  precision is not needed?
    Vector2 result = Vector2Subtract(position, kite->center);
    tkbc_kite_update_position(kite, &position);
    return result;
  }

  float dt = tkbc_get_frame_time();
  Vector2 d = Vector2Subtract(position, kite->old_center);
  Vector2 dnorm = Vector2Normalize(d);
  Vector2 dnormscale = Vector2Scale(dnorm, (Vector2Length(d) / duration * dt));

  if (Vector2Length(dnormscale) >=
      Vector2Length(Vector2Subtract(position, kite->center))) {
    tkbc_kite_update_position(kite, &position);
    return Vector2Subtract(position, kite->center);
  } else {
    Vector2 it = Vector2Add(kite->center, dnormscale);
    tkbc_kite_update_position(kite, &it);
    return dnormscale;
  }
}

/**
 * @brief The function handles the computation of the new rotation of the kite
 * corresponding to the called rotation action.
 *
 * @param env The global state of the application.
 * @param kite The kite that should be handled.
 * @param angle The new angle the kite should be rotated to or by depended on
 * the adding parameter.
 * @param duration The time it should take to interpolate the kite to the new
 * angle.
 * @param adding The parameter changes if the angle value is going to be added
 * to the kite or if the kite angle should change to the given angle.
 * @return The delta amount the kite angle changes.
 */
float tkbc_script_rotate(Kite *kite, float angle, float duration, bool adding) {

  // NOTE: For the instant rotation the computation can be simpler by just
  // calling the direction angles, but for future line wrap calculation the
  // actual rotation direction is called instead.
  if (duration <= 0) {
    if (adding) {
      tkbc_kite_update_angle(kite, kite->old_angle + angle);
    } else {
      tkbc_kite_update_angle(kite, angle);
    }
    return fabsf(angle);
  }

  float dt = tkbc_get_frame_time();
  float d = fabsf(angle);
  float ds = d / duration * dt;

  if (ds >= fabsf(kite->old_angle) + d) {
    if (adding) {
      tkbc_kite_update_angle(kite, kite->old_angle + angle);
    } else {
      tkbc_kite_update_angle(kite, angle);
    }
    return fabsf(ds);
  }
  // NOTE: For the non adding version the finish detection will stop the
  // calculation at the correct point so the angle computation is not needed
  // her.
  if (signbit(angle) != 0) {
    tkbc_kite_update_angle(kite, kite->angle - ds);
  } else {
    tkbc_kite_update_angle(kite, kite->angle + ds);
  }
  return fabsf(ds);
}

/**
 * @brief The function handles the computation of the new tip rotation of the
 * kite corresponding to the called tip rotation action.
 *
 * @param env The global state of the application.
 * @param kite The kite that should be handled.
 * @param tip The tip of the leading kites edge.
 * @param angle The new angle the kite should be rotated to or by depended on
 * the adding parameter.
 * @param duration The time it should take to interpolate the kite to the new
 * angle.
 * @param adding The parameter changes if the angle value is going to be added
 * to the kite or if the kite angle should change to the given angle.
 * @return The delta amount the kite angle changes.
 */
float tkbc_script_rotate_tip(Kite *kite, TIP tip, float angle, float duration,
                             bool adding) {

  // NOTE: For the instant rotation the computation can be simpler by just
  // calling the direction angles, but for future line wrap calculation the
  // actual rotation direction is called instead.
  if (duration <= 0) {
    if (adding) {
      tkbc_tip_rotation(kite, NULL, kite->old_angle + angle, tip);
    } else {
      tkbc_tip_rotation(kite, NULL, angle, tip);
    }
    return fabsf(angle);
  }

  float dt = tkbc_get_frame_time();
  float d = fabsf(angle);
  float ds = d / duration * dt;

  if (ds >= fabsf(kite->old_angle) + d) {
    // Provided the old position, because the kite center moves as a circle
    // around the old fixed position.
    if (adding) {
      tkbc_tip_rotation(kite, &kite->old_center, kite->old_angle + angle, tip);
    } else {
      tkbc_tip_rotation(kite, &kite->old_center, angle, tip);
    }
    return fabsf(ds);
  }
  // NOTE: For the non adding version the finish detection will stop the
  // calculation at the correct point so the angle computation is not needed
  // her.
  if (signbit(angle) != 0) {
    tkbc_tip_rotation(kite, NULL, kite->angle - ds, tip);
  } else {
    tkbc_tip_rotation(kite, NULL, kite->angle + ds, tip);
  }
  return fabsf(ds);
}

/**
 * @brief The function computes the intermediate angle that represents the
 * difference to zero from the current kite angle and respects the sign of the
 * 0 angle. The intermediate angle is returned and the action stays
 * unmodified.
 *
 * @param kite The kite the intermediate angel should be calculated for.
 * @param kind The action kind.
 * @param action The frame action that is a rotation variant.
 * @param duration The current frame duration.
 * @return The intermediate angle that represents the distance to 0.
 */
float tkbc_check_angle_zero(Kite *kite, Action_Kind kind, Action action,
                            float duration) {
  switch (kind) {
  case KITE_ROTATION:
  case KITE_ROTATION_ADD:
    if (action.as_rotation.angle != 0 || duration <= 0) {
      return action.as_rotation.angle;
    }

    if (signbit(action.as_rotation.angle) == 0) {
      /* Positive 0 */
      if (kite->old_angle < 0) {
        /* Negative angle 0 = -90 +90 */
        return fabsf(fmodf(kite->old_angle, 360));
      } else {
        /* Positive angle 0 = 90 +270 ->  270 = + 360 - (90) */
        /* The case where the old angle is already 0 is handled by the outer
         * mod. */
        return fmodf(360 - fmodf(kite->old_angle, 360), 360);
      }
    } else {
      /* Negative 0 */
      if (kite->old_angle < 0) {
        /* Negative angle 0 = -90 -270 -> -270 = -360 + 90 = -(360 - (90))
         */
        /* The case where the old angle is already 0 is handled by the outer
         * mod. */
        return -fmodf(360 - fmodf(kite->old_angle, 360), 360);

      } else {
        /* Positive angle 0 = 90 -90 */
        return -fabsf(fmodf(kite->old_angle, 360));
      }
    }

  case KITE_TIP_ROTATION:
  case KITE_TIP_ROTATION_ADD:
    if (action.as_tip_rotation.angle != 0 || duration <= 0) {
      return action.as_tip_rotation.angle;
    }

    if (signbit(action.as_tip_rotation.angle) == 0) {
      if (kite->old_angle < 0) {
        return fabsf(fmodf(kite->old_angle, 360));
      } else {
        return fmodf(360 - fmodf(kite->old_angle, 360), 360);
      }
    } else {
      if (kite->old_angle < 0) {
        return -fmodf(360 - fmodf(kite->old_angle, 360), 360);
      } else {
        return -fabsf(fmodf(kite->old_angle, 360));
      }
    }
  default:
    assert(0 && "UNREACHABLE tkbc_check_angle_zero()");
  }
}
