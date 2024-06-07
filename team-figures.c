#include "tkbc.h"
#include <assert.h>

void kite_script_team_grid(Env *env, Kite_Indexs kite_index_array) {
  assert(0 && "UNIMPLEMENTED");
}
void kite_script_team_box_left(Env *env, Kite_Indexs kite_index_array,
                                float box_size, float duration) {

  kite_register_frames(env, 1,
                       kite_gen_frame(KITE_MOVE_ADD, kite_index_array,
                                      &(CLITERAL(Move_Add_Action){
                                          .position.x = 0,
                                          .position.y = -box_size,
                                      }),
                                      duration));
  kite_register_frames(env, 1,
                       kite_gen_frame(KITE_ROTATION, kite_index_array,
                                      &(CLITERAL(Rotation_Action){.angle = 90}),
                                      duration));
  kite_register_frames(env, 1,
                       kite_gen_frame(KITE_MOVE_ADD, kite_index_array,
                                      &(CLITERAL(Move_Add_Action){
                                          .position.x = -box_size,
                                          .position.y = 0,
                                      }),
                                      duration));
  kite_register_frames(env, 1,
                       kite_gen_frame(KITE_ROTATION, kite_index_array,
                                      &(CLITERAL(Rotation_Action){.angle = 90}),
                                      duration));
  kite_register_frames(env, 1,
                       kite_gen_frame(KITE_MOVE_ADD, kite_index_array,
                                      &(CLITERAL(Move_Add_Action){
                                          .position.x = 0,
                                          .position.y = box_size,
                                      }),
                                      duration));
  kite_register_frames(env, 1,
                       kite_gen_frame(KITE_ROTATION, kite_index_array,
                                      &(CLITERAL(Rotation_Action){.angle = 90}),
                                      duration));

  kite_register_frames(env, 1,
                       kite_gen_frame(KITE_MOVE_ADD, kite_index_array,
                                      &(CLITERAL(Move_Add_Action){
                                          .position.x = box_size,
                                          .position.y = 0,
                                      }),
                                      duration));
  kite_register_frames(env, 1,
                       kite_gen_frame(KITE_ROTATION, kite_index_array,
                                      &(CLITERAL(Rotation_Action){.angle = 90}),
                                      duration));
}

void kite_script_team_box_right(Env *env, Kite_Indexs kite_index_array,
                                float box_size, float duration) {

  kite_register_frames(env, 1,
                       kite_gen_frame(KITE_MOVE_ADD, kite_index_array,
                                      &(CLITERAL(Move_Add_Action){
                                          .position.x = 0,
                                          .position.y = -box_size,
                                      }),
                                      duration));
  kite_register_frames(env, 1,
                       kite_gen_frame(KITE_ROTATION, kite_index_array,
                                      &(CLITERAL(Rotation_Action){.angle = -90}),
                                      duration));
  kite_register_frames(env, 1,
                       kite_gen_frame(KITE_MOVE_ADD, kite_index_array,
                                      &(CLITERAL(Move_Add_Action){
                                          .position.x = box_size,
                                          .position.y = 0,
                                      }),
                                      duration));
  kite_register_frames(env, 1,
                       kite_gen_frame(KITE_ROTATION, kite_index_array,
                                      &(CLITERAL(Rotation_Action){.angle = -90}),
                                      duration));
  kite_register_frames(env, 1,
                       kite_gen_frame(KITE_MOVE_ADD, kite_index_array,
                                      &(CLITERAL(Move_Add_Action){
                                          .position.x = 0,
                                          .position.y = box_size,
                                      }),
                                      duration));
  kite_register_frames(env, 1,
                       kite_gen_frame(KITE_ROTATION, kite_index_array,
                                      &(CLITERAL(Rotation_Action){.angle = -90}),
                                      duration));

  kite_register_frames(env, 1,
                       kite_gen_frame(KITE_MOVE_ADD, kite_index_array,
                                      &(CLITERAL(Move_Add_Action){
                                          .position.x = -box_size,
                                          .position.y = 0,
                                      }),
                                      duration));
  kite_register_frames(env, 1,
                       kite_gen_frame(KITE_ROTATION, kite_index_array,
                                      &(CLITERAL(Rotation_Action){.angle = -90}),
                                      duration));
}
