#ifndef TKBC_TEAM_FIGURES_API_H_
#define TKBC_TEAM_FIGURES_API_H_

#include "tkbc-types.h"

// ===========================================================================
// ========================== Script Team Figures ============================
// ===========================================================================

void tkbc_script_team_line(Env *env, Kite_Indexs kite_index_array,
                           size_t h_padding, Vector2 offset, float duration);
bool tkbc_script_team_grid(Env *env, Kite_Indexs kite_index_array, size_t rows,
                           size_t columns, size_t v_padding, size_t h_padding,
                           Vector2 offset, float duration);

bool tkbc_script_team_ball(Env *env, Kite_Indexs kite_index_array,
                           Vector2 position, Vector2 offset, float radius,
                           float duration);

bool tkbc_script_team_mountain(Env *env, Kite_Indexs kite_index_array,
                               size_t v_padding, size_t h_padding,
                               Vector2 offset, float duration);
bool tkbc_script_team_valley(Env *env, Kite_Indexs kite_index_array,
                             size_t v_padding, size_t h_padding, Vector2 offset,
                             float duration);

bool tkbc_script_team_arc(Env *env, Kite_Indexs kite_index_array,
                          size_t v_padding, size_t h_padding, Vector2 offset,
                          float angle, float duration);
bool tkbc_script_team_mouth(Env *env, Kite_Indexs kite_index_array,
                            size_t v_padding, size_t h_padding, Vector2 offset,
                            float angle, float duration);

void tkbc_script_team_box(Env *env, Kite_Indexs kite_index_array,
                          DIRECTION direction, float angle, float box_size,
                          float duration);
void tkbc_script_team_box_left(Env *env, Kite_Indexs kite_index_array,
                               float box_size, float duration);
void tkbc_script_team_box_right(Env *env, Kite_Indexs kite_index_array,
                                float box_size, float duration);

void tkbc_script_team_dimond(Env *env, Kite_Indexs kite_index_array,
                             DIRECTION direction, float angle, float box_size,
                             float duration);
void tkbc_script_team_dimond_left(Env *env, Kite_Indexs kite_index_array,
                                  float box_size, float duration);
void tkbc_script_team_dimond_right(Env *env, Kite_Indexs kite_index_array,
                                   float box_size, float duration);

#endif // TKBC_TEAM_FIGURES_API_H_

// ===========================================================================

#ifdef TKBC_TEAM_FIGURES_API_IMPLEMENTATION

#include <assert.h>
#include <complex.h>
#include <math.h>
#include <raymath.h>

#ifndef TKBC_SCRIPT_API_IMPLEMENTATION
#define TKBC_SCRIPT_API_IMPLEMENTATION
#include "tkbc-script-api.h"
#endif // TKBC_SCRIPT_API_IMPLEMENTATION

// ========================== Script Team Figures ============================

bool tkbc_script_team_ball(Env *env, Kite_Indexs kite_index_array,
                           Vector2 position, Vector2 offset, float radius,
                           float duration) {

  position = Vector2Add(position, offset);
  Vector2 place = position;

  size_t segments = kite_index_array.count;
  float segment_size = 360.0 / segments;
  float deg_base_rotation = segments / 2.0 * segment_size;

  Frames frames = {0};
  for (size_t i = 0; i < segments; ++i) {

    place.x +=
        ceilf(crealf((radius)*cexpf(I * (PI * (deg_base_rotation) / 180))));
    place.y +=
        floorf(cimagf((radius)*cexpf(I * (PI * (deg_base_rotation) / 180))));

    // ------------|-----------------------------
    // --------v---|-----------------------------
    // -----------v|-----------------------------
    // ------------|v----------------------------
    // ------------|---v-------------------------
    // ------------|-----v-----------------------
    // ------------|-------v---------------------
    // ------------|---------v-------------------
    // ------------|---------XV------------------
    // ------------|-------M-x--V----------------
    // ------------|-----M---x----V--------------
    // ------------|---M-----x-----V-------------
    // ____________|M________x______V____________
    // ------------------------------------------
    // ------------------------------------------
    // ------------------------------------------
    // ------------------------------------------
    // ------------------------------------------
    // ------------------------------------------
    // ------------------------------------------

    // --      Ausenwinkel  - innenwinkel
    float deg_angle =
        (180 - (180 - (deg_base_rotation + 90))) + deg_base_rotation;
    deg_base_rotation += segment_size;
    {
      Frame *frame =
          tkbc_frame_generate(KITE_MOVE, tkbc_indexs_append(i),
                              &(CLITERAL(Move_Action){.position.x = place.x,
                                                      .position.y = place.y}),
                              duration);

      if (frame == NULL)
        return false;

      tkbc_dap(&frames, *frame);
    }
    {
      // TODO: Implement the none adding rotation
      Frame *frame = tkbc_frame_generate(
          KITE_ROTATION_ADD, tkbc_indexs_append(i),
          &(CLITERAL(Rotation_Action){.angle = deg_angle}), 0);

      if (frame == NULL)
        return false;

      tkbc_dap(&frames, *frame);
    }
  }

  tkbc_register_frames_array(env, &frames);
  return true;
}

bool tkbc_script_team_mountain(Env *env, Kite_Indexs kite_index_array,
                               size_t v_padding, size_t h_padding,
                               Vector2 offset, float duration) {

  int w = GetScreenWidth();
  int h = GetScreenHeight();

  assert(env->kite_array->count > 0 && "No kites in the kite array!");
  Kite *kite = env->kite_array->elements[0].kite;

  float full_kite_width = kite->width + kite->spread;
  float full_kite_height = kite->height;
  float x_space = h_padding + full_kite_width;
  float y_space = v_padding + full_kite_height;

  size_t columns = kite_index_array.count;
  bool isodd = kite_index_array.count % 2 == 1;
  size_t rows =
      isodd ? kite_index_array.count / 2 + 1 : kite_index_array.count / 2;

  Vector2 anchor = {.x = w / 2.0 - ((columns / 2.0) * x_space - x_space / 2),
                    .y = h / 2.0 - ((rows / 2.0) * y_space + y_space / 2)};

  anchor = Vector2Add(anchor, offset);

  Frames frames = {0};
  size_t i = 0;
  size_t row = rows;
  for (size_t column = 0; column < columns; ++column) {
    if (kite_index_array.count <= i) {
      break;
    }
    {
      Frame *frame =
          tkbc_frame_generate(KITE_MOVE, tkbc_indexs_append(i),
                              &(CLITERAL(Move_Action){
                                  .position.x = anchor.x + x_space * column,
                                  .position.y = anchor.y + y_space * row,
                              }),
                              duration);

      if (frame == NULL)
        return false;

      tkbc_dap(&frames, *frame);
    }
    {
      // TODO: Implement the none adding rotation
      Frame *frame = tkbc_frame_generate(
          KITE_ROTATION_ADD, tkbc_indexs_append(i),
          &(CLITERAL(Rotation_Action){.angle = 0}), duration);

      if (frame == NULL)
        return false;

      tkbc_dap(&frames, *frame);
    }
    i++;

    if (column + 1 == columns / 2.0) {
      continue;
    } else if (column + 1 < columns / 2.0) {
      row--;
    } else {
      row++;
    }
  }

  tkbc_register_frames_array(env, &frames);
  return true;
}

bool tkbc_script_team_valley(Env *env, Kite_Indexs kite_index_array,
                             size_t v_padding, size_t h_padding, Vector2 offset,
                             float duration) {

  int w = GetScreenWidth();
  int h = GetScreenHeight();

  assert(env->kite_array->count > 0 && "No kites in the kite array!");
  Kite *kite = env->kite_array->elements[0].kite;

  float full_kite_width = kite->width + kite->spread;
  float full_kite_height = kite->height;
  float x_space = h_padding + full_kite_width;
  float y_space = v_padding + full_kite_height;

  size_t columns = kite_index_array.count;
  bool isodd = kite_index_array.count % 2 == 1;
  size_t rows =
      isodd ? kite_index_array.count / 2 + 1 : kite_index_array.count / 2;

  Vector2 anchor = {.x = w / 2.0 - ((columns / 2.0) * x_space - x_space / 2),
                    .y = h / 2.0 - ((rows / 2.0) * y_space + y_space / 2)};

  anchor = Vector2Add(anchor, offset);

  Frames frames = {0};
  size_t i = 0;
  size_t row = 1;
  for (size_t column = 0; column < columns; ++column) {
    if (kite_index_array.count <= i) {
      break;
    }
    {
      Frame *frame =
          tkbc_frame_generate(KITE_MOVE, tkbc_indexs_append(i),
                              &(CLITERAL(Move_Action){
                                  .position.x = anchor.x + x_space * column,
                                  .position.y = anchor.y + y_space * row,
                              }),
                              duration);

      if (frame == NULL)
        return false;

      tkbc_dap(&frames, *frame);
    }
    // TODO: Implement the none adding rotation
    {
      Frame *frame = tkbc_frame_generate(
          KITE_ROTATION_ADD, tkbc_indexs_append(i),
          &(CLITERAL(Rotation_Action){.angle = 0}), duration);

      if (frame == NULL)
        return false;

      tkbc_dap(&frames, *frame);
    }
    i++;

    if (column + 1 == columns / 2.0) {
      continue;
    } else if (column + 1 < columns / 2.0) {
      row++;
    } else {
      row--;
    }
  }

  tkbc_register_frames_array(env, &frames);
  return true;
}

bool tkbc_script_team_arc(Env *env, Kite_Indexs kite_index_array,
                          size_t v_padding, size_t h_padding, Vector2 offset,
                          float angle, float duration) {

  float start_angle = angle;
  int w = GetScreenWidth();
  int h = GetScreenHeight();

  assert(env->kite_array->count > 0 && "No kites in the kite array!");
  Kite *kite = env->kite_array->elements[0].kite;

  float full_kite_width = kite->width + kite->spread;
  float full_kite_height = kite->height;
  float x_space = h_padding + full_kite_width;
  float y_space = v_padding + full_kite_height;

  size_t columns = kite_index_array.count;
  bool isodd = kite_index_array.count % 2 == 1;
  size_t rows =
      isodd ? kite_index_array.count / 2 + 1 : kite_index_array.count / 2;

  Vector2 anchor = {.x = w / 2.0 - ((columns / 2.0) * x_space - x_space / 2),
                    .y = h / 2.0 - ((rows / 2.0) * y_space + y_space / 2)};

  anchor = Vector2Add(anchor, offset);

  Frames frames = {0};
  size_t i = 0;
  size_t row = rows;
  for (size_t column = 0; column < columns; ++column) {
    if (kite_index_array.count <= i) {
      break;
    }
    {
      Frame *frame =
          tkbc_frame_generate(KITE_MOVE, tkbc_indexs_append(i),
                              &(CLITERAL(Move_Action){
                                  .position.x = anchor.x + x_space * column,
                                  .position.y = anchor.y + y_space * row,
                              }),
                              duration);

      if (frame == NULL)
        return false;

      tkbc_dap(&frames, *frame);
    }
    if (column + 1 == ceil(columns / 2.0)) {
      angle = 0;
    }
    {
      Frame *frame = tkbc_frame_generate(
          KITE_ROTATION_ADD, tkbc_indexs_append(i),
          &(CLITERAL(Rotation_Action){.angle = angle}), duration);

      if (frame == NULL)
        return false;

      tkbc_dap(&frames, *frame);
    }
    i++;

    if (column + 1 == columns / 2.0) {
      angle = 0;
      continue;
    } else if (column + 1 < columns / 2.0) {
      angle = start_angle;
      row--;
    } else {
      angle = -start_angle;
      row++;
    }
  }

  tkbc_register_frames_array(env, &frames);
  return true;
}

bool tkbc_script_team_mouth(Env *env, Kite_Indexs kite_index_array,
                            size_t v_padding, size_t h_padding, Vector2 offset,
                            float angle, float duration) {
  angle = -angle;
  float start_angle = angle;
  int w = GetScreenWidth();
  int h = GetScreenHeight();

  assert(env->kite_array->count > 0 && "No kites in the kite array!");
  Kite *kite = env->kite_array->elements[0].kite;

  float full_kite_width = kite->width + kite->spread;
  float full_kite_height = kite->height;
  float x_space = h_padding + full_kite_width;
  float y_space = v_padding + full_kite_height;

  size_t columns = kite_index_array.count;
  bool isodd = kite_index_array.count % 2 == 1;
  size_t rows =
      isodd ? kite_index_array.count / 2 + 1 : kite_index_array.count / 2;

  Vector2 anchor = {.x = w / 2.0 - ((columns / 2.0) * x_space - x_space / 2),
                    .y = h / 2.0 - ((rows / 2.0) * y_space + y_space / 2)};

  anchor = Vector2Add(anchor, offset);

  Frames frames = {0};
  size_t i = 0;
  size_t row = 1;
  for (size_t column = 0; column < columns; ++column) {
    if (kite_index_array.count <= i) {
      break;
    }
    {
      Frame *frame =
          tkbc_frame_generate(KITE_MOVE, tkbc_indexs_append(i),
                              &(CLITERAL(Move_Action){
                                  .position.x = anchor.x + x_space * column,
                                  .position.y = anchor.y + y_space * row,
                              }),
                              duration);

      if (frame == NULL)
        return false;

      tkbc_dap(&frames, *frame);
    }
    if (column + 1 == ceil(columns / 2.0)) {
      angle = 0;
    }
    {
      Frame *frame = tkbc_frame_generate(
          KITE_ROTATION_ADD, tkbc_indexs_append(i),
          &(CLITERAL(Rotation_Action){.angle = angle}), duration);

      if (frame == NULL)
        return false;

      tkbc_dap(&frames, *frame);
    }
    i++;

    if (column + 1 == columns / 2.0) {
      angle = 0;
      continue;
    } else if (column + 1 < columns / 2.0) {
      angle = start_angle;
      row++;
    } else {
      angle = -start_angle;
      row--;
    }
  }

  tkbc_register_frames_array(env, &frames);
  return true;
}

void tkbc_script_team_line(Env *env, Kite_Indexs kite_index_array,
                           size_t h_padding, Vector2 offset, float duration) {

  tkbc_script_team_grid(env, kite_index_array, 1, kite_index_array.count, 0,
                        h_padding, offset, duration);
}

bool tkbc_script_team_grid(Env *env, Kite_Indexs kite_index_array, size_t rows,
                           size_t columns, size_t v_padding, size_t h_padding,
                           Vector2 offset, float duration) {

  int w = GetScreenWidth();
  int h = GetScreenHeight();

  assert(env->kite_array->count > 0 && "No kites in the kite array!");
  Kite *kite = env->kite_array->elements[0].kite;

  float full_kite_width = kite->width + kite->spread;
  float full_kite_height = kite->height;
  float x_space = h_padding + full_kite_width;
  float y_space = v_padding + full_kite_height;

  Vector2 anchor = {.x = w / 2.0 - ((columns / 2.0) * x_space - x_space / 2),
                    .y = h / 2.0 - ((rows / 2.0) * y_space + y_space / 2)};

  anchor = Vector2Add(anchor, offset);

  Frames frames = {0};
  size_t i = 0;
  for (size_t column = 0; column < columns; ++column) {
    for (size_t row = rows; row > 0; --row) {
      if (kite_index_array.count <= i) {
        break;
      }
      Frame *frame =
          tkbc_frame_generate(KITE_MOVE, tkbc_indexs_append(i++),
                              &(CLITERAL(Move_Action){
                                  .position.x = anchor.x + x_space * column,
                                  .position.y = anchor.y + y_space * row,
                              }),
                              duration);

      if (frame == NULL)
        return false;

      tkbc_dap(&frames, *frame);
    }
  }

  tkbc_register_frames_array(env, &frames);
  return true;
}

void tkbc_script_team_box(Env *env, Kite_Indexs kite_index_array,
                          DIRECTION direction, float angle, float box_size,
                          float duration) {

  // TODO: Think about the starting point.
  if (direction == RIGHT) {
    angle = -angle;
  }

  tkbc_register_frames(env, tkbc_frame_generate(KITE_MOVE_ADD, kite_index_array,
                                                &(CLITERAL(Move_Add_Action){
                                                    .position.x = 0,
                                                    .position.y = -box_size / 2,
                                                }),
                                                duration));

  // TODO: Implement the none adding rotation
  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION_ADD, kite_index_array,
                               &(CLITERAL(Rotation_Action){.angle = angle}),
                               duration));

  switch (direction) {
  case LEFT: {
    tkbc_register_frames(env,
                         tkbc_frame_generate(KITE_MOVE_ADD, kite_index_array,
                                             &(CLITERAL(Move_Add_Action){
                                                 .position.x = -box_size,
                                                 .position.y = 0,
                                             }),
                                             duration));

  } break;
  case RIGHT: {
    tkbc_register_frames(env,
                         tkbc_frame_generate(KITE_MOVE_ADD, kite_index_array,
                                             &(CLITERAL(Move_Add_Action){
                                                 .position.x = box_size,
                                                 .position.y = 0,
                                             }),
                                             duration));

  } break;
  default:
    assert(0 && "UNREACHABLE");
  }

  // TODO: Implement the none adding rotation
  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION_ADD, kite_index_array,
                               &(CLITERAL(Rotation_Action){.angle = angle}),
                               duration));

  tkbc_register_frames(env, tkbc_frame_generate(KITE_MOVE_ADD, kite_index_array,
                                                &(CLITERAL(Move_Add_Action){
                                                    .position.x = 0,
                                                    .position.y = box_size,
                                                }),
                                                duration));

  // TODO: Implement the none adding rotation
  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION_ADD, kite_index_array,
                               &(CLITERAL(Rotation_Action){.angle = angle}),
                               duration));

  switch (direction) {
  case LEFT: {

    tkbc_register_frames(env,
                         tkbc_frame_generate(KITE_MOVE_ADD, kite_index_array,
                                             &(CLITERAL(Move_Add_Action){
                                                 .position.x = box_size,
                                                 .position.y = 0,
                                             }),
                                             duration));
  } break;
  case RIGHT: {
    tkbc_register_frames(env,
                         tkbc_frame_generate(KITE_MOVE_ADD, kite_index_array,
                                             &(CLITERAL(Move_Add_Action){
                                                 .position.x = -box_size,
                                                 .position.y = 0,
                                             }),
                                             duration));
  } break;
  default:
    assert(0 && "UNREACHABLE");
  }

  // TODO: Implement the none adding rotation
  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION_ADD, kite_index_array,
                               &(CLITERAL(Rotation_Action){.angle = angle}),
                               duration));

  tkbc_register_frames(env, tkbc_frame_generate(KITE_MOVE_ADD, kite_index_array,
                                                &(CLITERAL(Move_Add_Action){
                                                    .position.x = 0,
                                                    .position.y = -box_size / 2,
                                                }),
                                                duration));
}

void tkbc_script_team_box_left(Env *env, Kite_Indexs kite_index_array,
                               float box_size, float duration) {
  tkbc_script_team_box(env, kite_index_array, LEFT, 90, box_size, duration);
}

void tkbc_script_team_box_right(Env *env, Kite_Indexs kite_index_array,
                                float box_size, float duration) {
  tkbc_script_team_box(env, kite_index_array, RIGHT, 90, box_size, duration);
}

void tkbc_script_team_dimond(Env *env, Kite_Indexs kite_index_array,
                             DIRECTION direction, float angle, float box_size,
                             float duration) {

  if (direction == RIGHT) {
    angle = -angle;
  }

  // TODO: Implement the none adding rotation
  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION_ADD, kite_index_array,
                               &(CLITERAL(Rotation_Action){.angle = angle / 2}),
                               duration));

  switch (direction) {
  case RIGHT: {
    tkbc_register_frames(
        env, tkbc_frame_generate(KITE_MOVE_ADD, kite_index_array,
                                 &(CLITERAL(Move_Add_Action){
                                     .position.x = box_size / sqrt(2),
                                     .position.y = -box_size / sqrt(2),
                                 }),
                                 duration));

  } break;
  case LEFT: {
    tkbc_register_frames(
        env, tkbc_frame_generate(KITE_MOVE_ADD, kite_index_array,
                                 &(CLITERAL(Move_Add_Action){
                                     .position.x = -box_size / sqrt(2),
                                     .position.y = -box_size / sqrt(2),
                                 }),
                                 duration));

  } break;
  default:
    assert(0 && "UNREACHABLE");
  }

  // TODO: Implement the none adding rotation
  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION_ADD, kite_index_array,
                               &(CLITERAL(Rotation_Action){.angle = angle}),
                               duration));

  switch (direction) {
  case RIGHT: {
    tkbc_register_frames(
        env, tkbc_frame_generate(KITE_MOVE_ADD, kite_index_array,
                                 &(CLITERAL(Move_Add_Action){
                                     .position.x = box_size / sqrt(2),
                                     .position.y = box_size / sqrt(2),
                                 }),
                                 duration));
  } break;
  case LEFT: {
    tkbc_register_frames(
        env, tkbc_frame_generate(KITE_MOVE_ADD, kite_index_array,
                                 &(CLITERAL(Move_Add_Action){
                                     .position.x = -box_size / sqrt(2),
                                     .position.y = box_size / sqrt(2),
                                 }),
                                 duration));
  } break;
  default:
    assert(0 && "UNREACHABLE");
  }

  // TODO: Implement the none adding rotation
  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION_ADD, kite_index_array,
                               &(CLITERAL(Rotation_Action){.angle = angle}),
                               duration));

  switch (direction) {
  case RIGHT: {
    tkbc_register_frames(
        env, tkbc_frame_generate(KITE_MOVE_ADD, kite_index_array,
                                 &(CLITERAL(Move_Add_Action){
                                     .position.x = -box_size / sqrt(2),
                                     .position.y = box_size / sqrt(2),
                                 }),
                                 duration));
  } break;
  case LEFT: {
    tkbc_register_frames(
        env, tkbc_frame_generate(KITE_MOVE_ADD, kite_index_array,
                                 &(CLITERAL(Move_Add_Action){
                                     .position.x = box_size / sqrt(2),
                                     .position.y = box_size / sqrt(2),
                                 }),
                                 duration));
  } break;
  default:
    assert(0 && "UNREACHABLE");
  }

  // TODO: Implement the none adding rotation
  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION_ADD, kite_index_array,
                               &(CLITERAL(Rotation_Action){.angle = angle}),
                               duration));

  switch (direction) {
  case RIGHT: {
    tkbc_register_frames(
        env, tkbc_frame_generate(KITE_MOVE_ADD, kite_index_array,
                                 &(CLITERAL(Move_Add_Action){
                                     .position.x = -box_size / sqrt(2),
                                     .position.y = -box_size / sqrt(2),
                                 }),
                                 duration));
  } break;
  case LEFT: {
    tkbc_register_frames(
        env, tkbc_frame_generate(KITE_MOVE_ADD, kite_index_array,
                                 &(CLITERAL(Move_Add_Action){
                                     .position.x = box_size / sqrt(2),
                                     .position.y = -box_size / sqrt(2),
                                 }),
                                 duration));
  } break;
  default:
    assert(0 && "UNREACHABLE");
  }

  // TODO: Implement the none adding rotation
  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION_ADD, kite_index_array,
                               &(CLITERAL(Rotation_Action){.angle = angle / 2}),
                               duration));
}

void tkbc_script_team_dimond_left(Env *env, Kite_Indexs kite_index_array,
                                  float box_size, float duration) {
  tkbc_script_team_dimond(env, kite_index_array, LEFT, 90, box_size, duration);
}

void tkbc_script_team_dimond_right(Env *env, Kite_Indexs kite_index_array,
                                   float box_size, float duration) {
  tkbc_script_team_dimond(env, kite_index_array, RIGHT, 90, box_size, duration);
}

#endif // TKBC_TEAM_FIGURES_API_IMPLEMENTATION
