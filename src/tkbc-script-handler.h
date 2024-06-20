#ifndef TKBC_SCRIPT_HANDLER_H_
#define TKBC_SCRIPT_HANDLER_H_

#include "tkbc-types.h"

// ===========================================================================
// ========================== Script Handler =================================
// ===========================================================================


Frame *kite_init_frame();
void kite_register_frame(Env *env, Frame *frame);
void kite_destroy_frames(Frames *frames);
void kite_frame_reset(Frame *frame);
void kite_render_frame(Env *env, Frame *frame);

bool kite_check_finished_frames(Env *env);
size_t kite_check_finished_frames_count(Env *env);

// ===========================================================================
// ========================== SCRIPT HANDLER INTERNAL ========================
// ===========================================================================

void kite_script_move(Kite *kite, Vector2 position, float duration);
void kite_script_rotate(Kite *kite, float angle, float duration);
void kite_script_rotate_tip(Kite *kite, TIP tip, float angle, float duration);


#endif // TKBC_SCRIPT_HANDLER_H_

// ===========================================================================

#ifdef TKBC_SCRIPT_HANDLER_IMPLEMENTATION

// ========================== Script Handler =================================

/**
 * @brief The function can be used to get a new allocated zero initialized
 * frame.
 *
 * @return A new on the heap allocated frame region is given back.
 */
Frame *kite_init_frame() {
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

void kite_destroy_frames(Frames *frames) {
  if (frames->count != 0) {
    for (size_t i = 0; i < frames->count; ++i) {
      free(frames->elements[i].kite_index_array);
      free(frames->elements[i].action);
    }
    frames->count = 0;
  }
}

void kite_frame_reset(Frame *frame) {
  frame->finished = true;
  frame->duration = 0;
  frame->kind = KITE_ACTION;
}

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

bool kite_check_finished_frames(Env *env) {
  for (size_t i = 0; i < env->frames->count; ++i) {
    if (!env->frames->elements[i].finished) {
      return false;
    }
  }
  return true;
}

size_t kite_check_finished_frames_count(Env *env) {
  int count = 0;
  for (size_t i = 0; i < env->frames->count; ++i) {
    if (env->frames->elements[i].finished) {
      count++;
    }
  }

  return count;
}

// ========================== SCRIPT HANDLER INTERNAL ========================


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

#endif // TKBC_SCRIPT_HANDLER_IMPLEMENTATION
