#ifndef TKBC_SCRIPT_HANDLER_H_
#define TKBC_SCRIPT_HANDLER_H_

#include "tkbc-types.h"
#include <raylib.h>

// ===========================================================================
// ========================== Script Handler =================================
// ===========================================================================

Frame *tkbc_init_frame(void);
void tkbc_register_frame(Env *env, Frame *frame);
void tkbc_destroy_frames(Frames *frames);
void tkbc_frame_reset(Frame *frame);
void tkbc_render_frame(Env *env, Frame *frame);

bool tkbc_check_finished_frames(Env *env);
size_t tkbc_check_finished_frames_count(Env *env);
void tkbc_scrub_frames(Env *env);

// ===========================================================================
// ========================== SCRIPT HANDLER INTERNAL ========================
// ===========================================================================

void tkbc_script_move(Kite *kite, Vector2 position, float duration);
void tkbc_script_rotate(Kite *kite, float angle, float duration);
void tkbc_script_rotate_tip(Kite *kite, TIP tip, float angle, float duration);

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

#define tkbc_ra_setup(type)                                                    \
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

void tkbc_register_frame(Env *env, Frame *frame) {

  tkbc_dap(env->frames, *frame);

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
    tkbc_ra_setup(Rotation_Action);
  } break;
  case KITE_TIP_ROTATION: {
    tkbc_ra_setup(Tip_Rotation_Action);
  } break;
  default:
    break;
  }
}

void tkbc_destroy_frames(Frames *frames) {
  if (frames->count != 0) {
    for (size_t i = 0; i < frames->count; ++i) {
      free(frames->elements[i].kite_index_array);
      free(frames->elements[i].action);
    }
    frames->count = 0;
  }
}

void tkbc_frame_reset(Frame *frame) {
  frame->finished = true;
  frame->duration = 0;
  frame->kind = KITE_ACTION;
}

void tkbc_render_frame(Env *env, Frame *frame) {
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
      tkbc_destroy_frames(env->frames);
      break;
    }

    Quit_Action *action = frame->action;
    if (frame->duration <= 0) {
      frame->finished = true;
      tkbc_destroy_frames(env->frames);
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

      tkbc_script_move(kite, Vector2Add(kite->old_center, action->position),
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
            tkbc_script_move(kite,
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

      tkbc_script_move(kite, action->position, frame->duration);

      if (Vector2Equals(kite->center, action->position)) {
        frame->finished = true;

        {
          // Every kite in the frame should get sync at the end of the frame.
          for (size_t i = 0; i < frame->kite_index_array->count; ++i) {
            size_t current_kite_index = env->frames->elements[frame->index]
                                            .kite_index_array->elements[i];
            Kite *kite = env->kite_array->elements[current_kite_index].kite;
            tkbc_script_move(kite, action->position, 0);
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

      tkbc_script_rotate(kite, action->angle, frame->duration);

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
            tkbc_script_rotate(kite, action->angle, 0);
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

      tkbc_script_rotate_tip(kite, action->tip, action->angle, frame->duration);

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
            tkbc_script_rotate_tip(kite, action->tip, action->angle, 0);
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
    assert(0 && "UNREACHABLE tkbc_render_frame()");
    break;
  }
}

bool tkbc_check_finished_frames(Env *env) {
  for (size_t i = 0; i < env->frames->count; ++i) {
    if (!env->frames->elements[i].finished) {
      return false;
    }
  }
  return true;
}

size_t tkbc_check_finished_frames_count(Env *env) {
  int count = 0;
  for (size_t i = 0; i < env->frames->count; ++i) {
    if (env->frames->elements[i].finished) {
      count++;
    }
  }

  return count;
}

void tkbc_scrub_frames(Env *env) {
  if (env->max_block_index == 0) {
    return;
  }

  int drag_left = 1;
  if (IsKeyDown(MOUSE_BUTTON_LEFT) && env->timeline_interaction) {
    if (drag_left) {
      // The block indexes are assumed in order and at the corresponding index.
      env->index_blocks->count = env->frames->block_index;

    } else {

      // The index should not be set to zero every time the begin script
      // function is executed.
      env->global_block_index++;
      tkbc_dap(env->index_blocks, env->global_block_index);

      // env->frames = new Frame();
    }
  }
}

// ========================== SCRIPT HANDLER INTERNAL ========================

void tkbc_script_move(Kite *kite, Vector2 position, float duration) {

  if (duration <= 0) {
    tkbc_center_rotation(kite, &position, kite->center_rotation);
    return;
  }

  if (Vector2Equals(kite->center, position)) {
    tkbc_center_rotation(kite, &position, kite->center_rotation);
    return;
  }

  Vector2 d = Vector2Subtract(position, kite->center);
  Vector2 dnorm = Vector2Normalize(d);
  Vector2 dnormscale = Vector2Scale(dnorm, duration);

  if (Vector2Length(dnormscale) >=
      Vector2Length(Vector2Subtract(position, kite->center))) {
    tkbc_center_rotation(kite, &position, kite->center_rotation);
  } else {
    Vector2 it = Vector2Add(kite->center, dnormscale);
    tkbc_center_rotation(kite, &it, kite->center_rotation);
  }
}

void tkbc_script_rotate(Kite *kite, float angle, float duration) {

  if (duration <= 0) {
    tkbc_center_rotation(kite, NULL, kite->old_angle + angle);
    kite->angle_sum -= angle;
    kite->remaining_angle -= angle;
    kite->segment_size -= angle / duration;
    return;
  }

  if (kite->remaining_angle <= 0) {
    tkbc_center_rotation(kite, NULL, kite->old_angle + kite->angle_sum);
    return;
  }

  kite->remaining_angle -= kite->segment_size * GetFrameTime();
  float doneangle = angle - kite->remaining_angle;
  float current_angle = doneangle + kite->segment_size;

  if (kite->remaining_angle <= 0)
    tkbc_center_rotation(kite, NULL, kite->old_angle + kite->angle_sum);
  else
    tkbc_center_rotation(kite, NULL, kite->old_angle + current_angle);
}

void tkbc_script_rotate_tip(Kite *kite, TIP tip, float angle, float duration) {

  if (duration <= 0) {
    tkbc_tip_rotation(kite, NULL, kite->old_angle + angle, tip);
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
    tkbc_tip_rotation(kite, &kite->old_center,
                      kite->old_angle + kite->angle_sum, tip);
  else
    tkbc_tip_rotation(kite, NULL, kite->old_angle + current_angle, tip);
}

#endif // TKBC_SCRIPT_HANDLER_IMPLEMENTATION
