#include "kite_utils.h"
#include "tkbc.h"

#include <assert.h>
#include <math.h>
#include <raylib.h>

void kite_script_team_grid(Env *env, Kite_Indexs kite_index_array, size_t rows,
                           size_t columns, size_t v_padding, size_t h_padding,
                           float duration) {

  int w = GetScreenWidth();
  int h = GetScreenHeight();

  assert(env->kite_array->count > 0 && "No kites in the kite array!");
  Kite *kite = env->kite_array->elements[0].kite;
  float full_kite_width = kite->width + kite->spread;
  float full_kite_height = kite->height;

  float x_space = h_padding + full_kite_width;
  float y_space = v_padding + full_kite_height;

  Vector2 anchor = {.x = w / 2.0 - ((columns / 2.0) * x_space - x_space / 2),
                    .y = h / 2.0 - ((rows / 2.0) * y_space - y_space / 2)};

  Frames frames = {0};
  size_t i = 0;
  for (size_t column = 0; column < columns; ++column) {
    for (size_t row = 0; row < rows; ++row) {
      if (kite_index_array.count <= i) {
        break;
      }
      Frame *frame =
          kite_gen_frame(KITE_MOVE, kite_indexs_append(i++),
                         &(CLITERAL(Move_Action){
                             .position.x = anchor.x + x_space * column,
                             .position.y = anchor.y + y_space * row,
                         }),
                         duration);
      kite_dap(&frames, *frame);
    }
  }

  kite_register_frames_array(env, &frames);
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
