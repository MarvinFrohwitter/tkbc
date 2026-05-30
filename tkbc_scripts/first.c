#include "../src/choreographer/tkbc-script-api.h"

// Some test scripts
#include "demo_angle_left_right_rotation.c"
#include "demo_rotation_checkup.c"
#include "demo_split_roll.c"
#include "demo_team_function_demonstration.c"

#include "demo_choreo.c"

// The env of type Env is passed automatically into the scope of the
// script_input it is not globally available.
tkbc_script_input {
  kite = *env->vanilla_kite;

  // NOTE: The fact that sometimes the id is hard-coded to 0 or 1 is just
  // possible because of the internal handling knowledge in general the ki's
  // that are returned in this call should be used.
#define kite_count 2
  Kite_Ids ki = tkbc_kite_array_generate(env, kite_count);
  static_assert(kite_count >= 2, "");
  zero = ki.elements[0];
  one = ki.elements[1];

  split_roll_demonstration(env, ki);
  choreo(env, ki);
  rotation_checkup_call(env, ki);
  team_function_demonstration(env, ki);
  angle_left_right_rotation(env, ki);

  //////////////////////////////////////////////////////////////////////
  //
  // BUG At the moment KITE_MOVE_ADD() and KITE_TIP_ROTATION_ADD() can't be
  //     called in the same SET(), because they internally uses not the same
  //     reposition function and the TIP location computation messes with moving
  //     the kite first.
  //     So basically the KITE_MOVE_ADD can overshoot and don't terminate,
  //     because the TIP computation moves the kite as well.
  //
  //     The solution would be to check for move and tip rotations explicitly.
  //     An compute the combined endposition up front and patch the potions
  //     where the kite should move to.
  //
  // Tho the workaround for now is to just call the normal rotation an move to
  // the correct position manually.
  //
  // tkbc_script_begin(); SET(
  //     KITE_MOVE_ADD(ID(zero), -kite.width, 0, rotation_duration),
  //     KITE_MOVE_ADD(ID(one), kite.width, 0, rotation_duration),

  //     KITE_TIP_ROTATION_ADD(ID(zero), 180, LEFT_TIP, rotation_duration),
  //     KITE_TIP_ROTATION_ADD(ID(one), 180, LEFT_TIP, rotation_duration)

  // );
  // tkbc_script_end();
  //////////////////////////////////////////////////////////////////////

  free(ki.elements);
}
