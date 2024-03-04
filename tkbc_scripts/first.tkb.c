#include "../tkbc.h"
#include <raylib.h>

void kite_script_input(State *state) {

  // TODO: Changing the FPS is not enough
  // TODO: And quitting the drawing and restart is not appropriate, because of
  // the loosing the state as well as confusing raylib.
  kite_script_begin(state);

  // TODO: Skip to the next displament frame also consider this for smoothness
  if (state->instruction_counter < 1) {
    kite_script_move(state->kite, 50, 50, FIXED);
    state->instruction_counter++;
  }
  if (state->instruction_counter < 2) {
    kite_script_move(state->kite, 80, 80, FIXED);
    state->instruction_counter++;
  }
  if (state->instruction_counter < 3) {
    kite_script_move(state->kite, 100, 100, FIXED);
    state->instruction_counter++;
  }
  if (state->instruction_counter < 4) {
    kite_script_move(state->kite, 200, 100, FIXED);
    state->instruction_counter++;
  }
  if (state->instruction_counter < 4) {
    kite_script_move(state->kite, 300, 100, FIXED);
    state->instruction_counter++;
  }
  if (state->instruction_counter < 5) {
    kite_script_move(state->kite, 400, 400, FIXED);
    state->instruction_counter++;

    kite_script_rotate(state->kite, 45, FIXED);
    kite_script_rotate(state->kite, 70, SMOOTH);

    kite_script_rotate_tip(state->kite, LEFT_TIP, 30, FIXED);
    kite_script_rotate_tip(state->kite, RIGHT_TIP, 30, FIXED);
    kite_script_rotate_tip(state->kite, LEFT_TIP, 30, SMOOTH);
    kite_script_rotate_tip(state->kite, RIGHT_TIP, 30, SMOOTH);
  }

  kite_script_end(state);
}
