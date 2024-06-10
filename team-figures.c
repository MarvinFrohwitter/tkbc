#include "tkbc.h"
#include <assert.h>
#include <math.h>

void kite_script_team_grid(Env *env, Kite_Indexs kite_index_array) {
  assert(0 && "UNIMPLEMENTED");
}

void kite_script_team_box(Env *env, Kite_Indexs kite_index_array,
                          DIRECTION direction, float angle, float box_size,
                          float duration) {

  // TODO: Think about the starting point.
  if (direction == RIGHT) {
    angle = -angle;
  }

  kite_register_frames(env, 1,
                       kite_gen_frame(KITE_MOVE_ADD, kite_index_array,
                                      &(CLITERAL(Move_Add_Action){
                                          .position.x = 0,
                                          .position.y = -box_size / 2,
                                      }),
                                      duration));

  kite_register_frames(
      env, 1,
      kite_gen_frame(KITE_ROTATION, kite_index_array,
                     &(CLITERAL(Rotation_Action){.angle = angle}), duration));

  switch (direction) {
  case LEFT: {
    kite_register_frames(env, 1,
                         kite_gen_frame(KITE_MOVE_ADD, kite_index_array,
                                        &(CLITERAL(Move_Add_Action){
                                            .position.x = -box_size,
                                            .position.y = 0,
                                        }),
                                        duration));

  } break;
  case RIGHT: {
    kite_register_frames(env, 1,
                         kite_gen_frame(KITE_MOVE_ADD, kite_index_array,
                                        &(CLITERAL(Move_Add_Action){
                                            .position.x = box_size,
                                            .position.y = 0,
                                        }),
                                        duration));

  } break;
  default:
    assert(0 && "UNREACHABLE");
  }

  kite_register_frames(
      env, 1,
      kite_gen_frame(KITE_ROTATION, kite_index_array,
                     &(CLITERAL(Rotation_Action){.angle = angle}), duration));

  kite_register_frames(env, 1,
                       kite_gen_frame(KITE_MOVE_ADD, kite_index_array,
                                      &(CLITERAL(Move_Add_Action){
                                          .position.x = 0,
                                          .position.y = box_size,
                                      }),
                                      duration));

  kite_register_frames(
      env, 1,
      kite_gen_frame(KITE_ROTATION, kite_index_array,
                     &(CLITERAL(Rotation_Action){.angle = angle}), duration));

  switch (direction) {
  case LEFT: {

    kite_register_frames(env, 1,
                         kite_gen_frame(KITE_MOVE_ADD, kite_index_array,
                                        &(CLITERAL(Move_Add_Action){
                                            .position.x = box_size,
                                            .position.y = 0,
                                        }),
                                        duration));
  } break;
  case RIGHT: {
    kite_register_frames(env, 1,
                         kite_gen_frame(KITE_MOVE_ADD, kite_index_array,
                                        &(CLITERAL(Move_Add_Action){
                                            .position.x = -box_size,
                                            .position.y = 0,
                                        }),
                                        duration));
  } break;
  default:
    assert(0 && "UNREACHABLE");
  }

  kite_register_frames(
      env, 1,
      kite_gen_frame(KITE_ROTATION, kite_index_array,
                     &(CLITERAL(Rotation_Action){.angle = angle}), duration));

  kite_register_frames(env, 1,
                       kite_gen_frame(KITE_MOVE_ADD, kite_index_array,
                                      &(CLITERAL(Move_Add_Action){
                                          .position.x = 0,
                                          .position.y = -box_size / 2,
                                      }),
                                      duration));
}

void kite_script_team_box_left(Env *env, Kite_Indexs kite_index_array,
                               float box_size, float duration) {
  kite_script_team_box(env, kite_index_array, LEFT, 90, box_size, duration);
}

void kite_script_team_box_right(Env *env, Kite_Indexs kite_index_array,
                                float box_size, float duration) {
  kite_script_team_box(env, kite_index_array, RIGHT, 90, box_size, duration);
}

void kite_script_team_dimond(Env *env, Kite_Indexs kite_index_array,
                             DIRECTION direction, float angle, float box_size,
                             float duration) {

  if (direction == RIGHT) {
    angle = -angle;
  }

  kite_register_frames(
      env, 1,
      kite_gen_frame(KITE_ROTATION, kite_index_array,
                     &(CLITERAL(Rotation_Action){.angle = angle / 2}),
                     duration));

  switch (direction) {
  case RIGHT: {
    kite_register_frames(env, 1,
                         kite_gen_frame(KITE_MOVE_ADD, kite_index_array,
                                        &(CLITERAL(Move_Add_Action){
                                            .position.x = box_size / sqrt(2),
                                            .position.y = -box_size / sqrt(2),
                                        }),
                                        duration));

  } break;
  case LEFT: {
    kite_register_frames(env, 1,
                         kite_gen_frame(KITE_MOVE_ADD, kite_index_array,
                                        &(CLITERAL(Move_Add_Action){
                                            .position.x = -box_size / sqrt(2),
                                            .position.y = -box_size / sqrt(2),
                                        }),
                                        duration));

  } break;
  default:
    assert(0 && "UNREACHABLE");
  }

  kite_register_frames(
      env, 1,
      kite_gen_frame(KITE_ROTATION, kite_index_array,
                     &(CLITERAL(Rotation_Action){.angle = angle}), duration));

  switch (direction) {
  case RIGHT: {
    kite_register_frames(env, 1,
                         kite_gen_frame(KITE_MOVE_ADD, kite_index_array,
                                        &(CLITERAL(Move_Add_Action){
                                            .position.x = box_size / sqrt(2),
                                            .position.y = box_size / sqrt(2),
                                        }),
                                        duration));
  } break;
  case LEFT: {
    kite_register_frames(env, 1,
                         kite_gen_frame(KITE_MOVE_ADD, kite_index_array,
                                        &(CLITERAL(Move_Add_Action){
                                            .position.x = -box_size / sqrt(2),
                                            .position.y = box_size / sqrt(2),
                                        }),
                                        duration));
  } break;
  default:
    assert(0 && "UNREACHABLE");
  }

  kite_register_frames(
      env, 1,
      kite_gen_frame(KITE_ROTATION, kite_index_array,
                     &(CLITERAL(Rotation_Action){.angle = angle}), duration));

  switch (direction) {
  case RIGHT: {
    kite_register_frames(env, 1,
                         kite_gen_frame(KITE_MOVE_ADD, kite_index_array,
                                        &(CLITERAL(Move_Add_Action){
                                            .position.x = -box_size / sqrt(2),
                                            .position.y = box_size / sqrt(2),
                                        }),
                                        duration));
  } break;
  case LEFT: {
    kite_register_frames(env, 1,
                         kite_gen_frame(KITE_MOVE_ADD, kite_index_array,
                                        &(CLITERAL(Move_Add_Action){
                                            .position.x = box_size / sqrt(2),
                                            .position.y = box_size / sqrt(2),
                                        }),
                                        duration));
  } break;
  default:
    assert(0 && "UNREACHABLE");
  }
  kite_register_frames(
      env, 1,
      kite_gen_frame(KITE_ROTATION, kite_index_array,
                     &(CLITERAL(Rotation_Action){.angle = angle}), duration));

  switch (direction) {
  case RIGHT: {
    kite_register_frames(env, 1,
                         kite_gen_frame(KITE_MOVE_ADD, kite_index_array,
                                        &(CLITERAL(Move_Add_Action){
                                            .position.x = -box_size / sqrt(2),
                                            .position.y = -box_size / sqrt(2),
                                        }),
                                        duration));
  } break;
  case LEFT: {
    kite_register_frames(env, 1,
                         kite_gen_frame(KITE_MOVE_ADD, kite_index_array,
                                        &(CLITERAL(Move_Add_Action){
                                            .position.x = box_size / sqrt(2),
                                            .position.y = -box_size / sqrt(2),
                                        }),
                                        duration));
  } break;
  default:
    assert(0 && "UNREACHABLE");
  }

  kite_register_frames(
      env, 1,
      kite_gen_frame(KITE_ROTATION, kite_index_array,
                     &(CLITERAL(Rotation_Action){.angle = angle / 2}),
                     duration));
}

void kite_script_team_dimond_left(Env *env, Kite_Indexs kite_index_array,
                                  float box_size, float duration) {
  kite_script_team_dimond(env, kite_index_array, LEFT, 90, box_size, duration);
}

void kite_script_team_dimond_right(Env *env, Kite_Indexs kite_index_array,
                                   float box_size, float duration) {
  kite_script_team_dimond(env, kite_index_array, RIGHT, 90, box_size, duration);
}
