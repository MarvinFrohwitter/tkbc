#include "../src/choreographer/tkbc-script-api.h"
#include "../src/choreographer/tkbc-team-figures-api.h"

void split_roll_demonstration(Env *env, Kite_Ids ki) {
  float rotation_duration = 1;

  tkbc_script_begin("split roll demonstration");
  SET(

      KITE_MOVE_ADD(ID(0), -100, -300, rotation_duration),
      KITE_MOVE_ADD(ID(1), 100, -300, rotation_duration),

      KITE_ROTATION_ADD(ID(0), -90, rotation_duration),
      KITE_ROTATION_ADD(ID(1), 90, rotation_duration)

  );

  tkbc_register_frames(env, tkbc_script_wait(1));

  tkbc_script_team_roll_split_up(env, ki, ODD, 2, 0, 360, 1);

  SET(

      KITE_ROTATION_ADD(ID(0), -180, rotation_duration),
      KITE_ROTATION_ADD(ID(1), 180, rotation_duration)

  );

  tkbc_register_frames(env, tkbc_script_wait(1));

  tkbc_script_team_roll_split_up(env, ki, EVEN, 2, 0, 360, 1);

  // tkbc_script_team_roll_up_anti_clockwise(env, ID(0), 1.3, 0, 180, 1);
  // tkbc_script_team_roll_down_clockwise(env, ID(0), 2, 0, 180, 1);

  // tkbc_script_team_roll_down_anti_clockwise(env, ID(1), 1, 0, 360, 1);
  // tkbc_script_team_roll_up_clockwise(env, ID(1), 1, 0, 360, 1);
  tkbc_script_end();
}
