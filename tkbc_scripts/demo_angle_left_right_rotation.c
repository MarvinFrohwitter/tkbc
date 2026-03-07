#include "../src/choreographer/tkbc-script-api.h"
#include "../src/choreographer/tkbc-team-figures-api.h"

void angle_left_right_rotation(Env *env, Kite_Ids ki) {
  size_t h_padding = 0;
  Vector2 offset = {0};
  Vector2 position = {.x = env->window_width / 2.0,
                      .y = env->window_height / 2.0};
  float wait_time = 0.5;
  float move_duration = 1;
  float rotation_duration = 1;
  tkbc_script_begin("angle_left_right_rotation");
  SET(KITE_MOVE_ADD(ki, 0, -300, 5));
  // TODO: The sign of the -0 is optimized away by the compiler even with
  // in external kite scripts is works and the rotation direction is respected
  // in the implementation. This has to be investigated.
  SET(KITE_ROTATION(ki, -0, 2));

  tkbc_script_team_line(env, ki, position, offset, h_padding, move_duration);

  SET(KITE_WAIT(wait_time));
  SET(KITE_ROTATION(ki, -0, 2));
  SET(KITE_WAIT(wait_time));
  SET(KITE_ROTATION_ADD(ki, -90, 0));
  SET(KITE_WAIT(wait_time));
  SET(KITE_ROTATION_ADD(ki, -180, 0));
  SET(KITE_WAIT(wait_time));

  SET(KITE_MOVE_ADD(ki, 0, -300, 5));
  SET(KITE_WAIT(wait_time));

  SET(KITE_WAIT(wait_time));
  tkbc_script_team_split_box_up(env, ki, ODD, 300, move_duration,
                                rotation_duration);
  tkbc_script_team_split_box_up(env, ki, EVEN, 300, move_duration,
                                rotation_duration);

  SET(KITE_MOVE_ADD(ki, 0, -300, 9), KITE_WAIT(1.5), KITE_QUIT(7));

  SET(KITE_WAIT(wait_time));
  tkbc_script_end();
}
