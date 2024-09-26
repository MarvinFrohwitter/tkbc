#include "tkbc-script-api.h"
#include "tkbc-team-figures-api.h"

void rotation_checkup(Env *env, Kite_Indexs ki) {

  float wait_time = 0.5;
  float rotation_duration = 3;

  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION_ADD, ki,
                               &(CLITERAL(Rotation_Add_Action){.angle = -90}),
                               rotation_duration));

  tkbc_register_frames(env, tkbc_script_wait(wait_time));

  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION, ki,
                               &(CLITERAL(Rotation_Action){.angle = -180}),
                               rotation_duration));

  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION, ki,
                               &(CLITERAL(Rotation_Action){.angle = 0}),
                               rotation_duration));

  tkbc_register_frames(env, tkbc_script_wait(wait_time));

  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION_ADD, ki,
                               &(CLITERAL(Rotation_Add_Action){.angle = 45}),
                               rotation_duration));

  tkbc_register_frames(env, tkbc_script_wait(wait_time));

  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION_ADD, ki,
                               &(CLITERAL(Rotation_Add_Action){.angle = 45}),
                               rotation_duration));

  tkbc_register_frames(env, tkbc_script_wait(wait_time));

  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION_ADD, ki,
                               &(CLITERAL(Rotation_Add_Action){.angle = 45}),
                               rotation_duration));

  tkbc_register_frames(env, tkbc_script_wait(wait_time));

  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION_ADD, ki,
                               &(CLITERAL(Rotation_Add_Action){.angle = -270}),
                               rotation_duration));

  tkbc_register_frames(env, tkbc_script_wait(wait_time));

  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION_ADD, ki,
                               &(CLITERAL(Rotation_Add_Action){.angle = -45}),
                               rotation_duration));

  tkbc_register_frames(env, tkbc_script_wait(wait_time));

  tkbc_register_frames(env,
                       tkbc_frame_generate(KITE_TIP_ROTATION_ADD, ki,
                                           &(CLITERAL(Tip_Rotation_Add_Action){
                                               .angle = 90, .tip = LEFT_TIP}),
                                           rotation_duration));

  tkbc_register_frames(env, tkbc_script_wait(wait_time));

  tkbc_register_frames(
      env, tkbc_frame_generate(
               KITE_TIP_ROTATION, ki,
               &(CLITERAL(Tip_Rotation_Action){.angle = 45, .tip = RIGHT_TIP}),
               rotation_duration));

  tkbc_register_frames(env, tkbc_script_wait(wait_time));

  tkbc_register_frames(
      env, tkbc_frame_generate(
               KITE_TIP_ROTATION, ki,
               &(CLITERAL(Tip_Rotation_Action){.angle = 180, .tip = RIGHT_TIP}),
               rotation_duration));

  tkbc_register_frames(env, tkbc_script_wait(wait_time));
}

void tkbc_script_input(Env *env) {

  tkbc_script_begin(env);

  // Kite_Indexs ki = tkbc_indexs_append(0, 1, 2, 3, 4, 5, 6, 7, 8);
  // Kite_Indexs ki = tkbc_indexs_append(0, 1, 2);
  Kite_Indexs ki = tkbc_indexs_generate(9);
  size_t h_padding = 0;
  size_t v_padding = 0;
  Vector2 offset = {0};
  // Vector2 position = {.x = env->window_width / 2.0,
  //                     .y = env->window_height / 2.0};
  float duration = 2;
  float wait_time = 0.5;
  // float rotation_duration = 1;
  // Kite *kite = env->kite_array->elements[0].kite;
  // float ball_radius = (kite->width + kite->spread);

  tkbc_register_frames(env,
                       tkbc_frame_generate(KITE_MOVE_ADD, ki,
                                           &(CLITERAL(Move_Add_Action){
                                               .position.x = 0,
                                               .position.y = -300,
                                           }),
                                           5),
                       tkbc_script_wait(1.5), tkbc_script_frames_quit(7));

  tkbc_script_team_line(env, ki, h_padding, offset, duration);
  tkbc_register_frames(env, tkbc_script_wait(wait_time));


  tkbc_register_frames(env, tkbc_script_wait(wait_time));

  rotation_checkup(env, ki);

  // tkbc_script_team_ball(env, ki, position, offset, ball_radius, duration);
  // tkbc_register_frames(
  //     env, tkbc_frame_generate(KITE_ROTATION_ADD, ki,
  //                              &(CLITERAL(Rotation_Action){.angle = -90}),
  //                              0));

  // tkbc_register_frames(env, tkbc_script_wait(1));

  // tkbc_script_team_split_box_up(env, ki, ODD, 300, 10, 5);

  // tkbc_script_team_split_box_up(env, ki, EVEN, 300, 10, 5);

  // tkbc_register_frames(env, tkbc_script_wait(1));

  tkbc_script_team_mountain(env, ki, v_padding, h_padding, offset, duration);
  tkbc_register_frames(env, tkbc_script_wait(1));
  tkbc_script_team_arc(env, ki, v_padding, h_padding, offset, 45, duration);
  tkbc_register_frames(env, tkbc_script_wait(1));
  tkbc_script_team_mountain(env, ki, v_padding, h_padding, offset, duration);
  tkbc_register_frames(env, tkbc_script_wait(1));
  tkbc_script_team_line(env, ki, h_padding, offset, duration);
  tkbc_register_frames(env, tkbc_script_wait(1));
  tkbc_script_team_valley(env, ki, v_padding, h_padding, offset, duration);
  tkbc_register_frames(env, tkbc_script_wait(1));
  tkbc_script_team_mouth(env, ki, v_padding, h_padding, offset, 45, duration);
  tkbc_register_frames(env, tkbc_script_wait(1));
  tkbc_script_team_valley(env, ki, v_padding, h_padding, offset, duration);
  tkbc_register_frames(env, tkbc_script_wait(1));

  tkbc_script_team_line(env, ki, h_padding, offset, duration);
  tkbc_register_frames(env, tkbc_script_wait(1));
  tkbc_script_team_grid(env, ki, 3, 3, v_padding, h_padding, offset, duration);
  tkbc_register_frames(env, tkbc_script_wait(1));
  tkbc_script_team_line(env, ki, h_padding, offset, duration);
  tkbc_register_frames(env, tkbc_script_wait(1));
  tkbc_script_team_grid(env, ki, 3, 3, v_padding, h_padding, offset, duration);

  tkbc_register_frames(env, tkbc_script_wait(1));
  // tkbc_script_team_box_left(env, ki, 300, duration);
  // tkbc_script_team_box_right(env, ki, 300, duration);
  // tkbc_register_frames(env, tkbc_script_wait(1));
  // tkbc_script_team_dimond_left(env, ki, 300, duration);
  // tkbc_script_team_dimond_right(env, ki, 300, duration);

  tkbc_script_end(env);
}
