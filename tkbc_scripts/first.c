#include "../src/choreographer/tkbc-script-api.h"
#include "../src/choreographer/tkbc-team-figures-api.h"
#include "limits.h"

void rotation_checkup(Env *env, Kite_Ids ki) {

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

void pair(Env *env, Kite_Ids ki) {
  Kite kite = *env->vanilla_kite;
  float wait_time = 0.5;
  float move_duration = 1;
  float rotation_duration = 1;
  int space = 100;

  tkbc_script_begin();
  COLLECTION(

      KITE_MOVE(ID(0), env->window_width / 2.0 - kite.width - space,
                env->window_height - kite.height - space, 0),
      KITE_MOVE(ID(1), env->window_width / 2.0 + kite.width + space,
                env->window_height - kite.height - space, 0),
      KITE_ROTATION(ki, 0, 0),

      // Just for documentation
      KITE_QUIT(wait_time)

  );
  SET(KITE_WAIT(wait_time));
  SET(

      KITE_MOVE_ADD(ID(0), kite.width + 2 * space + kite.width / 2.0,
                    -kite.width - space - space, move_duration),
      KITE_ROTATION_ADD(tkbc_indexs_append(0), -270, rotation_duration)

  );
  SET(

      KITE_MOVE_ADD(ID(1), -kite.width - 2 * space - kite.width / 2.0,
                    -kite.width - space - space, move_duration),
      KITE_ROTATION_ADD(tkbc_indexs_append(1), 270, rotation_duration)

  );
  SET(

      KITE_MOVE(ID(1), env->window_width / 2.0 + kite.width + space,
                env->window_height - kite.height - space, move_duration),
      KITE_ROTATION_ADD(tkbc_indexs_append(1), -315, rotation_duration)

  );
  SET(

      KITE_MOVE(ID(0), env->window_width / 2.0 - kite.width - space,
                env->window_height - kite.height - space, move_duration),
      KITE_ROTATION_ADD(tkbc_indexs_append(0), 315, rotation_duration)

  );

  tkbc_script_end();
}

// The env of type Env is passed automatically into the scope of the
// script_input it is not globally available.
tkbc_script_input {
  size_t h_padding = 0;
  size_t v_padding = 0;
  Vector2 offset = {0};
  Vector2 position = {.x = env->window_width / 2.0,
                      .y = env->window_height / 2.0};
  float wait_time = 0.5;
  float move_duration = 1;
  float rotation_duration = 1;
  // float ball_radius = (kite->width + kite->spread);
  Kite_Ids ki = tkbc_kite_array_generate(env, 2);

  pair(env, ki);

  tkbc_script_begin();
  tkbc_register_frames(env, tkbc_frame_generate(KITE_MOVE_ADD, ki,
                                                &(CLITERAL(Move_Add_Action){
                                                    .position.x = 0,
                                                    .position.y = -300,
                                                }),
                                                5));
  // TODO: The sign of the -0 is optimized away by the compiler even with
  // in external kite scripts is works and the rotation direction is respected
  // in the implementation. This has to be investigated.
  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION, ki,
                               &(CLITERAL(Rotation_Action){.angle = -0}), 2));

  tkbc_script_team_line(env, ki, position, offset, h_padding, move_duration);
  tkbc_register_frames(env, tkbc_script_wait(1));
  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION, ki,
                               &(CLITERAL(Rotation_Action){.angle = -0}), 2));
  tkbc_register_frames(env, tkbc_script_wait(1));
  tkbc_register_frames(
      env,
      tkbc_frame_generate(KITE_ROTATION_ADD, ki,
                          &(CLITERAL(Rotation_Add_Action){.angle = -90}), 0));
  tkbc_register_frames(env, tkbc_script_wait(1));
  tkbc_register_frames(
      env,
      tkbc_frame_generate(KITE_ROTATION_ADD, ki,
                          &(CLITERAL(Rotation_Add_Action){.angle = -180}), 0));
  tkbc_register_frames(env, tkbc_script_wait(1));

  tkbc_register_frames(env, tkbc_frame_generate(KITE_MOVE_ADD, ki,
                                                &(CLITERAL(Move_Add_Action){
                                                    .position.x = 0,
                                                    .position.y = -300,
                                                }),
                                                5));
  tkbc_register_frames(env, tkbc_script_wait(1));

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
  tkbc_script_end();

  tkbc_script_begin();
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
  tkbc_script_end();

  tkbc_script_begin();
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
  tkbc_script_team_diamond_left(env, ki, 300, move_duration, rotation_duration);
  tkbc_script_team_diamond_right(env, ki, 300, move_duration,
                                 rotation_duration);

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

  tkbc_script_end();
}
