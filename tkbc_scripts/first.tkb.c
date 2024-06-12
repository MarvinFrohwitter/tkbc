#include "../tkbc.h"
#include <raylib.h>

void kite_script_input(Env *env) {

  // TODO: Abstract the register block index away by dooing a macro and than
  // resetting the block index at the script end function

  kite_script_begin(env);

  Kite_Indexs ki = kite_indexs_append(0, 1, 2, 3, 4, 5, 6, 7, 8);
  size_t h_padding = 0;
  size_t v_padding = 0;

  kite_register_frames(env, 3,
                       kite_gen_frame(KITE_MOVE_ADD, ki,
                                      &(CLITERAL(Move_Add_Action){
                                          .position.x = 0,
                                          .position.y = -300,
                                      }),
                                      5),
                       kite_script_wait(1.5), kite_script_frames_quit(7));

  kite_script_team_grid(env, ki, 3, 3,v_padding, h_padding, 10);
  kite_script_team_line(env, ki, h_padding, 10);

  // kite_script_team_box_left(env, kite_indexs_append(0, 1, 2, 3), 300, 2);
  // kite_script_team_box_right(env, kite_indexs_append(0, 1, 2, 3), 300,
  // 2);

  // kite_script_team_dimond_left(env, kite_indexs_append(0, 1, 2, 3), 300,
  // 2); kite_script_team_dimond_right(env, kite_indexs_append(0, 1, 2, 3),
  // 300, 2);

  // kite_register_frames(
  //     env, 6,
  //     kite_gen_frame(KITE_ROTATION, kite_indexs_append(0, 1),
  //                    &(CLITERAL(Rotation_Action){.angle = 90}), 5),
  //     kite_gen_frame(KITE_ROTATION, kite_indexs_append(1),
  //                    &(CLITERAL(Rotation_Action){.angle = 90}), 5),
  //     kite_gen_frame(
  //         KITE_TIP_ROTATION, kite_indexs_append(3),
  //         &(CLITERAL(Tip_Rotation_Action){.angle = 90, .tip = LEFT_TIP}),
  //         5),
  //     kite_gen_frame(
  //         KITE_TIP_ROTATION, kite_indexs_append(3),
  //         &(CLITERAL(Tip_Rotation_Action){.angle = 45, .tip = LEFT_TIP}),
  //         2),

  //     kite_gen_frame(KITE_MOVE, kite_indexs_append(0),
  //                    &(CLITERAL(Move_Action){
  //                        .position.x = 100,
  //                        .position.y = 100,
  //                    }),
  //                    12),
  //     kite_script_wait(3), kite_script_frames_quit(7));

  // kite_register_frames(env, 1, kite_script_wait(3));

  // kite_register_frames(env, 2,
  //                      kite_gen_frame(KITE_ROTATION,
  //                      kite_indexs_append(3),
  //                                     &(CLITERAL(Rotation_Action){.angle
  //                                     = 40}), 0),
  //                      kite_gen_frame(KITE_MOVE, kite_indexs_append(0),
  //                                     &(CLITERAL(Move_Action){
  //                                         .position.x = 300,
  //                                         .position.y = 500,
  //                                     }),
  //                                     0));

  kite_script_end(env);
}
