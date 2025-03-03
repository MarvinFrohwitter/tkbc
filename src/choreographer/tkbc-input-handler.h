#ifndef TKBC_INPUT_HANDLER_H_
#define TKBC_INPUT_HANDLER_H_

#include "../global/tkbc-types.h"
#include "tkbc-keymaps.h"

// ===========================================================================
// ========================== KEYBOARD INPUT =================================
// ===========================================================================

void tkbc_input_handler(Key_Maps keymaps, Kite_State *state);
void tkbc_input_handler_kite_array(Env *env);
void tkbc_input_check_rotation(Key_Maps keymaps, Kite_State *s);
void tkbc_input_check_tip_turn(Key_Maps keymaps, Kite_State *s);
void tkbc_input_check_circle(Key_Maps keymaps, Kite_State *s);
void tkbc_input_check_movement(Key_Maps keymaps, Kite_State *state);
void tkbc_input_check_speed(Key_Maps keymaps, Kite_State *state);
void tkbc_input_check_mouse(Kite_State *state);

void tkbc_input_check_locked_angle(Kite_State *state);
void tkbc_mouse_control(Key_Maps keymaps, Kite_State *state);
void tkbc_input_check_rotation_mouse_control(Key_Maps keymaps, Kite_State *s);
void tkbc_input_check_tip_rotation_mouse_control(Key_Maps keymaps,
                                                 Kite_State *s);

void tkbc_calcluate_and_update_angle(Key_Maps keymaps, Kite_State *state);
void tkbc_calculate_new_kite_position(Key_Maps keymaps, Kite_State *state);
void tkbc_calculate_and_update_snapping_angle(Key_Maps keymaps,
                                              Kite_State *state);
bool tkbc_check_is_mouse_in_dead_zone(Kite_State *state,
                                      size_t dead_zone_radius);
bool tkbc_check_is_angle_locked(Key_Maps keymaps, Kite_State *state);

#endif // TKBC_INPUT_HANDLER_H_
