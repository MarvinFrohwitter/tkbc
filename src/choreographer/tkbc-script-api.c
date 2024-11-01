// ========================== SCRIPT API =====================================

#include "tkbc-script-api.h"
#include "../global/tkbc-types.h"
#include "../global/tkbc-utils.h"

/**
 * @brief The function is mandatory to wrap every manual script at the
 * beginning.
 *
 * @param env The global state of the application.
 */
void tkbc_script_begin(Env *env) {
  env->script_interrupt = true;
  env->script_setup = true;
  env->script_counter++;
  tkbc_register_frames(env, tkbc_script_wait(0));
}

/**
 * @brief The function is mandatory to wrap every manual script at the end.
 *
 * @param env The global state of the application.
 */
void tkbc_script_end(Env *env) {
  tkbc_register_frames(env, tkbc_script_wait(0));
  env->script_interrupt = false;

  if (env->script_setup) {
    assert(env->scratch_buf_block_frame->count > 0);
    env->scratch_buf_block_frame->script_id = env->script_counter;
    tkbc_dap(env->block_frames,
             *tkbc_deep_copy_block_frame(env->scratch_buf_block_frame));
    env->scratch_buf_block_frame->count = 0;

    env->script_setup = false;
    return;
  }

  if (tkbc_check_finished_frames(env) &&
      (env->block_frame->count == env->frames->block_index + 1)) {

    if (!env->script_finished) {
      env->script_finished = true;
      printf("=========== FROM THE SCRIPT END FUNCTION ========\n");
      printf("KITE: INFO: The script has finished successfully.\n");
      printf("=================================================\n");
    }
  }
}

/**
 * @brief The function can be used to update the block_frame calculation. It
 * switches the internal script buffer to the next block_frame. For the script
 * execution it is mandatory to call this function.
 *
 * @param env The global state of the application.
 */
void tkbc_script_update_frames(Env *env) {
  for (size_t i = 0; i < env->frames->count; ++i) {
    Frame *frame = &env->frames->elements[i];
    assert(frame != NULL);
    if (!frame->finished) {
      tkbc_render_frame(env, frame);
    }
  }

  if (env->frames->block_index + 1 < env->block_frame->count) {
    if (tkbc_check_finished_frames(env)) {

      // Overflow protection is just in case the finish detection fails or the
      // manual timeline interaction triggers a state that disables the previous
      // checks.
      assert(env->frames->block_index + 1 < env->block_frame->count);
      env->frames = &env->block_frame->elements[env->frames->block_index + 1];

      tkbc_patch_frames_current_time(env->frames);

      for (size_t i = 0; i < env->frames->kite_frame_positions->count; ++i) {
        Index k_index = env->frames->kite_frame_positions->elements[i].kite_id;
        Kite *kite = env->kite_array->elements[k_index].kite;

        // NOTE: The design limits the combined use of center rotations and tip
        // rotations. Introduce separate tracking variables, if the distinct use
        // in one frame is needed.

        kite->old_angle = kite->angle;
        kite->old_center = kite->center;

        env->frames->kite_frame_positions->elements[i].angle = kite->angle;
        env->frames->kite_frame_positions->elements[i].position = kite->center;
      }
    }
  } else {
    env->script_finished = true;
    printf("KITE: INFO: The script has finished successfully.\n");
  }
}

/**
 * @brief The function can be used to check if the currently loaded script has
 * finished.
 *
 * @param env The global state of the application.
 * @return True if the script has finished successfully, otherwise false.
 */
bool tkbc_script_finished(Env *env) { return env->script_finished; }

// ========================== SCRIPT HANDLER API =============================

/**
 * @brief The function is wrapped by a macro! The call creates an action frame
 * that blocks the block_frame execution by the specified time.
 *
 * @param env The global state of the application.
 * @param duration The time a frame should block/expand.
 * @return The new created wait action frame.
 */
Frame *tkbc__script_wait(Env *env, float duration) {
  // if (env->script_finished) {
  //   return NULL;
  // }
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
 * @brief The function is wrapped by a macro! The function that quits all the
 * current registered frames after the duration. It can be inserted as a normal
 * frame into a block that should be quit after a some duration time.
 *
 * @param duration The time in seconds after all the frames will quit.
 * @return The frame that is constructed to represent the force quit frame-block
 * frame.
 */
Frame *tkbc__script_frames_quit(Env *env, float duration) {
  // if (env->script_finished) {
  //   return NULL;
  // }
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

  // if (env->script_finished) {
  //   return NULL;
  // }
  if (!env->script_setup) {
    return NULL;
  }

  Frame *frame = tkbc_init_frame();
  void *action = tkbc_move_action_to_heap(raw_action, kind, true);

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

/**
 * @brief The function registers the given frames array into the global env
 * state that holds the block_frame. It is also responsible for patching
 * corresponding block frame positions and frame indices.
 *
 * @param env The global state of the application.
 * @param frames The collect frames that should be registered as a block_frame.
 */
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
      for (size_t i = 0; i < frame->kite_index_array->count; ++i) {
        Kite *kite =
            env->kite_array->elements[frame->kite_index_array->elements[i]]
                .kite;
        kite->old_angle = kite->angle;
        kite->old_center = kite->center;
      }
    } break;
    default: {
    }
    }
  }

  tkbc_patch_block_frame_kite_positions(env, frames);
  tkbc_dap(env->scratch_buf_block_frame, *tkbc_deep_copy_frames(frames));

  assert((int)env->scratch_buf_block_frame->count - 1 >= 0);
  env->scratch_buf_block_frame
      ->elements[env->scratch_buf_block_frame->count - 1]
      .block_index = env->scratch_buf_block_frame->count - 1;

  env->scratch_buf_frames->count = 0;
  if (!isscratch) {
    tkbc_destroy_frames(frames);
  }
}

/**
 * @brief The function is wrapped by a macro! The function can be used to
 * combine the given numbers of kite indices.
 *
 * @param _ The param is ignored just just for variadic function implementation.
 * @return The dynamic array list of kite indices.
 */
Kite_Indexs tkbc__indexs_append(size_t _, ...) {
  Kite_Indexs ki = {0};

  va_list args;
  va_start(args, _);
  for (;;) {
    // NOTE:(compiler) clang 18.1.8 has a compiler bug that can not use size_t
    // or equivalent unsigned long int in variadic functions. It is related to
    // compiler caching. So the option is to just use unsigned int instead.
    // unsigned int index = va_arg(args, unsigned int);
    Index index = va_arg(args, size_t);
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
