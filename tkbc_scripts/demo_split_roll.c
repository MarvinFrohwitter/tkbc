#include "../src/choreographer/tkbc-script-api.h"
#include "../src/choreographer/tkbc-team-figures-api.h"

extern Id zero;
extern Id one;

void split_roll_demonstration(Env *env, Kite_Ids ki) {
  float rotation_duration = 1;

  tkbc_script_begin("split roll demonstration");
  SET(

      KITE_MOVE_ADD(ID(zero), -100, -300, rotation_duration),
      KITE_MOVE_ADD(ID(one), 100, -300, rotation_duration),

      KITE_ROTATION_ADD(ID(zero), -90, rotation_duration),
      KITE_ROTATION_ADD(ID(one), 90, rotation_duration)

  );

  SET(KITE_WAIT(1));

  tkbc_script_team_roll_split_up(env, ki, ODD, 2, 0, 360, 1);

  SET(

      KITE_ROTATION_ADD(ID(zero), -180, rotation_duration),
      KITE_ROTATION_ADD(ID(one), 180, rotation_duration)

  );

  SET(KITE_WAIT(1));

  tkbc_script_team_roll_split_up(env, ki, EVEN, 2, 0, 360, 1);

  // tkbc_script_team_roll_up_anti_clockwise(env, ID(zero), 1.3, 0, 180, 1);
  // tkbc_script_team_roll_down_clockwise(env, ID(zero), 2, 0, 180, 1);

  // tkbc_script_team_roll_down_anti_clockwise(env, ID(one), 1, 0, 360, 1);
  // tkbc_script_team_roll_up_clockwise(env, ID(one), 1, 0, 360, 1);
  tkbc_script_end();
}
