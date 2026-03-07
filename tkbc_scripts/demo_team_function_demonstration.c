#include "../src/choreographer/tkbc-script-api.h"
#include "../src/choreographer/tkbc-team-figures-api.h"

void team_function_demonstration(Env *env, Kite_Ids ki) {
  size_t h_padding = 0;
  size_t v_padding = 0;
  Vector2 offset = {0};
  Vector2 position = {.x = env->window_width / 2.0,
                      .y = env->window_height / 2.0};
  float wait_time = 0.5;
  float move_duration = 1;
  float rotation_duration = 1;

  tkbc_script_begin("team function demonstration");
  SET(KITE_MOVE_ADD(ki, 0, 300, 5), KITE_WAIT(1.5), KITE_QUIT(7));

  tkbc_script_team_line(env, ki, position, offset, h_padding, move_duration);
  tkbc_register_frames(env, tkbc_script_wait(wait_time));
  // tkbc_script_team_ball(env, ki, position, offset, ball_radius,
  // move_duration);

  tkbc_register_frames(env, tkbc_script_wait(wait_time));

  tkbc_script_team_line(env, ki, position, offset, h_padding, move_duration);
  tkbc_register_frames(env, tkbc_script_wait(wait_time));

  tkbc_script_team_box_left(env, ki, 300, move_duration, rotation_duration);
  tkbc_script_team_box_right(env, ki, 300, move_duration, rotation_duration);
  SET(KITE_WAIT(1));
  tkbc_script_team_diamond_left(env, ki, 300, move_duration, rotation_duration);
  tkbc_script_team_diamond_right(env, ki, 300, move_duration,
                                 rotation_duration);

  SET(KITE_WAIT(1));
  SET(KITE_ROTATION_ADD(ki, -90, 3));

  SET(KITE_WAIT(1));

  tkbc_script_team_split_box_up(env, ki, ODD, 300, move_duration,
                                rotation_duration);
  tkbc_script_team_split_box_up(env, ki, EVEN, 300, move_duration,
                                rotation_duration);

  SET(KITE_WAIT(1));

  tkbc_script_team_mountain(env, ki, position, offset, v_padding, h_padding,
                            move_duration, rotation_duration);
  SET(KITE_WAIT(1));
  tkbc_script_team_arc(env, ki, position, offset, v_padding, h_padding, 45,
                       move_duration, rotation_duration);
  SET(KITE_WAIT(1));
  tkbc_script_team_mountain(env, ki, position, offset, v_padding, h_padding,
                            move_duration, rotation_duration);
  SET(KITE_WAIT(1));
  tkbc_script_team_line(env, ki, position, offset, h_padding, move_duration);
  SET(KITE_WAIT(1));
  tkbc_script_team_valley(env, ki, position, offset, v_padding, h_padding,
                          move_duration, rotation_duration);
  SET(KITE_WAIT(1));
  tkbc_script_team_mouth(env, ki, position, offset, v_padding, h_padding, 45,
                         move_duration, rotation_duration);
  SET(KITE_WAIT(1));
  tkbc_script_team_valley(env, ki, position, offset, v_padding, h_padding,
                          move_duration, rotation_duration);
  SET(KITE_WAIT(1));

  tkbc_script_team_line(env, ki, position, offset, h_padding, move_duration);
  SET(KITE_WAIT(1));
  tkbc_script_team_grid(env, ki, position, offset, v_padding, h_padding, 3, 3,
                        move_duration);
  SET(KITE_WAIT(1));
  tkbc_script_team_line(env, ki, position, offset, h_padding, move_duration);
  SET(KITE_WAIT(1));
  tkbc_script_team_grid(env, ki, position, offset, v_padding, h_padding, 3, 3,
                        move_duration);
  SET(KITE_WAIT(1));

  tkbc_script_end();
}
