#include "../../external/cassert/cassert.h"

#include "../choreographer/tkbc.h"
#include "../global/tkbc-types.h"

Test kite_update_internal() {
  Test test = cassert_init_test("tkbc_kite_update_internal()");

  Kite_State *kite_state = tkbc_init_kite();
  Kite kite = *kite_state->kite;
  free(kite_state->kite);
  free(kite_state);

  Vector2 position = {200, 100};
  float angle = 90;

  kite.center = position;
  kite.angle = angle;

  tkbc_kite_update_internal(&kite);
  // The approx factor is just for angle 90 and approximates the real
  // calculation good enough, with eps 0.01, that the sin computation can be
  // avoided.
  float approx = 2 / 3.0;

  cassert_float_eq(kite.center.x, position.x);
  cassert_float_eq(kite.center.y, position.y);
  cassert_float_eq(kite.angle, angle);

  cassert_float_eq(kite.old_center.x, 0);
  cassert_float_eq(kite.old_center.y, 0);
  cassert_float_eq(kite.old_angle, 0);

  cassert_float_eq(kite.left.v1.x, position.x);
  cassert_float_eq(kite.left.v1.y, position.y + kite.width / 2.0);

  cassert_float_eq_epsilon(kite.left.v2.x, position.x + kite.height);
  cassert_float_eq_epsilon(kite.left.v2.y,
                           position.y + kite.inner_space * approx);

  cassert_float_eq(kite.left.v3.x, position.x);
  cassert_float_eq(kite.left.v3.y, position.y - kite.overlap);

  cassert_float_eq(kite.right.v1.x, position.x);
  cassert_float_eq(kite.right.v1.y, position.y + kite.overlap);

  cassert_float_eq_epsilon(kite.right.v2.x, position.x + kite.height);
  cassert_float_eq_epsilon(kite.right.v2.y,
                           position.y - kite.inner_space * approx);

  cassert_float_eq(kite.right.v3.x, position.x);
  cassert_float_eq(kite.right.v3.y, position.y - kite.width / 2.0);

  cassert_float_eq(kite.rec.width, kite.width);
  cassert_float_eq(kite.rec.height, 3 * PI * kite.spread);
  cassert_float_eq(kite.rec.x, position.x);
  cassert_float_eq(kite.rec.y, position.y + kite.width / 2);

  return test;
}

Test kite_update_position() {
  Test test = cassert_init_test("tkbc_kite_update_position()");

  Kite_State *kite_state = tkbc_init_kite();
  Kite kite = *kite_state->kite;
  free(kite_state->kite);
  free(kite_state);

  Vector2 position = {200, 100};

  tkbc_kite_update_position(&kite, &position);
  // The approx factor is just for angle 0 and approximates the real
  // calculation good enough, with eps 0.01, that the sin computation can be
  // avoided.
  float approx = 2 / 3.0;

  cassert_float_eq(kite.center.x, position.x);
  cassert_float_eq(kite.center.y, position.y);
  cassert_float_eq(kite.angle, 0);

  cassert_float_eq(kite.old_center.x, 0);
  cassert_float_eq(kite.old_center.y, 0);
  cassert_float_eq(kite.old_angle, 0);

  cassert_float_eq(kite.left.v1.x, position.x - kite.width / 2);
  cassert_float_eq(kite.left.v1.y, position.y);

  cassert_float_eq_epsilon(kite.left.v2.x,
                           position.x - kite.inner_space * approx);
  cassert_float_eq_epsilon(kite.left.v2.y, position.y + kite.height);

  cassert_float_eq(kite.left.v3.x, position.x + kite.overlap);
  cassert_float_eq(kite.left.v3.y, position.y);

  cassert_float_eq(kite.right.v1.x, position.x - kite.overlap);
  cassert_float_eq(kite.right.v1.y, position.y);

  cassert_float_eq_epsilon(kite.right.v2.x,
                           position.x + kite.inner_space * approx);
  cassert_float_eq_epsilon(kite.right.v2.y, position.y + kite.height);

  cassert_float_eq(kite.right.v3.x, position.x + kite.width / 2);
  cassert_float_eq(kite.right.v3.y, position.y);

  cassert_float_eq(kite.rec.width, kite.width);
  cassert_float_eq(kite.rec.height, 3 * PI * kite.spread);
  cassert_float_eq(kite.rec.x, position.x - kite.width / 2);
  cassert_float_eq(kite.rec.y, position.y);

  return test;
}

Test kite_update_angle() {
  Test test = cassert_init_test("tkbc_kite_update_angle()");

  Kite_State *kite_state = tkbc_init_kite();
  Kite kite = *kite_state->kite;
  free(kite_state->kite);
  free(kite_state);
  float angle = 90;
  kite.center = (Vector2){0, 0};

  tkbc_kite_update_angle(&kite, angle);
  // The approx factor is just for angle 90 and approximates the real
  // calculation good enough, with eps 0.01, that the sin computation can be
  // avoided.
  float approx = 2 / 3.0;

  cassert_float_eq(kite.center.x, 0);
  cassert_float_eq(kite.center.y, 0);
  cassert_float_eq(kite.angle, angle);

  cassert_float_eq(kite.old_center.x, 0);
  cassert_float_eq(kite.old_center.y, 0);
  cassert_float_eq(kite.old_angle, 0);

  cassert_float_eq_epsilon(kite.left.v1.x, 0);
  cassert_float_eq_epsilon(kite.left.v1.y, kite.width / 2);

  cassert_float_eq_epsilon(kite.left.v2.x, kite.height);
  cassert_float_eq_epsilon(kite.left.v2.y, kite.inner_space * approx);

  cassert_float_eq_epsilon(kite.left.v3.x, 0);
  cassert_float_eq(kite.left.v3.y, -kite.overlap);

  cassert_float_eq_epsilon(kite.right.v1.x, 0);
  cassert_float_eq(kite.right.v1.y, +kite.overlap);

  cassert_float_eq_epsilon(kite.right.v2.x, kite.height);
  cassert_float_eq_epsilon(kite.right.v2.y, -kite.inner_space * approx);

  cassert_float_eq_epsilon(kite.right.v3.x, 0);
  cassert_float_eq(kite.right.v3.y, -kite.width / 2);

  cassert_float_eq(kite.rec.width, kite.width);
  cassert_float_eq(kite.rec.height, 3 * PI * kite.spread);
  cassert_float_eq_epsilon(kite.rec.x, 0);
  cassert_float_eq(kite.rec.y, kite.width / 2);

  return test;
}

Test center_rotation() {
  Test test = cassert_init_test("tkbc_center_rotation()");

  Kite_State *kite_state = tkbc_init_kite();
  Kite kite = *kite_state->kite;

  Vector2 position = {200, 100};
  float angle = 90;
  tkbc_center_rotation(&kite, &position, angle);
  // The approx factor is just for angle 90 and approximates the real
  // calculation good enough, with eps 0.01, that the sin computation can be
  // avoided.
  float approx = 2 / 3.0;

  cassert_float_eq(kite.center.x, position.x);
  cassert_float_eq(kite.center.y, position.y);
  cassert_float_eq(kite.angle, angle);

  cassert_float_eq(kite.old_center.x, 0);
  cassert_float_eq(kite.old_center.y, 0);
  cassert_float_eq(kite.old_angle, 0);

  cassert_float_eq(kite.left.v1.x, position.x);
  cassert_float_eq(kite.left.v1.y, position.y + kite.width / 2.0);

  cassert_float_eq_epsilon(kite.left.v2.x, position.x + kite.height);
  cassert_float_eq_epsilon(kite.left.v2.y,
                           position.y + kite.inner_space * approx);

  cassert_float_eq(kite.left.v3.x, position.x);
  cassert_float_eq(kite.left.v3.y, position.y - kite.overlap);

  cassert_float_eq(kite.right.v1.x, position.x);
  cassert_float_eq(kite.right.v1.y, position.y + kite.overlap);

  cassert_float_eq_epsilon(kite.right.v2.x, position.x + kite.height);
  cassert_float_eq_epsilon(kite.right.v2.y,
                           position.y - kite.inner_space * approx);

  cassert_float_eq(kite.right.v3.x, position.x);
  cassert_float_eq(kite.right.v3.y, position.y - kite.width / 2.0);

  cassert_float_eq(kite.rec.width, kite.width);
  cassert_float_eq(kite.rec.height, 3 * PI * kite.spread);
  cassert_float_eq(kite.rec.x, position.x);
  cassert_float_eq(kite.rec.y, position.y + kite.width / 2);

  tkbc_destroy_kite(kite_state);
  return test;
}

Test tip_rotation_left() {
  Test test = cassert_init_test("tkbc_tip_rotation(LEFT)");

  Kite_State *kite_state = tkbc_init_kite();
  Kite kite = *kite_state->kite;

  Vector2 position = {200, 100};
  float angle = 90;
  tkbc_tip_rotation(&kite, &position, angle, LEFT_TIP);
  // The approx factor is just for angle 90 and approximates the real
  // calculation good enough, with eps 0.01, that the sin computation can be
  // avoided.
  float approx = 2 / 3.0;

  cassert_float_eq(kite.center.x, position.x - kite.width / 2);
  cassert_float_eq(kite.center.y, position.y - kite.width / 2);

  cassert_float_eq(kite.angle, angle);

  cassert_float_eq(kite.old_center.x, 0);
  cassert_float_eq(kite.old_center.y, 0);
  cassert_float_eq(kite.old_angle, 0);

  cassert_float_eq(kite.left.v1.x, position.x - kite.width / 2);
  cassert_float_eq(kite.left.v1.y, position.y);

  cassert_float_eq_epsilon(kite.left.v2.x,
                           position.x - kite.width / 2 + kite.height);
  cassert_float_eq_epsilon(kite.left.v2.y, position.y - kite.width / 2 +
                                               kite.inner_space * approx);

  cassert_float_eq(kite.left.v3.x, position.x - kite.width / 2);
  cassert_float_eq(kite.left.v3.y, position.y - kite.width / 2 - kite.overlap);

  cassert_float_eq(kite.right.v1.x, position.x - kite.width / 2);
  cassert_float_eq(kite.right.v1.y, position.y - kite.width / 2 + kite.overlap);

  cassert_float_eq_epsilon(kite.right.v2.x,
                           position.x - kite.width / 2 + kite.height);
  cassert_float_eq_epsilon(kite.right.v2.y, position.y - kite.width / 2 -
                                                kite.inner_space * approx);

  cassert_float_eq(kite.right.v3.x, position.x - kite.width / 2);
  cassert_float_eq(kite.right.v3.y, position.y - kite.width);

  cassert_float_eq(kite.rec.width, kite.width);
  cassert_float_eq(kite.rec.height, 3 * PI * kite.spread);
  cassert_float_eq(kite.rec.x, position.x - kite.width / 2);
  cassert_float_eq(kite.rec.y, position.y);

  tkbc_destroy_kite(kite_state);
  return test;
}

Test tip_rotation_right() {
  Test test = cassert_init_test("tkbc_tip_rotation(RIGHT)");

  Kite_State *kite_state = tkbc_init_kite();
  Kite kite = *kite_state->kite;

  Vector2 position = {200, 100};
  float angle = 90;
  tkbc_tip_rotation(&kite, &position, angle, RIGHT_TIP);
  // The approx factor is just for angle 90 and approximates the real
  // calculation good enough, with eps 0.01, that the sin computation can be
  // avoided.
  float approx = 2 / 3.0;

  cassert_float_eq(kite.center.x, position.x + kite.width / 2);
  cassert_float_eq(kite.center.y, position.y + kite.width / 2);

  cassert_float_eq(kite.angle, angle);

  cassert_float_eq(kite.old_center.x, 0);
  cassert_float_eq(kite.old_center.y, 0);
  cassert_float_eq(kite.old_angle, 0);

  cassert_float_eq(kite.left.v1.x, position.x + kite.width / 2);
  cassert_float_eq(kite.left.v1.y, position.y + kite.width);

  cassert_float_eq_epsilon(kite.left.v2.x,
                           position.x + kite.width / 2 + kite.height);
  cassert_float_eq_epsilon(kite.left.v2.y, position.y + kite.width / 2 +
                                               kite.inner_space * approx);

  cassert_float_eq(kite.left.v3.x, position.x + kite.width / 2);
  cassert_float_eq(kite.left.v3.y, position.y + kite.width / 2 - kite.overlap);

  cassert_float_eq(kite.right.v1.x, position.x + kite.width / 2);
  cassert_float_eq(kite.right.v1.y, position.y + kite.width / 2 + kite.overlap);

  cassert_float_eq_epsilon(kite.right.v2.x,
                           position.x + kite.width / 2 + kite.height);
  cassert_float_eq_epsilon(kite.right.v2.y, position.y + kite.width / 2 -
                                                kite.inner_space * approx);

  cassert_float_eq(kite.right.v3.x, position.x + kite.width / 2);
  cassert_float_eq(kite.right.v3.y, position.y);

  cassert_float_eq(kite.rec.width, kite.width);
  cassert_float_eq(kite.rec.height, 3 * PI * kite.spread);
  cassert_float_eq(kite.rec.x, position.x + kite.width / 2);
  cassert_float_eq(kite.rec.y, position.y + kite.width);

  tkbc_destroy_kite(kite_state);
  return test;
}

void tkbc_test_geometrics(Tests *tests) {
  cassert_dap(tests, center_rotation());
  cassert_dap(tests, tip_rotation_left());
  cassert_dap(tests, tip_rotation_right());
  cassert_dap(tests, kite_update_internal());
  cassert_dap(tests, kite_update_position());
  cassert_dap(tests, kite_update_angle());
}
