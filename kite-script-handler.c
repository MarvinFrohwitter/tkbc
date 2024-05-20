// ===========================================================================
// ========================== Script Handler =================================
// ===========================================================================

#include "tkbc.h"
#include <assert.h>
#include <math.h>
#include <stddef.h>

/**
 * @brief [TODO:description]
 *
 * @param state [TODO:parameter]
 */
void kite_script_begin(State *state) { state->interrupt_script = true; }
void kite_script_end(State *state) { state->interrupt_script = false; }

/**
 * @brief [TODO:description]
 *
 * @param kite [TODO:parameter]
 * @param steps_x [TODO:parameter]
 * @param steps_y [TODO:parameter]
 * @param parameters [TODO:parameter]
 */
void kite_script_move(Kite *kite, float steps_x, float steps_y,
                      PARAMETERS parameters) {

  Vector2 pos = {0};

  switch (parameters) {
  case FIXED: {
  final_pos:
    pos.x = steps_x;
    pos.y = steps_y;
    kite_center_rotation(kite, &pos, 0);
  } break;
  case SMOOTH: {

    size_t maxiter = fmaxf(fabsf(steps_x), fabsf(steps_y));
    float iter_space_x = fabsf(steps_x) / maxiter;
    float iter_space_y = fabsf(steps_x) / maxiter;

    // TODO: What we do if the x and y are different
    assert(steps_x == steps_y);
    for (size_t i = 0; i < maxiter; ++i) {

      if (floorf(pos.x) != steps_x) {
        pos.x = steps_x > 0 ? pos.x + iter_space_x : pos.x - iter_space_x;
      }

      if (floorf(pos.y) != steps_y) {
        pos.y = steps_y > 0 ? pos.y + iter_space_y : pos.y - iter_space_y;
      }
      if (steps_x == 0)
        pos.x = steps_x;
      if (steps_y == 0)
        pos.y = steps_y;

      kite_center_rotation(kite, &pos, 0);
    }

    // Just in case the rounding of the iterations is not enough to reach the
    // final position.
    goto final_pos;

  } break;
  default:
    assert(0 && "ERROR: kite_script_move: UNREACHABLE");
  }
}

/**
 * @brief [TODO:description]
 *
 * @param kite [TODO:parameter]
 * @param angle [TODO:parameter]
 * @param parameters [TODO:parameter]
 */
void kite_script_rotate(Kite *kite, float angle, PARAMETERS parameters) {

  switch (parameters) {
  case FIXED: {
    kite_center_rotation(kite, NULL, angle);
  } break;
  case SMOOTH: {
    if (angle < 0) {
      for (size_t i = 0; i >= angle; --i) {
        kite_center_rotation(kite, NULL, kite->center_rotation + i);
      }
    } else {
      for (size_t i = 0; i <= angle; ++i) {
        kite_center_rotation(kite, NULL, kite->center_rotation + i);
      }
    }

    // Just in case because we accept floats that could potentially be not an
    // integer. Draw the rest of the rotation.
    kite_center_rotation(kite, NULL, angle);

  } break;
  default:
    assert(0 && "ERROR: kite_script_rotate: UNREACHABLE");
  }
}

/**
 * @brief [TODO:description]
 *
 * @param kite [TODO:parameter]
 * @param tip [TODO:parameter]
 * @param angle [TODO:parameter]
 * @param parameters [TODO:parameter]
 */
void kite_script_rotate_tip(Kite *kite, TIP tip, float angle,
                            PARAMETERS parameters) {

  switch (parameters) {
  case FIXED: {
    switch (tip) {
    case LEFT_TIP:
    case RIGHT_TIP:
      kite_tip_rotation(kite, NULL, angle, tip);
      break;
    default:
      assert(0 && "ERROR: kite_script_rotate_tip: FIXED: UNREACHABLE");
    }

  } break;
  case SMOOTH: {
    switch (tip) {
    case LEFT_TIP:
    case RIGHT_TIP:
      if (angle < 0) {
        for (size_t i = 0; i >= angle; --i) {
          kite_tip_rotation(kite, NULL, kite->center_rotation + angle, tip);
        }
      } else {
        for (size_t i = 0; i <= angle; ++i) {
          kite_tip_rotation(kite, NULL, kite->center_rotation + angle, tip);
        }
      }
      break;
    default:
      assert(0 && "ERROR: kite_script_rotate_tip: SMOOTH: UNREACHABLE");
    }

    // Just in case because we accept floats that could potentially be not an
    // integer. Draw the rest of the rotation.
    kite_tip_rotation(kite, NULL, angle, tip);

  } break;
  default:
    assert(0 && "ERROR: kite_script_rotate_tip: UNREACHABLE");
  }
}
