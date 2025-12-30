#include "../src/choreographer/tkbc-script-api.h"
#include "../src/choreographer/tkbc-team-figures-api.h"

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

void rotation_checkup_call(Env *env, Kite_Ids ki) {
  size_t h_padding = 0;
  Vector2 offset = {0};
  Vector2 position = {.x = env->window_width / 2.0,
                      .y = env->window_height / 2.0};
  float wait_time = 0.5;
  float move_duration = 1;

  tkbc_script_begin("rotation_checkup");
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
}
