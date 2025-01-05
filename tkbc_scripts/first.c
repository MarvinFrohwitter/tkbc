#include "../src/choreographer/tkbc-script-api.h"
#include "../src/choreographer/tkbc-team-figures-api.h"
#include "limits.h"

void rotation_checkup(Env *env, Kite_Ids ki) {

  float wait_time = 0.5;
  float rotation_duration = 1;

  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION_ADD, ki,
                               &(CLITERAL(Rotation_Add_Action){.angle = -90}),
                               rotation_duration));

  tkbc_register_frames(env, tkbc_script_wait(wait_time));

  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION, ki,
                               &(CLITERAL(Rotation_Action){.angle = -180}),
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

  // Kite_Indexs ki = tkbc_indexs_append(0, 1, 2, 3, 4, 5, 6, 7, 8);
  // Kite_Indexs ki = tkbc_indexs_append(0, 1, 2);

  Kite kite = *env->vanilla_kite;
  size_t h_padding = 0;
  size_t v_padding = 0;
  Vector2 offset = {0};
  Vector2 position = {.x = env->window_width / 2.0,
                      .y = env->window_height / 2.0};
  float move_duration = 1;
  float wait_time = 0.5;
  float rotation_duration = 1;
  // float ball_radius = (kite->width + kite->spread);

  Kite_Ids ki = tkbc_kite_array_generate(env, 2);
  // To set the setup to false.
  // return;

  int space = 100;
  tkbc_script_begin(env);
  tkbc_register_frames(
      env,
      tkbc_frame_generate(
          KITE_MOVE, tkbc_indexs_append(0),
          &(CLITERAL(Move_Action){
              .position.x = env->window_width / 2.0 - kite.width - space,
              .position.y = env->window_height - kite.height - space,
          }),
          0),
      tkbc_frame_generate(
          KITE_MOVE, tkbc_indexs_append(1),
          &(CLITERAL(Move_Action){
              .position.x = env->window_width / 2.0 + kite.width + space,
              .position.y = env->window_height - kite.height - space,
          }),
          0),
      tkbc_frame_generate(KITE_ROTATION, ki,
                          &(CLITERAL(Rotation_Action){.angle = 0}), 0));
  tkbc_register_frames(env, tkbc_script_wait(wait_time));
  tkbc_register_frames(
      env,
      tkbc_frame_generate(
          KITE_MOVE_ADD, tkbc_indexs_append(0),
          &(CLITERAL(Move_Add_Action){
              .position.x = kite.width + 2 * space + kite.width / 2.0,
              .position.y = -kite.width - space - space,
          }),
          move_duration),
      tkbc_frame_generate(KITE_ROTATION_ADD, tkbc_indexs_append(0),
                          &(CLITERAL(Rotation_Add_Action){.angle = -270}),
                          rotation_duration));

  tkbc_register_frames(
      env,
      tkbc_frame_generate(
          KITE_MOVE_ADD, tkbc_indexs_append(1),
          &(CLITERAL(Move_Add_Action){
              .position.x = -kite.width - 2 * space - kite.width / 2.0,
              .position.y = -kite.width - space - space,
          }),
          move_duration),
      tkbc_frame_generate(KITE_ROTATION_ADD, tkbc_indexs_append(1),
                          &(CLITERAL(Rotation_Add_Action){.angle = 270}),
                          rotation_duration));
  tkbc_register_frames(
      env,
      tkbc_frame_generate(
          KITE_MOVE, tkbc_indexs_append(1),
          &(CLITERAL(Move_Action){
              .position.x = env->window_width / 2.0 + kite.width + space,
              .position.y = env->window_height - kite.height - space,
          }),
          move_duration),
      tkbc_frame_generate(KITE_ROTATION_ADD, tkbc_indexs_append(1),
                          &(CLITERAL(Rotation_Add_Action){.angle = -315}),
                          rotation_duration));
  tkbc_register_frames(
      env,
      tkbc_frame_generate(
          KITE_MOVE, tkbc_indexs_append(0),
          &(CLITERAL(Move_Action){
              .position.x = env->window_width / 2.0 - kite.width - space,
              .position.y = env->window_height - kite.height - space,
          }),
          move_duration),
      tkbc_frame_generate(KITE_ROTATION_ADD, tkbc_indexs_append(0),
                          &(CLITERAL(Rotation_Add_Action){.angle = 315}),
                          rotation_duration));

  tkbc_script_end(env);

  tkbc_script_begin(env);
  tkbc_register_frames(env, tkbc_frame_generate(KITE_MOVE_ADD, ki,
                                                &(CLITERAL(Move_Add_Action){
                                                    .position.x = 0,
                                                    .position.y = -300,
                                                }),
                                                5));

  tkbc_script_team_line(env, ki, position, offset, h_padding, move_duration);
  tkbc_register_frames(env, tkbc_script_wait(1));
  // TODO: BROKEN 0 duration.
  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION, ki,
                               &(CLITERAL(Rotation_Action){.angle = 0}), 0));
  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION, ki,
                               &(CLITERAL(Rotation_Action){.angle = -90}), 1));
  tkbc_register_frames(env, tkbc_script_wait(4));

  // TODO: The sign of the -0 is optimized away by the compiler in external kite
  // scripts is works and the rotation direction is respected in the
  // implementation. This has to be investigated.
  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION, ki,
                               &(CLITERAL(Rotation_Action){.angle = -0}), 2));
  tkbc_register_frames(env, tkbc_script_wait(1));
  tkbc_script_team_split_box_up(env, ki, ODD, 300, move_duration,
                                rotation_duration);
  tkbc_script_team_split_box_up(env, ki, EVEN, 300, move_duration,
                                rotation_duration);

  tkbc_register_frames(env,
                       tkbc_frame_generate(KITE_MOVE_ADD, ki,
                                           &(CLITERAL(Move_Add_Action){
                                               .position.x = 0,
                                               .position.y = -300,
                                           }),
                                           9),
                       tkbc_script_wait(1.5), tkbc_script_frames_quit(7));

  tkbc_register_frames(env, tkbc_script_wait(wait_time));
  tkbc_script_end(env);

  tkbc_script_begin(env);
  tkbc_register_frames(env,
                       tkbc_frame_generate(KITE_MOVE_ADD, ki,
                                           &(CLITERAL(Move_Add_Action){
                                               .position.x = 0,
                                               .position.y = -300,
                                           }),
                                           5),
                       tkbc_script_wait(1.5), tkbc_script_frames_quit(7));

  tkbc_script_team_line(env, ki, position, offset, h_padding, move_duration);
  tkbc_register_frames(env, tkbc_script_wait(wait_time));
  rotation_checkup(env, ki);
  tkbc_script_end(env);

  tkbc_script_begin(env);

  tkbc_register_frames(env,
                       tkbc_frame_generate(KITE_MOVE_ADD, ki,
                                           &(CLITERAL(Move_Add_Action){
                                               .position.x = 0,
                                               .position.y = -300,
                                           }),
                                           5),
                       tkbc_script_wait(1.5), tkbc_script_frames_quit(7));

  tkbc_script_team_line(env, ki, position, offset, h_padding, move_duration);
  tkbc_register_frames(env, tkbc_script_wait(wait_time));
  // tkbc_script_team_ball(env, ki, position, offset, ball_radius,
  // move_duration);

  tkbc_register_frames(env, tkbc_script_wait(wait_time));

  tkbc_script_team_line(env, ki, position, offset, h_padding, move_duration);
  tkbc_register_frames(env, tkbc_script_wait(wait_time));

  tkbc_script_team_box_left(env, ki, 300, move_duration, rotation_duration);
  tkbc_script_team_box_right(env, ki, 300, move_duration, rotation_duration);
  tkbc_register_frames(env, tkbc_script_wait(1));
  tkbc_script_team_dimond_left(env, ki, 300, move_duration, rotation_duration);
  tkbc_script_team_dimond_right(env, ki, 300, move_duration, rotation_duration);

  tkbc_register_frames(env, tkbc_script_wait(1));
  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION_ADD, ki,
                               &(CLITERAL(Rotation_Action){.angle = -90}), 3));

  tkbc_register_frames(env, tkbc_script_wait(1));

  tkbc_script_team_split_box_up(env, ki, ODD, 300, move_duration,
                                rotation_duration);
  tkbc_script_team_split_box_up(env, ki, EVEN, 300, move_duration,
                                rotation_duration);

  tkbc_register_frames(env, tkbc_script_wait(1));

  tkbc_script_team_mountain(env, ki, position, offset, v_padding, h_padding,
                            move_duration, rotation_duration);
  tkbc_register_frames(env, tkbc_script_wait(1));
  tkbc_script_team_arc(env, ki, position, offset, v_padding, h_padding, 45,
                       move_duration, rotation_duration);
  tkbc_register_frames(env, tkbc_script_wait(1));
  tkbc_script_team_mountain(env, ki, position, offset, v_padding, h_padding,
                            move_duration, rotation_duration);
  tkbc_register_frames(env, tkbc_script_wait(1));
  tkbc_script_team_line(env, ki, position, offset, h_padding, move_duration);
  tkbc_register_frames(env, tkbc_script_wait(1));
  tkbc_script_team_valley(env, ki, position, offset, v_padding, h_padding,
                          move_duration, rotation_duration);
  tkbc_register_frames(env, tkbc_script_wait(1));
  tkbc_script_team_mouth(env, ki, position, offset, v_padding, h_padding, 45,
                         move_duration, rotation_duration);
  tkbc_register_frames(env, tkbc_script_wait(1));
  tkbc_script_team_valley(env, ki, position, offset, v_padding, h_padding,
                          move_duration, rotation_duration);
  tkbc_register_frames(env, tkbc_script_wait(1));

  tkbc_script_team_line(env, ki, position, offset, h_padding, move_duration);
  tkbc_register_frames(env, tkbc_script_wait(1));
  tkbc_script_team_grid(env, ki, position, offset, v_padding, h_padding, 3, 3,
                        move_duration);
  tkbc_register_frames(env, tkbc_script_wait(1));
  tkbc_script_team_line(env, ki, position, offset, h_padding, move_duration);
  tkbc_register_frames(env, tkbc_script_wait(1));
  tkbc_script_team_grid(env, ki, position, offset, v_padding, h_padding, 3, 3,
                        move_duration);

  tkbc_register_frames(env, tkbc_script_wait(1));

  tkbc_script_end(env);
}
