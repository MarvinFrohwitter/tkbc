#include "../tkbc.h"
#include <raylib.h>

void kite_script_input(Env *env) {

  // TODO: Abstract the register block index away by dooing a macro and than
  // resetting the block index at the script end function

  kite_script_begin(env);

  kite_register_frames(
      env, 1, 6,
      kite_gen_frame(KITE_ROTATION, kite_indexs_append(2, 0, 1),
                     &(CLITERAL(Rotation_Action){.angle = 90}), 5),
      kite_gen_frame(KITE_ROTATION, kite_indexs_append(1, 1),
                     &(CLITERAL(Rotation_Action){.angle = 90}), 5),
      kite_gen_frame(
          KITE_TIP_ROTATION, kite_indexs_append(1, 3),
          &(CLITERAL(Tip_Rotation_Action){.angle = 90, .tip = RIGHT_TIP}), 5),

      kite_gen_frame(KITE_MOVE, kite_indexs_append(1, 0),
                     &(CLITERAL(Move_Action){
                         .position.x = 100,
                         .position.y = 100,
                     }),
                     12),
      kite_script_wait(3), kite_script_frames_quit(7));

  kite_register_frames(env, 2, 1, kite_script_wait(3));

  kite_register_frames(env, 3, 2,
                       kite_gen_frame(KITE_ROTATION, kite_indexs_append(1, 3),
                                      &(CLITERAL(Rotation_Action){.angle = 40}),
                                      0),
                       kite_gen_frame(KITE_MOVE, kite_indexs_append(1, 0),
                                      &(CLITERAL(Move_Action){
                                          .position.x = 300,
                                          .position.y = 500,
                                      }),
                                      0));

  kite_script_end(env);
}
