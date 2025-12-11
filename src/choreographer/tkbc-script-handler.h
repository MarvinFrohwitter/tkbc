#ifndef TKBC_SCRIPT_HANDLER_H_
#define TKBC_SCRIPT_HANDLER_H_

#include "../global/tkbc-types.h"

// ===========================================================================
// ========================== Script Handler =================================
// ===========================================================================

Frame *tkbc_init_frame(Space *space);
Kite_State *tkbc_get_kite_state_by_id(Env *env, size_t id);
Kite *tkbc_get_kite_by_id(Env *env, size_t id);
Kite *tkbc_get_kite_by_id_unwrap(Env *env, size_t id);
bool tkbc_scripts_contains_id(Scripts scripts, Id script_id);
bool tkbc_contains_id(Kite_Ids kite_ids, size_t id);
Frame tkbc_deep_copy_frame(Space *space, Frame *frame);
Frames tkbc_deep_copy_frames(Space *space, Frames *frames);
Script tkbc_deep_copy_script(Space *space, Script *script);
void tkbc_destroy_frames_internal_data(Frames *frames);
void tkbc_reset_frames_internal_data(Frames *frames);
void tkbc_render_frame(Env *env, Frame *frame);

void tkbc_patch_frames_current_time(Frames *frames);
void tkbc_remap_script_kite_id_arrays_to_kite_ids(Env *env, Script *script,
                                                  Kite_Ids kite_ids);
void tkbc_patch_frames_kite_positions(Env *env, Frames *frames);
bool tkbc_check_finished_frames(Env *env);
size_t tkbc_check_finished_frames_count(Env *env);
void tkbc_load_next_script(Env *env);
bool tkbc_load_script_id(Env *env, size_t script_id);
void tkbc_unload_script(Env *env);
void tkbc_add_script(Env *env, Script script);
void tkbc_input_handler_script(Env *env);
void tkbc_set_kite_positions_from_kite_frames_positions(Env *env);
void tkbc_scrub_frames(Env *env);

// ===========================================================================
// ========================== SCRIPT HANDLER INTERNAL ========================
// ===========================================================================

Vector2 tkbc_script_move(Kite *kite, Vector2 position, float duration);
float tkbc_script_rotate(Kite *kite, float angle, float duration, bool adding);
float tkbc_script_rotate_tip(Kite *kite, TIP tip, float angle, float duration,
                             bool adding);
float tkbc_check_angle_zero(Kite *kite, Action_Kind kind, Action action,
                            float duration);

#endif // TKBC_SCRIPT_HANDLER_H_
