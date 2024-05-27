// ===========================================================================
// ========================== Script Handler =================================
// ===========================================================================

#include "kite_utils.h"
#include "tkbc.h"
#include <assert.h>
#include <raylib.h>
#include <raymath.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Frame *kite_frame_init() {
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
 * @brief [TODO:description]
 *
 * @param kind [TODO:parameter]
 * @param kite_indexs [TODO:parameter]
 * @param raw_action [TODO:parameter]
 * @param duration [TODO:parameter]
 * @return [TODO:return]
 */
Frame *kite_gen_frame(Action_Kind kind, Kite_Indexs kite_indexs,
                      void *raw_action, float duration) {

  void *action;
  Frame *frame = kite_frame_init();
  switch (kind) {
  case KITE_MOVE: {
    action_alloc(Move_Action);
    action = (Move_Action *)raw_action;

  } break;
  case KITE_ROTATION: {
    action_alloc(Rotation_Action);
    action = (Rotation_Action *)raw_action;
  } break;
  case KITE_TIP_ROTATION: {
    action_alloc(Tip_Rotation_Action);
    action = (Tip_Rotation_Action *)raw_action;
  } break;
  default:
    action_alloc(Move_Action);
    action = (Move_Action *)raw_action;
    break;
  }

  for (size_t i = 0; i < kite_indexs.count; ++i) {
    kite_dap(frame->kite_index_array, kite_indexs.elements[i]);
  }

  frame->duration = duration;
  frame->kind = kind;
  frame->action = action;
  frame->finished = false;
  return frame;
}

Frame *kite_script_wait(float duration) {
  Wait_Action *action;
  action_alloc(Wait_Action);
  action->starttime = GetTime();

  Frame *frame = kite_frame_init();
  frame->finished = false;
  frame->duration = duration;
  frame->kind = KITE_WAIT;
  frame->action = action;
  frame->kite_index_array = NULL;

  return frame;
}

/**
 * @brief The function that quits all the current registered frames after the
 * duration.
 *
 * @param duration [TODO:parameter]
 * @return [TODO:return]
 */
Frame *kite_script_frames_quit(float duration) {
  Quit_Action *action;
  action_alloc(Quit_Action);
  action->starttime = GetTime();

  Frame *frame = kite_frame_init();
  frame->finished = false;
  frame->duration = duration;
  frame->kind = KITE_QUIT;
  frame->action = action;
  frame->kite_index_array = NULL;

  return frame;
}

void kite_register_frames(Env *env, size_t frame_count, ...) {

  if (kite_check_finished_frames(env)) {
    kite_frames_reset(env);
  } else {
    return;
  }

  va_list args;
  va_start(args, frame_count);
  for (size_t i = 0; i < frame_count; ++i) {
    Frame *frame = va_arg(args, Frame *);
    kite_register_frame(env, frame);
  }
  va_end(args);
}

void kite_register_frame(Env *env, Frame *frame) {

  kite_dap(env->frames, *frame);

  assert(env->frames->count != 0);
  env->frames->elements[env->frames->count - 1].index = env->frames->count - 1;

  if (frame->kind == KITE_ROTATION) {
    for (size_t i = 0; i < frame->kite_index_array->count; ++i) {
      Rotation_Action *ra = frame->action;

      env->kite_array->elements[frame->kite_index_array->elements[i].index]
          .kite->segments = frame->duration;
      env->kite_array->elements[frame->kite_index_array->elements[i].index]
          .kite->remaining_angle = ra->angle;
    }
  }
  env->frames->frame_counter++;
}

/**
 * @brief The function kite_array_destroy_frames() frees all the allocated
 * actions in every frame in the array.
 *
 * @param env The global state of the application.
 */
void kite_array_destroy_frames(Env *env) {
  for (size_t i = 0; i < env->frames->count; ++i) {
    free(env->frames->elements[i].kite_index_array);
  }
}

/**
 * @brief [TODO:description]
 *
 * @param frame [TODO:parameter]
 */
void kite_frame_reset(Frame *frame) {
  frame->finished = true;
  frame->duration = 0;
  frame->kind = KITE_ACTION;
  // TODO: Think about removing the frame from the array completely
  // frame->index = 0;
}

/**
 * @brief [TODO:description]
 *
 * @param frame [TODO:parameter]
 */
void kite_frames_reset(Env *env) {
  if (env->frames->count != 0) {
    kite_array_destroy_frames(env);
    env->frames->count = 0;
    env->frames->frame_counter = 0;
  }
}

// ---------------------------------------------------------------------------

/**
 * @brief [TODO:description]
 *
 * @param env [TODO:parameter]
 */
void kite_update_frames(Env *env) {
  for (size_t i = 0; i < env->frames->count; ++i) {
    Frame *frame = &env->frames->elements[i];
    if (!frame->finished) {
      kite_render_frame(env, frame);

    } else {
      kite_frame_reset(&env->frames->elements[i]);
    }
  }
}

bool kite_check_finished_frames(Env *env) {
  for (size_t i = 0; i < env->frames->count; ++i) {
    if (!env->frames->elements[i].finished) {
      return false;
    }
  }
  return true;
}

// ---------------------------------------------------------------------------

/**
 * @brief [TODO:description]
 *
 * @param env [TODO:parameter]
 * @param frame [TODO:parameter]
 */
void kite_render_frame(Env *env, Frame *frame) {
  switch (frame->kind) {
  case KITE_WAIT: {
    Wait_Action *action = frame->action;
    if (frame->duration <= 0) {
      frame->finished = true;
    } else {
      double current_time = GetTime();
      frame->duration -= current_time - action->starttime;
      action->starttime = current_time;
    }

  } break;
  case KITE_QUIT: {
    Quit_Action *action = frame->action;
    if (frame->duration <= 0) {
      frame->finished = true;
      kite_frames_reset(env);
    } else {
      double current_time = GetTime();
      frame->duration -= current_time - action->starttime;
      action->starttime = current_time;
    }

  } break;
  case KITE_MOVE: {

    Move_Action *action = frame->action;
    Frame *env_frame = &env->frames->elements[frame->index];
    assert(env_frame->kite_index_array->count > 0);

    for (size_t i = 0; i < env_frame->kite_index_array->count; ++i) {
      size_t current_kite_index =
          env_frame->kite_index_array->elements[i].index;
      Kite *kite = env->kite_array->elements[current_kite_index].kite;

      kite_script_move(kite, action->position, frame->duration);

      if (Vector2Equals(kite->center, action->position)) {
        frame->finished = true;
      }
    }

  } break;
  case KITE_ROTATION: {

    Rotation_Action *action = frame->action;

    for (size_t i = 0;
         i < env->frames->elements[frame->index].kite_index_array->count; ++i) {
      size_t current_kite_index = env->frames->elements[frame->index]
                                      .kite_index_array->elements[i]
                                      .index;
      Kite *kite = env->kite_array->elements[current_kite_index].kite;

      kite_script_rotate(kite, action->angle, frame->duration);

      if (action->angle == kite->center_rotation) {
        frame->finished = true;
      }
    }
  } break;
  case KITE_TIP_ROTATION: {

    Tip_Rotation_Action *action = frame->action;

    for (size_t i = 0;
         i < env->frames->elements[frame->index].kite_index_array->count; ++i) {
      size_t current_kite_index = env->frames->elements[frame->index]
                                      .kite_index_array->elements[i]
                                      .index;
      Kite *kite = env->kite_array->elements[current_kite_index].kite;

      kite_script_rotate_tip(kite, action->tip, action->angle, frame->duration);

      if (action->angle == kite->center_rotation) {
        frame->finished = true;
      }
    }
  } break;
  default:
    assert(0 && "UNREACHABLE kite_render_frame()");
    break;
  }
}

// ---------------------------------------------------------------------------

float kite_lerp(float a, float b, float t) { return a + (t * (b - a)); }

/**
 * @brief [TODO:description]
 *
 * @param state [TODO:parameter]
 */
void kite_script_begin(State *state) { state->interrupt_script = true; }
void kite_script_end(State *state) { state->interrupt_script = false; }

/**
 * @brief [TODO:description]
 *
 * @param kite [TODO:parameter]
 * @param steps_x [TODO:parameter]
 * @param steps_y [TODO:parameter]
 * @param duration [TODO:parameter]
 */
void kite_script_move(Kite *kite, Vector2 position, float duration) {

  if (duration == 0) {
    kite_center_rotation(kite, &position, 0);
    return;
  }

  Vector2 begin = Vector2Normalize(kite->center);
  Vector2 begin0to2 = Vector2AddValue(begin, 1);
  Vector2 begin0to1 = Vector2Scale(begin0to2, 0.5);

  Vector2 end = Vector2Normalize(position);
  Vector2 end0to2 = Vector2AddValue(end, 1);
  Vector2 end0to1 = Vector2Scale(end0to2, 0.5);

  Vector2 it = Vector2Lerp(begin0to1, end0to1, duration);

  kite_center_rotation(kite, &it, 0);
}

/**
 * @brief [TODO:description]
 *
 * @param kite [TODO:parameter]
 * @param angle [TODO:parameter]
 * @param duration [TODO:parameter]
 */
void kite_script_rotate(Kite *kite, float angle, float duration) {

  // if (duration == 0 || kite->segments == 0) {
  if (duration == 0) {
    kite_center_rotation(kite, NULL, angle);
    return;
  }

  kite_center_rotation(kite, NULL, kite->center_rotation + angle);

  // TODO: Just in case because we accept floats that could potentially be not
  // an integer. Draw the rest of the rotation. kite_center_rotation(kite,
  // NULL, angle);
}

/**
 * @brief [TODO:description]
 *
 * @param kite [TODO:parameter]
 * @param tip [TODO:parameter]
 * @param angle [TODO:parameter]
 * @param duration [TODO:parameter]
 */
void kite_script_rotate_tip(Kite *kite, TIP tip, float angle, float duration) {

  switch (tip) {
  case LEFT_TIP:
  case RIGHT_TIP:
    kite_tip_rotation(kite, NULL, angle, tip);
    break;
  default:
    assert(0 && "ERROR: kite_script_rotate_tip: FIXED: UNREACHABLE");
  }

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

    // Just in case because we accept floats that could potentially be not an
    // integer. Draw the rest of the rotation.
    kite_tip_rotation(kite, NULL, angle, tip);
  }
}

/**
 * @brief [TODO:description]
 *
 * @param index_count [TODO:parameter]
 * @return [TODO:return]
 */
Kite_Indexs kite_indexs_append(size_t index_count, ...) {

  Kite_Indexs ki = {0};

  va_list args;
  va_start(args, index_count);

  for (size_t i = 0; i < index_count; ++i) {
    kite_dap(&ki, va_arg(args, Index));
  }
  va_end(args);

  return ki;
}
