#include "../../external/cassert/cassert.h"

#include "../choreographer/tkbc-script-api.h"
#include "../choreographer/tkbc.h"
#include "../global/tkbc-types.h"
#include "../global/tkbc-utils.h"
#include <math.h>
#include <stdlib.h>

#include "../../external/space/space.h"

/**
 * @brief Sets up an env with a single kite and returns its id list.
 */
static Kite_Ids bezier_test_setup_env(Env **out_env) {
    Env *env = tkbc_init_env();
    Kite_State s0 = tkbc_init_kite();
    s0.kite_id = 0;
    s0.is_active = true;
    s0.is_script_kite = true;
    tkbc_dap(&env->kite_array, s0);
    *out_env = env;
    return tkbc_indexs_range(0, 1);
}

Test bezier_quadratic_emits_move_and_rotation(void) {
    Test test = cassert_init_test("tkbc_kite_bezier_quadratic()");
    Env *env = NULL;
    Kite_Ids ids = bezier_test_setup_env(&env);

    Vector2 p1 = {.x = 0, .y = 0};
    Vector2 p2 = {.x = 50, .y = 100};
    Vector2 p3 = {.x = 100, .y = 0};
    tkbc_kite_bezier_quadratic(env, ids, p1, p2, p3, 1.0f, 90.0f, 2.0f);

    // Every segment is its own block holding a MOVE and a ROTATION_ADD that
    // run at the same time.
    cassert_size_t_neq(env->scratch_buf_script.count, (size_t) 0);
    for (size_t i = 0; i < env->scratch_buf_script.count; ++i) {
        Frames *block = &env->scratch_buf_script.elements[i];
        cassert_size_t_eq(block->count, (size_t) 2);
        cassert_int_eq((int) block->elements[0].kind, ACTION_KITE_MOVE);
        cassert_int_eq((int) block->elements[1].kind, ACTION_KITE_ROTATION_ADD);
    }

    // The last segment ends exactly on the curve end point.
    Frames *last = &env->scratch_buf_script.elements[env->scratch_buf_script.count - 1];
    cassert_float_eq(last->elements[0].action.as_move.position.x, p3.x);
    cassert_float_eq(last->elements[0].action.as_move.position.y, p3.y);

    // The rotation is split evenly across the segments and sums up to the full
    // requested angle.
    float angle_sum = 0.0f;
    for (size_t i = 0; i < env->scratch_buf_script.count; ++i) {
        angle_sum += env->scratch_buf_script.elements[i].elements[1].action.as_rotation_add.angle;
    }
    cassert_bool_eq(fabsf(angle_sum - 90.0f) < 0.01f, true);

    free(ids.elements);
    tkbc_destroy_env(env);
    return test;
}

Test bezier_cubic_emits_move_and_rotation(void) {
    Test test = cassert_init_test("tkbc_kite_bezier_cubic()");
    Env *env = NULL;
    Kite_Ids ids = bezier_test_setup_env(&env);

    Vector2 p1 = {.x = 0, .y = 0};
    Vector2 p2 = {.x = 0, .y = 100};
    Vector2 p3 = {.x = 100, .y = 100};
    Vector2 p4 = {.x = 100, .y = 0};
    tkbc_kite_bezier_cubic(env, ids, p1, p2, p3, p4, 1.0f, -45.0f, 1.0f);

    cassert_size_t_neq(env->scratch_buf_script.count, (size_t) 0);
    for (size_t i = 0; i < env->scratch_buf_script.count; ++i) {
        Frames *block = &env->scratch_buf_script.elements[i];
        cassert_size_t_eq(block->count, (size_t) 2);
        cassert_int_eq((int) block->elements[0].kind, ACTION_KITE_MOVE);
        cassert_int_eq((int) block->elements[1].kind, ACTION_KITE_ROTATION_ADD);
    }

    Frames *last = &env->scratch_buf_script.elements[env->scratch_buf_script.count - 1];
    cassert_float_eq(last->elements[0].action.as_move.position.x, p4.x);
    cassert_float_eq(last->elements[0].action.as_move.position.y, p4.y);

    float angle_sum = 0.0f;
    for (size_t i = 0; i < env->scratch_buf_script.count; ++i) {
        angle_sum += env->scratch_buf_script.elements[i].elements[1].action.as_rotation_add.angle;
    }
    cassert_bool_eq(fabsf(angle_sum - (-45.0f)) < 0.01f, true);

    free(ids.elements);
    tkbc_destroy_env(env);
    return test;
}

Test bezier_without_rotation_only_moves(void) {
    Test test = cassert_init_test("tkbc_kite_bezier_quadratic(no rotation)");
    Env *env = NULL;
    Kite_Ids ids = bezier_test_setup_env(&env);

    Vector2 p1 = {.x = 0, .y = 0};
    Vector2 p2 = {.x = 50, .y = 100};
    Vector2 p3 = {.x = 100, .y = 0};
    tkbc_kite_bezier_quadratic(env, ids, p1, p2, p3, 1.0f, 0.0f, 0.0f);

    cassert_size_t_neq(env->scratch_buf_script.count, (size_t) 0);
    for (size_t i = 0; i < env->scratch_buf_script.count; ++i) {
        Frames *block = &env->scratch_buf_script.elements[i];
        cassert_size_t_eq(block->count, (size_t) 1);
        cassert_int_eq((int) block->elements[0].kind, ACTION_KITE_MOVE);
    }

    free(ids.elements);
    tkbc_destroy_env(env);
    return test;
}

Test bezier_quadratic_add_emits_relative_move(void) {
    Test test = cassert_init_test("tkbc_kite_bezier_quadratic_add()");
    Env *env = NULL;
    Kite_Ids ids = bezier_test_setup_env(&env);

    Vector2 p2 = {.x = 50, .y = 100};
    Vector2 p3 = {.x = 100, .y = 0};
    tkbc_kite_bezier_quadratic_add(env, ids, p2, p3, 1.0f, 90.0f, 2.0f);

    // Every segment is its own block holding a relative move and a rotation
    // that run at the same time.
    cassert_size_t_neq(env->scratch_buf_script.count, (size_t) 0);
    Vector2 displacement = {.x = 0, .y = 0};
    float angle_sum = 0.0f;
    for (size_t i = 0; i < env->scratch_buf_script.count; ++i) {
        Frames *block = &env->scratch_buf_script.elements[i];
        cassert_size_t_eq(block->count, (size_t) 2);
        cassert_int_eq((int) block->elements[0].kind, ACTION_KITE_MOVE_ADD);
        cassert_int_eq((int) block->elements[1].kind, ACTION_KITE_ROTATION_ADD);

        displacement.x += block->elements[0].action.as_move_add.position.x;
        displacement.y += block->elements[0].action.as_move_add.position.y;
        angle_sum += block->elements[1].action.as_rotation_add.angle;
    }

    // The relative start point is the origin, so the segment deltas sum up to
    // the relative end point of the curve.
    cassert_float_eq(displacement.x, p3.x);
    cassert_float_eq(displacement.y, p3.y);
    cassert_bool_eq(fabsf(angle_sum - 90.0f) < 0.01f, true);

    free(ids.elements);
    tkbc_destroy_env(env);
    return test;
}

Test bezier_cubic_add_emits_relative_move(void) {
    Test test = cassert_init_test("tkbc_kite_bezier_cubic_add()");
    Env *env = NULL;
    Kite_Ids ids = bezier_test_setup_env(&env);

    Vector2 p2 = {.x = 0, .y = 100};
    Vector2 p3 = {.x = 100, .y = 100};
    Vector2 p4 = {.x = 100, .y = 0};
    tkbc_kite_bezier_cubic_add(env, ids, p2, p3, p4, 1.0f, -45.0f, 1.0f);

    cassert_size_t_neq(env->scratch_buf_script.count, (size_t) 0);
    Vector2 displacement = {.x = 0, .y = 0};
    float angle_sum = 0.0f;
    for (size_t i = 0; i < env->scratch_buf_script.count; ++i) {
        Frames *block = &env->scratch_buf_script.elements[i];
        cassert_size_t_eq(block->count, (size_t) 2);
        cassert_int_eq((int) block->elements[0].kind, ACTION_KITE_MOVE_ADD);
        cassert_int_eq((int) block->elements[1].kind, ACTION_KITE_ROTATION_ADD);

        displacement.x += block->elements[0].action.as_move_add.position.x;
        displacement.y += block->elements[0].action.as_move_add.position.y;
        angle_sum += block->elements[1].action.as_rotation_add.angle;
    }

    // The relative start point is the origin, so the segment deltas sum up to
    // the relative end point of the curve.
    cassert_float_eq(displacement.x, p4.x);
    cassert_float_eq(displacement.y, p4.y);
    cassert_bool_eq(fabsf(angle_sum - (-45.0f)) < 0.01f, true);

    free(ids.elements);
    tkbc_destroy_env(env);
    return test;
}

/**
 * @brief Run all script api unit tests.
 *
 * @param tests Pointer to the Tests struct to register results in.
 */
void tkbc_test_script_api(Tests *tests) {
    cassert_dap(tests, bezier_quadratic_emits_move_and_rotation());
    cassert_dap(tests, bezier_cubic_emits_move_and_rotation());
    cassert_dap(tests, bezier_without_rotation_only_moves());
    cassert_dap(tests, bezier_quadratic_add_emits_relative_move());
    cassert_dap(tests, bezier_cubic_add_emits_relative_move());
}
