// ===========================================================================
// ========================== Script Handler =================================
// ===========================================================================

#include "kite_utils.h"
#include "tkbc.h"
#include <assert.h>
#include <math.h>
#include <raylib.h>
#include <stddef.h>

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
Frame *kite_gen_frame(Action_Kind kind, Kite_Indexs kite_indexs, void *raw_action, float duration) {

  // TODO: Variadic function for the kite numbers.

  void *action;
  Frame *frame = kite_frame_init();
  switch (kind) {
  case KITE_MOVE: {
    action = (Move_Action *)raw_action;

  } break;
  case KITE_ROTATION: {
    action = (Rotation_Action *)raw_action;
  } break;
  case KITE_TIP_ROTATION: {
    action = (Tip_Rotation_Action *)raw_action;
  } break;
  default:
    action = (Move_Action *)raw_action;
    break;
  }

  for (size_t i = 0; i < kite_indexs.count; ++i) {
    kite_da_append(frame->kite_index_array, kite_indexs.items[i]);
  }
  frame->duration = duration;
  frame->kind = kind;
  frame->action = action;
  return frame;
}

void kite_register_frame(Env *env, Frame *frame) {
  // TODO: Variadic function
  kite_da_append(env->frames, *frame);
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
    free(env->frames->items[i].action);
  }
}

/**
 * @brief [TODO:description]
 *
 * @param frame [TODO:parameter]
 */
void kite_frame_reset(Frame *frame) {
  frame->duration = 0;
  frame->action = NULL;
  frame->kite_index_array = NULL;
  frame->kind = KITE_ACTION;
  frame->index = 0;
}

// ---------------------------------------------------------------------------

/**
 * @brief [TODO:description]
 *
 * @param env [TODO:parameter]
 */
void kite_update_frames(Env *env) {
  for (size_t i = 0; i < env->frames->count; ++i) {
    Frame *frame = &env->frames->items[i];
    if (frame->duration <= 1 && frame->duration != 0) {
      frame->duration *= GetFrameTime();
      kite_render_frame(env, frame);

    } else {
      kite_frame_reset(&env->frames->items[i]);
    }
  }
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
  case KITE_MOVE: {

    Move_Action *action = frame->action;

    for (size_t i = 0;
         i < env->frames->items[frame->index].kite_index_array->count; ++i) {
      size_t current_kite_index =
          env->frames->items[frame->index].kite_index_array->items[i];
      Kite *kite = env->kite_array->items[current_kite_index].kite;

      kite_script_move(kite, action->position.x, action->position.y,
                       action->parameters);
    }

  } break;
  case KITE_ROTATION: {

    Rotation_Action *action = frame->action;

    for (size_t i = 0;
         i < env->frames->items[frame->index].kite_index_array->count; ++i) {
      size_t current_kite_index =
          env->frames->items[frame->index].kite_index_array->items[i];
      Kite *kite = env->kite_array->items[current_kite_index].kite;

      kite_script_rotate(kite, action->angle, action->parameters);
    }
  } break;
  case KITE_TIP_ROTATION: {
  } break;

    Tip_Rotation_Action *action = frame->action;

    for (size_t i = 0;
         i < env->frames->items[frame->index].kite_index_array->count; ++i) {
      size_t current_kite_index =
          env->frames->items[frame->index].kite_index_array->items[i];
      Kite *kite = env->kite_array->items[current_kite_index].kite;

      kite_script_rotate_tip(kite, action->tip, action->angle,
                             action->parameters);
    }
  default:
    break;
  }
}

// ---------------------------------------------------------------------------

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
 * @param parameters [TODO:parameter]
 */
void kite_script_move(Kite *kite, float steps_x, float steps_y,
                      PARAMETERS parameters) {

  Vector2 pos = {0};

  switch (parameters) {
  case FIXED: {
  final_pos:
    pos.x = steps_x;
    pos.y = steps_y;
    kite_center_rotation(kite, &pos, 0);
  } break;
  case SMOOTH: {

    size_t maxiter = fmaxf(fabsf(steps_x), fabsf(steps_y));
    float iter_space_x = fabsf(steps_x) / maxiter;
    float iter_space_y = fabsf(steps_x) / maxiter;

    // TODO: What we do if the x and y are different
    assert(steps_x == steps_y);
    for (size_t i = 0; i < maxiter; ++i) {

      if (floorf(pos.x) != steps_x) {
        pos.x = steps_x > 0 ? pos.x + iter_space_x : pos.x - iter_space_x;
      }

      if (floorf(pos.y) != steps_y) {
        pos.y = steps_y > 0 ? pos.y + iter_space_y : pos.y - iter_space_y;
      }
      if (steps_x == 0)
        pos.x = steps_x;
      if (steps_y == 0)
        pos.y = steps_y;

      kite_center_rotation(kite, &pos, 0);
    }

    // Just in case the rounding of the iterations is not enough to reach the
    // final position.
    goto final_pos;

  } break;
  default:
    assert(0 && "ERROR: kite_script_move: UNREACHABLE");
  }
}

/**
 * @brief [TODO:description]
 *
 * @param kite [TODO:parameter]
 * @param angle [TODO:parameter]
 * @param parameters [TODO:parameter]
 */
void kite_script_rotate(Kite *kite, float angle, PARAMETERS parameters) {

  switch (parameters) {
  case FIXED: {
    kite_center_rotation(kite, NULL, angle);
  } break;
  case SMOOTH: {
    if (angle < 0) {
      for (size_t i = 0; i >= angle; --i) {
        kite_center_rotation(kite, NULL, kite->center_rotation + i);
      }
    } else {
      for (size_t i = 0; i <= angle; ++i) {
        kite_center_rotation(kite, NULL, kite->center_rotation + i);
      }
    }

    // Just in case because we accept floats that could potentially be not an
    // integer. Draw the rest of the rotation.
    kite_center_rotation(kite, NULL, angle);

  } break;
  default:
    assert(0 && "ERROR: kite_script_rotate: UNREACHABLE");
  }
}

/**
 * @brief [TODO:description]
 *
 * @param kite [TODO:parameter]
 * @param tip [TODO:parameter]
 * @param angle [TODO:parameter]
 * @param parameters [TODO:parameter]
 */
void kite_script_rotate_tip(Kite *kite, TIP tip, float angle,
                            PARAMETERS parameters) {

  switch (parameters) {
  case FIXED: {
    switch (tip) {
    case LEFT_TIP:
    case RIGHT_TIP:
      kite_tip_rotation(kite, NULL, angle, tip);
      break;
    default:
      assert(0 && "ERROR: kite_script_rotate_tip: FIXED: UNREACHABLE");
    }

  } break;
  case SMOOTH: {
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
      break;
    default:
      assert(0 && "ERROR: kite_script_rotate_tip: SMOOTH: UNREACHABLE");
    }

    // Just in case because we accept floats that could potentially be not an
    // integer. Draw the rest of the rotation.
    kite_tip_rotation(kite, NULL, angle, tip);

  } break;
  default:
    assert(0 && "ERROR: kite_script_rotate_tip: UNREACHABLE");
  }
}
