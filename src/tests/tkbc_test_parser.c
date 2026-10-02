#include "../../external/cassert/cassert.h"

#include "../choreographer/tkbc-parser.h"
#include "../choreographer/tkbc.h"
#include "../global/tkbc-types.h"
#include "../global/tkbc-utils.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "../../external/space/space.h"

/**
 * @brief Sets up an env with two kites and returns their id list.
 */
static Kite_Ids parser_test_setup_env(Env **out_env) {
    Env *env = tkbc_init_env();

    Kite_State s0 = tkbc_init_kite();
    s0.kite_id = 0;
    s0.is_active = true;
    s0.is_script_kite = true;
    tkbc_dap(&env->kite_array, s0);

    Kite_State s1 = tkbc_init_kite();
    s1.kite_id = 1;
    s1.is_active = true;
    s1.is_script_kite = true;
    tkbc_dap(&env->kite_array, s1);

    *out_env = env;
    return tkbc_indexs_range(0, 2);
}

/**
 * @brief Parses the given bezier source with the parser helper and frees the
 * helper local scratch state again.
 *
 * @param env The global state of the application.
 * @param script_kis The already generated script kite ids.
 * @param source The mutable source string that is parsed.
 * @param cubic True for a cubic curve, false for a quadratic curve.
 * @param add True for the additive variant.
 * @param brace True if the curve is parsed inside a parallel frame block.
 * @return True if the parsing and frame construction has worked, otherwise
 * false.
 */
static bool parser_test_parse_bezier(Env *env, Kite_Ids *script_kis, char *source, bool cubic, bool add, bool brace) {
    Kite_Id_Remap remap = {0};
    Content tmp_buffer = {0};
    // The lexer takes ownership of the content and frees it in lexer_del.
    char *content = strdup(source);
    Lexer *l = lexer_new("bezier-test", content, strlen(content), 0);
    bool ok = tkbc_parse_bezier_curve(env, l, cubic, add, script_kis, &remap, brace, &tmp_buffer);
    lexer_del(l);
    if (tmp_buffer.elements) {
        free(tmp_buffer.elements);
    }
    if (remap.file_ids.elements) {
        free(remap.file_ids.elements);
    }
    if (remap.env_ids.elements) {
        free(remap.env_ids.elements);
    }
    return ok;
}

Test parse_bezier_quadratic_curve(void) {
    Test test = cassert_init_test("tkbc_parse_bezier_curve(quadratic)");
    Env *env = NULL;
    Kite_Ids script_kis = parser_test_setup_env(&env);

    char source[] = "(0 1) 0 0 50 100 100 0 1 90 2";
    bool ok = parser_test_parse_bezier(env, &script_kis, source, false, false, false);
    cassert_bool_eq(ok, true);

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
    cassert_float_eq(last->elements[0].action.as_move.position.x, 100.0f);
    cassert_float_eq(last->elements[0].action.as_move.position.y, 0.0f);

    free(script_kis.elements);
    tkbc_destroy_env(env);
    return test;
}

Test parse_bezier_cubic_curve(void) {
    Test test = cassert_init_test("tkbc_parse_bezier_curve(cubic)");
    Env *env = NULL;
    Kite_Ids script_kis = parser_test_setup_env(&env);

    char source[] = "(0 1) 0 0 0 100 100 100 100 0 1 -45 1";
    bool ok = parser_test_parse_bezier(env, &script_kis, source, true, false, false);
    cassert_bool_eq(ok, true);

    cassert_size_t_neq(env->scratch_buf_script.count, (size_t) 0);
    for (size_t i = 0; i < env->scratch_buf_script.count; ++i) {
        Frames *block = &env->scratch_buf_script.elements[i];
        cassert_size_t_eq(block->count, (size_t) 2);
        cassert_int_eq((int) block->elements[0].kind, ACTION_KITE_MOVE);
        cassert_int_eq((int) block->elements[1].kind, ACTION_KITE_ROTATION_ADD);
    }

    Frames *last = &env->scratch_buf_script.elements[env->scratch_buf_script.count - 1];
    cassert_float_eq(last->elements[0].action.as_move.position.x, 100.0f);
    cassert_float_eq(last->elements[0].action.as_move.position.y, 0.0f);

    free(script_kis.elements);
    tkbc_destroy_env(env);
    return test;
}

Test parse_bezier_quadratic_curve_add(void) {
    Test test = cassert_init_test("tkbc_parse_bezier_curve(quadratic_add)");
    Env *env = NULL;
    Kite_Ids script_kis = parser_test_setup_env(&env);

    char source[] = "(0 1) 50 100 100 0 1 90 2";
    bool ok = parser_test_parse_bezier(env, &script_kis, source, false, true, false);
    cassert_bool_eq(ok, true);

    // The relative start point is the origin, so the segment deltas sum up to
    // the relative end point of the curve.
    cassert_size_t_neq(env->scratch_buf_script.count, (size_t) 0);
    Vector2 displacement = {.x = 0, .y = 0};
    for (size_t i = 0; i < env->scratch_buf_script.count; ++i) {
        Frames *block = &env->scratch_buf_script.elements[i];
        cassert_size_t_eq(block->count, (size_t) 2);
        cassert_int_eq((int) block->elements[0].kind, ACTION_KITE_MOVE_ADD);
        cassert_int_eq((int) block->elements[1].kind, ACTION_KITE_ROTATION_ADD);

        displacement.x += block->elements[0].action.as_move_add.position.x;
        displacement.y += block->elements[0].action.as_move_add.position.y;
    }
    cassert_float_eq(displacement.x, 100.0f);
    cassert_float_eq(displacement.y, 0.0f);

    free(script_kis.elements);
    tkbc_destroy_env(env);
    return test;
}

Test parse_bezier_cubic_curve_add(void) {
    Test test = cassert_init_test("tkbc_parse_bezier_curve(cubic_add)");
    Env *env = NULL;
    Kite_Ids script_kis = parser_test_setup_env(&env);

    char source[] = "(0 1) 0 100 100 100 100 0 1 -45 1";
    bool ok = parser_test_parse_bezier(env, &script_kis, source, true, true, false);
    cassert_bool_eq(ok, true);

    cassert_size_t_neq(env->scratch_buf_script.count, (size_t) 0);
    Vector2 displacement = {.x = 0, .y = 0};
    for (size_t i = 0; i < env->scratch_buf_script.count; ++i) {
        Frames *block = &env->scratch_buf_script.elements[i];
        cassert_size_t_eq(block->count, (size_t) 2);
        cassert_int_eq((int) block->elements[0].kind, ACTION_KITE_MOVE_ADD);
        cassert_int_eq((int) block->elements[1].kind, ACTION_KITE_ROTATION_ADD);

        displacement.x += block->elements[0].action.as_move_add.position.x;
        displacement.y += block->elements[0].action.as_move_add.position.y;
    }
    cassert_float_eq(displacement.x, 100.0f);
    cassert_float_eq(displacement.y, 0.0f);

    free(script_kis.elements);
    tkbc_destroy_env(env);
    return test;
}

Test parse_bezier_curve_inside_brace_is_rejected(void) {
    Test test = cassert_init_test("tkbc_parse_bezier_curve(brace rejected)");
    Env *env = NULL;
    Kite_Ids script_kis = parser_test_setup_env(&env);

    char source[] = "(0 1) 0 0 50 100 100 0 1 90 2";
    bool ok = parser_test_parse_bezier(env, &script_kis, source, false, false, true);
    cassert_bool_eq(ok, false);
    cassert_size_t_eq(env->scratch_buf_script.count, (size_t) 0);

    free(script_kis.elements);
    tkbc_destroy_env(env);
    return test;
}

/**
 * @brief Run all parser unit tests.
 *
 * @param tests Pointer to the Tests struct to register results in.
 */
void tkbc_test_parser(Tests *tests) {
    cassert_dap(tests, parse_bezier_quadratic_curve());
    cassert_dap(tests, parse_bezier_cubic_curve());
    cassert_dap(tests, parse_bezier_quadratic_curve_add());
    cassert_dap(tests, parse_bezier_cubic_curve_add());
    cassert_dap(tests, parse_bezier_curve_inside_brace_is_rejected());
}
