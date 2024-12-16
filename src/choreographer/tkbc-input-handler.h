#ifndef TKBC_INPUT_HANDLER_H_
#define TKBC_INPUT_HANDLER_H_

#include "../global/tkbc-types.h"

// ===========================================================================
// ========================== KEYBOARD INPUT =================================
// ===========================================================================

void tkbc_input_handler(Kite_State *state);
void tkbc_input_handler_kite_array(Env *env);
void tkbc_input_check_rotation(Kite_State *state);
void tkbc_input_check_tip_turn(Kite_State *state);
void tkbc_input_check_circle(Kite_State *state);
void tkbc_input_check_movement(Kite_State *state);
void tkbc_input_check_speed(Kite_State *state);
void tkbc_input_check_mouse(Kite_State *state);

#endif // TKBC_INPUT_HANDLER_H_
