#include "../src/choreographer/tkbc-script-api.h"
#include "../src/choreographer/tkbc-team-figures-api.h"

void rotation_checkup(Env *env, Kite_Ids ki) {

  float wait_time = 0.5;
  float rotation_duration = 3;

  SET(KITE_ROTATION_ADD(ki, -90, rotation_duration));

  SET(KITE_WAIT(wait_time));

  SET(KITE_ROTATION(ki, -180, rotation_duration));

  SET(KITE_WAIT(wait_time));

  SET(KITE_ROTATION_ADD(ki, 45, rotation_duration));

  SET(KITE_WAIT(wait_time));

  SET(KITE_ROTATION_ADD(ki, 45, rotation_duration));

  SET(KITE_WAIT(wait_time));

  SET(KITE_ROTATION_ADD(ki, 45, rotation_duration));

  SET(KITE_WAIT(wait_time));

  SET(KITE_ROTATION_ADD(ki, -270, rotation_duration));

  SET(KITE_WAIT(wait_time));

  SET(KITE_ROTATION_ADD(ki, -45, rotation_duration));

  SET(KITE_WAIT(wait_time));

  SET(KITE_TIP_ROTATION_ADD(ki, 90, LEFT_TIP, rotation_duration));

  SET(KITE_WAIT(wait_time));

  SET(KITE_TIP_ROTATION(ki, 45, RIGHT_TIP, rotation_duration));

  SET(KITE_WAIT(wait_time));

  SET(KITE_TIP_ROTATION(ki, 180, RIGHT_TIP, rotation_duration));

  SET(KITE_WAIT(wait_time));
}

void rotation_checkup_call(Env *env, Kite_Ids ki) {
  size_t h_padding = 0;
  Vector2 offset = {0};
  Vector2 position = {.x = env->window_width / 2.0,
                      .y = env->window_height / 2.0};
  float wait_time = 0.5;
  float move_duration = 1;

  tkbc_script_begin("rotation_checkup");
  SET(KITE_MOVE_ADD(ki, 0, -300, 5), KITE_WAIT(1.5), KITE_QUIT(7));

  tkbc_script_team_line(env, ki, position, offset, h_padding, move_duration);
  tkbc_register_frames(env, tkbc_script_wait(wait_time));
  rotation_checkup(env, ki);
  tkbc_script_end();
}
