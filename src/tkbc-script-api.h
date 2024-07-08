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
  env->global_block_index = 0;
  env->attempts_block_index = 0;
  tkbc_register_frames(env, tkbc_script_wait(0));
}

void tkbc_script_end(Env *env) {
  env->script_interrupt = false;
  size_t old_max_block_index = env->max_block_index;
  if (!env->script_finished) {
    env->max_block_index =
        tkbc_max(env->max_block_index, env->attempts_block_index);
  }

  if (env->script_setup) {
    env->script_setup = false;
    return;
  }

  if (env->max_block_index == old_max_block_index &&
      tkbc_check_finished_frames(env) &&
      env->frames->block_index + 1 == env->max_block_index) {

    if (!env->script_finished) {
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
}

void tkbc_script_update_frames(Env *env) {
  for (size_t i = 0; i < env->frames->count; ++i) {
    Frame *frame = &env->frames->elements[i];
    assert(frame != NULL);
    if (!frame->finished) {
      tkbc_render_frame(env, frame);

    } else {
      tkbc_frame_reset(&env->frames->elements[i]);
    }
  }
}

bool tkbc_script_finished(Env *env) {
  return env->script_finished ? true : false;
}

// ========================== SCRIPT HANDLER API =============================

Frame *tkbc__script_wait(Env *env, float duration) {
  if (env->script_finished) {
    return NULL;
  }
  if (!tkbc_check_finished_frames(env)) {
    return NULL;
  }

  Wait_Action *action;
  action_alloc(Wait_Action);
  action->starttime = GetTime();

  Frame *frame = tkbc_init_frame();
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
Frame *tkbc__script_frames_quit(Env *env, float duration) {
  if (env->script_finished) {
    return NULL;
  }
  if (!tkbc_check_finished_frames(env)) {
    return NULL;
  }

  Quit_Action *action;
  action_alloc(Quit_Action);
  action->starttime = GetTime();

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
  if (!tkbc_check_finished_frames(env)) {
    return NULL;
  }

  void *action;
  Frame *frame = tkbc_init_frame();
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
    assert(0 && "Unsupported Kite Action");
  } break;
  }

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

  env->attempts_block_index++;

  if (!tkbc_check_finished_frames(env)) {
    tkbc_destroy_frames(frames);
    return;
  }

  size_t block_index = env->global_block_index++;
  for (size_t i = 0; i < env->index_blocks->count; ++i) {
    if (block_index == env->index_blocks->elements[i]) {
      tkbc_destroy_frames(frames);
      return;
    }
  }

  tkbc_destroy_frames(env->frames);
  for (size_t i = 0; i < frames->count; ++i) {
    tkbc_register_frame(env, &frames->elements[i]);
  }

  env->frames->block_index = block_index;
  tkbc_dap(env->index_blocks, block_index);
  env->scratch_buf_frames->count = 0;
}

Kite_Indexs tkbc__indexs_append(size_t _, ...) {
  Kite_Indexs ki = {0};

  va_list args;
  va_start(args, _);
  for (;;) {
    Index index = va_arg(args, Index);
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
