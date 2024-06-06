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
    ((Move_Action *)action)->position.x =
        ((Move_Action *)raw_action)->position.x;
    ((Move_Action *)action)->position.y =
        ((Move_Action *)raw_action)->position.y;

  } break;
  case KITE_ROTATION: {

    action_alloc(Rotation_Action);
    ((Rotation_Action *)action)->angle = ((Rotation_Action *)raw_action)->angle;

  } break;
  case KITE_TIP_ROTATION: {

    action_alloc(Tip_Rotation_Action);
    ((Tip_Rotation_Action *)action)->tip =
        ((Tip_Rotation_Action *)raw_action)->tip;

    ((Tip_Rotation_Action *)action)->angle =
        ((Tip_Rotation_Action *)raw_action)->angle;

  } break;
  default: {
    assert(0 && "UNREACHABLE kite_gen_frame()");
  } break;
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

void kite_register_frames(Env *env, size_t block_index, size_t frame_count,
                          ...) {

  if (!kite_check_finished_frames(env)) {
    return;
  }

  for (size_t i = 0; i < env->index_blocks->count; ++i) {
    if (block_index == env->index_blocks->elements[i]) {
      return;
    }
  }

  kite_frames_reset(env);
  va_list args;
  va_start(args, frame_count);
  for (size_t i = 0; i < frame_count; ++i) {
    Frame *frame = va_arg(args, Frame *);
    kite_register_frame(env, frame);
  }
  va_end(args);

  env->frames->block_index = block_index;
  kite_dap(env->index_blocks, block_index);
}

#define kite_ra_setup(type)                                                    \
  do {                                                                         \
    for (size_t i = 0; i < frame->kite_index_array->count; ++i) {              \
      type *ra = frame->action;                                                \
      Kite *kite = env->kite_array                                             \
                       ->elements[frame->kite_index_array->elements[i].index]  \
                       .kite;                                                  \
                                                                               \
      kite->segment_size += ra->angle / frame->duration;                       \
      kite->remaining_angle += ra->angle;                                      \
      kite->angle_sum += ra->angle;                                            \
      kite->old_angle = kite->center_rotation;                                 \
      kite->old_center = kite->center;                                         \
    }                                                                          \
  } while (0)

void kite_register_frame(Env *env, Frame *frame) {

  kite_dap(env->frames, *frame);

  assert(env->frames->count != 0);
  env->frames->elements[env->frames->count - 1].index = env->frames->count - 1;

  switch (frame->kind) {
  case KITE_ROTATION: {
    kite_ra_setup(Rotation_Action);
  } break;
  case KITE_TIP_ROTATION: {
    kite_ra_setup(Tip_Rotation_Action);
  } break;
  default:
    break;
  }
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

/**
 * @brief [TODO:description]
 *
 * @param env [TODO:parameter]
 * @return [TODO:return]
 */
bool kite_check_finished_frames(Env *env) {
  for (size_t i = 0; i < env->frames->count; ++i) {
    if (!env->frames->elements[i].finished) {
      return false;
    }
  }
  return true;
}

/**
 * @brief [TODO:description]
 *
 * @param env [TODO:parameter]
 * @return [TODO:return]
 */
size_t kite_check_finished_frames_count(Env *env) {
  int count = 0;
  for (size_t i = 0; i < env->frames->count; ++i) {
    if (env->frames->elements[i].finished) {
      count++;
    }
  }

  return count;
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
      frame->duration = 0;
    } else {
      double current_time = GetTime();
      frame->duration -= current_time - action->starttime;
      action->starttime = current_time;
    }

  } break;
  case KITE_QUIT: {

    if (kite_check_finished_frames_count(env) == env->frames->count - 1) {
      frame->finished = true;
      kite_frames_reset(env);
      break;
    }

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

        {
          // Every kite in the frame should get sync at the end of the frame.
          for (size_t i = 0; i < frame->kite_index_array->count; ++i) {
            size_t current_kite_index = env->frames->elements[frame->index]
                                            .kite_index_array->elements[i]
                                            .index;
            Kite *kite = env->kite_array->elements[current_kite_index].kite;
            kite_script_move(kite, action->position, 0);
          }
        }
        break;
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

      if (kite->remaining_angle <= 0 ||
          FloatEquals(kite->old_angle + kite->angle_sum,
                      kite->center_rotation)) {
        frame->finished = true;

        {
          // Every kite in the frame should get sync at the end of the frame.
          for (size_t i = 0; i < frame->kite_index_array->count; ++i) {
            size_t current_kite_index = env->frames->elements[frame->index]
                                            .kite_index_array->elements[i]
                                            .index;
            Kite *kite = env->kite_array->elements[current_kite_index].kite;
            kite_script_rotate(kite, action->angle, 0);
          }
        }

        kite->old_angle = kite->center_rotation;
        kite->old_center = kite->center;

        kite->angle_sum = 0;
        kite->remaining_angle = 0;
        kite->segment_size = 0;
        break;
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

      if (kite->remaining_angle <= 0 ||
          FloatEquals(kite->old_angle + kite->angle_sum,
                      kite->center_rotation)) {
        frame->finished = true;

        {
          // Every kite in the frame should get sync at the end of the frame.
          for (size_t i = 0; i < frame->kite_index_array->count; ++i) {
            size_t current_kite_index = env->frames->elements[frame->index]
                                            .kite_index_array->elements[i]
                                            .index;
            Kite *kite = env->kite_array->elements[current_kite_index].kite;
            kite_script_rotate_tip(kite, action->tip, action->angle, 0);
          }
        }

        kite->old_angle = kite->center_rotation;
        kite->old_center = kite->center;

        kite->angle_sum = 0;
        kite->remaining_angle = 0;
        kite->segment_size = 0;
        break;
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
void kite_script_begin(Env *env) {
  env->interrupt_script = true;
  kite_register_frames(env, 0, 1, kite_script_wait(0));
}
void kite_script_end(Env *env) {
  // env->index_blocks->count = 0;
  env->interrupt_script = false;

  return;
}

/**
 * @brief [TODO:description]
 *
 * @param kite [TODO:parameter]
 * @param steps_x [TODO:parameter]
 * @param steps_y [TODO:parameter]
 * @param duration [TODO:parameter]
 */
void kite_script_move(Kite *kite, Vector2 position, float duration) {

  if (duration <= 0) {
    kite_center_rotation(kite, &position, kite->center_rotation);
    return;
  }

  if (Vector2Equals(kite->center, position)) {
    kite_center_rotation(kite, &position, kite->center_rotation);
    return;
  }

  Vector2 d = Vector2Subtract(position, kite->center);
  Vector2 dnorm = Vector2Normalize(d);
  Vector2 dnormscale = Vector2Scale(dnorm, duration);

  if (Vector2Length(dnormscale) >=
      Vector2Length(Vector2Subtract(position, kite->center))) {
    kite_center_rotation(kite, &position, kite->center_rotation);
  } else {
    Vector2 it = Vector2Add(kite->center, dnormscale);
    kite_center_rotation(kite, &it, kite->center_rotation);
  }
}

/**
 * @brief [TODO:description]
 *
 * @param kite [TODO:parameter]
 * @param angle [TODO:parameter]
 * @param duration [TODO:parameter]
 */
void kite_script_rotate(Kite *kite, float angle, float duration) {

  if (duration <= 0) {
    kite_center_rotation(kite, NULL, kite->old_angle + angle);
    kite->angle_sum -= angle;
    kite->remaining_angle -= angle;
    kite->segment_size -= angle / duration;
    return;
  }

  if (kite->remaining_angle <= 0) {
    kite_center_rotation(kite, NULL, kite->old_angle + kite->angle_sum);
    return;
  }

  kite->remaining_angle -= kite->segment_size * GetFrameTime();
  float doneangle = angle - kite->remaining_angle;
  float current_angle = doneangle + kite->segment_size;

  if (kite->remaining_angle <= 0)
    kite_center_rotation(kite, NULL, kite->old_angle + kite->angle_sum);
  else
    kite_center_rotation(kite, NULL, kite->old_angle + current_angle);
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

  if (duration <= 0) {
    kite_tip_rotation(kite, NULL, kite->old_angle + angle, tip);
    kite->angle_sum -= angle;
    kite->remaining_angle -= angle;
    kite->segment_size -= angle / duration;
    return;
  }

  if (kite->remaining_angle <= 0) {
    kite_tip_rotation(kite, NULL, kite->old_angle + kite->angle_sum, tip);
    return;
  }

  kite->remaining_angle -= kite->segment_size * GetFrameTime();
  float doneangle = angle - kite->remaining_angle;
  float current_angle = doneangle + kite->segment_size;

  if (kite->remaining_angle <= 0)
    // Provided the old position, because the kite center moves as a circle
    // around the old fixed position.
    kite_tip_rotation(kite, &kite->old_center,
                      kite->old_angle + kite->angle_sum, tip);
  else
    kite_tip_rotation(kite, NULL, kite->old_angle + current_angle, tip);
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
