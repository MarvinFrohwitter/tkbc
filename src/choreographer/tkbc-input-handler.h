#ifndef TKBC_INPUT_HANDLER_H_
#define TKBC_INPUT_HANDLER_H_

#include "../global/tkbc-types.h"
#include "tkbc-keymaps.h"

// ===========================================================================
// ========================== KEYBOARD INPUT =================================
// ===========================================================================

void tkbc_input_handler(KeyMaps keymaps, Kite_State *state);
void tkbc_input_handler_kite_array(Env *env);
void tkbc_input_check_rotation(KeyMaps keymaps, Kite_State *s);
void tkbc_input_check_tip_turn(KeyMaps keymaps, Kite_State *s);
void tkbc_input_check_circle(KeyMaps keymaps, Kite_State *s);
void tkbc_input_check_movement(KeyMaps keymaps, Kite_State *state);
void tkbc_input_check_speed(KeyMaps keymaps, Kite_State *state);
void tkbc_input_check_mouse(Kite_State *state);
void tkbc_mouse_control(KeyMaps keymaps, Kite_State *state);

#endif // TKBC_INPUT_HANDLER_H_
