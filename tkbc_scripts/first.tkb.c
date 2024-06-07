#include "../tkbc.h"
#include <raylib.h>

void kite_script_input(Env *env) {

  // TODO: Abstract the register block index away by dooing a macro and than
  // resetting the block index at the script end function

  kite_script_begin(env);

  kite_register_frames(env, 3,
                       kite_gen_frame(KITE_MOVE_ADD,
                                      kite_indexs_append(4, 0, 1, 2, 3),
                                      &(CLITERAL(Move_Add_Action){
                                          .position.x = 0,
                                          .position.y = -300,
                                      }),
                                      5),
                       kite_script_wait(1.5), kite_script_frames_quit(7));

  // TODO: The thing has not to be in grid call that separate for more
  // configuration option or give the function a parameter.
  // kite_script_team_grid(env, kite_index_array);

  kite_script_team_box_left(env, kite_indexs_append(4, 0, 1, 2, 3), 300, 2);
  kite_script_team_box_right(env, kite_indexs_append(4, 0, 1, 2, 3), 300, 2);

  // kite_register_frames(
  //     env, 6,
  //     kite_gen_frame(KITE_ROTATION, kite_indexs_append(2, 0, 1),
  //                    &(CLITERAL(Rotation_Action){.angle = 90}), 5),
  //     kite_gen_frame(KITE_ROTATION, kite_indexs_append(1, 1),
  //                    &(CLITERAL(Rotation_Action){.angle = 90}), 5),
  //     kite_gen_frame(
  //         KITE_TIP_ROTATION, kite_indexs_append(1, 3),
  //         &(CLITERAL(Tip_Rotation_Action){.angle = 90, .tip = LEFT_TIP}), 5),
  //     kite_gen_frame(
  //         KITE_TIP_ROTATION, kite_indexs_append(1, 3),
  //         &(CLITERAL(Tip_Rotation_Action){.angle = 45, .tip = LEFT_TIP}), 2),

  //     kite_gen_frame(KITE_MOVE, kite_indexs_append(1, 0),
  //                    &(CLITERAL(Move_Action){
  //                        .position.x = 100,
  //                        .position.y = 100,
  //                    }),
  //                    12),
  //     kite_script_wait(3), kite_script_frames_quit(7));

  // kite_register_frames(env, 1, kite_script_wait(3));

  // kite_register_frames(env, 2,
  //                      kite_gen_frame(KITE_ROTATION, kite_indexs_append(1,
  //                      3),
  //                                     &(CLITERAL(Rotation_Action){.angle =
  //                                     40}), 0),
  //                      kite_gen_frame(KITE_MOVE, kite_indexs_append(1, 0),
  //                                     &(CLITERAL(Move_Action){
  //                                         .position.x = 300,
  //                                         .position.y = 500,
  //                                     }),
  //                                     0));

  kite_script_end(env);
}
