#ifndef TKBC_SCRIPT_API_H_
#define TKBC_SCRIPT_API_H_

#include "../global/tkbc-types.h"
#include "tkbc-script-handler.h"
#include <stdbool.h>

// ===========================================================================
// ========================== SCRIPT API =====================================
// ===========================================================================

void tkbc__script_input(Env *env);
void tkbc__script_begin(Env *env);
void tkbc__script_end(Env *env);

#define tkbc_script_input void tkbc__script_input(Env *env)

#define tkbc_script_begin tkbc__script_begin(env);
#define tkbc_script_end tkbc__script_end(env);

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

Frame *tkbc__frame_generate(Env *env, Action_Kind kind, Kite_Ids kite_indexs,
                            void *raw_action, float duration);
#define tkbc_frame_generate(kind, kite_indexs, raw_action, duration)           \
  tkbc__frame_generate(env, kind, kite_indexs, raw_action, duration)
void tkbc__register_frames(Env *env, ...);
#define tkbc_register_frames(env, ...)                                         \
  tkbc__register_frames(env, __VA_ARGS__, NULL)
#define SET(...) tkbc__register_frames(env, __VA_ARGS__, NULL)
#define COLLECTION(...) SET(__VA_ARGS__)

#define KITE_WAIT(duration) tkbc_script_wait(duration)
#define KITE_QUIT(duration) tkbc_script_frames_quit(duration)

#define KITE_MOVE_ADD(kite_ids, pos_x, pos_y, duration)                        \
  tkbc_frame_generate(                                                         \
      KITE_MOVE_ADD, kite_ids,                                                 \
      (&(Move_Add_Action){.position.x = pos_x, .position.y = pos_y}),          \
      duration)

#define KITE_MOVE(kite_ids, pos_x, pos_y, duration)                            \
  tkbc_frame_generate(                                                         \
      KITE_MOVE, kite_ids,                                                     \
      (&(Move_Action){.position.x = pos_x, .position.y = pos_y}), duration)

#define KITE_ROTATION_ADD(kite_ids, new_angle, duration)                       \
  tkbc_frame_generate(KITE_ROTATION_ADD, kite_ids,                             \
                      (&(Rotation_Add_Action){.angle = new_angle}), duration)

#define KITE_ROTATION(kite_ids, new_angle, duration)                           \
  tkbc_frame_generate(KITE_ROTATION, kite_ids,                                 \
                      (&(Rotation_Action){.angle = new_angle}), duration)

#define KITE_TIP_ROTATION_ADD(kite_ids, new_angle, new_tip, duration)          \
  tkbc_frame_generate(                                                         \
      KITE_TIP_ROTATION_ADD, kite_ids,                                         \
      (&(Tip_Rotation_Add_Action){.angle = new_angle, .tip = new_tip}),        \
      duration)

#define KITE_TIP_ROTATION(kite_ids, new_angle, new_tip, duration)              \
  tkbc_frame_generate(                                                         \
      KITE_TIP_ROTATION, kite_ids,                                             \
      (&(Tip_Rotation_Action){.angle = new_angle, .tip = new_tip}), duration)

void tkbc_register_frames_array(Env *env, Frames *frames);

Kite_Ids tkbc__indexs_append(size_t _, ...);
#define tkbc_indexs_append(...) tkbc__indexs_append(0, __VA_ARGS__, INT_MAX)
#define ID(...) tkbc_indexs_append(__VA_ARGS__)

Kite_Ids tkbc_indexs_range(int start, int end);
#define tkbc_indexs_generate(count) tkbc_indexs_range(0, count)
Kite_Ids tkbc_kite_array_generate(Env *env, size_t kite_count);
void tkbc_print_script(FILE *stream, Block_Frame *block_frame);

#endif // TKBC_SCRIPT_API_H_
