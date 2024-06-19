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

/**
 * @brief The function kite_frame_init() can be used to get a new allocated zero
 * initialized frame.
 *
 * @return A new on the heap allocated frame region is given back.
 */
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
 * @brief The function kite_gen_frame() creates a new frame and fills in all the
 * given parameters. It handles all the provided actions. The returned frame can
 * be passed into the register function.
 *
 * @param env The environment that holds the current state of the application.
 * It is passed implicit by the macro call.
 * @param kind The action kind to identify the given raw_action.
 * @param kite_indexs The list of kite indexes that are present in the kite
 * array, were the frame action should be applied to.
 * @param raw_action The action that matches the given kind.
 * @param duration The duration the action should take.
 * @return The frame that is constructed to represent the given action.
 */
Frame *kite__gen_frame(Env *env, Action_Kind kind, Kite_Indexs kite_indexs,
                       void *raw_action, float duration) {

  if (env->script_finished) {
    return NULL;
  }
  if (!kite_check_finished_frames(env)) {
    return NULL;
  }

  void *action;
  Frame *frame = kite_frame_init();
  switch (kind) {
  case KITE_MOVE_ADD:
  case KITE_MOVE: {
    action_alloc(Move_Action);
    ((Move_Action *)action)->position.x =
        ((Move_Action *)raw_action)->position.x;
    ((Move_Action *)action)->position.y =
        ((Move_Action *)raw_action)->position.y;

  } break;
  case KITE_ROTATION_ADD: {

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
 * duration. It can be inserted as a normal frame into a block that should be
 * quit after a some duration time.
 *
 * @param duration The time in seconds after all the frames will quit.
 * @return The frame that is constructed to represent the force quit frame-block
 * frame.
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

/**
 * @brief The function kite_register_frames() can be used to collect all given
 * frames into one frame list and register them as a new frame block.
 *
 * @param env The environment that holds the current state of the application.
 */
void kite__register_frames(Env *env, ...) {
  env->scratch_buf_frames->count = 0;

  va_list args;
  va_start(args, env);
  Frame *frame = va_arg(args, Frame *);
  while (frame != NULL) {
    kite_dap(env->scratch_buf_frames, *frame);
    frame = va_arg(args, Frame *);
  }
  va_end(args);
  kite_register_frames_array(env, env->scratch_buf_frames);
}

void kite_register_frames_array(Env *env, Frames *frames) {
  assert(frames != NULL);

  if (frames->count == 0) {
    return;
  }

  env->attempts_block_index++;

  if (!kite_check_finished_frames(env)) {
    kite_destroy_frames(frames);
    return;
  }

  size_t block_index = env->global_block_index++;
  for (size_t i = 0; i < env->index_blocks->count; ++i) {
    if (block_index == env->index_blocks->elements[i]) {
      kite_destroy_frames(frames);
      return;
    }
  }

  kite_destroy_frames(env->frames);
  for (size_t i = 0; i < frames->count; ++i) {
    kite_register_frame(env, &frames->elements[i]);
  }

  env->frames->block_index = block_index;
  kite_dap(env->index_blocks, block_index);
}

#define kite_ra_setup(type)                                                    \
  do {                                                                         \
    for (size_t i = 0; i < frame->kite_index_array->count; ++i) {              \
      type *ra = frame->action;                                                \
      Kite *kite =                                                             \
          env->kite_array->elements[frame->kite_index_array->elements[i]]      \
              .kite;                                                           \
                                                                               \
      kite->segment_size += fabsf(ra->angle / frame->duration);                \
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
  case KITE_MOVE:
  case KITE_MOVE_ADD: {
    for (size_t i = 0; i < frame->kite_index_array->count; ++i) {
      Kite *kite =
          env->kite_array->elements[frame->kite_index_array->elements[i]].kite;
      kite->old_center = kite->center;
    }

  } break;
  case KITE_ROTATION_ADD: {
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
void kite_destroy_frames(Frames *frames) {
  if (frames->count != 0) {
    for (size_t i = 0; i < frames->count; ++i) {
      free(frames->elements[i].kite_index_array);
      free(frames->elements[i].action);
    }
    frames->count = 0;
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
    assert(frame != NULL);
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
      kite_destroy_frames(env->frames);
      break;
    }

    Quit_Action *action = frame->action;
    if (frame->duration <= 0) {
      frame->finished = true;
      kite_destroy_frames(env->frames);
    } else {
      double current_time = GetTime();
      frame->duration -= current_time - action->starttime;
      action->starttime = current_time;
    }

  } break;
  case KITE_MOVE_ADD: {

    Move_Add_Action *action = frame->action;
    Frame *env_frame = &env->frames->elements[frame->index];
    assert(env_frame->kite_index_array->count > 0);

    for (size_t i = 0; i < env_frame->kite_index_array->count; ++i) {
      size_t current_kite_index = env_frame->kite_index_array->elements[i];
      Kite *kite = env->kite_array->elements[current_kite_index].kite;

      // Description of the calculation for the position
      // kite.center + (action->position - (kite.center - kite->old_center))
      // kite.center + (action->position - kite.center + kite->old_center)
      // kite.center + action->position - kite.center + kite->old_center

      kite_script_move(kite, Vector2Add(kite->old_center, action->position),
                       frame->duration);

      if (Vector2Equals(kite->center,
                        Vector2Add(kite->old_center, action->position))) {
        frame->finished = true;

        {
          // Every kite in the frame should get sync at the end of the frame.
          for (size_t i = 0; i < frame->kite_index_array->count; ++i) {
            size_t current_kite_index = env->frames->elements[frame->index]
                                            .kite_index_array->elements[i];
            Kite *kite = env->kite_array->elements[current_kite_index].kite;
            kite_script_move(kite,
                             Vector2Add(kite->old_center, action->position), 0);
          }
        }
        break;
      }
    }

  } break;
  case KITE_MOVE: {

    Move_Action *action = frame->action;
    Frame *env_frame = &env->frames->elements[frame->index];
    assert(env_frame->kite_index_array->count > 0);

    for (size_t i = 0; i < env_frame->kite_index_array->count; ++i) {
      size_t current_kite_index = env_frame->kite_index_array->elements[i];
      Kite *kite = env->kite_array->elements[current_kite_index].kite;

      kite_script_move(kite, action->position, frame->duration);

      if (Vector2Equals(kite->center, action->position)) {
        frame->finished = true;

        {
          // Every kite in the frame should get sync at the end of the frame.
          for (size_t i = 0; i < frame->kite_index_array->count; ++i) {
            size_t current_kite_index = env->frames->elements[frame->index]
                                            .kite_index_array->elements[i];
            Kite *kite = env->kite_array->elements[current_kite_index].kite;
            kite_script_move(kite, action->position, 0);
          }
        }
        break;
      }
    }

  } break;
  case KITE_ROTATION_ADD: {

    Rotation_Action *action = frame->action;

    for (size_t i = 0;
         i < env->frames->elements[frame->index].kite_index_array->count; ++i) {
      size_t current_kite_index =
          env->frames->elements[frame->index].kite_index_array->elements[i];
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
                                            .kite_index_array->elements[i];
            Kite *kite = env->kite_array->elements[current_kite_index].kite;
            kite_script_rotate(kite, action->angle, 0);
          }
        }

        // TODO: Remove all of it because the registration updates it.
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
      size_t current_kite_index =
          env->frames->elements[frame->index].kite_index_array->elements[i];
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
                                            .kite_index_array->elements[i];
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
int kite_max(int a, int b) { return a <= b ? b : a; }

/**
 * @brief [TODO:description]
 *
 * @param state [TODO:parameter]
 */
void kite_script_begin(Env *env) {
  env->script_interrupt = true;
  env->global_block_index = 0;
  env->attempts_block_index = 0;
  kite_register_frames(env, kite_script_wait(0));
}
void kite_script_end(Env *env) {
  env->script_interrupt = false;

  if (!env->script_finished) {
    env->max_block_index =
        kite_max(env->max_block_index, env->attempts_block_index);
  }

  if (env->max_block_index - 1 <= env->frames->block_index &&
      !env->script_finished) {
    env->script_finished = true;
    env->global_block_index = 0;
    env->max_block_index = 0;
    env->attempts_block_index = 0;

    printf("KITE: INFO: The script has finished successfully.\n");

    // TODO: Think about loading a new script.
    // env->index_blocks->count = 0;
    // free the blocks
  }
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
Kite_Indexs kite__indexs_append(size_t _, ...) {
  Kite_Indexs ki = {0};

  va_list args;
  va_start(args, _);
  for (;;) {
    Index index = va_arg(args, Index);
    if (INT_MAX != index) {
      kite_dap(&ki, index);
    } else {
      break;
    }
  }
  va_end(args);

  return ki;
}
