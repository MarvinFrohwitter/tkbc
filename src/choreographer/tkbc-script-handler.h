#ifndef TKBC_SCRIPT_HANDLER_H_
#define TKBC_SCRIPT_HANDLER_H_

#include "../global/tkbc-types.h"

// ===========================================================================
// ========================== Script Handler =================================
// ===========================================================================

Frame *tkbc_init_frame(void);
Frames *tkbc_deep_copy_frames(Frames *frames);
Block_Frame *tkbc_deep_copy_block_frame(Block_Frame *block_frame);
void tkbc_destroy_frames(Frames *frames);
void tkbc_render_frame(Env *env, Frame *frame);

void tkbc_patch_frames_current_time(Frames *frames);
void tkbc_patch_block_frame_kite_positions(Env *env, Frames *frames);
bool tkbc_check_finished_frames(Env *env);
size_t tkbc_check_finished_frames_count(Env *env);
void tkbc_input_handler_script(Env *env);
void tkbc_set_kite_positions_from_kite_frames_positions(Env *env);
void tkbc_scrub_frames(Env *env);

// ===========================================================================
// ========================== SCRIPT HANDLER INTERNAL ========================
// ===========================================================================

void tkbc_script_move(Kite *kite, Vector2 position, float duration);

void tkbc_script_rotate(Env *env, Kite_State *state, float angle,
                        float duration, bool adding);

void tkbc_script_rotate_tip(Env *env, Kite_State *state, TIP tip, float angle,
                            float duration, bool adding);

#endif // TKBC_SCRIPT_HANDLER_H_