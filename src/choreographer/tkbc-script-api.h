#ifndef TKBC_SCRIPT_API_H_
#define TKBC_SCRIPT_API_H_

#include "../global/tkbc-types.h"
#include "../../external/space/space.h"
#include "tkbc-script-handler.h"
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>

// ===========================================================================
// ========================== SCRIPT API =====================================
// ===========================================================================

void tkbc__script_input(Env *env);
void tkbc__script_begin(Env *env);
void tkbc__script_end(Env *env);
void tkbc_set_script_name(Script *script, const char *name);

#define tkbc_script_input void tkbc__script_input(Env *env)

#define tkbc_script_begin(...)                                                 \
  do {                                                                         \
    tkbc__script_begin(env);                                                   \
    const char *tmp = "" __VA_ARGS__;                                          \
    if (!*tmp) {                                                               \
      tmp = NULL;                                                              \
    }                                                                          \
    tkbc_set_script_name(&env->scratch_buf_script, tmp);                       \
  } while (0)

#define tkbc_script_end() tkbc__script_end(env)

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

Frame *tkbc__frame_generate(Env *env, Action_Kind kind, Kite_Ids kite_ids,
                            Action raw_action, float duration);
#define tkbc_frame_generate(kind, kite_ids, raw_action, duration)              \
  tkbc__frame_generate(env, kind, kite_ids, *(Action *)raw_action, duration)
void tkbc__register_frames(Env *env, ...);
#define tkbc_register_frames(env, ...)                                         \
  tkbc__register_frames(env, __VA_ARGS__, NULL)
#define SET(...) tkbc__register_frames(env, __VA_ARGS__, NULL)
#define COLLECTION(...) SET(__VA_ARGS__)

#define KITE_WAIT(duration) tkbc_script_wait((duration))
#define KITE_QUIT(duration) tkbc_script_frames_quit((duration))

#define KITE_MOVE_ADD(kite_ids, pos_x, pos_y, duration)                        \
  tkbc_frame_generate(                                                         \
      KITE_MOVE_ADD, (kite_ids),                                               \
      (&(Move_Add_Action){.position.x = (pos_x), .position.y = (pos_y)}),      \
      (duration))

#define KITE_MOVE(kite_ids, pos_x, pos_y, duration)                            \
  tkbc_frame_generate(                                                         \
      KITE_MOVE, (kite_ids),                                                   \
      (&(Move_Action){.position.x = (pos_x), .position.y = (pos_y)}),          \
      (duration))

#define KITE_ROTATION_ADD(kite_ids, new_angle, duration)                       \
  tkbc_frame_generate(KITE_ROTATION_ADD, (kite_ids),                           \
                      (&(Rotation_Add_Action){.angle = (new_angle)}),          \
                      (duration))

#define KITE_ROTATION(kite_ids, new_angle, duration)                           \
  tkbc_frame_generate(KITE_ROTATION, (kite_ids),                               \
                      (&(Rotation_Action){.angle = (new_angle)}), (duration))

#define KITE_TIP_ROTATION_ADD(kite_ids, new_angle, new_tip, duration)          \
  tkbc_frame_generate(                                                         \
      KITE_TIP_ROTATION_ADD, (kite_ids),                                       \
      (&(Tip_Rotation_Add_Action){.angle = (new_angle), .tip = (new_tip)}),    \
      (duration))

#define KITE_TIP_ROTATION(kite_ids, new_angle, new_tip, duration)              \
  tkbc_frame_generate(                                                         \
      KITE_TIP_ROTATION, (kite_ids),                                           \
      (&(Tip_Rotation_Action){.angle = (new_angle), .tip = (new_tip)}),        \
      (duration))

void tkbc_register_frames_array(Env *env, Frames *frames);
void tkbc_sript_team_scratch_buf_frames_append_and_free(Env *env, Frame *frame);

Kite_Ids tkbc__indexs_append(Space *space, ...);
#define tkbc_indexs_append(...) tkbc__indexs_append(&env->id_space, __VA_ARGS__, UINT_MAX)
#define ID(...) tkbc_indexs_append(__VA_ARGS__)

Kite_Ids tkbc_indexs_range(int start, int end);
#define tkbc_indexs_generate(count) tkbc_indexs_range(0, count)
Kite_Ids tkbc_kite_array_generate(Env *env, size_t kite_count);
void tkbc_print_script(FILE *stream, Script *script);

#endif // TKBC_SCRIPT_API_H_
