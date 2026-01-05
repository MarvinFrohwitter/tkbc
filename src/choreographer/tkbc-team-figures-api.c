#include "tkbc-team-figures-api.h"
#include "../global/tkbc-types.h"
#include "../global/tkbc-utils.h"
#include "tkbc-script-api.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>

#include "raymath.h"
#include "tkbc-script-handler.h"

/**
 * @brief The function can be used to let the kites roll up but at a different
 * starting position.
 *
 * @param env The global state of the application.
 * @param kite_index_array The kites represented by there kite_id.
 * @param radius The radius of the circle to fly.
 * @param begin_angle_1 The angle where the computation begin given as an
 * absolute value.
 * @param end_angle_1 The angle where the computation end given as an absolute
 * value.
 * @param begin_angle_2 The angle where the computation begin given as an
 * absolute value.
 * @param end_angle_2 The angle where the computation end given as an absolute
 * value.
 * @param move_duration_1 The time that the repositioning of the kites should
 * take.
 * @param move_duration_2 The time that the repositioning of the kites should
 * take.
 * @return True if the internal frame actions could be created with no errors,
 * otherwise false.
 */
bool tkbc_script_team_roll_two_diffrent_angle(
    Env *env, Kite_Ids kite_index_array, float radius, size_t begin_angle_1,
    size_t end_angle_1, size_t begin_angle_2, size_t end_angle_2,
    float move_duration_1, float move_duration_2) {
  assert(kite_index_array.count == 2);

  Frame *frame = NULL;
  const float duration_1 = move_duration_1 / (end_angle_1 - begin_angle_1);
  const float duration_2 = move_duration_2 / (end_angle_2 - begin_angle_2);
  Vector2 position;

  assert((end_angle_1 - begin_angle_1) == (end_angle_2 - begin_angle_2));
  for (size_t deg = 0; deg < (end_angle_1 - begin_angle_1); ++deg) {
    tkbc_reset_frames_internal_data(&env->scratch_buf_frames);
    position = (Vector2){
        .x = radius * cosf(PI * (deg + begin_angle_1) / 180),
        .y = -radius * sinf(PI * (deg + begin_angle_1) / 180),
    };

    {
      frame = KITE_MOVE_ADD(ID(kite_index_array.elements[0]), position.x,
                            position.y, duration_1);
      if (frame == NULL)
        return false;
      space_dap(&env->script_creation_space, &env->scratch_buf_frames, *frame);
    }
    {
      frame =
          KITE_ROTATION_ADD(ID(kite_index_array.elements[0]), 1, duration_1);
      if (frame == NULL)
        return false;
      space_dap(&env->script_creation_space, &env->scratch_buf_frames, *frame);
    }

    position = (Vector2){
        .x = radius * cosf(PI * (deg + begin_angle_2) / 180),
        .y = -radius * sinf(PI * (deg + begin_angle_2) / 180),
    };

    {
      frame = KITE_MOVE_ADD(ID(kite_index_array.elements[1]), position.x,
                            position.y, duration_2);
      if (frame == NULL)
        return false;
      space_dap(&env->script_creation_space, &env->scratch_buf_frames, *frame);
    }
    {
      frame =
          KITE_ROTATION_ADD(ID(kite_index_array.elements[1]), 1, duration_2);
      if (frame == NULL)
        return false;
      space_dap(&env->script_creation_space, &env->scratch_buf_frames, *frame);
    }
    tkbc_register_frames_array(env, &env->scratch_buf_frames);
  }

  return true;
}

/**
 * @brief The function can be used to let the kites roll up if odd is given
 * the odd kites counted from the right rolls clockwise the evens roll
 * anticlockwise.
 *
 * @param env The global state of the application.
 * @param kite_index_array The kites represented by there kite_id.
 * @param odd_even The kite group that should start facing up, counted from the
 * right.
 * @param radius The radius of the circle.
 * @param begin_angle The angle where the computation begin given as an absolute
 * value in degrees.
 * @param end_angle The angle where the computation end given as an absolute
 * value in degrees.
 * @param move_duration The time that the repositioning of the kites should take
 * in seconds.
 * @return True if the internal frame actions could be created with no errors,
 * otherwise false.
 */
bool tkbc_script_team_roll_split_up(Env *env, Kite_Ids kite_index_array,
                                    ODD_EVEN odd_even, float radius,
                                    size_t begin_angle, size_t end_angle,
                                    float move_duration) {
  Frame *frame = NULL;
  float duration = move_duration / (end_angle - begin_angle);
  for (size_t deg = begin_angle; deg < end_angle; ++deg) {
    tkbc_reset_frames_internal_data(&env->scratch_buf_frames);
    for (size_t i = 0; i < kite_index_array.count; ++i) {
      if (i % 2 == odd_even) {
        Vector2 position = (Vector2){
            .x = radius * cosf(PI * deg / 180),
            .y = -radius * sinf(PI * deg / 180),
        };
        {
          frame = KITE_MOVE_ADD(ID(kite_index_array.elements[i]), position.x,
                                position.y, duration);
          if (frame == NULL)
            return false;
          space_dap(&env->script_creation_space, &env->scratch_buf_frames,
                    *frame);
        }
        {
          frame =
              KITE_ROTATION_ADD(ID(kite_index_array.elements[i]), 1, duration);
          if (frame == NULL)
            return false;
          space_dap(&env->script_creation_space, &env->scratch_buf_frames,
                    *frame);
        }
      } else {
        Vector2 position = (Vector2){
            .x = -radius * cosf(PI * deg / 180),
            .y = -radius * sinf(PI * deg / 180),
        };
        {
          frame = KITE_MOVE_ADD(ID(kite_index_array.elements[i]), position.x,
                                position.y, duration);
          if (frame == NULL)
            return false;
          space_dap(&env->script_creation_space, &env->scratch_buf_frames,
                    *frame);
        }
        {
          frame =
              KITE_ROTATION_ADD(ID(kite_index_array.elements[i]), -1, duration);
          if (frame == NULL)
            return false;
          space_dap(&env->script_creation_space, &env->scratch_buf_frames,
                    *frame);
        }
      }
    }
    tkbc_register_frames_array(env, &env->scratch_buf_frames);
  }
  return true;
}

/**
 * @brief The function can be used to let the kites roll down if odd is given
 * the odd kites counted from the right rolls anticlockwise the evens roll
 * clockwise.
 *
 * @param env The global state of the application.
 * @param kite_index_array The kites represented by there kite_id.
 * @param odd_even The kite group that should start facing up, counted from the
 * right.
 * @param radius The radius of the circle.
 * @param begin_angle The angle where the computation begin given as an absolute
 * value in degrees.
 * @param end_angle The angle where the computation end given as an absolute
 * value in degrees.
 * @param move_duration The time that the repositioning of the kites should take
 * in seconds.
 * @return True if the internal frame actions could be created with no errors,
 * otherwise false.
 */
bool tkbc_script_team_roll_split_down(Env *env, Kite_Ids kite_index_array,
                                      ODD_EVEN odd_even, float radius,
                                      size_t begin_angle, size_t end_angle,
                                      float move_duration) {
  Frame *frame = NULL;
  float duration = move_duration / (end_angle - begin_angle);
  int sign;

  for (size_t deg = begin_angle; deg < end_angle; ++deg) {
    tkbc_reset_frames_internal_data(&env->scratch_buf_frames);
    for (size_t i = 0; i < kite_index_array.count; ++i) {
      sign = i % 2 == odd_even ? -1 : 1;
      Vector2 position = (Vector2){
          .x = -sign * radius * cosf(PI * deg / 180),
          .y = radius * sinf(PI * deg / 180),
      };
      {
        frame = KITE_MOVE_ADD(ID(kite_index_array.elements[i]), position.x,
                              position.y, duration);
        if (frame == NULL)
          return false;
        space_dap(&env->script_creation_space, &env->scratch_buf_frames,
                  *frame);
      }
      {
        frame =
            KITE_ROTATION_ADD(ID(kite_index_array.elements[i]), sign, duration);
        if (frame == NULL)
          return false;
        space_dap(&env->script_creation_space, &env->scratch_buf_frames,
                  *frame);
      }
    }
    tkbc_register_frames_array(env, &env->scratch_buf_frames);
  }
  return true;
}

/**
 * @brief The function can be used to let the kites roll up in an
 * anticlockwise rotation.
 *
 * @param env The global state of the application.
 * @param kite_index_array The kites represented by there kite_id.
 * @param radius The radius of the circle.
 * @param begin_angle The angle where the computation begin given as an absolute
 * value in degrees.
 * @param end_angle The angle where the computation end given as an absolute
 * value in degrees.
 * @param move_duration The time that the repositioning of the kites should take
 * in seconds.
 * @return True if the internal frame actions could be created with no errors,
 * otherwise false.
 */
bool tkbc_script_team_roll_up_anti_clockwise(Env *env,
                                             Kite_Ids kite_index_array,
                                             float radius, size_t begin_angle,
                                             size_t end_angle,
                                             float move_duration) {
  Frame *frame = NULL;
  float duration = move_duration / (end_angle - begin_angle);
  for (size_t deg = begin_angle; deg < end_angle; ++deg) {
    tkbc_reset_frames_internal_data(&env->scratch_buf_frames);
    Vector2 position = (Vector2){
        .x = radius * cosf(PI * deg / 180),
        .y = -radius * sinf(PI * deg / 180),
    };
    for (size_t i = 0; i < kite_index_array.count; ++i) {
      {
        frame = KITE_MOVE_ADD(ID(kite_index_array.elements[i]), position.x,
                              position.y, duration);
        if (frame == NULL)
          return false;
        space_dap(&env->script_creation_space, &env->scratch_buf_frames,
                  *frame);
      }
      {
        frame =
            KITE_ROTATION_ADD(ID(kite_index_array.elements[i]), 1, duration);
        if (frame == NULL)
          return false;
        space_dap(&env->script_creation_space, &env->scratch_buf_frames,
                  *frame);
      }
    }
    tkbc_register_frames_array(env, &env->scratch_buf_frames);
  }
  return true;
}

/**
 * @brief The function can be used to let the kites roll up in an
 * clockwise rotation.
 *
 * @param env The global state of the application.
 * @param kite_index_array The kites represented by there kite_id.
 * @param radius The radius of the circle.
 * @param begin_angle The angle where the computation begin given as an absolute
 * value in degrees.
 * @param end_angle The angle where the computation end given as an absolute
 * value in degrees.
 * @param move_duration The time that the repositioning of the kites should take
 * in seconds.
 * @return True if the internal frame actions could be created with no errors,
 * otherwise false.
 */
bool tkbc_script_team_roll_up_clockwise(Env *env, Kite_Ids kite_index_array,
                                        float radius, size_t begin_angle,
                                        size_t end_angle, float move_duration) {
  Frame *frame = NULL;
  float duration = move_duration / (end_angle - begin_angle);
  for (size_t deg = begin_angle; deg < end_angle; ++deg) {
    tkbc_reset_frames_internal_data(&env->scratch_buf_frames);
    Vector2 position = (Vector2){
        .x = -radius * cosf(PI * deg / 180),
        .y = -radius * sinf(PI * deg / 180),
    };
    for (size_t i = 0; i < kite_index_array.count; ++i) {
      {
        frame = KITE_MOVE_ADD(ID(kite_index_array.elements[i]), position.x,
                              position.y, duration);
        if (frame == NULL)
          return false;
        space_dap(&env->script_creation_space, &env->scratch_buf_frames,
                  *frame);
      }
      {
        frame =
            KITE_ROTATION_ADD(ID(kite_index_array.elements[i]), -1, duration);
        if (frame == NULL)
          return false;
        space_dap(&env->script_creation_space, &env->scratch_buf_frames,
                  *frame);
      }
    }
    tkbc_register_frames_array(env, &env->scratch_buf_frames);
  }
  return true;
}

/**
 * @brief The function can be used to let the kites roll down in an
 * anticlockwise rotation.
 *
 * @param env The global state of the application.
 * @param kite_index_array The kites represented by there kite_id.
 * @param radius The radius of the circle.
 * @param begin_angle The angle where the computation begin given as an absolute
 * value in degrees.
 * @param end_angle The angle where the computation end given as an absolute
 * value in degrees.
 * @param move_duration The time that the repositioning of the kites should take
 * in seconds.
 * @return True if the internal frame actions could be created with no errors,
 * otherwise false.
 */
bool tkbc_script_team_roll_down_anti_clockwise(Env *env,
                                               Kite_Ids kite_index_array,
                                               float radius, size_t begin_angle,
                                               size_t end_angle,
                                               float move_duration) {
  Frame *frame = NULL;
  float duration = move_duration / (end_angle - begin_angle);
  for (size_t deg = begin_angle; deg < end_angle; ++deg) {
    tkbc_reset_frames_internal_data(&env->scratch_buf_frames);
    Vector2 position = (Vector2){
        .x = -radius * cosf(PI * deg / 180),
        .y = radius * sinf(PI * deg / 180),
    };
    for (size_t i = 0; i < kite_index_array.count; ++i) {
      {
        frame = KITE_MOVE_ADD(ID(kite_index_array.elements[i]), position.x,
                              position.y, duration);
        if (frame == NULL)
          return false;
        space_dap(&env->script_creation_space, &env->scratch_buf_frames,
                  *frame);
      }
      {
        frame =
            KITE_ROTATION_ADD(ID(kite_index_array.elements[i]), 1, duration);
        if (frame == NULL)
          return false;
        space_dap(&env->script_creation_space, &env->scratch_buf_frames,
                  *frame);
      }
    }
    tkbc_register_frames_array(env, &env->scratch_buf_frames);
  }
  return true;
}

/**
 * @brief The function can be used to let the kites roll down in an
 * clockwise rotation.
 *
 * @param env The global state of the application.
 * @param kite_index_array The kites represented by there kite_id.
 * @param radius The radius of the circle.
 * @param begin_angle The angle where the computation begin given as an absolute
 * value in degrees.
 * @param end_angle The angle where the computation end given as an absolute
 * value in degrees.
 * @param move_duration The time that the repositioning of the kites should take
 * in seconds.
 * @return True if the internal frame actions could be created with no errors,
 * otherwise false.
 */
bool tkbc_script_team_roll_down_clockwise(Env *env, Kite_Ids kite_index_array,
                                          float radius, size_t begin_angle,
                                          size_t end_angle,
                                          float move_duration) {
  Frame *frame = NULL;
  float duration = move_duration / (end_angle - begin_angle);
  for (size_t deg = begin_angle; deg < end_angle; ++deg) {
    tkbc_reset_frames_internal_data(&env->scratch_buf_frames);
    Vector2 position = (Vector2){
        .x = radius * cosf(PI * deg / 180),
        .y = radius * sinf(PI * deg / 180),
    };
    for (size_t i = 0; i < kite_index_array.count; ++i) {
      {
        frame = KITE_MOVE_ADD(ID(kite_index_array.elements[i]), position.x,
                              position.y, duration);
        if (frame == NULL)
          return false;
        space_dap(&env->script_creation_space, &env->scratch_buf_frames,
                  *frame);
      }
      {
        frame =
            KITE_ROTATION_ADD(ID(kite_index_array.elements[i]), -1, duration);
        if (frame == NULL)
          return false;
        space_dap(&env->script_creation_space, &env->scratch_buf_frames,
                  *frame);
      }
    }
    tkbc_register_frames_array(env, &env->scratch_buf_frames);
  }
  return true;
}

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
  Frame *frame = NULL;

  tkbc_reset_frames_internal_data(&env->scratch_buf_frames);
  for (size_t i = 0; i < kite_index_array.count; ++i) {

    place.x += radius * cosf(PI * deg_base_rotation / 180);
    place.y += radius * sinf(PI * deg_base_rotation / 180);

    float deg_angle =
        (180 - (180 - (deg_base_rotation + 90))) + deg_base_rotation;
    deg_base_rotation += segment_size;
    {
      frame = KITE_MOVE(ID(kite_index_array.elements[i]), place.x, place.y,
                        move_duration);
      if (frame == NULL)
        return false;
      space_dap(&env->script_creation_space, &env->scratch_buf_frames, *frame);
    }
    {
      frame = KITE_ROTATION(ID(kite_index_array.elements[i]), deg_angle,
                            rotation_duration);
      if (frame == NULL)
        return false;
      space_dap(&env->script_creation_space, &env->scratch_buf_frames, *frame);
    }
  }

  tkbc_register_frames_array(env, &env->scratch_buf_frames);
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
                               float v_padding, float h_padding,
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
  Frame *frame = NULL;

  tkbc_reset_frames_internal_data(&env->scratch_buf_frames);
  size_t row = rows;
  for (size_t i = 0, column = 0; column < columns; ++i, ++column) {
    if (kite_index_array.count <= i) {
      break;
    }
    {
      frame = KITE_MOVE(ID(kite_index_array.elements[i]),
                        anchor.x + x_space * column, anchor.y + y_space * row,
                        move_duration);
      if (frame == NULL)
        return false;
      space_dap(&env->script_creation_space, &env->scratch_buf_frames, *frame);
    }
    {
      frame =
          KITE_ROTATION(ID(kite_index_array.elements[i]), 0, rotation_duration);
      if (frame == NULL)
        return false;
      space_dap(&env->script_creation_space, &env->scratch_buf_frames, *frame);
    }

    if (column + 1 == columns / 2.0) {
      continue;
    } else if (column + 1 < columns / 2.0) {
      row--;
    } else {
      row++;
    }
  }

  tkbc_register_frames_array(env, &env->scratch_buf_frames);
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
                             Vector2 position, Vector2 offset, float v_padding,
                             float h_padding, float move_duration,
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
  Frame *frame = NULL;

  tkbc_reset_frames_internal_data(&env->scratch_buf_frames);
  size_t row = 1;
  for (size_t i = 0, column = 0; column < columns; ++i, ++column) {
    if (kite_index_array.count <= i) {
      break;
    }
    {
      frame = KITE_MOVE(ID(kite_index_array.elements[i]),
                        anchor.x + x_space * column, anchor.y + y_space * row,
                        move_duration);
      if (frame == NULL)
        return false;

      space_dap(&env->script_creation_space, &env->scratch_buf_frames, *frame);
    }
    {
      frame =
          KITE_ROTATION(ID(kite_index_array.elements[i]), 0, rotation_duration);
      if (frame == NULL)
        return false;

      space_dap(&env->script_creation_space, &env->scratch_buf_frames, *frame);
    }

    if (column + 1 == columns / 2.0) {
      continue;
    } else if (column + 1 < columns / 2.0) {
      row++;
    } else {
      row--;
    }
  }

  tkbc_register_frames_array(env, &env->scratch_buf_frames);
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
                          Vector2 offset, float v_padding, float h_padding,
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
  Frame *frame = NULL;

  tkbc_reset_frames_internal_data(&env->scratch_buf_frames);
  size_t row = rows;
  for (size_t i = 0, column = 0; column < columns; ++i, ++column) {
    if (kite_index_array.count <= i) {
      break;
    }
    {
      frame = KITE_MOVE(ID(kite_index_array.elements[i]),
                        anchor.x + x_space * column, anchor.y + y_space * row,
                        move_duration);
      if (frame == NULL)
        return false;

      space_dap(&env->script_creation_space, &env->scratch_buf_frames, *frame);
    }
    if (column + 1 == (columns + 1) / 2) {
      angle = 0;
    }
    {
      frame = KITE_ROTATION(ID(kite_index_array.elements[i]), angle,
                            rotation_duration);
      if (frame == NULL)
        return false;

      space_dap(&env->script_creation_space, &env->scratch_buf_frames, *frame);
    }

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

  tkbc_register_frames_array(env, &env->scratch_buf_frames);
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
                            Vector2 position, Vector2 offset, float v_padding,
                            float h_padding, float angle, float move_duration,
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
  Frame *frame = NULL;

  tkbc_reset_frames_internal_data(&env->scratch_buf_frames);
  size_t row = 1;
  for (size_t i = 0, column = 0; column < columns; ++i, ++column) {
    if (kite_index_array.count <= i) {
      break;
    }
    {
      frame = KITE_MOVE(ID(kite_index_array.elements[i]),
                        anchor.x + x_space * column, anchor.y + y_space * row,
                        move_duration);
      if (frame == NULL)
        return false;

      space_dap(&env->script_creation_space, &env->scratch_buf_frames, *frame);
    }
    if (column + 1 == (columns + 1) / 2) {
      angle = 0;
    }
    {
      frame = KITE_ROTATION(ID(kite_index_array.elements[i]), angle,
                            rotation_duration);
      if (frame == NULL)
        return false;

      space_dap(&env->script_creation_space, &env->scratch_buf_frames, *frame);
    }

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

  tkbc_register_frames_array(env, &env->scratch_buf_frames);
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
                           Vector2 position, Vector2 offset, float h_padding,
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
                           Vector2 position, Vector2 offset, float v_padding,
                           float h_padding, size_t rows, size_t columns,
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
  Frame *frame = NULL;

  tkbc_reset_frames_internal_data(&env->scratch_buf_frames);
  size_t i = 0;
  for (size_t column = 0; column < columns; ++column) {
    for (size_t row = rows; row > 0; --row) {
      if (kite_index_array.count <= i) {
        break;
      }
      frame = KITE_MOVE(ID(kite_index_array.elements[i++]),
                        anchor.x + x_space * column, anchor.y + y_space * row,
                        move_duration);
      if (frame == NULL)
        return false;

      space_dap(&env->script_creation_space, &env->scratch_buf_frames, *frame);
    }
  }

  tkbc_register_frames_array(env, &env->scratch_buf_frames);
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

  if (direction == RIGHT) {
    angle = -angle;
  }

  SET(KITE_MOVE_ADD(kite_index_array, 0, -box_size / 2, move_duration));
  SET(KITE_ROTATION_ADD(kite_index_array, angle, rotation_duration));

  switch (direction) {
  case LEFT:
    SET(KITE_MOVE_ADD(kite_index_array, -box_size, 0, move_duration));
    break;
  case RIGHT:
    SET(KITE_MOVE_ADD(kite_index_array, box_size, 0, move_duration));
    break;
  default:
    assert(0 && "UNREACHABLE");
  }

  SET(KITE_ROTATION_ADD(kite_index_array, angle, rotation_duration));
  SET(KITE_MOVE_ADD(kite_index_array, 0, box_size, move_duration));
  SET(KITE_ROTATION_ADD(kite_index_array, angle, rotation_duration));

  switch (direction) {
  case LEFT:
    SET(KITE_MOVE_ADD(kite_index_array, box_size, 0, move_duration));
    break;
  case RIGHT:
    SET(KITE_MOVE_ADD(kite_index_array, -box_size, 0, move_duration));
    break;
  default:
    assert(0 && "UNREACHABLE");
  }

  SET(KITE_ROTATION_ADD(kite_index_array, angle, rotation_duration));
  SET(KITE_MOVE_ADD(kite_index_array, 0, -box_size / 2, move_duration));
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
  tkbc_reset_frames_internal_data(&env->scratch_buf_frames);
  Frame *frame = NULL;

  float angle = 90;
  int pos = 1;
  int neg = -1;
  if (odd_even == EVEN) { // Switch the sign
    pos = -1;
    neg = 1;
  }

  for (int cycle = 0; cycle < 4; ++cycle) {
    for (int i = kite_index_array.count - 1; i >= 0;) {
      frame = KITE_ROTATION_ADD(ID(kite_index_array.elements[i--]), pos * angle,
                                rotation_duration);
      if (frame == NULL)
        return false;
      space_dap(&env->script_creation_space, &env->scratch_buf_frames, *frame);

      if (i < 0) {
        break;
      }

      frame = KITE_ROTATION_ADD(ID(kite_index_array.elements[i--]), neg * angle,
                                rotation_duration);
      if (frame == NULL)
        return false;

      space_dap(&env->script_creation_space, &env->scratch_buf_frames, *frame);
    }
    tkbc_register_frames_array(env, &env->scratch_buf_frames);

    for (int i = kite_index_array.count - 1; i >= 0;) {
      frame = KITE_MOVE_ADD(ID(kite_index_array.elements[i--]), 0,
                            neg * box_size, move_duration);
      if (frame == NULL)
        return false;

      space_dap(&env->script_creation_space, &env->scratch_buf_frames, *frame);
      if (i < 0) {
        break;
      }

      frame = KITE_MOVE_ADD(ID(kite_index_array.elements[i--]), 0,
                            pos * box_size, move_duration);
      if (frame == NULL)
        return false;

      space_dap(&env->script_creation_space, &env->scratch_buf_frames, *frame);
    }
    tkbc_register_frames_array(env, &env->scratch_buf_frames);
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
void tkbc_script_team_diamond(Env *env, Kite_Ids kite_index_array,
                              DIRECTION direction, float angle, float box_size,
                              float move_duration, float rotation_duration) {

  int sign = -1;
  if (direction == RIGHT) {
    angle = -angle;
    sign = 1;
  }

  double sqrt2 = sqrt(2);

  SET(KITE_ROTATION_ADD(kite_index_array, angle / 2, rotation_duration));

  SET(KITE_MOVE_ADD(kite_index_array, sign * box_size / sqrt2,
                    -box_size / sqrt2, move_duration));

  SET(KITE_ROTATION_ADD(kite_index_array, angle, rotation_duration));

  SET(KITE_MOVE_ADD(kite_index_array, sign * box_size / sqrt2, box_size / sqrt2,
                    move_duration));

  SET(KITE_ROTATION_ADD(kite_index_array, angle, rotation_duration));

  SET(KITE_MOVE_ADD(kite_index_array, -sign * box_size / sqrt2,
                    box_size / sqrt2, move_duration));

  SET(KITE_ROTATION_ADD(kite_index_array, angle, rotation_duration));

  SET(KITE_MOVE_ADD(kite_index_array, -sign * box_size / sqrt2,
                    -box_size / sqrt2, move_duration));

  SET(KITE_ROTATION_ADD(kite_index_array, angle / 2, rotation_duration));
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
void tkbc_script_team_diamond_left(Env *env, Kite_Ids kite_index_array,
                                   float box_size, float move_duration,
                                   float rotation_duration) {
  tkbc_script_team_diamond(env, kite_index_array, LEFT, 90, box_size,
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
void tkbc_script_team_diamond_right(Env *env, Kite_Ids kite_index_array,
                                    float box_size, float move_duration,
                                    float rotation_duration) {
  tkbc_script_team_diamond(env, kite_index_array, RIGHT, 90, box_size,
                           move_duration, rotation_duration);
}
