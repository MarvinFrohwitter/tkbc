#include "tkbc-team-figures-api.h"
#include "../global/tkbc-types.h"
#include "../global/tkbc-utils.h"
#include "tkbc-script-api.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>

#include "raymath.h"

// ========================== Script Team Figures ============================

/**
 * @brief The function can be used to move the given kites via index to a ball
 * position facing out.
 *
 * @param env The global state of the application.
 * @param kite_index_array The kites, that should be arranged to a ball,
 * represented by there kite_id.
 * @param position The center position of the ball.
 * @param offset An offset from the center the ball should move to.
 * @param radius The radius of the created ball from the ball center to the
 * kites center leading edge.
 * @param move_duration The time that the repositioning of the kites should take
 * in seconds.
 * @param rotation_duration The time that the rotation to the correct angle
 * should take in seconds.
 * @return True if the internal frame actions could be created with no errors,
 * otherwise false.
 */
bool tkbc_script_team_ball(Env *env, Kite_Ids kite_index_array,
                           Vector2 position, Vector2 offset, float radius,
                           float move_duration, float rotation_duration) {

  position = Vector2Add(position, offset);
  Vector2 place = position;

  size_t segments = kite_index_array.count;
  float segment_size = 360.0 / segments;
  float deg_base_rotation = segments / 2.0 * segment_size;

  env->scratch_buf_frames->count = 0;
  for (size_t i = 0; i < segments; ++i) {

    place.x += radius * cosf(PI * deg_base_rotation / 180);
    place.y += radius * sinf(PI * deg_base_rotation / 180);

    float deg_angle =
        (180 - (180 - (deg_base_rotation + 90))) + deg_base_rotation;
    deg_base_rotation += segment_size;
    {
      Frame *frame =
          tkbc_frame_generate(KITE_MOVE, tkbc_indexs_append(i),
                              &(CLITERAL(Move_Action){.position.x = place.x,
                                                      .position.y = place.y}),
                              move_duration);

      if (frame == NULL)
        return false;

      tkbc_dap(env->scratch_buf_frames, *frame);
    }
    {
      Frame *frame = tkbc_frame_generate(
          KITE_ROTATION, tkbc_indexs_append(i),
          &(CLITERAL(Rotation_Action){.angle = deg_angle}), rotation_duration);

      if (frame == NULL)
        return false;

      tkbc_dap(env->scratch_buf_frames, *frame);
    }
  }

  tkbc_register_frames_array(env, env->scratch_buf_frames);
  return true;
}

/**
 * @brief The function can be used to create the mountain figure.
 *
 * @param env The global state of the application.
 * @param kite_index_array The kites represented by there kite_id.
 * @param position The center position of the mountain.
 * @param offset An offset from the center the mountain should move to.
 * @param v_padding The vertical padding between the kites.
 * @param h_padding The horizontal padding between the kites.
 * @param move_duration The time that the repositioning of the kites should take
 * in seconds.
 * @param rotation_duration The time that the rotation to the correct angle
 * should take in seconds.
 * @return True if the internal frame actions could be created with no errors,
 * otherwise false.
 */
bool tkbc_script_team_mountain(Env *env, Kite_Ids kite_index_array,
                               Vector2 position, Vector2 offset,
                               size_t v_padding, size_t h_padding,
                               float move_duration, float rotation_duration) {

  assert(env->kite_array->count > 0 && "No kites in the kite array!");
  Kite *kite = env->vanilla_kite;

  float full_kite_width = kite->width + kite->spread;
  float full_kite_height = kite->height;
  float x_space = h_padding + full_kite_width;
  float y_space = v_padding + full_kite_height;

  size_t columns = kite_index_array.count;
  bool isodd = kite_index_array.count % 2 == 1;
  size_t rows =
      isodd ? kite_index_array.count / 2 + 1 : kite_index_array.count / 2;

  Vector2 anchor = {.x = position.x - ((columns / 2.0) * x_space - x_space / 2),
                    .y = position.y - ((rows / 2.0) * y_space + y_space / 2)};

  anchor = Vector2Add(anchor, offset);

  env->scratch_buf_frames->count = 0;
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
                              move_duration);

      if (frame == NULL)
        return false;

      tkbc_dap(env->scratch_buf_frames, *frame);
    }
    {
      Frame *frame = tkbc_frame_generate(
          KITE_ROTATION, tkbc_indexs_append(i),
          &(CLITERAL(Rotation_Action){.angle = 0}), rotation_duration);

      if (frame == NULL)
        return false;

      tkbc_dap(env->scratch_buf_frames, *frame);
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

  tkbc_register_frames_array(env, env->scratch_buf_frames);
  return true;
}

/**
 * @brief The function can be used to create the valley figure.
 *
 * @param env The global state of the application.
 * @param kite_index_array The kites represented by there kite_id.
 * @param position The center position of the valley.
 * @param offset An offset from the center the valley should move to.
 * @param v_padding The vertical padding between the kites.
 * @param h_padding The horizontal padding between the kites.
 * @param move_duration The time that the repositioning of the kites should take
 * in seconds.
 * @param rotation_duration The time that the rotation to the correct angle
 * should take in seconds.
 * @return True if the internal frame actions could be created with no errors,
 * otherwise false.
 */
bool tkbc_script_team_valley(Env *env, Kite_Ids kite_index_array,
                             Vector2 position, Vector2 offset, size_t v_padding,
                             size_t h_padding, float move_duration,
                             float rotation_duration) {

  assert(env->kite_array->count > 0 && "No kites in the kite array!");
  Kite *kite = env->vanilla_kite;

  float full_kite_width = kite->width + kite->spread;
  float full_kite_height = kite->height;
  float x_space = h_padding + full_kite_width;
  float y_space = v_padding + full_kite_height;

  size_t columns = kite_index_array.count;
  bool isodd = kite_index_array.count % 2 == 1;
  size_t rows =
      isodd ? kite_index_array.count / 2 + 1 : kite_index_array.count / 2;

  Vector2 anchor = {.x = position.x - ((columns / 2.0) * x_space - x_space / 2),
                    .y = position.y - ((rows / 2.0) * y_space + y_space / 2)};

  anchor = Vector2Add(anchor, offset);

  env->scratch_buf_frames->count = 0;
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
                              move_duration);

      if (frame == NULL)
        return false;

      tkbc_dap(env->scratch_buf_frames, *frame);
    }
    {
      Frame *frame = tkbc_frame_generate(
          KITE_ROTATION, tkbc_indexs_append(i),
          &(CLITERAL(Rotation_Action){.angle = 0}), rotation_duration);

      if (frame == NULL)
        return false;

      tkbc_dap(env->scratch_buf_frames, *frame);
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

  tkbc_register_frames_array(env, env->scratch_buf_frames);
  return true;
}

/**
 * @brief The function can be used to create the arc figure.
 *
 * @param env The global state of the application.
 * @param kite_index_array The kites represented by there kite_id.
 * @param position The center position of the arc.
 * @param offset An offset from the center the arc should move to.
 * @param v_padding The vertical padding between the kites.
 * @param h_padding The horizontal padding between the kites.
 * @param angle The angle at which the kites should facing out.
 * @param move_duration The time that the repositioning of the kites should take
 * in seconds.
 * @param rotation_duration The time that the rotation to the correct angle
 * should take in seconds.
 * @return True if the internal frame actions could be created with no errors,
 * otherwise false.
 */
bool tkbc_script_team_arc(Env *env, Kite_Ids kite_index_array, Vector2 position,
                          Vector2 offset, size_t v_padding, size_t h_padding,
                          float angle, float move_duration,
                          float rotation_duration) {
  float start_angle = angle;

  assert(env->kite_array->count > 0 && "No kites in the kite array!");
  Kite *kite = env->vanilla_kite;

  float full_kite_width = kite->width + kite->spread;
  float full_kite_height = kite->height;

  float default_arc_vspace = sinf(angle * PI / 180) * full_kite_width / 2;
  float default_arc_hspace = sinf(angle * PI / 180) * full_kite_height / 2;

  float x_space = h_padding + full_kite_width + default_arc_hspace;
  float y_space = v_padding + full_kite_height + default_arc_vspace;

  size_t columns = kite_index_array.count;
  bool isodd = kite_index_array.count % 2 == 1;
  size_t rows =
      isodd ? kite_index_array.count / 2 + 1 : kite_index_array.count / 2;

  Vector2 anchor = {.x = position.x - ((columns / 2.0) * x_space - x_space / 2),
                    .y = position.y - ((rows / 2.0) * y_space + y_space / 2)};

  anchor = Vector2Add(anchor, offset);
  env->scratch_buf_frames->count = 0;

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
                              move_duration);

      if (frame == NULL)
        return false;

      tkbc_dap(env->scratch_buf_frames, *frame);
    }
    if (column + 1 == ceil(columns / 2.0)) {
      angle = 0;
    }
    {
      Frame *frame = tkbc_frame_generate(
          KITE_ROTATION, tkbc_indexs_append(i),
          &(CLITERAL(Rotation_Action){.angle = angle}), rotation_duration);

      if (frame == NULL)
        return false;

      tkbc_dap(env->scratch_buf_frames, *frame);
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

  tkbc_register_frames_array(env, env->scratch_buf_frames);
  return true;
}

/**
 * @brief The function can be used to create the mouth figure.
 *
 * @param env The global state of the application.
 * @param kite_index_array The kites represented by there kite_id.
 * @param position The center position of the mouth.
 * @param offset An offset from the center the mouth should move to.
 * @param v_padding The vertical padding between the kites.
 * @param h_padding The horizontal padding between the kites.
 * @param angle The angle at which the kites should facing in.
 * @param move_duration The time that the repositioning of the kites should take
 * in seconds.
 * @param rotation_duration The time that the rotation to the correct angle
 * should take in seconds.
 * @return True if the internal frame actions could be created with no errors,
 * otherwise false.
 */
bool tkbc_script_team_mouth(Env *env, Kite_Ids kite_index_array,
                            Vector2 position, Vector2 offset, size_t v_padding,
                            size_t h_padding, float angle, float move_duration,
                            float rotation_duration) {
  angle = -angle;
  float start_angle = angle;

  assert(env->kite_array->count > 0 && "No kites in the kite array!");
  Kite *kite = env->vanilla_kite;

  float full_kite_width = kite->width + kite->spread;
  float full_kite_height = kite->height;

  float default_arc_vspace = sinf(angle * PI / 180) * full_kite_width / 2;
  float default_arc_hspace = sinf(angle * PI / 180) * full_kite_height / 2;

  float x_space = h_padding + full_kite_width - default_arc_hspace;
  float y_space = v_padding + full_kite_height - default_arc_vspace;

  size_t columns = kite_index_array.count;
  bool isodd = kite_index_array.count % 2 == 1;
  size_t rows =
      isodd ? kite_index_array.count / 2 + 1 : kite_index_array.count / 2;

  Vector2 anchor = {.x = position.x - ((columns / 2.0) * x_space - x_space / 2),
                    .y = position.y - ((rows / 2.0) * y_space + y_space / 2)};

  anchor = Vector2Add(anchor, offset);

  env->scratch_buf_frames->count = 0;
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
                              move_duration);

      if (frame == NULL)
        return false;

      tkbc_dap(env->scratch_buf_frames, *frame);
    }
    if (column + 1 == ceil(columns / 2.0)) {
      angle = 0;
    }
    {
      Frame *frame = tkbc_frame_generate(
          KITE_ROTATION, tkbc_indexs_append(i),
          &(CLITERAL(Rotation_Action){.angle = angle}), rotation_duration);

      if (frame == NULL)
        return false;

      tkbc_dap(env->scratch_buf_frames, *frame);
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

  tkbc_register_frames_array(env, env->scratch_buf_frames);
  return true;
}

/**
 * @brief The function can be used to create a single horizontal line.
 *
 *
 * @param env The global state of the application.
 * @param kite_index_array The kites represented by there kite_id.
 * @param position The center position of the line.
 * @param offset An offset from the center the line should move to.
 * @param h_padding The horizontal padding between the kites.
 * @param move_duration The time that the repositioning of the kites should take
 * in seconds.
 * @return True if the internal frame actions could be created with no errors,
 * otherwise false.
 */
bool tkbc_script_team_line(Env *env, Kite_Ids kite_index_array,
                           Vector2 position, Vector2 offset, size_t h_padding,
                           float move_duration) {

  return tkbc_script_team_grid(env, kite_index_array, position, offset, 0,
                               h_padding, 1, kite_index_array.count,
                               move_duration);
}

/**
 * @brief The function can be used to create the grid figure.
 *
 * @param env The global state of the application.
 * @param kite_index_array The kites represented by there kite_id.
 * @param position The center position of the grid.
 * @param offset An offset from the center the grid should move to.
 * @param v_padding The vertical padding between the kites.
 * @param h_padding The horizontal padding between the kites.
 * @param rows The amount of rows in the grid.
 * @param columns The amount of columns in the grid.
 * @param move_duration The time that the repositioning of the kites should take
 * in seconds.
 * @return True if the internal frame actions could be created with no errors,
 * otherwise false.
 */
bool tkbc_script_team_grid(Env *env, Kite_Ids kite_index_array,
                           Vector2 position, Vector2 offset, size_t v_padding,
                           size_t h_padding, size_t rows, size_t columns,
                           float move_duration) {

  assert(env->kite_array->count > 0 && "No kites in the kite array!");
  Kite *kite = env->vanilla_kite;

  float full_kite_width = kite->width + kite->spread;
  float full_kite_height = kite->height;
  float x_space = h_padding + full_kite_width;
  float y_space = v_padding + full_kite_height;

  Vector2 anchor = {.x = position.x - ((columns / 2.0) * x_space - x_space / 2),
                    .y = position.y - ((rows / 2.0) * y_space + y_space / 2)};

  anchor = Vector2Add(anchor, offset);

  env->scratch_buf_frames->count = 0;
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
                              move_duration);

      if (frame == NULL)
        return false;

      tkbc_dap(env->scratch_buf_frames, *frame);
    }
  }

  tkbc_register_frames_array(env, env->scratch_buf_frames);
  return true;
}

/**
 * @brief The function can be used to move the given kites via index as a box.
 *
 * @param env The global state of the application.
 * @param kite_index_array The kites represented by there kite_id.
 * @param direction The direction where the box should start.
 * @param angle The angle at which the kites should start.
 * @param box_size The size represented by a single side of the complete box.
 * @param move_duration The time that the repositioning of the kites should take
 * in seconds.
 * @param rotation_duration The time that the rotation to the correct angle
 * should take in seconds.
 */
void tkbc_script_team_box(Env *env, Kite_Ids kite_index_array,
                          DIRECTION direction, float angle, float box_size,
                          float move_duration, float rotation_duration) {

  // TODO: Think about the starting point.
  if (direction == RIGHT) {
    angle = -angle;
  }

  tkbc_register_frames(env, tkbc_frame_generate(KITE_MOVE_ADD, kite_index_array,
                                                &(CLITERAL(Move_Add_Action){
                                                    .position.x = 0,
                                                    .position.y = -box_size / 2,
                                                }),
                                                move_duration));

  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION_ADD, kite_index_array,
                               &(CLITERAL(Rotation_Action){.angle = angle}),
                               rotation_duration));

  switch (direction) {
  case LEFT: {
    tkbc_register_frames(env,
                         tkbc_frame_generate(KITE_MOVE_ADD, kite_index_array,
                                             &(CLITERAL(Move_Add_Action){
                                                 .position.x = -box_size,
                                                 .position.y = 0,
                                             }),
                                             move_duration));

  } break;
  case RIGHT: {
    tkbc_register_frames(env,
                         tkbc_frame_generate(KITE_MOVE_ADD, kite_index_array,
                                             &(CLITERAL(Move_Add_Action){
                                                 .position.x = box_size,
                                                 .position.y = 0,
                                             }),
                                             move_duration));

  } break;
  default:
    assert(0 && "UNREACHABLE");
  }

  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION_ADD, kite_index_array,
                               &(CLITERAL(Rotation_Action){.angle = angle}),
                               rotation_duration));

  tkbc_register_frames(env, tkbc_frame_generate(KITE_MOVE_ADD, kite_index_array,
                                                &(CLITERAL(Move_Add_Action){
                                                    .position.x = 0,
                                                    .position.y = box_size,
                                                }),
                                                move_duration));

  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION_ADD, kite_index_array,
                               &(CLITERAL(Rotation_Action){.angle = angle}),
                               rotation_duration));

  switch (direction) {
  case LEFT: {

    tkbc_register_frames(env,
                         tkbc_frame_generate(KITE_MOVE_ADD, kite_index_array,
                                             &(CLITERAL(Move_Add_Action){
                                                 .position.x = box_size,
                                                 .position.y = 0,
                                             }),
                                             move_duration));
  } break;
  case RIGHT: {
    tkbc_register_frames(env,
                         tkbc_frame_generate(KITE_MOVE_ADD, kite_index_array,
                                             &(CLITERAL(Move_Add_Action){
                                                 .position.x = -box_size,
                                                 .position.y = 0,
                                             }),
                                             move_duration));
  } break;
  default:
    assert(0 && "UNREACHABLE");
  }

  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION_ADD, kite_index_array,
                               &(CLITERAL(Rotation_Action){.angle = angle}),
                               rotation_duration));

  tkbc_register_frames(env, tkbc_frame_generate(KITE_MOVE_ADD, kite_index_array,
                                                &(CLITERAL(Move_Add_Action){
                                                    .position.x = 0,
                                                    .position.y = -box_size / 2,
                                                }),
                                                move_duration));
}

/**
 * @brief The function can be used to move the given kites via index as the team
 * figure left box.
 *
 * @param env The global state of the application.
 * @param kite_index_array The kites represented by there kite_id.
 * @param box_size The size represented by a single side of the complete box.
 * @param move_duration The time that the repositioning of the kites should take
 * in seconds.
 * @param rotation_duration The time that the rotation to the correct angle
 * should take in seconds.
 */
void tkbc_script_team_box_left(Env *env, Kite_Ids kite_index_array,
                               float box_size, float move_duration,
                               float rotation_duration) {
  tkbc_script_team_box(env, kite_index_array, LEFT, 90, box_size, move_duration,
                       rotation_duration);
}

/**
 * @brief The function can be used to move the given kites via index as the team
 * figure right box.
 *
 * @param env The global state of the application.
 * @param kite_index_array The kites represented by there kite_id.
 * @param box_size The size represented by a single side of the complete box.
 * @param move_duration The time that the repositioning of the kites should take
 * in seconds.
 * @param rotation_duration The time that the rotation to the correct angle
 * should take in seconds.
 */
void tkbc_script_team_box_right(Env *env, Kite_Ids kite_index_array,
                                float box_size, float move_duration,
                                float rotation_duration) {
  tkbc_script_team_box(env, kite_index_array, RIGHT, 90, box_size,
                       move_duration, rotation_duration);
}

/**
 * @brief The function can be used to create the split box figure starting
 * horizontally and upwards.
 *
 * @param env The global state of the application.
 * @param kite_index_array The kites represented by there kite_id.
 * @param odd_even The kite group that should start facing up, counted from the
 * right.
 * @param box_size The size represented by a single side of the complete box.
 * @param move_duration The time that the repositioning of the kites should take
 * in seconds.
 * @param rotation_duration The time that the rotation to the correct angle
 * should take in seconds.
 * @return True if the internal frame actions could be created with no errors,
 * otherwise false.
 */
bool tkbc_script_team_split_box_up(Env *env, Kite_Ids kite_index_array,
                                   ODD_EVEN odd_even, float box_size,
                                   float move_duration,
                                   float rotation_duration) {
  env->scratch_buf_frames->count = 0;
  Frame *frame = NULL;

  float angle = 90;
  switch (odd_even) {
  case ODD: {
    for (int i = kite_index_array.count - 1; i >= 0;) {
      frame = tkbc_frame_generate(KITE_ROTATION_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Rotation_Action){.angle = angle}),
                                  rotation_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
      if (i < 0) {
        break;
      }
      frame = tkbc_frame_generate(KITE_ROTATION_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Rotation_Action){.angle = -angle}),
                                  rotation_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
    }
    tkbc_register_frames_array(env, env->scratch_buf_frames);

    for (int i = kite_index_array.count - 1; i >= 0;) {
      frame = tkbc_frame_generate(KITE_MOVE_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Move_Add_Action){
                                      .position.x = 0,
                                      .position.y = -box_size,
                                  }),
                                  move_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
      if (i < 0) {
        break;
      }
      frame = tkbc_frame_generate(KITE_MOVE_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Move_Add_Action){
                                      .position.x = 0,
                                      .position.y = box_size,
                                  }),
                                  move_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
    }
    tkbc_register_frames_array(env, env->scratch_buf_frames);

    for (int i = kite_index_array.count - 1; i >= 0;) {
      frame = tkbc_frame_generate(KITE_ROTATION_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Rotation_Action){.angle = angle}),
                                  rotation_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
      if (i < 0) {
        break;
      }
      frame = tkbc_frame_generate(KITE_ROTATION_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Rotation_Action){.angle = -angle}),
                                  rotation_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
    }
    tkbc_register_frames_array(env, env->scratch_buf_frames);

    for (int i = kite_index_array.count - 1; i >= 0;) {
      frame = tkbc_frame_generate(KITE_MOVE_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Move_Add_Action){
                                      .position.x = -box_size,
                                      .position.y = 0,
                                  }),
                                  move_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
      if (i < 0) {
        break;
      }
      frame = tkbc_frame_generate(KITE_MOVE_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Move_Add_Action){
                                      .position.x = -box_size,
                                      .position.y = 0,
                                  }),
                                  move_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
    }
    tkbc_register_frames_array(env, env->scratch_buf_frames);

    for (int i = kite_index_array.count - 1; i >= 0;) {
      frame = tkbc_frame_generate(KITE_ROTATION_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Rotation_Action){.angle = angle}),
                                  rotation_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
      if (i < 0) {
        break;
      }
      frame = tkbc_frame_generate(KITE_ROTATION_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Rotation_Action){.angle = -angle}),
                                  rotation_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
    }
    tkbc_register_frames_array(env, env->scratch_buf_frames);

    for (int i = kite_index_array.count - 1; i >= 0;) {
      frame = tkbc_frame_generate(KITE_MOVE_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Move_Add_Action){
                                      .position.x = 0,
                                      .position.y = box_size,
                                  }),
                                  move_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
      if (i < 0) {
        break;
      }
      frame = tkbc_frame_generate(KITE_MOVE_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Move_Add_Action){
                                      .position.x = 0,
                                      .position.y = -box_size,
                                  }),
                                  move_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
    }
    tkbc_register_frames_array(env, env->scratch_buf_frames);

    for (int i = kite_index_array.count - 1; i >= 0;) {
      frame = tkbc_frame_generate(KITE_ROTATION_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Rotation_Action){.angle = angle}),
                                  rotation_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
      if (i < 0) {
        break;
      }
      frame = tkbc_frame_generate(KITE_ROTATION_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Rotation_Action){.angle = -angle}),
                                  rotation_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
    }
    tkbc_register_frames_array(env, env->scratch_buf_frames);

    for (int i = kite_index_array.count - 1; i >= 0;) {
      frame = tkbc_frame_generate(KITE_MOVE_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Move_Add_Action){
                                      .position.x = box_size,
                                      .position.y = 0,
                                  }),
                                  move_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
      if (i < 0) {
        break;
      }
      frame = tkbc_frame_generate(KITE_MOVE_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Move_Add_Action){
                                      .position.x = box_size,
                                      .position.y = 0,
                                  }),
                                  move_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
    }
    tkbc_register_frames_array(env, env->scratch_buf_frames);
  } break;
  case EVEN: {
    for (int i = kite_index_array.count - 1; i >= 0;) {
      frame = tkbc_frame_generate(KITE_ROTATION_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Rotation_Action){.angle = -angle}),
                                  rotation_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
      if (i < 0) {
        break;
      }
      frame = tkbc_frame_generate(KITE_ROTATION_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Rotation_Action){.angle = angle}),
                                  rotation_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
    }
    tkbc_register_frames_array(env, env->scratch_buf_frames);

    for (int i = kite_index_array.count - 1; i >= 0;) {
      frame = tkbc_frame_generate(KITE_MOVE_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Move_Add_Action){
                                      .position.x = 0,
                                      .position.y = box_size,
                                  }),
                                  move_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
      if (i < 0) {
        break;
      }
      frame = tkbc_frame_generate(KITE_MOVE_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Move_Add_Action){
                                      .position.x = 0,
                                      .position.y = -box_size,
                                  }),
                                  move_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
    }
    tkbc_register_frames_array(env, env->scratch_buf_frames);

    for (int i = kite_index_array.count - 1; i >= 0;) {
      frame = tkbc_frame_generate(KITE_ROTATION_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Rotation_Action){.angle = -angle}),
                                  rotation_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
      if (i < 0) {
        break;
      }
      frame = tkbc_frame_generate(KITE_ROTATION_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Rotation_Action){.angle = angle}),
                                  rotation_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
    }
    tkbc_register_frames_array(env, env->scratch_buf_frames);

    for (int i = kite_index_array.count - 1; i >= 0;) {
      frame = tkbc_frame_generate(KITE_MOVE_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Move_Add_Action){
                                      .position.x = -box_size,
                                      .position.y = 0,
                                  }),
                                  move_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
      if (i < 0) {
        break;
      }
      frame = tkbc_frame_generate(KITE_MOVE_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Move_Add_Action){
                                      .position.x = -box_size,
                                      .position.y = 0,
                                  }),
                                  move_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
    }
    tkbc_register_frames_array(env, env->scratch_buf_frames);

    for (int i = kite_index_array.count - 1; i >= 0;) {
      frame = tkbc_frame_generate(KITE_ROTATION_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Rotation_Action){.angle = -angle}),
                                  rotation_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
      if (i < 0) {
        break;
      }
      frame = tkbc_frame_generate(KITE_ROTATION_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Rotation_Action){.angle = angle}),
                                  rotation_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
    }
    tkbc_register_frames_array(env, env->scratch_buf_frames);

    for (int i = kite_index_array.count - 1; i >= 0;) {
      frame = tkbc_frame_generate(KITE_MOVE_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Move_Add_Action){
                                      .position.x = 0,
                                      .position.y = -box_size,
                                  }),
                                  move_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
      if (i < 0) {
        break;
      }
      frame = tkbc_frame_generate(KITE_MOVE_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Move_Add_Action){
                                      .position.x = 0,
                                      .position.y = box_size,
                                  }),
                                  move_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
    }
    tkbc_register_frames_array(env, env->scratch_buf_frames);

    for (int i = kite_index_array.count - 1; i >= 0;) {
      frame = tkbc_frame_generate(KITE_ROTATION_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Rotation_Action){.angle = -angle}),
                                  rotation_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
      if (i < 0) {
        break;
      }
      frame = tkbc_frame_generate(KITE_ROTATION_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Rotation_Action){.angle = angle}),
                                  rotation_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
    }
    tkbc_register_frames_array(env, env->scratch_buf_frames);

    for (int i = kite_index_array.count - 1; i >= 0;) {
      frame = tkbc_frame_generate(KITE_MOVE_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Move_Add_Action){
                                      .position.x = box_size,
                                      .position.y = 0,
                                  }),
                                  move_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
      if (i < 0) {
        break;
      }
      frame = tkbc_frame_generate(KITE_MOVE_ADD, tkbc_indexs_append(i--),
                                  &(CLITERAL(Move_Add_Action){
                                      .position.x = box_size,
                                      .position.y = 0,
                                  }),
                                  move_duration);
      if (frame == NULL)
        return false;
      tkbc_dap(env->scratch_buf_frames, *frame);
    }
    tkbc_register_frames_array(env, env->scratch_buf_frames);
  } break;
  default:
    assert(0 && "UNREACHABLE");
  }
  return true;
}

/**
 * @brief The function can be used to create the diamond figure.
 *
 * @param env The global state of the application.
 * @param kite_index_array The kites represented by there kite_id.
 * @param direction The direction where the diamond should start.
 * @param angle The angle at which the kites should start the figure (45 deg).
 * @param box_size The size represented by a single side of the complete box.
 * @param move_duration The time that the repositioning of the kites should take
 * in seconds.
 * @param rotation_duration The time that the rotation to the correct angle
 * should take in seconds.
 */
void tkbc_script_team_dimond(Env *env, Kite_Ids kite_index_array,
                             DIRECTION direction, float angle, float box_size,
                             float move_duration, float rotation_duration) {

  if (direction == RIGHT) {
    angle = -angle;
  }

  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION_ADD, kite_index_array,
                               &(CLITERAL(Rotation_Action){.angle = angle / 2}),
                               rotation_duration));

  switch (direction) {
  case RIGHT: {
    tkbc_register_frames(
        env, tkbc_frame_generate(KITE_MOVE_ADD, kite_index_array,
                                 &(CLITERAL(Move_Add_Action){
                                     .position.x = box_size / sqrt(2),
                                     .position.y = -box_size / sqrt(2),
                                 }),
                                 move_duration));

  } break;
  case LEFT: {
    tkbc_register_frames(
        env, tkbc_frame_generate(KITE_MOVE_ADD, kite_index_array,
                                 &(CLITERAL(Move_Add_Action){
                                     .position.x = -box_size / sqrt(2),
                                     .position.y = -box_size / sqrt(2),
                                 }),
                                 move_duration));

  } break;
  default:
    assert(0 && "UNREACHABLE");
  }

  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION_ADD, kite_index_array,
                               &(CLITERAL(Rotation_Action){.angle = angle}),
                               rotation_duration));

  switch (direction) {
  case RIGHT: {
    tkbc_register_frames(
        env, tkbc_frame_generate(KITE_MOVE_ADD, kite_index_array,
                                 &(CLITERAL(Move_Add_Action){
                                     .position.x = box_size / sqrt(2),
                                     .position.y = box_size / sqrt(2),
                                 }),
                                 move_duration));
  } break;
  case LEFT: {
    tkbc_register_frames(
        env, tkbc_frame_generate(KITE_MOVE_ADD, kite_index_array,
                                 &(CLITERAL(Move_Add_Action){
                                     .position.x = -box_size / sqrt(2),
                                     .position.y = box_size / sqrt(2),
                                 }),
                                 move_duration));
  } break;
  default:
    assert(0 && "UNREACHABLE");
  }

  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION_ADD, kite_index_array,
                               &(CLITERAL(Rotation_Action){.angle = angle}),
                               rotation_duration));

  switch (direction) {
  case RIGHT: {
    tkbc_register_frames(
        env, tkbc_frame_generate(KITE_MOVE_ADD, kite_index_array,
                                 &(CLITERAL(Move_Add_Action){
                                     .position.x = -box_size / sqrt(2),
                                     .position.y = box_size / sqrt(2),
                                 }),
                                 move_duration));
  } break;
  case LEFT: {
    tkbc_register_frames(
        env, tkbc_frame_generate(KITE_MOVE_ADD, kite_index_array,
                                 &(CLITERAL(Move_Add_Action){
                                     .position.x = box_size / sqrt(2),
                                     .position.y = box_size / sqrt(2),
                                 }),
                                 move_duration));
  } break;
  default:
    assert(0 && "UNREACHABLE");
  }

  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION_ADD, kite_index_array,
                               &(CLITERAL(Rotation_Action){.angle = angle}),
                               rotation_duration));

  switch (direction) {
  case RIGHT: {
    tkbc_register_frames(
        env, tkbc_frame_generate(KITE_MOVE_ADD, kite_index_array,
                                 &(CLITERAL(Move_Add_Action){
                                     .position.x = -box_size / sqrt(2),
                                     .position.y = -box_size / sqrt(2),
                                 }),
                                 move_duration));
  } break;
  case LEFT: {
    tkbc_register_frames(
        env, tkbc_frame_generate(KITE_MOVE_ADD, kite_index_array,
                                 &(CLITERAL(Move_Add_Action){
                                     .position.x = box_size / sqrt(2),
                                     .position.y = -box_size / sqrt(2),
                                 }),
                                 move_duration));
  } break;
  default:
    assert(0 && "UNREACHABLE");
  }

  tkbc_register_frames(
      env, tkbc_frame_generate(KITE_ROTATION_ADD, kite_index_array,
                               &(CLITERAL(Rotation_Action){.angle = angle / 2}),
                               rotation_duration));
}

/**
 * @brief The function can be used toe create the diamond figure starting to
 * left out of a vertical column.
 *
 * @param env The global state of the application.
 * @param kite_index_array The kites represented by there kite_id.
 * @param box_size The size represented by a single side of the complete box.
 * @param move_duration The time that the repositioning of the kites should take
 * in seconds.
 * @param rotation_duration The time that the rotation to the correct angle
 * should take in seconds.
 */
void tkbc_script_team_dimond_left(Env *env, Kite_Ids kite_index_array,
                                  float box_size, float move_duration,
                                  float rotation_duration) {
  tkbc_script_team_dimond(env, kite_index_array, LEFT, 90, box_size,
                          move_duration, rotation_duration);
}

/**
 * @brief The function can be used toe create the diamond figure starting to
 * right out of a vertical column.
 *
 * @param env The global state of the application.
 * @param kite_index_array The kites represented by there kite_id.
 * @param box_size The size represented by a single side of the complete box.
 * @param move_duration The time that the repositioning of the kites should take
 * in seconds.
 * @param rotation_duration The time that the rotation to the correct angle
 * should take in seconds.
 */
void tkbc_script_team_dimond_right(Env *env, Kite_Ids kite_index_array,
                                   float box_size, float move_duration,
                                   float rotation_duration) {
  tkbc_script_team_dimond(env, kite_index_array, RIGHT, 90, box_size,
                          move_duration, rotation_duration);
}
