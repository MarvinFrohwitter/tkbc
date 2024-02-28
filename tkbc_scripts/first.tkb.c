#include "../tkbc.h"

void kite_script_input(State *state) {

  // TODO: Changing the FPS is not enough
  // TODO: And quitting the drawing and restart is not appropriate, because of
  // the loosing the state as well as confusing raylib.
  kite_script_begin(state);

  ClearBackground(RED);

  kite_script_move(state->kite, 50, 50, FIXED);
  kite_script_move(state->kite, 80, 80, FIXED);
  kite_script_move(state->kite, 100, 100, FIXED);

  // rotate(45, SMOOTH);

  kite_script_end(state);
}

// move 5 smooth
// move 5 fixed
// rotate 45 smooth
// rotate 45 fixed
// rotatetip left 45 smooth
// rotatetip left 45 fixed
// rotatetip right 45 smooth
// rotatetip right 45 fixed
