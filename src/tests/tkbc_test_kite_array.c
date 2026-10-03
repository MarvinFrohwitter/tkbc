#include "../../external/cassert/cassert.h"

#include "../choreographer/tkbc.h"
#include "../global/tkbc-types.h"

#include "../../external/space/space.h"

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
}
