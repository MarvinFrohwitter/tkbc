#ifndef TKBC_SCRIPT_HANDLER_H_
#define TKBC_SCRIPT_HANDLER_H_

#include "tkbc-types.h"
#include "tkbc.h"
#include <raylib.h>
#include <string.h>

// ===========================================================================
// ========================== Script Handler =================================
// ===========================================================================

Frame *tkbc_init_frame(void);
void tkbc_register_frame(Env *env, Frame *frame);
Frames *tkbc_deep_copy_frames(Frames *frames);
void tkbc_destroy_frames(Frames *frames);
void tkbc_frame_reset(Frame *frame);
void tkbc_render_frame(Env *env, Frame *frame);

void tkbc_patch_block_frames_kite_positions(Env *env, Frames *frames);
bool tkbc_check_finished_frames(Env *env);
size_t tkbc_check_finished_frames_count(Env *env);
void tkbc_input_handler_script(Env *env);
void tkbc_set_kite_positions_from_kite_frames_positions(Env *env);
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

// TODO: DEPRECATED
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

Frames *tkbc_deep_copy_frames(Frames *frames) {
  if (frames->elements == NULL || frames == NULL) {
    return NULL;
  }

  Frames *new_frames = calloc(1, sizeof(*new_frames));
  if (new_frames == NULL) {
    fprintf(stderr, "ERROR: No more memory can be allocated.\n");
    return NULL;
  }

  new_frames->kite_frame_positions =
      calloc(1, sizeof(*new_frames->kite_frame_positions));
  if (new_frames->kite_frame_positions == NULL) {
    fprintf(stderr, "ERROR: No more memory can be allocated.\n");
    return NULL;
  }

  tkbc_dapc(new_frames->kite_frame_positions,
            frames->kite_frame_positions->elements,
            frames->kite_frame_positions->count);

  for (size_t i = 0; i < frames->count; ++i) {
    Frame new_frame = frames->elements[i];

    void *old_action = frames->elements[i].action;
    if (old_action == NULL) {
      tkbc_dap(new_frames, new_frame);
      continue;
    } else {
      new_frame.action = tkbc_move_action_to_heap(
          frames->elements[i].action, frames->elements[i].kind, true);
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

// TODO: DEPRECATED
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

void tkbc_patch_block_frames_kite_positions(Env *env, Frames *frames) {
  for (size_t i = 0; i < frames->count; ++i) {
    if (frames->elements[i].kite_index_array == NULL) {
      continue;
    }
    for (size_t j = 0; j < frames->elements[i].kite_index_array->count; ++j) {
      Index kite_index = frames->elements[i].kite_index_array->elements[j];
      Kite_Position kite_position = {
          .kite_id = kite_index,
          .position = env->kite_array->elements[kite_index].kite->center,
          .angle = env->kite_array->elements[kite_index].kite->center_rotation,
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
        if (frames->kite_frame_positions->elements[k].kite_id == kite_index) {
          contains = true;
          break;
        }
      }

      if (!contains) {
        tkbc_dap(frames->kite_frame_positions, kite_position);
      }
    }
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

void tkbc_input_handler_script(Env *env) {
  if (IsKeyPressed(KEY_SPACE)) {
    env->script_finished = !env->script_finished;
  }

  tkbc_scrub_frames(env);
}

void tkbc_set_kite_positions_from_kite_frames_positions(Env *env) {
  for (size_t i = 0; i < env->frames->kite_frame_positions->count; i++) {
    Index k_index = env->frames->kite_frame_positions->elements[i].kite_id;
    Kite *kite = env->kite_array->elements[k_index].kite;
    Vector2 position = env->frames->kite_frame_positions->elements[i].position;
    float angle = env->frames->kite_frame_positions->elements[i].angle;

    tkbc_center_rotation(kite, &position, angle);
  }
}

void tkbc_scrub_frames(Env *env) {
  if (env->max_block_index <= 0) {
    return;
  }

  bool drag_left =
      GetMouseX() - env->timeline_front.x + env->timeline_front.width <= 0;
  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && env->timeline_interaction) {
    if (drag_left) {
      // The block indexes are assumed in order and at the corresponding index.
      size_t index = env->frames->block_index - 1;
      env->frames = tkbc_deep_copy_frames(&env->block_frames->elements[index]);

      tkbc_set_kite_positions_from_kite_frames_positions(env);

    } else {
      size_t index = env->frames->block_index + 1;
      if (index < env->block_frames->count) {
        env->frames =
            tkbc_deep_copy_frames(&env->block_frames->elements[index]);
      }
      tkbc_set_kite_positions_from_kite_frames_positions(env);

      // The index should not be set to zero every time the begin script
      // function is executed.
      // env->global_block_index++;
      // tkbc_dap(env->index_blocks, env->global_block_index);
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
