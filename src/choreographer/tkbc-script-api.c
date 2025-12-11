// ========================== SCRIPT API =====================================

#include "tkbc-script-api.h"
#include "../../external/space/space.h"
#include "../global/tkbc-types.h"
#include "../global/tkbc-utils.h"
#include "tkbc-script-converter.h"
#include "tkbc-script-handler.h"
#include "tkbc.h"
#include <stdio.h>
#include <string.h>

/**
 * @brief The function is mandatory to wrap every manual script at the
 * beginning.
 *
 * @param env The global state of the application.
 */
void tkbc__script_begin(Env *env) {
  env->script_interrupt = true;
  env->script_setup = true;
  tkbc_register_frames(env, tkbc_script_wait(0));
}

/**
 * @brief The function sets the provided name as a name for the given script.
 *
 * @param script The script where the name should be assigned.
 * @param name The new name for the script.
 */
void tkbc_set_script_name(Script *script, const char *name) {
  script->name = name;
}

/**
 * @brief The function is mandatory to wrap every manual script at the end.
 *
 * @param env The global state of the application.
 */
void tkbc__script_end(Env *env) {
  env->script_interrupt = false;

  // To ensure the script begin is called.
  if (!env->script_setup) {
    return;
  }
  env->script_setup = false;

  assert(env->scratch_buf_script.count > 0);
  env->scratch_buf_script.script_id = env->scripts.count + 1;

  tkbc_add_script(env, tkbc_deep_copy_script(&env->scripts_space,
                                             &env->scratch_buf_script));

  env->scratch_buf_script.name = NULL;
  env->scratch_buf_script.count = 0;
}

/**
 * @brief The function can be used to update the script calculation. It
 * switches the internal script buffer to the next script. For the script
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

  if (!tkbc_check_finished_frames(env)) {
    return;
  }

  if (env->frames->frames_index + 1 >= env->script->count) {
    env->script_finished = true;
    tkbc_fprintf(stderr, "INFO", "The script has finished successfully.\n");
    return;
  }

  // Overflow protection is just in case the finish detection fails or the
  // manual timeline interaction triggers a state that disables the previous
  // checks.
  assert(env->frames->frames_index + 1 < env->script->count);
  env->frames = &env->script->elements[env->frames->frames_index + 1];

  tkbc_patch_frames_current_time(env->frames);

  //
  // TODO: Add is_scrubed to every script so that the positions don't have to
  // be computed again after visiting the script once. Maybe just scrub left can
  // be implemented.
  // Keep in mind if the scripts will be stored to the disk, that feature is not
  // yet implemented, it has to recompute the positions in case the storing does
  // not include the positions. Marvin Frohwitter 25.06.2025
  for (size_t i = 0; i < env->frames->kite_frame_positions.count; ++i) {
    Id id = env->frames->kite_frame_positions.elements[i].kite_id;
    Kite *kite = tkbc_get_kite_by_id(env, id);
    if (!kite) {
      assert(0 && "Inconsistent kite_array!");
    }

    // NOTE: The design limits the combined use of center rotations and
    // tip rotations. Introduce separate tracking variables, if the
    // distinct use in one frame is needed.
    kite->old_angle = kite->angle;
    kite->old_center = kite->center;

    env->frames->kite_frame_positions.elements[i].angle = kite->angle;
    env->frames->kite_frame_positions.elements[i].position = kite->center;
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
 * that blocks the script execution by the specified time.
 *
 * @param env The global state of the application.
 * @param duration The time a frame should block/expand.
 * @return The new created wait action frame.
 */
Frame *tkbc__script_wait(Env *env, float duration) {
  if (!env->script_setup) {
    return NULL;
  }

  Wait_Action action;
  action.starttime = tkbc_get_time();

  Frame *frame = tkbc_init_frame(&env->script_creation_space);
  if (!frame) {
    return NULL;
  }
  frame->duration = duration;
  frame->kind = KITE_WAIT;
  frame->action.as_wait = action;
  return frame;
}

/**
 * @brief The function is wrapped by a macro! The function that quits all the
 * current registered frames after the duration. It can be inserted as a
 * normal frame into a block that should be quit after a some duration time.
 *
 * @param duration The time in seconds after all the frames will quit.
 * @return The frame that is constructed to represent the force quit
 * frame-block frame.
 */
Frame *tkbc__script_frames_quit(Env *env, float duration) {
  if (!env->script_setup) {
    return NULL;
  }

  Quit_Action action;
  action.starttime = tkbc_get_time();

  Frame *frame = tkbc_init_frame(&env->script_creation_space);
  if (!frame) {
    return NULL;
  }
  frame->duration = duration;
  frame->kind = KITE_QUIT;
  frame->action.as_quit = action;
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
 * @param kite_ids The list of kite indizes that are present in the kite
 * array, were the frame action should be applied to.
 * @param raw_action The action that matches the given kind.
 * @param duration The duration the action should take.
 * @return The frame that is constructed to represent the given action or NULL
 * if the frame could not be allocated or the call has happen outside a script
 * declaration.
 */
Frame *tkbc__frame_generate(Env *env, Action_Kind kind, Kite_Ids kite_ids,
                            Action raw_action, float duration) {
  if (!env->script_setup) {
    return NULL;
  }

  Frame *frame = tkbc_init_frame(&env->script_creation_space);
  if (!frame) {
    return NULL;
  }
  space_dapc(&env->script_creation_space, &frame->kite_id_array,
             kite_ids.elements, kite_ids.count);

  if (kite_ids.script_id_append) {
    frame->kite_id_array.script_id_append = true;
    space_reset_space(&env->id_space);
  }
  frame->duration = duration;
  frame->kind = kind;
  frame->action = raw_action;
  return frame;
}

/**
 * @brief The function can be used to collect all given frames into one frame
 * list and register them as a new frame block. The provided memory is freed
 * while internal use.
 *
 * @param env The environment that holds the current state of the application.
 */
void tkbc__register_frames(Env *env, ...) {
  tkbc_reset_frames_internal_data(&env->scratch_buf_frames);

  va_list args;
  va_start(args, env);
  Frame *frame = va_arg(args, Frame *);
  while (frame != NULL) {
    space_dap(&env->script_creation_space, &env->scratch_buf_frames,
              tkbc_deep_copy_frame(&env->script_creation_space, frame));

    if (frame->kite_id_array.script_id_append) {
      frame->kite_id_array.script_id_append = false;
      space_reset_space(&env->id_space);
    }
    // This allows the user to create a frame ptr manually and pass it and not
    // use the provided script API that uses the space buffer..
    if (!space__find_planet_from_ptr(&env->script_creation_space, frame)) {
      free(frame);
    }
    frame = va_arg(args, Frame *);
  }
  va_end(args);
  tkbc_register_frames_array(env, &env->scratch_buf_frames);
}

/**
 * @brief The function copies the given frame to the scratch_buf_frames and
 * frees the original pointer and dependent on the creation also the
 * kite_id_array.
 *
 * @param env The global state of the application.
 * @param frame The frames the should be appended to the scratch_buf_frames.
 */
// TODO: rename tkbc_script_team_scratch_buf_frames_append()
void tkbc_sript_team_scratch_buf_frames_append_and_free(Env *env,
                                                        Frame *frame) {
  space_dap(&env->script_creation_space, &env->scratch_buf_frames,
            tkbc_deep_copy_frame(&env->script_creation_space, frame));

  // NOTE: This function expects to get a frame created by the
  // tkbc__frame_generate() function that handles the allocation of the id
  // regarding frame->kite_id_array.script_id_append so this should not be
  // handled in this function. All given kite_ids in the frame should be freed.
}

/**
 * @brief The function registers the given frames array into the global env
 * state that holds the script. It is also responsible for patching
 * corresponding block frame positions and frame indices.
 *
 * @param env The global state of the application.
 * @param frames The collect frames that should be registered as a
 * script.
 */
void tkbc_register_frames_array(Env *env, Frames *frames) {
  assert(frames != NULL);
  if (frames->count == 0) {
    return;
  }
  bool isscratch = false;
  if (&env->scratch_buf_frames == frames) {
    isscratch = true;
  }

  for (size_t i = 0; i < frames->count; ++i) {
    // Patching frames
    Frame *frame = &frames->elements[i];
    frame->index = i;
    if (frame->kind == KITE_WAIT || frame->kind == KITE_QUIT) {
      continue;
    }

    for (size_t j = 0; j < frame->kite_id_array.count; ++j) {
      Kite *kite = tkbc_get_kite_by_id(env, frame->kite_id_array.elements[j]);
      kite->old_angle = kite->angle;
      kite->old_center = kite->center;
    }
  }
  tkbc_patch_frames_kite_positions(env, frames);
  Frames copy_frames =
      tkbc_deep_copy_frames(&env->script_creation_space, frames);
  space_dap(&env->script_creation_space, &env->scratch_buf_script, copy_frames);
  tkbc_reset_frames_internal_data(frames);

  assert((int)env->scratch_buf_script.count - 1 >= 0);
  env->scratch_buf_script.elements[env->scratch_buf_script.count - 1]
      .frames_index = env->scratch_buf_script.count - 1;

  if (!isscratch) {
    tkbc_destroy_frames_internal_data(frames);
  }
}

/**
 * @brief The function is wrapped by a macro! The function can be used to
 * combine the given numbers of kite indices.
 *
 * @param _ The param is ignored just just for variadic function
 * implementation.
 * @return The dynamic array list of kite indices.
 */
Kite_Ids tkbc__indexs_append(Space *space, ...) {
  Kite_Ids ki = {0};

  va_list args;
  va_start(args, space);
  for (;;) {
    // NOTE:(compiler) clang 18.1.8 has a compiler bug that can not use size_t
    // or equivalent unsigned long int in variadic functions. It is related to
    // compiler caching. So the option is to just use unsigned int instead.
    // unsigned int index = va_arg(args, unsigned int);
    Index index = va_arg(args, size_t);
    if (UINT_MAX != index) {
      space_dap(space, &ki, index);
    } else {
      break;
    }
  }
  va_end(args);

  ki.script_id_append = true;
  return ki;
}

/**
 * @brief The function can be used to create indexes in the given range.
 *
 * @param start The index where to start, it is inclusive.
 * @param end The index where to end, it is exclusive.
 * @return The list of generated indexes form the given range.
 */
Kite_Ids tkbc_indexs_range(int start, int end) {
  Kite_Ids ki = {0};
  for (; start < end; ++start) {
    tkbc_dap(&ki, start);
  }
  return ki;
}

/**
 * @brief The function initializes the amount of kites that are provided in
 * the arguments and inserts them in the global kite_array. It also sets a
 * different color for each kite, rather than the default color.
 *
 * @param env The global state of the application.
 * @param kite_count The amount of kites that are pushed to the kite array.
 * @return The kite indies that are appended to the kite array.
 */
Kite_Ids tkbc_kite_array_generate(Env *env, size_t kite_count) {
  if (kite_count <= 0) {
    return (Kite_Ids){0};
  }
  for (size_t i = 0; i < kite_count; ++i) {
    tkbc_dap(env->kite_array, tkbc_init_kite());
    // The id starts from 0.
    env->kite_array->elements[env->kite_array->count - 1].kite_id =
        env->kite_id_counter++;
    env->kite_array->elements[env->kite_array->count - 1].kite->body_color =
        tkbc_get_random_color();
  }

  tkbc_kite_array_start_position(env->kite_array, env->window_width,
                                 env->window_height);

  Kite_Ids ids = tkbc_indexs_range(
      env->kite_array->elements[env->kite_array->count - kite_count].kite_id,
      env->kite_array->elements[env->kite_array->count - kite_count].kite_id +
          kite_count);
  return ids;
}

/**
 * @brief The function prints the given script that has to be in memory to the
 * specified output in a debug format.
 *
 * @param stream The stream where the output of the debug print will end up, if
 * NULL is provided the print will be omitted.
 * @param script The pointer to the script that should be printed.
 */
void tkbc_print_script(FILE *stream, Script *script) {
  if (!stream) {
    return;
  }

  fprintf(stream, "Script: %zu\n", script->script_id);
  for (size_t block = 0; block < script->count; ++block) {
    fprintf(stream, "  Block-Index: %zu\n",
            script->elements[block].frames_index);

    fprintf(stream, "    Kite-Frames:\n");
    for (size_t frame = 0; frame < script->elements[block].count; ++frame) {
      fprintf(stream, "      {\n");
      fprintf(stream, "      Index:%zu\n",
              script->elements[block].elements[frame].index);

      if (script->elements[block].elements[frame].kite_id_array.count) {
        fprintf(
            stream, "      Kite-Ids: [%zu",
            script->elements[block].elements[frame].kite_id_array.elements[0]);

        for (size_t index = 1;
             index <
             script->elements[block].elements[frame].kite_id_array.count;
             ++index) {
          fprintf(stream, ", %zu",
                  script->elements[block]
                      .elements[frame]
                      .kite_id_array.elements[index]);
        }
      } else {
        fprintf(stream, "      Kite-Indies: [");
      }
      fprintf(stream, "]\n");

      fprintf(stream, "      Duration:%fs\n",
              script->elements[block].elements[frame].duration);
      fprintf(stream, "      Finished:%s\n",
              script->elements[block].elements[frame].finished ? "TRUE"
                                                               : "FALSE");
      int kind = script->elements[block].elements[frame].kind;
      switch (kind) {

      case KITE_QUIT: {
        fprintf(stream, "      Action-Kind: KITE_QUIT\n");
        Quit_Action action =
            script->elements[block].elements[frame].action.as_quit;
        fprintf(stream, "        Time:%fs)\n", (float)action.starttime);
      } break;

      case KITE_WAIT: {
        fprintf(stream, "      Action-Kind: KITE_WAIT\n");
        Wait_Action action =
            script->elements[block].elements[frame].action.as_wait;
        fprintf(stream, "        Time:%fs)\n", (float)action.starttime);
      } break;

      case KITE_MOVE: {
        fprintf(stream, "      Action-Kind: Kite_Move\n");
        Move_Action action =
            script->elements[block].elements[frame].action.as_move;
        fprintf(stream, "        Position:(%f,%f)\n", action.position.y,
                action.position.y);
      } break;

      case KITE_MOVE_ADD: {
        fprintf(stream, "      Action-Kind: KITE_MOVE_ADD\n");
        Move_Add_Action action =
            script->elements[block].elements[frame].action.as_move_add;
        fprintf(stream, "        Position:(%f,%f)\n", action.position.y,
                action.position.y);
      } break;

      case KITE_ROTATION: {
        fprintf(stream, "      Action-Kind: KITE_ROTATION\n");
        Rotation_Action action =
            script->elements[block].elements[frame].action.as_rotation;
        fprintf(stream, "        Angle:%f\n", action.angle);
      } break;

      case KITE_ROTATION_ADD: {
        fprintf(stream, "      Action-Kind: KITE_ROTATION_ADD\n");
        Rotation_Add_Action action =
            script->elements[block].elements[frame].action.as_rotation_add;
        fprintf(stream, "        Angle:%f\n", action.angle);
      } break;

      case KITE_TIP_ROTATION: {
        fprintf(stream, "      Action-Kind: KITE_TIP_ROTATION\n");
        Tip_Rotation_Action action =
            script->elements[block].elements[frame].action.as_tip_rotation;
        fprintf(stream, "        Angle:%f\n", action.angle);
        fprintf(stream, "        Tip:%s\n",
                action.tip == LEFT_TIP ? "LEFT_TIP" : "RIGHT_TIP");
      } break;

      case KITE_TIP_ROTATION_ADD: {
        fprintf(stream, "      Action-Kind: KITE_TIP_ROTATION_ADD\n");
        Tip_Rotation_Add_Action action =
            script->elements[block].elements[frame].action.as_tip_rotation_add;
        fprintf(stream, "        Angle:%f\n", action.angle);
        fprintf(stream, "        Tip:%s\n",
                action.tip == LEFT_TIP ? "LEFT_TIP" : "RIGHT_TIP");
      } break;

      default: {
        assert("UNREACHABLE tkbc_print_scipt");
      }
      }

      fprintf(stream, "      }\n");
    }

    fprintf(stream, "    Kite-Frame-Positions:\n");
    fprintf(stream, "    {\n");
    for (size_t kite_frame_poition = 0;
         kite_frame_poition <
         script->elements[block].kite_frame_positions.count;
         ++kite_frame_poition) {

      fprintf(stream, "      {\n");
      Kite_Position *kp =
          &script->elements[block]
               .kite_frame_positions.elements[kite_frame_poition];
      fprintf(stream, "      Kite:%zu\n", kp->kite_id);
      fprintf(stream, "      Angle:%f\n", kp->angle);
      fprintf(stream, "      Position:(%f,%f)\n", kp->position.x,
              kp->position.y);
      fprintf(stream, "      }\n");
    }
    fprintf(stream, "    }\n");
  }
}

/**
 * @brief The function prints all scripts that are in memory to the specified
 * output in a  debug format.
 *
 * @param stream The stream where the output of the debug print will end up, if
 * NULL is provided the print will be omitted.
 * @param env The global state of the application.
 */
void tkbc_print_all_scripts(FILE *stream, Env *env) {
  if (!stream) {
    return;
  }
  for (size_t i = 0; i < env->scripts.count; ++i) {
    tkbc_print_script(stream, &env->scripts.elements[i]);
  }
}

/**
 * @brief The function prints all scripts that are in memory to the specified
 * output in a  debug format. In a second step the scripts are going to be
 * exported and saved in .kite file with the corresponding script name, that is
 * specified at the script declaration time or it will get a custom generated
 * name with an id, if no name was specified.
 *
 * @param stream The stream where the output of the debug print will end up, if
 * NULL is provided the print will be omitted.
 * @param env The global state of the application.
 */
void tkbc_debug_print_and_export_all_scripts(FILE *stream, Env *env) {
  for (size_t i = 0; i < env->scripts.count; ++i) {
    if (stream) {
      tkbc_print_script(stream, &env->scripts.elements[i]);
    }
    int ret = tkbc_export_all_scripts_to_dot_kite_file_from_mem(env);
    assert(ret == 0 && "ERROR: Not all the scripts are correctly exported.");
  }
}
