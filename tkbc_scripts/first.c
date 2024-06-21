#include "tkbc-script-api.h"
#include "tkbc-team-figures-api.h"

#include <raylib.h>
#include <raymath.h>


void tkbc_script_input(Env *env) {

  tkbc_script_begin(env);

  // Kite_Indexs ki = tkbc_indexs_append(0, 1, 2, 3, 4, 5, 6, 7, 8);
  // Kite_Indexs ki = tkbc_indexs_append(0, 1, 2);
  Kite_Indexs ki = tkbc_indexs_generate(4);
  size_t h_padding = 0;
  size_t v_padding = 0;
  Vector2 offset = Vector2Zero();
  Vector2 position = {.x = GetScreenWidth() / 2.0,
                      .y = GetScreenHeight() / 2.0};
  float duration = 6;
  Kite *kite = env->kite_array->elements[0].kite;
  float ball_radius = (kite->width + kite->spread);

  tkbc_register_frames(env,
                       tkbc_frame_generate(KITE_MOVE_ADD, ki,
                                           &(CLITERAL(Move_Add_Action){
                                               .position.x = 0,
                                               .position.y = -300,
                                           }),
                                           5),
                       tkbc_script_wait(1.5), tkbc_script_frames_quit(7));

  tkbc_script_team_line(env, ki, h_padding, offset, duration);

  tkbc_register_frames(env, tkbc_script_wait(1));

  tkbc_script_team_ball(env, ki, position, offset, ball_radius, duration);

  tkbc_register_frames(env, tkbc_script_wait(1));

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
  tkbc_script_team_box_left(env, ki, 300, duration);
  tkbc_script_team_box_right(env, ki, 300, duration);
  tkbc_register_frames(env, tkbc_script_wait(1));
  tkbc_script_team_dimond_left(env, ki, 300, duration);
  tkbc_script_team_dimond_right(env, ki, 300, duration);

  tkbc_script_end(env);
}
