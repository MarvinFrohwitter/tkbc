#ifndef TKBC_SCRIPT_API_H_
#define TKBC_SCRIPT_API_H_

#include "../global/tkbc-types.h"
#include "tkbc-script-handler.h"
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

Frame *tkbc__frame_generate(Env *env, Action_Kind kind, Kite_Ids kite_indexs,
                            void *raw_action, float duration);
#define tkbc_frame_generate(kind, kite_indexs, raw_action, duration)           \
  tkbc__frame_generate(env, kind, kite_indexs, raw_action, duration)
void tkbc__register_frames(Env *env, ...);
#define tkbc_register_frames(env, ...)                                         \
  tkbc__register_frames(env, __VA_ARGS__, NULL)
void tkbc_register_frames_array(Env *env, Frames *frames);

Kite_Ids tkbc__indexs_append(size_t _, ...);
#define tkbc_indexs_append(...) tkbc__indexs_append(0, __VA_ARGS__, INT_MAX)
Kite_Ids tkbc_indexs_range(int start, int end);
#define tkbc_indexs_generate(count) tkbc_indexs_range(0, count)
Kite_Ids tkbc_kite_array_generate(Env *env, size_t kite_count);
void tkbc_print_script(FILE *stream, Block_Frame *block_frame);

#endif // TKBC_SCRIPT_API_H_
