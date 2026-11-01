#include "../../external/cassert/cassert.h"

#include "../choreographer/tkbc.h"
#include "../global/tkbc-types.h"

#include "../../external/space/space.h"

#include <stdlib.h>

/**
 * @brief Fills the given kite array with one kite per given is_script_kite
 * flag. The kite_ids start at 1 and increase by one per added kite, therefore
 * the flag at index i belongs to the kite_id i + 1.
 *
 * @param kite_array The kite array that should be filled.
 * @param is_script_kites The flags that describe the added kites.
 * @param amount The amount of flags and therefore of added kites.
 */
static void kite_array_setup(Kite_States *kite_array, const bool *is_script_kites, size_t amount) {
    for (size_t i = 0; i < amount; ++i) {
        Kite_State state = tkbc_init_kite();
        state.kite_id = i + 1;
        state.is_script_kite = is_script_kites[i];
        tkbc_dap(kite_array, state);
    }
}

/**
 * @brief Checks that the kite array holds exactly the expected kite_ids in
 * exactly that order. This is a plain helper instead of a cassert macro,
 * because the cassert macros register into the local test variable and are
 * therefore not usable from within a helper function.
 *
 * @param kite_array The kite array that should be inspected.
 * @param expected_ids The kite_ids that the array should hold in that order.
 * @param amount The amount of expected kite_ids.
 * @return True if the array matches the expectation, otherwise false.
 */
static bool kite_array_holds_ids(Kite_States kite_array, const size_t *expected_ids, size_t amount) {
    if (kite_array.count != amount) {
        return false;
    }
    for (size_t i = 0; i < amount; ++i) {
        if (kite_array.elements[i].kite_id != expected_ids[i]) {
            return false;
        }
        if (kite_array.elements[i].kite == NULL) {
            return false;
        }
    }
    return true;
}

Test remove_non_script_kites_except_removes_all_plain_kites(void) {
    Test test = cassert_init_test("tkbc_remove_non_script_kites_except(all plain kites)");

    Kite_States kite_array = {0};
    bool is_script_kites[] = {false, false, false, false};
    kite_array_setup(&kite_array, is_script_kites, 4);

    // The given kite_id is not part of the array, therefore everything is
    // unwanted. Removing while iterating used to shift the not yet inspected
    // kites one slot to the front and left roughly half of them behind.
    tkbc_remove_non_script_kites_except(&kite_array, 99);
    cassert_size_t_eq(kite_array.count, (size_t) 0);

    tkbc_destroy_kite_array(&kite_array);
    return test;
}

Test remove_non_script_kites_except_removes_two_plain_kites(void) {
    Test test = cassert_init_test("tkbc_remove_non_script_kites_except(two plain kites)");

    Kite_States kite_array = {0};
    bool is_script_kites[] = {false, false};
    kite_array_setup(&kite_array, is_script_kites, 2);

    tkbc_remove_non_script_kites_except(&kite_array, 99);
    cassert_size_t_eq(kite_array.count, (size_t) 0);

    tkbc_destroy_kite_array(&kite_array);
    return test;
}

Test remove_non_script_kites_except_keeps_script_kites_and_given_id(void) {
    Test test = cassert_init_test("tkbc_remove_non_script_kites_except(script kites and given id)");

    Kite_States kite_array = {0};
    // kite_id 2 is a script kite, kite_id 1 is the given one to keep.
    bool is_script_kites[] = {false, true, false, false, false};
    kite_array_setup(&kite_array, is_script_kites, 5);

    tkbc_remove_non_script_kites_except(&kite_array, 1);

    size_t expected_ids[] = {1, 2};
    cassert_bool_eq(kite_array_holds_ids(kite_array, expected_ids, 2), true);

    tkbc_destroy_kite_array(&kite_array);
    return test;
}

Test remove_non_script_kites_except_keeps_trailing_script_kite(void) {
    Test test = cassert_init_test("tkbc_remove_non_script_kites_except(trailing script kite)");

    Kite_States kite_array = {0};
    // The script kite sits at the very end of the array.
    bool is_script_kites[] = {false, false, false, true};
    kite_array_setup(&kite_array, is_script_kites, 4);

    tkbc_remove_non_script_kites_except(&kite_array, 99);

    size_t expected_ids[] = {4};
    cassert_bool_eq(kite_array_holds_ids(kite_array, expected_ids, 1), true);

    tkbc_destroy_kite_array(&kite_array);
    return test;
}

Test remove_non_script_kites_except_keeps_trailing_given_id(void) {
    Test test = cassert_init_test("tkbc_remove_non_script_kites_except(trailing given id)");

    Kite_States kite_array = {0};
    // No script kite at all, so only the given kite_id may survive.
    bool is_script_kites[] = {false, false, false};
    kite_array_setup(&kite_array, is_script_kites, 3);

    tkbc_remove_non_script_kites_except(&kite_array, 3);

    size_t expected_ids[] = {3};
    cassert_bool_eq(kite_array_holds_ids(kite_array, expected_ids, 1), true);

    tkbc_destroy_kite_array(&kite_array);
    return test;
}

Test remove_non_script_kites_except_keeps_all_script_kites(void) {
    Test test = cassert_init_test("tkbc_remove_non_script_kites_except(all script kites)");

    Kite_States kite_array = {0};
    bool is_script_kites[] = {true, true, true};
    kite_array_setup(&kite_array, is_script_kites, 3);

    tkbc_remove_non_script_kites_except(&kite_array, 99);

    size_t expected_ids[] = {1, 2, 3};
    cassert_bool_eq(kite_array_holds_ids(kite_array, expected_ids, 3), true);

    tkbc_destroy_kite_array(&kite_array);
    return test;
}

Test remove_non_script_kites_except_handles_empty_and_null_array(void) {
    Test test = cassert_init_test("tkbc_remove_non_script_kites_except(empty and null)");

    Kite_States kite_array = {0};
    tkbc_remove_non_script_kites_except(&kite_array, 1);
    cassert_size_t_eq(kite_array.count, (size_t) 0);

    // Must not crash.
    tkbc_remove_non_script_kites_except(NULL, 1);

    return test;
}

/**
 * @brief Creates a locally controlled kite that can be mirrored by ghosts.
 *
 * @param kite_id The kite id that the target should get.
 * @return The initialized target kite state.
 */
static Kite_State ghost_test_target(size_t kite_id) {
    Kite_State target = tkbc_init_kite();
    target.kite_id = kite_id;
    target.is_active = true;
    target.is_kite_input_handler_active = true;
    target.is_script_kite = true;
    target.kite->center.x = 111.0f;
    target.kite->center.y = 222.0f;
    target.kite->angle = 10.0f;
    target.kite->body_color = (Color){10, 20, 30, 255};
    target.kite->top_color = (Color){40, 50, 60, 255};
    target.kite->texture_id = tkbc_uuid_from_number(kite_id);
    tkbc_kite_update_internal(target.kite);
    return target;
}

/**
 * @brief Searches the ghost array for the ghost of the given remote source.
 *
 * @param env The global state of the application.
 * @param source_kite_id The remote client kite id.
 * @return The matching ghost or NULL if there is none.
 */
static Kite_State *ghost_test_find_by_source(Env *env, int64_t source_kite_id) {
    for (size_t i = 0; i < env->ghost_kites.count; ++i) {
        if (env->ghost_kites.elements[i].ghost_source_kite_id == source_kite_id) {
            return &env->ghost_kites.elements[i];
        }
    }
    return NULL;
}

/**
 * @brief Frees the target kite and all ghosts of a ghost test.
 *
 * @param env The global state of the application.
 * @param target The target kite state.
 */
static void ghost_test_teardown(Env *env, Kite_State *target) {
    tkbc_destroy_kite_array(&env->ghost_kites);
    free(target->kite);
    target->kite = NULL;
}

Test ghost_upsert_copies_appearance_and_remote_transform(void) {
    Test test = cassert_init_test("tkbc_upsert_ghost_kite(copies appearance and remote transform)");

    Env test_env = {0};
    Kite_State target = ghost_test_target(5);

    Kite_State *ghost = tkbc_update_or_create_ghost_kite(&test_env, &target, 7, 300.0f, 400.0f, 30.0f);

    cassert_size_t_eq(test_env.ghost_kites.count, (size_t) 1);
    cassert_bool_eq(ghost != NULL, true);
    if (ghost != NULL) {
        cassert_bool_eq(ghost->is_ghost, true);
        cassert_bool_eq(ghost->is_active, true);
        cassert_size_t_eq(ghost->ghost_target_kite_id, (size_t) 5);
        cassert_int64_t_eq(ghost->ghost_source_kite_id, (int64_t) 7);

        // The remote client is authoritative for position and angle.
        cassert_float_eq(ghost->kite->center.x, 300.0f);
        cassert_float_eq(ghost->kite->center.y, 400.0f);
        cassert_float_eq(ghost->kite->angle, 30.0f);

        // The ghost keeps the appearance of the original kite.
        cassert_bool_eq(tkbc_uuid_equals(ghost->kite->texture_id, target.kite->texture_id), true);
        cassert_struct_eq(ghost->kite->body_color, target.kite->body_color);
        cassert_struct_eq(ghost->kite->top_color, target.kite->top_color);
        cassert_float_eq(ghost->kite->width, target.kite->width);
        cassert_float_eq(ghost->kite->height, target.kite->height);

        // The original kite must not be modified by the ghost.
        cassert_float_eq(target.kite->center.x, 111.0f);
        cassert_float_eq(target.kite->center.y, 222.0f);
        cassert_float_eq(target.kite->angle, 10.0f);

        // The ghost is stored apart from the real kite array.
        cassert_size_t_eq(test_env.kite_array.count, (size_t) 0);
    }

    ghost_test_teardown(&test_env, &target);
    return test;
}

Test ghost_upsert_keeps_one_ghost_per_source(void) {
    Test test = cassert_init_test("tkbc_upsert_ghost_kite(one ghost per source)");

    Env test_env = {0};
    Kite_State target = ghost_test_target(3);

    tkbc_update_or_create_ghost_kite(&test_env, &target, 11, 100.0f, 100.0f, 0.0f);
    cassert_size_t_eq(test_env.ghost_kites.count, (size_t) 1);

    // The same remote client moves its own ghost instead of adding another one.
    tkbc_update_or_create_ghost_kite(&test_env, &target, 11, 150.0f, 160.0f, 20.0f);
    cassert_size_t_eq(test_env.ghost_kites.count, (size_t) 1);

    Kite_State *first = ghost_test_find_by_source(&test_env, 11);
    cassert_bool_eq(first != NULL, true);
    if (first != NULL) {
        cassert_float_eq(first->kite->center.x, 150.0f);
        cassert_float_eq(first->kite->center.y, 160.0f);
        cassert_float_eq(first->kite->angle, 20.0f);
    }

    // A second remote client keeps its own, independent ghost.
    tkbc_update_or_create_ghost_kite(&test_env, &target, 12, 200.0f, 250.0f, 40.0f);
    cassert_size_t_eq(test_env.ghost_kites.count, (size_t) 2);

    Kite_State *second = ghost_test_find_by_source(&test_env, 12);
    cassert_bool_eq(second != NULL, true);
    if (second != NULL) {
        cassert_float_eq(second->kite->center.x, 200.0f);
        cassert_float_eq(second->kite->center.y, 250.0f);
        cassert_float_eq(second->kite->angle, 40.0f);
    }

    ghost_test_teardown(&test_env, &target);
    return test;
}

Test ghost_upsert_refreshes_appearance_of_existing_ghost(void) {
    Test test = cassert_init_test("tkbc_upsert_ghost_kite(refreshes appearance)");

    Env test_env = {0};
    Kite_State target = ghost_test_target(4);

    tkbc_update_or_create_ghost_kite(&test_env, &target, 9, 50.0f, 60.0f, 5.0f);

    // The locally controlled kite changes its design, the next remote update
    // must mirror the new appearance.
    target.kite->body_color = (Color){70, 80, 90, 255};
    target.kite->top_color = (Color){100, 110, 120, 255};
    target.kite->texture_id = tkbc_uuid_from_number(99);

    Kite_State *ghost = tkbc_update_or_create_ghost_kite(&test_env, &target, 9, 70.0f, 80.0f, 15.0f);

    cassert_size_t_eq(test_env.ghost_kites.count, (size_t) 1);
    cassert_bool_eq(ghost != NULL, true);
    if (ghost != NULL) {
        cassert_struct_eq(ghost->kite->body_color, target.kite->body_color);
        cassert_struct_eq(ghost->kite->top_color, target.kite->top_color);
        cassert_bool_eq(tkbc_uuid_equals(ghost->kite->texture_id, target.kite->texture_id), true);
        cassert_float_eq(ghost->kite->center.x, 70.0f);
        cassert_float_eq(ghost->kite->center.y, 80.0f);
    }

    ghost_test_teardown(&test_env, &target);
    return test;
}

Test ghost_upsert_does_not_require_local_control(void) {
    Test test = cassert_init_test("tkbc_upsert_ghost_kite(no local control required)");

    Env test_env = {0};
    Kite_State target = ghost_test_target(6);
    // The ghost also has to be shown on clients that do not control the kite
    // themselves, so the helper must not depend on the input handler flag.
    target.is_kite_input_handler_active = false;

    Kite_State *ghost = tkbc_update_or_create_ghost_kite(&test_env, &target, 21, 123.0f, 234.0f, 45.0f);

    cassert_size_t_eq(test_env.ghost_kites.count, (size_t) 1);
    cassert_bool_eq(ghost != NULL, true);
    if (ghost != NULL) {
        cassert_float_eq(ghost->kite->center.x, 123.0f);
        cassert_float_eq(ghost->kite->center.y, 234.0f);
        cassert_float_eq(ghost->kite->angle, 45.0f);
    }

    ghost_test_teardown(&test_env, &target);
    return test;
}

Test ghost_clear_for_target_keeps_other_targets(void) {
    Test test = cassert_init_test("tkbc_clear_ghost_kites_for_target(keeps other targets)");

    Env test_env = {0};
    Kite_State target_a = ghost_test_target(1);
    Kite_State target_b = ghost_test_target(2);

    tkbc_update_or_create_ghost_kite(&test_env, &target_a, 10, 10.0f, 10.0f, 0.0f);
    tkbc_update_or_create_ghost_kite(&test_env, &target_a, 11, 20.0f, 20.0f, 0.0f);
    tkbc_update_or_create_ghost_kite(&test_env, &target_b, 10, 30.0f, 30.0f, 0.0f);
    cassert_size_t_eq(test_env.ghost_kites.count, (size_t) 3);

    tkbc_clear_ghost_kites_for_target(&test_env, 1);

    cassert_size_t_eq(test_env.ghost_kites.count, (size_t) 1);
    cassert_size_t_eq(test_env.ghost_kites.elements[0].ghost_target_kite_id, (size_t) 2);

    tkbc_destroy_kite_array(&test_env.ghost_kites);
    free(target_a.kite);
    free(target_b.kite);
    return test;
}

Test ghost_clear_for_source_keeps_other_sources(void) {
    Test test = cassert_init_test("tkbc_clear_ghost_kites_for_source(keeps other sources)");

    Env test_env = {0};
    Kite_State target_a = ghost_test_target(1);
    Kite_State target_b = ghost_test_target(2);

    tkbc_update_or_create_ghost_kite(&test_env, &target_a, 10, 10.0f, 10.0f, 0.0f);
    tkbc_update_or_create_ghost_kite(&test_env, &target_b, 10, 20.0f, 20.0f, 0.0f);
    tkbc_update_or_create_ghost_kite(&test_env, &target_a, 11, 30.0f, 30.0f, 0.0f);
    cassert_size_t_eq(test_env.ghost_kites.count, (size_t) 3);

    // The client with kite id 10 disconnected, both of its ghosts are stale.
    tkbc_clear_ghost_kites_for_source(&test_env, 10);

    cassert_size_t_eq(test_env.ghost_kites.count, (size_t) 1);
    cassert_int64_t_eq(test_env.ghost_kites.elements[0].ghost_source_kite_id, (int64_t) 11);

    tkbc_destroy_kite_array(&test_env.ghost_kites);
    free(target_a.kite);
    free(target_b.kite);
    return test;
}

Test ghost_clear_for_target_and_source_keeps_other_sources(void) {
    Test test = cassert_init_test("tkbc_clear_ghost_kites_for_target_and_source(keeps other sources)");

    Env test_env = {0};
    Kite_State target_a = ghost_test_target(1);
    Kite_State target_b = ghost_test_target(2);

    tkbc_update_or_create_ghost_kite(&test_env, &target_a, 10, 10.0f, 10.0f, 0.0f);
    tkbc_update_or_create_ghost_kite(&test_env, &target_a, 11, 20.0f, 20.0f, 0.0f);
    tkbc_update_or_create_ghost_kite(&test_env, &target_b, 10, 30.0f, 30.0f, 0.0f);
    cassert_size_t_eq(test_env.ghost_kites.count, (size_t) 3);

    // Only the ghost of source 10 on target 1 is stale, the other source of
    // the same target and the same source on another target stay.
    tkbc_clear_ghost_kites_for_target_and_source(&test_env, 1, 10);

    cassert_size_t_eq(test_env.ghost_kites.count, (size_t) 2);
    cassert_bool_eq(ghost_test_find_by_source(&test_env, 11) != NULL, true);

    cassert_size_t_eq(test_env.ghost_kites.count, (size_t) 2);
    bool found_target_b = false;
    for (size_t i = 0; i < test_env.ghost_kites.count; ++i) {
        Kite_State *ghost = &test_env.ghost_kites.elements[i];
        if (ghost->ghost_target_kite_id == 2 && ghost->ghost_source_kite_id == 10) {
            found_target_b = true;
        }
    }
    cassert_bool_eq(found_target_b, true);

    tkbc_destroy_kite_array(&test_env.ghost_kites);
    free(target_a.kite);
    free(target_b.kite);
    return test;
}

Test ghost_clear_all_removes_every_ghost(void) {
    Test test = cassert_init_test("tkbc_clear_ghost_kites(removes every ghost)");

    Env test_env = {0};
    Kite_State target_a = ghost_test_target(1);
    Kite_State target_b = ghost_test_target(2);

    tkbc_update_or_create_ghost_kite(&test_env, &target_a, 10, 10.0f, 10.0f, 0.0f);
    tkbc_update_or_create_ghost_kite(&test_env, &target_b, 11, 20.0f, 20.0f, 0.0f);
    cassert_size_t_eq(test_env.ghost_kites.count, (size_t) 2);

    tkbc_clear_ghost_kites(&test_env);
    cassert_size_t_eq(test_env.ghost_kites.count, (size_t) 0);

    // Clearing an empty array and a NULL env must not crash.
    tkbc_clear_ghost_kites(&test_env);
    tkbc_clear_ghost_kites(NULL);

    tkbc_destroy_kite_array(&test_env.ghost_kites);
    free(target_a.kite);
    free(target_b.kite);
    return test;
}

/**
 * @brief Run all kite array unit tests.
 *
 * @param tests Pointer to the Tests struct to register results in.
 */
void tkbc_test_kite_array(Tests *tests) {
    cassert_dap(tests, remove_non_script_kites_except_removes_all_plain_kites());
    cassert_dap(tests, remove_non_script_kites_except_removes_two_plain_kites());
    cassert_dap(tests, remove_non_script_kites_except_keeps_script_kites_and_given_id());
    cassert_dap(tests, remove_non_script_kites_except_keeps_trailing_script_kite());
    cassert_dap(tests, remove_non_script_kites_except_keeps_trailing_given_id());
    cassert_dap(tests, remove_non_script_kites_except_keeps_all_script_kites());
    cassert_dap(tests, remove_non_script_kites_except_handles_empty_and_null_array());
    cassert_dap(tests, ghost_upsert_copies_appearance_and_remote_transform());
    cassert_dap(tests, ghost_upsert_keeps_one_ghost_per_source());
    cassert_dap(tests, ghost_upsert_refreshes_appearance_of_existing_ghost());
    cassert_dap(tests, ghost_upsert_does_not_require_local_control());
    cassert_dap(tests, ghost_clear_for_target_keeps_other_targets());
    cassert_dap(tests, ghost_clear_for_source_keeps_other_sources());
    cassert_dap(tests, ghost_clear_for_target_and_source_keeps_other_sources());
    cassert_dap(tests, ghost_clear_all_removes_every_ghost());
}
