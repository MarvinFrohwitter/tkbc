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
}
