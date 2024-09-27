#ifndef TKBC_SCRIPT_API_H_
#define TKBC_SCRIPT_API_H_

#include "tkbc-script-handler.h"
#include "tkbc-types.h"
#include <stdbool.h>

// ===========================================================================
// ========================== SCRIPT API =====================================
// ===========================================================================

void tkbc_script_input(Env *env);
void tkbc_script_begin(Env *env);
void tkbc_script_end(Env *env);

void tkbc_script_update_frames(Env *env);
bool tkbc_script_finished(Env *env);

// ===========================================================================
// ========================== SCRIPT HANDLER API =============================
// ===========================================================================

Frame *tkbc__script_wait(Env *env, float duration);
#define tkbc_script_wait(duration) tkbc__script_wait(env, duration)

Frame *tkbc__script_frames_quit(Env *env, float duration);
#define tkbc_script_frames_quit(duration)                                      \
  tkbc__script_frames_quit(env, duration)

Frame *tkbc__frame_generate(Env *env, Action_Kind kind, Kite_Indexs kite_indexs,
                            void *raw_action, float duration);
#define tkbc_frame_generate(kind, kite_indexs, raw_action, duration)           \
  tkbc__frame_generate(env, kind, kite_indexs, raw_action, duration)
void tkbc__register_frames(Env *env, ...);
#define tkbc_register_frames(env, ...)                                         \
  tkbc__register_frames(env, __VA_ARGS__, NULL)
void tkbc_register_frames_array(Env *env, Frames *frames);

Kite_Indexs tkbc__indexs_append(size_t _, ...);
#define tkbc_indexs_append(...) tkbc__indexs_append(0, __VA_ARGS__, INT_MAX)
Kite_Indexs tkbc_indexs_range(int start, int end);
#define tkbc_indexs_generate(count) tkbc_indexs_range(0, count)

#endif // TKBC_SCRIPT_API_H_

// ===========================================================================

#ifdef TKBC_SCRIPT_API_IMPLEMENTATION

#ifndef TKBC_UTILS_IMPLEMENTATION
#define TKBC_UTILS_IMPLEMENTATION
#include "tkbc-utils.h"
#endif // TKBC_UTILS_IMPLEMENTATION

// ========================== SCRIPT API =====================================

void tkbc_script_begin(Env *env) {
  env->script_interrupt = true;
  tkbc_register_frames(env, tkbc_script_wait(0));
}

void tkbc_script_end(Env *env) {
  env->script_interrupt = false;
  if (env->script_setup) {
    env->frames = &env->block_frames->elements[0];
    env->script_setup = false;
    return;
  }

  if (tkbc_check_finished_frames(env) &&
      (env->block_frames->count == env->frames->block_index + 1)) {

    if (!env->script_finished) {
      env->script_finished = true;
      printf("=========== FROM THE SCRIPT END FUNCTION ========\n");
      printf("KITE: INFO: The script has finished successfully.\n");
      printf("=================================================\n");
    }
  }
}

void tkbc_script_update_frames(Env *env) {
  for (size_t i = 0; i < env->frames->count; ++i) {
    Frame *frame = &env->frames->elements[i];
    assert(frame != NULL);
    if (!frame->finished) {
      tkbc_render_frame(env, frame);
    }
  }

  if (env->frames->block_index + 1 < env->block_frames->count) {
    if (tkbc_check_finished_frames(env)) {
      env->frames = &env->block_frames->elements[env->frames->block_index + 1];

      tkbc_patch_frames_current_time(env->frames);

      for (size_t i = 0; i < env->frames->kite_frame_positions->count; ++i) {
        Index k_index = env->frames->kite_frame_positions->elements[i].kite_id;
        Kite *kite = env->kite_array->elements[k_index].kite;

        // NOTE: The design limits the combined use of center rotations and tip
        // rotations. Introduce separate tracking variables, if the distinct use
        // in one frame is needed.

        kite->old_angle = kite->center_rotation;
        kite->old_center = kite->center;

        env->frames->kite_frame_positions->elements[i].angle =
            kite->center_rotation;
        env->frames->kite_frame_positions->elements[i].position = kite->center;
      }
    }
  } else {
    env->script_finished = true;
    printf("KITE: INFO: The script has finished successfully.\n");
  }
}

bool tkbc_script_finished(Env *env) { return env->script_finished; }

// ========================== SCRIPT HANDLER API =============================

Frame *tkbc__script_wait(Env *env, float duration) {
  if (env->script_finished) {
    return NULL;
  }
  if (!env->script_setup) {
    return NULL;
  }

  Wait_Action *action;
  action_alloc(Wait_Action);
  ((Wait_Action *)action)->starttime = GetTime();

  Frame *frame = tkbc_init_frame();
  frame->finished = false;
  frame->duration = duration;
  frame->kind = KITE_WAIT;
  (frame->action) = action;
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
Frame *tkbc__script_frames_quit(Env *env, float duration) {
  if (env->script_finished) {
    return NULL;
  }
  if (!env->script_setup) {
    return NULL;
  }

  Quit_Action *action;
  action_alloc(Quit_Action);
  ((Quit_Action *)action)->starttime = GetTime();

  Frame *frame = tkbc_init_frame();
  frame->finished = false;
  frame->duration = duration;
  frame->kind = KITE_QUIT;
  frame->action = action;
  frame->kite_index_array = NULL;

  return frame;
}

/**
 * @brief The function creates a new frame and fills in all the given
 * parameters. It handles all the provided actions. The returned frame can be
 * passed into the register function.
 *
 * @param env The environment that holds the current state of the application.
 * It is passed implicit by the macro call.
 * @param kind The action kind to identify the given raw_action.
 * @param kite_indexs The list of kite indizes that are present in the kite
 * array, were the frame action should be applied to.
 * @param raw_action The action that matches the given kind.
 * @param duration The duration the action should take.
 * @return The frame that is constructed to represent the given action.
 */
Frame *tkbc__frame_generate(Env *env, Action_Kind kind, Kite_Indexs kite_indexs,
                            void *raw_action, float duration) {

  if (env->script_finished) {
    return NULL;
  }
  if (!env->script_setup) {
    return NULL;
  }

  Frame *frame = tkbc_init_frame();
  void *action = tkbc_move_action_to_heap(raw_action, kind, false);

  tkbc_dapc(frame->kite_index_array, kite_indexs.elements, kite_indexs.count);

  frame->duration = duration;
  frame->kind = kind;
  frame->action = action;
  frame->finished = false;
  return frame;
}

/**
 * @brief The function can be used to collect all given frames into one frame
 * list and register them as a new frame block.
 *
 * @param env The environment that holds the current state of the application.
 */
void tkbc__register_frames(Env *env, ...) {
  env->scratch_buf_frames->count = 0;

  va_list args;
  va_start(args, env);
  Frame *frame = va_arg(args, Frame *);
  while (frame != NULL) {
    tkbc_dap(env->scratch_buf_frames, *frame);
    frame = va_arg(args, Frame *);
  }
  va_end(args);
  tkbc_register_frames_array(env, env->scratch_buf_frames);
}

void tkbc_register_frames_array(Env *env, Frames *frames) {
  assert(frames != NULL);
  if (frames->count == 0) {
    return;
  }
  bool isscratch = false;
  if (env->scratch_buf_frames == frames) {
    isscratch = true;
  }

  for (size_t i = 0; i < frames->count; ++i) {
    // Patching frames
    Frame *frame = &frames->elements[i];
    frame->index = i;
    assert(ACTION_KIND_COUNT == 9 &&
           "NOT ALL THE Action_Kinds ARE IMPLEMENTED");
    switch (frame->kind) {
    case KITE_MOVE:
    case KITE_MOVE_ADD: {
      for (size_t i = 0; i < frame->kite_index_array->count; ++i) {
        Kite *kite =
            env->kite_array->elements[frame->kite_index_array->elements[i]]
                .kite;
        kite->old_center = kite->center;
      }

    } break;
    case KITE_ROTATION_ADD:
    case KITE_TIP_ROTATION_ADD: {
      tkbc_ra_setup();
    } break;
    default: {
    }
    }
  }


  tkbc_patch_block_frames_kite_positions(env, frames);
  tkbc_dap(env->block_frames, *tkbc_deep_copy_frames(frames));

  assert(env->block_frames->count - 1 >= 0);
  env->block_frames->elements[env->block_frames->count - 1].block_index =
      env->block_frames->count - 1;

  env->scratch_buf_frames->count = 0;
  if (!isscratch) {
    tkbc_destroy_frames(frames);
  }
}

Kite_Indexs tkbc__indexs_append(size_t _, ...) {
  Kite_Indexs ki = {0};

  va_list args;
  va_start(args, _);
  for (;;) {
    // NOTE:(compiler) clang has a compiler bug that can not use size_t or
    // equivalent unsigned long int in variadic functions.
    // So the option was to just use unsigned int instead.
    Index index = va_arg(args, unsigned int);
    if (INT_MAX != index) {
      tkbc_dap(&ki, index);
    } else {
      break;
    }
  }
  va_end(args);

  return ki;
}

/**
 * @brief The function can be used to create indexes in the given range.
 *
 * @param start The index where to start, it is inclusive.
 * @param end The index where to end, it is exclusive.
 * @return The list of generated indexes form the given range.
 */
Kite_Indexs tkbc_indexs_range(int start, int end) {
  Kite_Indexs ki = {0};
  for (; start < end; ++start) {
    tkbc_dap(&ki, start);
  }
  return ki;
}

#endif // TKBC_SCRIPT_API_IMPLEMENTATION
