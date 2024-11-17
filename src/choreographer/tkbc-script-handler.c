#include <assert.h>
#include <math.h>
#include <raylib.h>
#include <stdlib.h>

#include "../global/tkbc-types.h"
#include "../global/tkbc-utils.h"
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
  Frame *frame = calloc(1, sizeof(*frame));
  if (frame == NULL) {
    fprintf(stderr, "ERROR: No more memory can be allocated.\n");
    return NULL;
  }
  frame->kite_index_array = calloc(1, sizeof(*frame->kite_index_array));
  if (frame->kite_index_array == NULL) {
    fprintf(stderr, "ERROR: No more memory can be allocated.\n");
    return NULL;
  }

  return frame;
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
 * @return The new allocated and value ready copy of the frames.
 */
Frames *tkbc_deep_copy_frames(Frames *frames) {
  if (frames->elements == NULL || frames == NULL) {
    return NULL;
  }

  Frames *new_frames = calloc(1, sizeof(*new_frames));
  if (new_frames == NULL) {
    fprintf(stderr, "ERROR: No more memory can be allocated.\n");
    return NULL;
  }

  if (frames->kite_frame_positions != NULL) {
    new_frames->kite_frame_positions =
        calloc(1, sizeof(*new_frames->kite_frame_positions));
    if (new_frames->kite_frame_positions == NULL) {
      fprintf(stderr, "ERROR: No more memory can be allocated.\n");
      return NULL;
    }

    tkbc_dapc(new_frames->kite_frame_positions,
              frames->kite_frame_positions->elements,
              frames->kite_frame_positions->count);
  }

  for (size_t i = 0; i < frames->count; ++i) {
    Frame new_frame = frames->elements[i];

    void *old_action = frames->elements[i].action;
    if (old_action == NULL) {
      tkbc_dap(new_frames, new_frame);
      continue;
    } else {
      new_frame.action = tkbc_move_action_to_heap(
          frames->elements[i].action, frames->elements[i].kind, false);
    }

    Kite_Indexs *old_kite_index_array = frames->elements[i].kite_index_array;
    if (old_kite_index_array == NULL) {
      tkbc_dap(new_frames, new_frame);
      continue;
    }

    Kite_Indexs *new_kite_index_array =
        calloc(1, sizeof(*new_kite_index_array));
    if (new_kite_index_array == NULL) {
      fprintf(stderr, "ERROR: No more memory can be allocated.\n");
      return NULL;
    }

    tkbc_dapc(new_kite_index_array, old_kite_index_array->elements,
              old_kite_index_array->count);

    new_frame.kite_index_array = new_kite_index_array;

    tkbc_dap(new_frames, new_frame);
  }

  new_frames->block_index = frames->block_index;
  return new_frames;
}

/**
 * @brief The function copies every single value even the values that are just
 * represented by a pointer of the struct Block_Frame to a new instance. It can
 * be used to move a creation of a temporary struct of type Block_Frame to a
 * permanently stored one.
 *
 * @param block_frame The pointer that holds the values that should be copied.
 * @return The new allocated and value ready copy of the block_frame.
 */
Block_Frame *tkbc_deep_copy_block_frame(Block_Frame *block_frame) {

  Block_Frame *new_block_frame = calloc(1, sizeof(*new_block_frame));
  if (new_block_frame == NULL) {
    fprintf(stderr, "ERROR: No more memory can be allocated.\n");
    return NULL;
  }

  assert(block_frame->count > 0);
  for (size_t i = 0; i < block_frame->count; ++i) {
    tkbc_dap(new_block_frame,
             *tkbc_deep_copy_frames(&block_frame->elements[i]));
  }
  new_block_frame->script_id = block_frame->script_id;
  return new_block_frame;
}

/**
 * @brief The function can be used to free all the elements and related memory
 * of the given frames. It recursevly handles all the internal saved values.
 *
 * @param frames The frames the memory should be free.
 */
void tkbc_destroy_frames(Frames *frames) {
  if (frames->count != 0) {
    for (size_t i = 0; i < frames->count; ++i) {
      free(frames->elements[i].kite_index_array);
      free(frames->elements[i].action);
    }

    if (frames->kite_frame_positions != NULL) {
      free(frames->kite_frame_positions->elements);
    }
    frames->count = 0;
  }
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
  assert(ACTION_KIND_COUNT == 9 && "NOT ALL THE Action_Kinds ARE IMPLEMENTED");
  switch (frame->kind) {
  case KITE_WAIT: {
    Wait_Action *action = frame->action;
    if (frame->duration <= 0) {
      frame->finished = true;
      frame->duration = 0;
    } else {
      double current_time = GetTime();
      frame->duration -= current_time - action->starttime;
      action->starttime = current_time;
    }

  } break;
  case KITE_QUIT: {

    if (tkbc_check_finished_frames_count(env) == env->frames->count - 1) {
      frame->finished = true;
      break;
    }

    Quit_Action *action = frame->action;
    if (frame->duration <= 0) {
      frame->finished = true;
    } else {
      double current_time = GetTime();
      frame->duration -= current_time - action->starttime;
      action->starttime = current_time;
    }

  } break;
  case KITE_MOVE_ADD: {
    Kite *kite;
    Move_Add_Action *action = frame->action;
    Frame *env_frame = &env->frames->elements[frame->index];
    assert(env_frame->kite_index_array->count > 0);

    for (size_t i = 0; i < env_frame->kite_index_array->count; ++i) {
      for (size_t k = 0; k < env->kite_array->count; ++k) {
        if (env_frame->kite_index_array->elements[i] ==
            env->kite_array->elements[i].kite_id) {
          kite = env->kite_array->elements[k].kite;
          break;
        }
      }

      // Description of the calculation for the position
      // kite.center + (action->position - (kite.center - kite->old_center))
      // kite.center + (action->position - kite.center + kite->old_center)
      // kite.center + action->position - kite.center + kite->old_center

      tkbc_script_move(kite, Vector2Add(kite->old_center, action->position),
                       frame->duration);

      if (Vector2Equals(kite->center,
                        Vector2Add(kite->old_center, action->position))) {
        frame->finished = true;

        {
          // Every kite in the frame should get sync at the end of the frame.
          for (size_t i = 0; i < frame->kite_index_array->count; ++i) {
            for (size_t k = 0; k < env->kite_array->count; ++k) {
              if (env->frames->elements[frame->index]
                      .kite_index_array->elements[i] ==
                  env->kite_array->elements[i].kite_id) {
                kite = env->kite_array->elements[k].kite;
                break;
              }
            }

            tkbc_script_move(kite,
                             Vector2Add(kite->old_center, action->position), 0);
          }
        }
        break;
      }
    }

  } break;
  case KITE_MOVE: {
    Kite *kite;
    Move_Action *action = frame->action;
    Frame *env_frame = &env->frames->elements[frame->index];
    assert(env_frame->kite_index_array->count > 0);

    for (size_t i = 0; i < env_frame->kite_index_array->count; ++i) {
      for (size_t k = 0; k < env->kite_array->count; ++k) {
        if (env_frame->kite_index_array->elements[i] ==
            env->kite_array->elements[i].kite_id) {
          kite = env->kite_array->elements[k].kite;
          break;
        }
      }

      tkbc_script_move(kite, action->position, frame->duration);

      if (Vector2Equals(kite->center, action->position)) {
        frame->finished = true;

        {
          // Every kite in the frame should get sync at the end of the frame.
          for (size_t i = 0; i < frame->kite_index_array->count; ++i) {
            for (size_t k = 0; k < env->kite_array->count; ++k) {
              if (env->frames->elements[frame->index]
                      .kite_index_array->elements[i] ==
                  env->kite_array->elements[i].kite_id) {
                kite = env->kite_array->elements[k].kite;
                break;
              }
            }
            tkbc_script_move(kite, action->position, 0);
          }
        }
        break;
      }
    }

  } break;
  case KITE_ROTATION_ADD: {
    Kite_State *state;
    Kite *kite;
    Rotation_Action *action = frame->action;
    Frame *env_frame = &env->frames->elements[frame->index];
    assert(env_frame->kite_index_array->count > 0);

    for (size_t i = 0; i < env_frame->kite_index_array->count; ++i) {
      for (size_t k = 0; k < env->kite_array->count; ++k) {
        if (env_frame->kite_index_array->elements[i] ==
            env->kite_array->elements[i].kite_id) {
          state = &env->kite_array->elements[k];
          kite = state->kite;
          break;
        }
      }

      tkbc_script_rotate(env, state, action->angle, frame->duration, true);

      float remaining_angle = 0;
      for (size_t k = 0; k < env->frames->kite_frame_positions->count; ++k) {
        if (env->frames->kite_frame_positions->elements[k].kite_id ==
            state->kite_id) {

          remaining_angle =
              env->frames->kite_frame_positions->elements[k].remaining_angle;
          break;
        }
      }

      if (remaining_angle <= 0 ||
          FloatEquals(floorf(kite->old_angle + action->angle),
                      floorf(kite->angle))) {
        frame->finished = true;
      }
    }
  } break;
  case KITE_ROTATION: {
    Kite_State *state;
    Kite *kite;
    Rotation_Action *action = frame->action;
    Frame *env_frame = &env->frames->elements[frame->index];
    assert(env_frame->kite_index_array->count > 0);

    for (size_t i = 0; i < env_frame->kite_index_array->count; ++i) {
      for (size_t k = 0; k < env->kite_array->count; ++k) {
        if (env_frame->kite_index_array->elements[i] ==
            env->kite_array->elements[i].kite_id) {
          state = &env->kite_array->elements[k];
          kite = state->kite;
          break;
        }
      }

      tkbc_script_rotate(env, state, action->angle, frame->duration, false);

      float remaining_angle = 0;
      for (size_t k = 0; k < env->frames->kite_frame_positions->count; ++k) {
        if (env->frames->kite_frame_positions->elements[k].kite_id ==
            state->kite_id) {

          remaining_angle =
              env->frames->kite_frame_positions->elements[k].remaining_angle;
          break;
        }
      }

      // NOTE: Different from the ADDing version.
      if (remaining_angle <= 0 ||
          FloatEquals(floorf(action->angle), floorf(kite->angle))) {
        frame->finished = true;
      }
    }
  } break;
  case KITE_TIP_ROTATION_ADD: {
    Kite_State *state;
    Kite *kite;
    Tip_Rotation_Action *action = frame->action;
    Frame *env_frame = &env->frames->elements[frame->index];
    assert(env_frame->kite_index_array->count > 0);

    for (size_t i = 0; i < env_frame->kite_index_array->count; ++i) {
      for (size_t k = 0; k < env->kite_array->count; ++k) {
        if (env_frame->kite_index_array->elements[i] ==
            env->kite_array->elements[i].kite_id) {
          state = &env->kite_array->elements[k];
          kite = state->kite;
          break;
        }
      }

      tkbc_script_rotate_tip(env, state, action->tip, action->angle,
                             frame->duration, true);

      float remaining_angle = 0;
      for (size_t k = 0; k < env->frames->kite_frame_positions->count; ++k) {
        if (env->frames->kite_frame_positions->elements[k].kite_id ==
            state->kite_id) {

          remaining_angle =
              env->frames->kite_frame_positions->elements[k].remaining_angle;
          break;
        }
      }

      if (remaining_angle <= 0 ||
          FloatEquals(floorf(kite->old_angle + action->angle),
                      floorf(kite->angle))) {
        frame->finished = true;
      }
    }
  } break;
  case KITE_TIP_ROTATION: {
    Kite_State *state;
    Kite *kite;
    Tip_Rotation_Action *action = frame->action;
    Frame *env_frame = &env->frames->elements[frame->index];
    assert(env_frame->kite_index_array->count > 0);

    for (size_t i = 0; i < env_frame->kite_index_array->count; ++i) {
      for (size_t k = 0; k < env->kite_array->count; ++k) {
        if (env_frame->kite_index_array->elements[i] ==
            env->kite_array->elements[i].kite_id) {
          state = &env->kite_array->elements[k];
          kite = state->kite;
          break;
        }
      }

      tkbc_script_rotate_tip(env, state, action->tip, action->angle,
                             frame->duration, false);

      float remaining_angle = 0;
      for (size_t k = 0; k < env->frames->kite_frame_positions->count; ++k) {
        if (env->frames->kite_frame_positions->elements[k].kite_id ==
            state->kite_id) {

          remaining_angle =
              env->frames->kite_frame_positions->elements[k].remaining_angle;
          break;
        }
      }

      // NOTE: Different from the ADDing version.
      if (remaining_angle <= 0 ||
          FloatEquals(floorf(action->angle), floorf(kite->angle))) {
        frame->finished = true;
      }
    }
  } break;
  default:
    assert(0 && "UNREACHABLE tkbc_render_frame()");
    break;
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
      ((Wait_Action *)frame->action)->starttime = GetTime();
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
    if (frames->elements[i].kite_index_array == NULL) {
      continue;
    }

    for (size_t j = 0; j < frames->elements[i].kite_index_array->count; ++j) {
      Index kite_id = frames->elements[i].kite_index_array->elements[j];
      Index kite_index = 0;
      for (size_t index = 0; index < env->kite_array->count; ++index) {
        if (kite_id == env->kite_array->elements[index].kite_id) {
          kite_index = index;
          break;
        }
      }

      Frame *f = &frames->elements[i];
      float patch_angle = 0;
      assert(ACTION_KIND_COUNT == 9 &&
             "NOT ALL THE Action_Kinds ARE IMPLEMENTED");
      switch (f->kind) {
      case KITE_TIP_ROTATION:
      case KITE_ROTATION: {
        Rotation_Action *a = f->action;
        patch_angle =
            fabsf(env->kite_array->elements[kite_index].kite->angle - a->angle);

      } break;
      case KITE_TIP_ROTATION_ADD:
      case KITE_ROTATION_ADD: {
        Rotation_Action *a = f->action;
        patch_angle = fabsf(a->angle);
      } break;
      default: {
      }
      }

      Kite_Position kite_position = {
          .kite_id = kite_id,
          .position = env->kite_array->elements[kite_index].kite->center,
          .angle = env->kite_array->elements[kite_index].kite->angle,
          .remaining_angle = patch_angle,
      };

      // For the case just frames buffer is on the stack allocated.
      if (frames->kite_frame_positions == NULL) {
        frames->kite_frame_positions =
            calloc(1, sizeof(*frames->kite_frame_positions));
        if (frames->kite_frame_positions == NULL) {
          fprintf(stderr, "ERROR: No more memory can be allocated.\n");
          assert(0 && "ERROR: No more memory left!");
        }
      }

      bool contains = false;
      for (size_t k = 0; k < frames->kite_frame_positions->count; ++k) {
        if (frames->kite_frame_positions->elements[k].kite_id == kite_id) {
          contains = true;
          // NOTE: Patching remaining_angle in case the kite_position was
          // already added by just a move action, but later the corresponding
          // angle action is handled.
          assert(ACTION_KIND_COUNT == 9 &&
                 "NOT ALL THE Action_Kinds ARE IMPLEMENTED");
          switch (f->kind) {
          case KITE_MOVE:
          case KITE_MOVE_ADD: {
            frames->kite_frame_positions->elements[k].position =
                kite_position.position;
          } break;
          case KITE_TIP_ROTATION:
          case KITE_ROTATION:
          case KITE_TIP_ROTATION_ADD:
          case KITE_ROTATION_ADD: {
            frames->kite_frame_positions->elements[k].remaining_angle =
                kite_position.remaining_angle;
            frames->kite_frame_positions->elements[k].angle =
                kite_position.angle;
          } break;
          default: {
          }
          }

          break;
        }
      }

      if (!contains) {
        tkbc_dap(frames->kite_frame_positions, kite_position);
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
 * @brief The function can collect the amount of finished frames in the current
 * block frame execution.
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
 * @brief The function checks for the user input that is related to a script
 * execution. It can control the timeline and stop and start the execution.
 *
 * @param env The global state of the application.
 */
void tkbc_input_handler_script(Env *env) {
  if (IsKeyPressed(KEY_SPACE)) {
    env->script_finished = !env->script_finished;
  }
  if (IsKeyPressed(KEY_TAB)) {
    assert(env->script_counter <= env->block_frames->count);

    if (env->script_counter > 0) {
      // Switch to next script.
      assert(env->block_frames->count != 0);

      size_t count = env->block_frames->count;

      // NOTE: For this to work for the first iteration it relies on the calloc
      // functionality to zero out the rest of the struct.
      size_t id = env->block_frame->script_id;

      size_t script_index = id % count;

      env->block_frame = &env->block_frames->elements[script_index];

      env->frames = &env->block_frame->elements[0];

      tkbc_set_kite_positions_from_kite_frames_positions(env);

      env->script_finished = false;
    }
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
  // TODO: Think about kites that are move in the previous frame but not in the
  // current one. The kites can end up in wired locations, because the state is
  // not exactly as if the script has executed from the beginning.
  Kite *kite;
  for (size_t i = 0; i < env->frames->kite_frame_positions->count; ++i) {
    for (size_t k = 0; k < env->kite_array->count; ++k) {
      if (env->frames->kite_frame_positions->elements[i].kite_id ==
          env->kite_array->elements[i].kite_id) {
        kite = env->kite_array->elements[k].kite;
        break;
      }
    }

    Vector2 position = env->frames->kite_frame_positions->elements[i].position;
    float angle = env->frames->kite_frame_positions->elements[i].angle;

    tkbc_center_rotation(kite, &position, angle);

    // For the correct recomputation of the action where the slider is set to.
    kite->old_angle = kite->angle;
    kite->old_center = kite->center;
  }
}

/**
 * @brief The function computes the frame state of the timeline and syncs up the
 * currently loaded frame. It computes mouse control of the timeline.
 *
 * @param env The global state of the application.
 */
void tkbc_scrub_frames(Env *env) {
  if (env->block_frame->count <= 0) {
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
 */
void tkbc_script_move(Kite *kite, Vector2 position, float duration) {

  if (duration <= 0) {
    tkbc_center_rotation(kite, &position, kite->angle);
    return;
  }

  if (Vector2Equals(kite->center, position)) {
    tkbc_center_rotation(kite, &position, kite->angle);
    return;
  }

  float dt = GetFrameTime();
  Vector2 d = Vector2Subtract(position, kite->old_center);
  Vector2 dnorm = Vector2Normalize(d);
  Vector2 dnormscale = Vector2Scale(dnorm, (Vector2Length(d) / duration * dt));

  if (Vector2Length(dnormscale) >=
      Vector2Length(Vector2Subtract(position, kite->center))) {
    tkbc_center_rotation(kite, &position, kite->angle);
  } else {
    Vector2 it = Vector2Add(kite->center, dnormscale);
    tkbc_center_rotation(kite, &it, kite->angle);
  }
}

/**
 * @brief The function handles the computation of the new rotation of the kite
 * corresponding to the called rotation action.
 *
 * @param env The global state of the application.
 * @param state The kite state that should be handled.
 * @param angle The new angle the kite should be rotated to or by depended on
 * the adding parameter.
 * @param duration The time it should take to interpolate the kite to the new
 * angle.
 * @param adding The parameter changes if the angle value is going to be added
 * to the kite or if the kite angle should change to the given angle.
 */
void tkbc_script_rotate(Env *env, Kite_State *state, float angle,
                        float duration, bool adding) {

  // NOTE: For the instant rotation the computation can be simpler by just
  // calling the direction angles, but for future line wrap calculation the
  // actual rotation direction is called instead.
  if (duration <= 0) {
    if (adding) {
      if (angle <= 0) {
        tkbc_center_rotation(state->kite, NULL, state->kite->angle - angle);
      } else {
        tkbc_center_rotation(state->kite, NULL, state->kite->angle + angle);
      }
    } else {
      if (angle <= 0) {
        tkbc_center_rotation(state->kite, NULL, -angle);
      } else {
        tkbc_center_rotation(state->kite, NULL, angle);
      }
    }

    for (size_t k = 0; k < env->frames->kite_frame_positions->count; ++k) {
      if (env->frames->kite_frame_positions->elements[k].kite_id ==
          state->kite_id) {
        env->frames->kite_frame_positions->elements[k].remaining_angle -=
            fabsf(angle);
        break;
      }
    }
    return;
  }

  float dt = GetFrameTime();
  int fps = GetFPS();

  float d = angle - state->kite->old_angle;
  float dnorm = fmodf(d, 360);
  float dnormscale = dnorm + (d / (duration * dt));

  float segment_size = fabsf(angle / duration) / (float)fps;
  float remaining_angle = 0;
  for (size_t k = 0; k < env->frames->kite_frame_positions->count; ++k) {
    if (env->frames->kite_frame_positions->elements[k].kite_id ==
        state->kite_id) {

      if (!adding) {
        float pre_angle = env->frames->kite_frame_positions->elements[k].angle;
        segment_size = fabsf((pre_angle - angle) / duration) / (float)fps;
      }

      env->frames->kite_frame_positions->elements[k].remaining_angle -=
          segment_size * GetFrameTime();

      remaining_angle =
          env->frames->kite_frame_positions->elements[k].remaining_angle;
      break;
    }
  }

  if (remaining_angle <= 0) {

    // TODO: This part is not needed if the computation of the float for the
    // finished frame is correctly detected.

    if (adding) {
      if (angle <= 0) {
        tkbc_center_rotation(state->kite, NULL, state->kite->old_angle - angle);
      } else {
        tkbc_center_rotation(state->kite, NULL, state->kite->old_angle + angle);
      }
    } else {
      if (angle <= 0) {
        tkbc_center_rotation(state->kite, NULL, -angle);
      } else {
        tkbc_center_rotation(state->kite, NULL, angle);
      }
    }

  } else {
    if (angle <= 0) {
      // tkbc_center_rotation(state->kite, NULL,
      //                      state->kite->angle - segment_size);
      tkbc_center_rotation(state->kite, NULL, state->kite->angle - dnormscale);
    } else {
      // tkbc_center_rotation(state->kite, NULL,
      //                      state->kite->angle + segment_size);
      tkbc_center_rotation(state->kite, NULL, state->kite->angle + dnormscale);
    }
  }
}

/**
 * @brief The function handles the computation of the new tip rotation of the
 * kite corresponding to the called tip rotation action.
 *
 * @param env The global state of the application.
 * @param state The kite state that should be handled.
 * @param tip The tip of the leading kites edge.
 * @param angle The new angle the kite should be rotated to or by depended on
 * the adding parameter.
 * @param duration The time it should take to interpolate the kite to the new
 * angle.
 * @param adding The parameter changes if the angle value is going to be added
 * to the kite or if the kite angle should change to the given angle.
 */
void tkbc_script_rotate_tip(Env *env, Kite_State *state, TIP tip, float angle,
                            float duration, bool adding) {

  // NOTE: For the instant rotation the computation can be simpler by just
  // calling the direction angles, but for future line wrap calculation the
  // actual rotation direction is called instead.
  if (duration <= 0) {
    if (adding) {
      if (angle <= 0) {
        tkbc_tip_rotation(state->kite, NULL, state->kite->angle - angle, tip);
      } else {
        tkbc_tip_rotation(state->kite, NULL, state->kite->angle + angle, tip);
      }
    } else {
      if (angle <= 0) {
        tkbc_tip_rotation(state->kite, NULL, -angle, tip);
      } else {
        tkbc_tip_rotation(state->kite, NULL, angle, tip);
      }
    }

    for (size_t k = 0; k < env->frames->kite_frame_positions->count; ++k) {
      if (env->frames->kite_frame_positions->elements[k].kite_id ==
          state->kite_id) {
        env->frames->kite_frame_positions->elements[k].remaining_angle -=
            fabsf(angle);
        break;
      }
    }
    return;
  }

  int fps = GetFPS();
  float segment_size = fabsf(angle / duration) / (float)fps;
  float remaining_angle = 0;

  for (size_t k = 0; k < env->frames->kite_frame_positions->count; ++k) {
    if (env->frames->kite_frame_positions->elements[k].kite_id ==
        state->kite_id) {

      if (!adding) {
        float pre_angle = env->frames->kite_frame_positions->elements[k].angle;
        segment_size = fabsf((pre_angle - angle) / duration) / (float)fps;
      }

      env->frames->kite_frame_positions->elements[k].remaining_angle -=
          segment_size * GetFrameTime();

      remaining_angle =
          env->frames->kite_frame_positions->elements[k].remaining_angle;
      break;
    }
  }

  if (remaining_angle <= 0) {

    // TODO: This part is not needed if the computation of the float for the
    // finished frame is correctly detected.

    // Provided the old position, because the kite center moves as a circle
    // around the old fixed position.

    if (angle <= 0) {
      tkbc_tip_rotation(state->kite, &state->kite->old_center,
                        state->kite->old_angle - angle, tip);
    } else {
      tkbc_tip_rotation(state->kite, &state->kite->old_center,
                        state->kite->old_angle + angle, tip);
    }

  } else {
    if (angle <= 0) {
      tkbc_tip_rotation(state->kite, NULL, state->kite->angle - segment_size,
                        tip);
    } else {
      tkbc_tip_rotation(state->kite, NULL, state->kite->angle + segment_size,
                        tip);
    }
  }
}
