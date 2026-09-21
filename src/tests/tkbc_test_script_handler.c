#include "../../external/cassert/cassert.h"

#include "../choreographer/tkbc-script-api.h"
#include "../choreographer/tkbc-script-handler.h"
#include "../choreographer/tkbc.h"
#include "../global/tkbc-types.h"
#include "../global/tkbc-utils.h"
#include <stdlib.h>
#include <string.h>

#include "../../external/space/space.h"
#include "../global/tkbc-utils.h"

Test init_frame(void) {
    Test test = cassert_init_test("tkbc_init_frame()");

    uintptr_t stack = 0;
    Frame *frame = NULL;
    cassert_ptr_neq(&frame, &stack + 0);
    cassert_set_last_cassert_description(&test, "A stack variable and the next one should not have the same stack "
                                                "address.");

    void *frame_before = frame;
    cassert_ptr_eq(frame, frame_before);

    Space space = {.alloc_method = SPACE_METHOD_MALLOC};
    frame = tkbc_init_frame(&space);
    cassert_ptr_neq(frame, NULL);  // To ensure the allocation is valid, otherwise
                                   // the next test is pointless.
    cassert_ptr_neq(frame, frame_before);
    cassert_set_last_cassert_description(&test, "Frame before and after should not be the same.");

    free(frame);
    frame = NULL;

    space_free_space_internals_without_freeing_data(&space);
    return test;
}

Test get_kite_by_id(void) {
    Test test = cassert_init_test("tkbc_get_kite_by_id()");
    Env *env = tkbc_init_env();
    Kite_State kite_state0 = tkbc_init_kite();
    kite_state0.kite_id = 0;
    Kite_State kite_state1 = tkbc_init_kite();
    kite_state1.kite_id = 1;
    Kite_State kite_state2 = tkbc_init_kite();
    kite_state2.kite_id = 2;
    tkbc_dap(&env->kite_array, kite_state0);
    tkbc_dap(&env->kite_array, kite_state1);
    tkbc_dap(&env->kite_array, kite_state2);
    cassert_size_t_eq(env->kite_array.count, 3);

    Kite *kite = tkbc_get_kite_by_id(env, 0);
    cassert_ptr_eq(kite_state0.kite, kite);

    kite = tkbc_get_kite_by_id(env, kite_state0.kite_id);
    cassert_ptr_eq(kite_state0.kite, kite);
    kite = tkbc_get_kite_by_id(env, kite_state1.kite_id);
    cassert_ptr_eq(kite_state1.kite, kite);
    kite = tkbc_get_kite_by_id(env, kite_state2.kite_id);
    cassert_ptr_eq(kite_state2.kite, kite);

    kite = tkbc_get_kite_by_id(env, kite_state0.kite_id + 100);
    cassert_ptr_eq(NULL, kite);

    tkbc_destroy_env(env);
    return test;
}

Test get_kite_state_by_id(void) {
    Test test = cassert_init_test("tkbc_get_kite_state_by_id()");
    Env *env = tkbc_init_env();
    Kite_State kite_state0 = tkbc_init_kite();
    kite_state0.kite_id = 0;
    Kite_State kite_state1 = tkbc_init_kite();
    kite_state1.kite_id = 1;
    Kite_State kite_state2 = tkbc_init_kite();
    kite_state2.kite_id = 2;
    tkbc_dap(&env->kite_array, kite_state0);
    tkbc_dap(&env->kite_array, kite_state1);
    tkbc_dap(&env->kite_array, kite_state2);
    cassert_size_t_eq(env->kite_array.count, 3);

    Kite_State *ret_kite_state = tkbc_get_kite_state_by_id(env, 0);
    cassert_ptr_neq(&kite_state0, ret_kite_state);
    cassert_ptr_eq(kite_state0.kite, ret_kite_state->kite);
    cassert_size_t_eq(kite_state0.kite_id, ret_kite_state->kite_id);

    ret_kite_state = tkbc_get_kite_state_by_id(env, kite_state0.kite_id);
    cassert_ptr_neq(&kite_state0, ret_kite_state);
    cassert_ptr_eq(kite_state0.kite, ret_kite_state->kite);
    cassert_size_t_eq(kite_state0.kite_id, ret_kite_state->kite_id);

    ret_kite_state = tkbc_get_kite_state_by_id(env, kite_state1.kite_id);
    cassert_ptr_neq(&kite_state1, ret_kite_state);
    cassert_ptr_eq(kite_state1.kite, ret_kite_state->kite);
    cassert_size_t_eq(kite_state1.kite_id, ret_kite_state->kite_id);

    ret_kite_state = tkbc_get_kite_state_by_id(env, kite_state2.kite_id);
    cassert_ptr_neq(&kite_state2, ret_kite_state);
    cassert_ptr_eq(kite_state2.kite, ret_kite_state->kite);
    cassert_size_t_eq(kite_state2.kite_id, ret_kite_state->kite_id);

    ret_kite_state = tkbc_get_kite_state_by_id(env, kite_state0.kite_id + 100);
    cassert_ptr_eq(NULL, ret_kite_state);

    tkbc_destroy_env(env);
    return test;
}

Test contains_id(void) {
    Test test = cassert_init_test("tkbc_contains_id()");
    Kite_Ids kite_ids = tkbc_indexs_range(4, 13);
    cassert_size_t_eq(kite_ids.count, 9);

    bool contains = tkbc_contains_id(kite_ids, 12);
    cassert_bool_eq(contains, true);
    contains = tkbc_contains_id(kite_ids, 13);
    cassert_bool_neq(contains, true);
    contains = tkbc_contains_id(kite_ids, 0);
    cassert_bool_neq(contains, true);
    contains = tkbc_contains_id(kite_ids, 1);
    cassert_bool_neq(contains, true);
    contains = tkbc_contains_id(kite_ids, 2);
    cassert_bool_neq(contains, true);
    contains = tkbc_contains_id(kite_ids, 3);
    cassert_bool_neq(contains, true);

    contains = tkbc_contains_id(kite_ids, 4);
    cassert_bool_eq(contains, true);
    contains = tkbc_contains_id(kite_ids, 8);
    cassert_bool_eq(contains, true);
    contains = tkbc_contains_id(kite_ids, 9);
    cassert_bool_eq(contains, true);

    free(kite_ids.elements);
    return test;
}

Test deep_copy_frame(void) {
    Test test = cassert_init_test("tkbc_deep_copy_frame()");
    Space space = {.alloc_method = SPACE_METHOD_MALLOC};
    Frame *frame = tkbc_init_frame(&space);
    frame->kind = ACTION_KITE_WAIT;
    Kite_Ids kite_ids = tkbc_indexs_range(0, 8);
    tkbc_dapc(&frame->kite_id_array, kite_ids.elements, kite_ids.count);
    Frame frame_copy = tkbc_deep_copy_frame(&space, frame);
    cassert_size_t_eq(frame->kite_id_array.count, 8);

    cassert_ptr_neq(frame, &frame_copy);
    cassert_ptr_neq(frame->kite_id_array.elements, &frame_copy.kite_id_array.elements);

    cassert_bool_eq(frame->finished, frame_copy.finished);
    cassert_int_eq(frame->kind, frame_copy.kind);
    cassert_float_eq(frame->duration, frame_copy.duration);
    cassert_float_eq(frame->original_duration, frame_copy.original_duration);
    cassert_size_t_eq(frame->index, frame_copy.index);

    free(kite_ids.elements);
    kite_ids.elements = NULL;

    free(frame_copy.kite_id_array.elements);
    frame_copy.kite_id_array.elements = NULL;

    free(frame->kite_id_array.elements);
    frame->kite_id_array.elements = NULL;

    free(frame);
    frame = NULL;

    space_free_space_internals_without_freeing_data(&space);
    return test;
}

Test deep_copy_frames(void) {
    Test test = cassert_init_test("tkbc_deep_copy_frames()");

    Frames *frames = malloc(sizeof(*frames));
    if (frames == NULL) {
        tkbc_fprintf(stderr, "ERROR", "No more memory can be allocated.\n");
        abort();
    }
    memset(frames, 0, sizeof(*frames));
    Frame *frame = malloc(sizeof(*frame));
    if (frame == NULL) {
        tkbc_fprintf(stderr, "ERROR", "No more memory can be allocated.\n");
        abort();
    }
    memset(frame, 0, sizeof(*frame));

    frame->kite_id_array = tkbc_indexs_range(1, 3);
    frame->kind = ACTION_KITE_MOVE;
    frame->action.as_move.position = (Vector2){.x = 300, .y = 400};
    cassert_dap(frames, *frame);
    cassert_dap(&frames->kite_frame_positions,
                ((Kite_Position){.kite_id = 1, .position.x = 100, .position.y = 200, .angle = 90}));
    cassert_dap(&frames->kite_frame_positions,
                ((Kite_Position){.kite_id = 2, .position.x = 150, .position.y = 250, .angle = 180}));

    Space space = {.alloc_method = SPACE_METHOD_MALLOC};
    Frames new_frames = tkbc_deep_copy_frames(&space, frames);

    cassert_ptr_neq(frames, &new_frames);
    cassert_int_eq(frames->count, new_frames.count);
    cassert_int_eq(frames->capacity, new_frames.capacity);
    cassert_int_eq(frames->frames_index, new_frames.frames_index);
    cassert_ptr_neq(&frames->kite_frame_positions, &new_frames.kite_frame_positions);

    cassert_ptr_neq(frames->kite_frame_positions.elements, new_frames.kite_frame_positions.elements);
    cassert_int_eq(frames->kite_frame_positions.elements->kite_id, new_frames.kite_frame_positions.elements->kite_id);
    cassert_float_eq(frames->kite_frame_positions.elements->position.x,
                     new_frames.kite_frame_positions.elements->position.x);
    cassert_float_eq(frames->kite_frame_positions.elements->position.y,
                     new_frames.kite_frame_positions.elements->position.y);
    cassert_float_eq(frames->kite_frame_positions.elements->angle, new_frames.kite_frame_positions.elements->angle);

    cassert_ptr_neq(frames->elements, new_frames.elements);
    cassert_ptr_neq(&frames->elements->kite_id_array, &new_frames.elements->kite_id_array);
    cassert_ptr_neq(frames->elements->kite_id_array.elements, new_frames.elements->kite_id_array.elements);

    cassert_int_eq(frames->elements->kite_id_array.count, new_frames.elements->kite_id_array.count);
    cassert_int_eq(frames->elements->kite_id_array.capacity, new_frames.elements->kite_id_array.capacity);

    cassert_bool_eq(frames->elements->finished, new_frames.elements->finished);
    cassert_int_eq(frames->elements->index, new_frames.elements->index);
    cassert_int_eq(frames->elements->kind, new_frames.elements->kind);
    cassert_float_eq(frames->elements->duration, new_frames.elements->duration);
    cassert_float_eq(frames->elements->action.as_move.position.x, new_frames.elements->action.as_move.position.x);
    cassert_float_eq(frames->elements->action.as_move.position.y, new_frames.elements->action.as_move.position.y);

    free(frame);

    free(frames->elements->kite_id_array.elements);
    free(frames->elements);
    free(frames->kite_frame_positions.elements);
    free(frames);

    free(new_frames.elements->kite_id_array.elements);
    free(new_frames.elements);
    free(new_frames.kite_frame_positions.elements);
    space_free_space_internals_without_freeing_data(&space);
    return test;
}

Test deep_copy_script(void) {
    Test test = cassert_init_test("deep_copy_script()");
    Script script = {0};
    Frames frames = {0};
    Frame frame = {0};

    frame.kite_id_array = tkbc_indexs_range(0, 3);
    frame.kind = ACTION_KITE_MOVE;
    frame.action.as_move.position = (Vector2){.x = 300, .y = 400};
    cassert_dap(&frames, frame);

    cassert_dap(&frames.kite_frame_positions,
                ((Kite_Position){.kite_id = 1, .position.x = 100, .position.y = 200, .angle = 90}));
    cassert_dap(&frames.kite_frame_positions,
                ((Kite_Position){.kite_id = 2, .position.x = 150, .position.y = 250, .angle = 180}));

    cassert_dap(&script, frames);

    Space space = {.alloc_method = SPACE_METHOD_MALLOC};
    Script new_script = tkbc_deep_copy_script(&space, &script);
    cassert_ptr_neq(&script, &new_script);
    cassert_int_eq(script.count, new_script.count);
    cassert_int_eq(script.capacity, new_script.capacity);
    cassert_bool_eq(script.was_send, new_script.was_send);

    cassert_type_compare_function(ANY_ALLOCED_EQ, script.id, new_script.id, tkbc_uuid_equals);

    // cassert_type_compare_function_param(ANY_ALLOCED_EQ, &script.name_input, &new_script.name_input, memcmp, sizeof(script.name_input));

    // The name buffer is deep copied: same content, different allocation.
    // The name is stored only in name_input.text; deep copy owns its own allocation.
    cassert_size_t_eq(script.name_input.text.count, new_script.name_input.text.count);
    cassert_size_t_eq(script.name_input.max_char, new_script.name_input.max_char);
    if (script.name_input.text.elements || new_script.name_input.text.elements) {
        cassert_ptr_neq(script.name_input.text.elements, new_script.name_input.text.elements);
        cassert_string_eq(script.name_input.text.elements ? script.name_input.text.elements : "",
                          new_script.name_input.text.elements ? new_script.name_input.text.elements : "");
    }

    cassert_ptr_neq(&script.elements, &new_script.elements);

    cassert_ptr_neq(script.elements, new_script.elements);

    for (size_t i = 0; i < script.count; ++i) {
        tkbc_destroy_frames_internal_data(&script.elements[i]);
    }
    for (size_t i = 0; i < new_script.count; ++i) {
        tkbc_destroy_frames_internal_data(&new_script.elements[i]);
    }
    free(script.elements);
    free(new_script.elements);
    script.elements = NULL;
    new_script.elements = NULL;
    cassert_ptr_eq(script.elements, NULL);
    cassert_ptr_eq(new_script.elements, NULL);

    space_free_space_internals_without_freeing_data(&space);
    return test;
}

Test destroy_frames_internal_data(void) {
    Test test = cassert_init_test("tkbc_destroy_frames_internal_data()");
    Frames frames = {0};
    Frame frame = {0};

    frame.kite_id_array = tkbc_indexs_range(0, 3);
    frame.kind = ACTION_KITE_MOVE;
    frame.action.as_move.position = (Vector2){.x = 300, .y = 400};
    cassert_dap(&frames, frame);

    cassert_dap(&frames.kite_frame_positions,
                ((Kite_Position){.kite_id = 1, .position.x = 100, .position.y = 200, .angle = 90}));
    cassert_dap(&frames.kite_frame_positions,
                ((Kite_Position){.kite_id = 2, .position.x = 150, .position.y = 250, .angle = 180}));

    cassert_ptr_neq(frames.elements, NULL);
    cassert_size_t_neq(frames.count, 0);
    cassert_size_t_neq(frames.capacity, 0);
    cassert_ptr_neq(frames.kite_frame_positions.elements, NULL);
    cassert_size_t_neq(frames.kite_frame_positions.count, 0);
    cassert_size_t_neq(frames.kite_frame_positions.capacity, 0);

    Kite_Ids *ids = &frames.elements->kite_id_array;

    cassert_ptr_neq(frames.elements->kite_id_array.elements, NULL);
    cassert_size_t_neq(frames.elements->kite_id_array.count, 0);
    cassert_size_t_neq(frames.elements->kite_id_array.capacity, 0);

    tkbc_destroy_frames_internal_data(&frames);

    {
        // This data is not guaranteed to be the expected values. Use after free.
        // cassert_ptr_eq(ids->elements, NULL);

        cassert_ptr_neq(ids, NULL);
        // cassert_size_t_neq(ids->count, 3);
        // cassert_size_t_eq(ids->capacity, 0);
    }

    cassert_ptr_eq(frames.elements, NULL);
    cassert_size_t_eq(frames.count, 0);
    cassert_size_t_eq(frames.capacity, 0);

    cassert_ptr_eq(frames.kite_frame_positions.elements, NULL);
    cassert_size_t_eq(frames.kite_frame_positions.count, 0);
    cassert_size_t_eq(frames.kite_frame_positions.capacity, 0);

    return test;
}

Test reset_frames_internal_data(void) {
    Test test = cassert_init_test("tkbc_reset_frames_internal_data()");
    Frames frames = {0};
    Frame frame = {0};

    frame.kite_id_array = tkbc_indexs_range(0, 3);
    frame.kind = ACTION_KITE_MOVE;
    frame.action.as_move.position = (Vector2){.x = 300, .y = 400};
    cassert_dap(&frames, frame);

    cassert_dap(&frames.kite_frame_positions,
                ((Kite_Position){.kite_id = 1, .position.x = 100, .position.y = 200, .angle = 90}));
    cassert_dap(&frames.kite_frame_positions,
                ((Kite_Position){.kite_id = 2, .position.x = 150, .position.y = 250, .angle = 180}));

    cassert_ptr_neq(frames.elements, NULL);
    cassert_size_t_neq(frames.count, 0);
    cassert_size_t_neq(frames.capacity, 0);
    cassert_ptr_neq(frames.kite_frame_positions.elements, NULL);
    cassert_size_t_neq(frames.kite_frame_positions.count, 0);
    cassert_size_t_neq(frames.kite_frame_positions.capacity, 0);

    Kite_Ids *ids = &frames.elements->kite_id_array;

    cassert_ptr_neq(frames.elements->kite_id_array.elements, NULL);
    cassert_size_t_neq(frames.elements->kite_id_array.count, 0);
    cassert_size_t_neq(frames.elements->kite_id_array.capacity, 0);

    size_t hack_tracking_count_number_for_memory_managment = frames.count;
    tkbc_reset_frames_internal_data(&frames);

    cassert_ptr_neq(ids, NULL);
    cassert_ptr_neq(ids->elements, NULL);
    cassert_size_t_neq(ids->count, 3);
    cassert_size_t_eq(ids->count, 0);
    cassert_size_t_eq(ids->capacity, 0);

    cassert_ptr_neq(frames.elements, NULL);
    cassert_size_t_eq(frames.count, 0);
    cassert_size_t_neq(frames.capacity, 0);

    cassert_ptr_neq(frames.kite_frame_positions.elements, NULL);
    cassert_size_t_eq(frames.kite_frame_positions.count, 0);
    cassert_size_t_neq(frames.kite_frame_positions.capacity, 0);

    frames.count = hack_tracking_count_number_for_memory_managment;
    tkbc_destroy_frames_internal_data(&frames);
    return test;
}

Test calculate_script_byte_size_allocated(void) {
    Test test = cassert_init_test("tkbc_calculate_script_byte_size_allocated()");

    Script script = {0};
    size_t calculated_size = tkbc_calculate_script_byte_size_allocated(script);

    size_t basic_struct_size = sizeof(Script);
    cassert_size_t_eq(calculated_size, 0);
    cassert_size_t_neq(calculated_size, basic_struct_size);
    cassert_set_last_cassert_description(&test, "For empty script, calculated size should equal struct size.");

    return test;
}

static void upscale_test_build_script(Script *script, Id kite_id, Action_Kind kind, Action action, float duration,
                                        Vector2 start_pos, float start_angle) {
    memset(script, 0, sizeof(*script));
    space_init_capacity(&script->space, 64);
    Frames block = {0};
    Frame f = {0};
    f.kind = kind;
    f.action = action;
    f.duration = duration;
    f.original_duration = duration;
    f.finished = false;
    f.index = 0;
    if (kind != ACTION_KITE_WAIT && kind != ACTION_KITE_QUIT) {
        space_dap(&script->space, &f.kite_id_array, kite_id);
    }
    space_dap(&script->space, &block, f);
    block.frames_index = 0;
    if (kind != ACTION_KITE_WAIT && kind != ACTION_KITE_QUIT) {
        Kite_Position kp = {.kite_id = kite_id, .position = start_pos, .angle = start_angle};
        space_dap(&script->space, &block.kite_frame_positions, kp);
    }
    space_dap(&script->space, script, block);
}

Test upscale_move_absolute(void) {
    Test test = cassert_init_test("tkbc_upscale_script(MOVE)");
    Env *env = tkbc_init_env();
    Kite_State s0 = tkbc_init_kite();
    s0.kite_id = 0;
    s0.is_active = true;
    Vector2 start = {.x = 0, .y = 0};
    tkbc_center_rotation(s0.kite, &start, 0);
    s0.kite->old_center = s0.kite->center;
    s0.kite->old_angle = s0.kite->angle;
    tkbc_dap(&env->kite_array, s0);

    Space space = {0};
    space_init_capacity(&space, 64);
    Action a = {0};
    a.as_move.position = (Vector2){.x = 60, .y = 0};
    Script script = {0};
    upscale_test_build_script(&script, 0, ACTION_KITE_MOVE, a, 1.0f, start, 0);

    tkbc_upscale_script(env, &script, 60.0f);
    cassert_size_t_eq(script.count, 60);
    // First slice moves 1/60th of the way, last slice reaches the target.
    cassert_float_eq(script.elements[0].elements[0].action.as_move.position.x, 1.0f);
    cassert_float_eq(script.elements[59].elements[0].action.as_move.position.x, 60.0f);
    cassert_float_eq(script.elements[0].elements[0].duration, 1.0f / 60.0f);
    // Scrub to the middle lands on the baked start of that slice.
    env->script = &script;
    env->frames = &script.elements[0];
    tkbc_scrub_to_index(env, 30);
    Kite *k = tkbc_get_kite_by_id(env, 0);
    cassert_float_eq(k->center.x, 30.0f);

    // Idempotent second pass keeps the count.
    tkbc_upscale_script(env, &script, 60.0f);
    cassert_size_t_eq(script.count, 60);

    space_free_space(&script.space);
    tkbc_destroy_env(env);
    return test;
}

Test upscale_move_add(void) {
    Test test = cassert_init_test("tkbc_upscale_script(MOVE_ADD)");
    Env *env = tkbc_init_env();
    Kite_State s0 = tkbc_init_kite();
    s0.kite_id = 0;
    s0.is_active = true;
    Vector2 start = {.x = 0, .y = 0};
    tkbc_center_rotation(s0.kite, &start, 0);
    s0.kite->old_center = s0.kite->center;
    s0.kite->old_angle = s0.kite->angle;
    tkbc_dap(&env->kite_array, s0);

    Space space = {0};
    space_init_capacity(&space, 64);
    Action a = {0};
    a.as_move_add.position = (Vector2){.x = 60, .y = 0};
    Script script = {0};
    upscale_test_build_script(&script, 0, ACTION_KITE_MOVE_ADD, a, 1.0f, start, 0);

    tkbc_upscale_script(env, &script, 60.0f);
    cassert_size_t_eq(script.count, 60);
    // Relative offsets stay start-independent: each slice adds (1,0).
    cassert_float_eq(script.elements[0].elements[0].action.as_move_add.position.x, 1.0f);
    cassert_int_eq(script.elements[0].elements[0].kind, ACTION_KITE_MOVE_ADD);

    space_free_space(&script.space);
    tkbc_destroy_env(env);
    return test;
}

Test upscale_wait(void) {
    Test test = cassert_init_test("tkbc_upscale_script(WAIT)");
    Env *env = tkbc_init_env();

    Script script = {0};
    space_init_capacity(&script.space, 64);
    Frames block = {0};
    Frame f = {0};
    f.kind = ACTION_KITE_WAIT;
    f.duration = 0.5f;
    f.original_duration = 0.5f;
    f.index = 0;
    space_dap(&script.space, &block, f);
    block.frames_index = 0;
    space_dap(&script.space, &script, block);

    tkbc_upscale_script(env, &script, 60.0f);
    cassert_size_t_eq(script.count, 30);

    space_free_space(&script.space);
    tkbc_destroy_env(env);
    return test;
}

Test upscale_rotation_add(void) {
    Test test = cassert_init_test("tkbc_upscale_script(ROTATION_ADD)");
    Env *env = tkbc_init_env();
    Kite_State s0 = tkbc_init_kite();
    s0.kite_id = 0;
    s0.is_active = true;
    Vector2 start = {.x = 0, .y = 0};
    tkbc_center_rotation(s0.kite, &start, 0);
    s0.kite->old_center = s0.kite->center;
    s0.kite->old_angle = s0.kite->angle;
    tkbc_dap(&env->kite_array, s0);

    Script script = {0};
    space_init_capacity(&script.space, 64);
    Frames block = {0};
    Frame f = {0};
    f.kind = ACTION_KITE_ROTATION_ADD;
    f.action.as_rotation_add.angle = 90.0f;
    f.duration = 1.0f;
    f.original_duration = 1.0f;
    f.index = 0;
    space_dap(&script.space, &f.kite_id_array, (Id) 0);
    space_dap(&script.space, &block, f);
    block.frames_index = 0;
    Kite_Position kp = {.kite_id = 0, .position = start, .angle = 0};
    space_dap(&script.space, &block.kite_frame_positions, kp);
    space_dap(&script.space, &script, block);

    tkbc_upscale_script(env, &script, 60.0f);
    cassert_size_t_eq(script.count, 60);
    cassert_bool_eq(fabsf(script.elements[0].elements[0].action.as_rotation_add.angle - 1.5f) < 0.001f, true);

    space_free_space(&script.space);
    tkbc_destroy_env(env);
    return test;
}

static Env *scrub_test_setup_env(Script *script) {
    Env *env = tkbc_init_env();
    Kite_State s0 = tkbc_init_kite();
    s0.kite_id = 0;
    s0.is_active = true;
    s0.is_script_kite = true;
    Vector2 start = {.x = 0, .y = 0};
    tkbc_center_rotation(s0.kite, &start, 0);
    s0.kite->old_center = s0.kite->center;
    s0.kite->old_angle = s0.kite->angle;
    tkbc_dap(&env->kite_array, s0);

    Action a = {0};
    a.as_move.position = (Vector2){.x = 60, .y = 0};
    upscale_test_build_script(script, 0, ACTION_KITE_MOVE, a, 1.0f, start, 0);
    tkbc_upscale_script(env, script, 60.0f);

    env->script = script;
    env->frames = &script->elements[0];
    env->script_finished = false;
    return env;
}

Test scrub_stays_in_script_mode(void) {
    Test test = cassert_init_test("scrub stays inside script mode");
    Script script = {0};
    Env *env = scrub_test_setup_env(&script);

    // Scrub to start, end, and past the end: the script must stay loaded,
    // paused, with script kites still presented (no unload, no visibility
    // flip to non-script kites).
    tkbc_scrub_to_index(env, 0);
    cassert_ptr_neq(env->script, NULL);
    cassert_ptr_neq(env->frames, NULL);
    cassert_size_t_eq(env->frames->frames_index, 0);
    cassert_bool_eq(env->script_finished, true);
    cassert_bool_eq(env->kite_array.elements[0].is_active, true);

    tkbc_scrub_to_index(env, script.count - 1);
    cassert_ptr_neq(env->script, NULL);
    cassert_ptr_neq(env->frames, NULL);
    cassert_size_t_eq(env->frames->frames_index, script.count - 1);
    cassert_bool_eq(env->script_finished, true);
    cassert_bool_eq(env->kite_array.elements[0].is_active, true);
    cassert_bool_eq(fabsf(tkbc_get_kite_by_id(env, 0)->center.x - 59.0f) < 0.01f, true);

    // Past-the-end clamps into range instead of finishing/unloading.
    tkbc_scrub_to_index(env, script.count + 100);
    cassert_ptr_neq(env->script, NULL);
    cassert_ptr_neq(env->frames, NULL);
    cassert_size_t_eq(env->frames->frames_index, script.count - 1);
    cassert_bool_eq(env->kite_array.elements[0].is_active, true);

    space_free_space(&script.space);
    tkbc_destroy_env(env);
    return test;
}

Test finish_stays_loaded(void) {
    Test test = cassert_init_test("natural finish stays loaded");
    Script script = {0};
    Env *env = scrub_test_setup_env(&script);

    // Simulate all frames done on the final slice, then run the finish path.
    env->frames = &script.elements[script.count - 1];
    for (size_t j = 0; j < env->frames->count; ++j) {
        env->frames->elements[j].finished = true;
    }
    env->script_finished = false;
    tkbc_script_update_frames(env);

    // Finished, but still loaded in script mode: only an explicit NO SCRIPT
    // (unload) may terminate execution.
    cassert_bool_eq(env->script_finished, true);
    cassert_ptr_neq(env->script, NULL);
    cassert_ptr_neq(env->frames, NULL);

    space_free_space(&script.space);
    tkbc_destroy_env(env);
    return test;
}

Test toggle_at_end_clamps(void) {
    Test test = cassert_init_test("toggle at end clamps, no wrap");
    Script script = {0};
    Env *env = scrub_test_setup_env(&script);

    // Scrub-paused at the final slice, then toggle to play: resumes in place
    // at the end (clamped, no wrap-around to the start) and stays loaded.
    tkbc_scrub_to_index(env, script.count - 1);
    tkbc_toggle_script_execution(env);
    cassert_size_t_eq(env->frames->frames_index, script.count - 1);
    cassert_bool_eq(env->script_finished, false);
    cassert_ptr_neq(env->script, NULL);
    cassert_ptr_neq(env->frames, NULL);

    // Plain pause/resume in the middle is a simple flip.
    tkbc_scrub_to_index(env, 10);
    tkbc_toggle_script_execution(env);  // paused -> resume
    cassert_bool_eq(env->script_finished, false);
    cassert_size_t_eq(env->frames->frames_index, 10);
    tkbc_toggle_script_execution(env);  // playing -> pause
    cassert_bool_eq(env->script_finished, true);
    cassert_size_t_eq(env->frames->frames_index, 10);

    space_free_space(&script.space);
    tkbc_destroy_env(env);
    return test;
}

static void bake_test_build_two_block_script(Script *script, Vector2 stale_start) {
    memset(script, 0, sizeof(*script));
    space_init_capacity(&script->space, 128);
    // Block 0: MOVE (0,0) -> (60,0) in 1s, correct stored start.
    {
        Frames block = {0};
        Frame f = {0};
        f.kind = ACTION_KITE_MOVE;
        f.action.as_move.position = (Vector2){.x = 60, .y = 0};
        f.duration = 1.0f;
        f.original_duration = 1.0f;
        f.index = 0;
        space_dap(&script->space, &f.kite_id_array, (Id) 0);
        space_dap(&script->space, &block, f);
        block.frames_index = 0;
        Kite_Position kp = {.kite_id = 0, .position = {.x = 0, .y = 0}, .angle = 0};
        space_dap(&script->space, &block.kite_frame_positions, kp);
        space_dap(&script->space, script, block);
    }
    // Block 1: MOVE -> (120,0) in 1s, but with a STALE stored start as seen
    // before any play (e.g. patched from an unrelated kite position).
    {
        Frames block = {0};
        Frame f = {0};
        f.kind = ACTION_KITE_MOVE;
        f.action.as_move.position = (Vector2){.x = 120, .y = 0};
        f.duration = 1.0f;
        f.original_duration = 1.0f;
        f.index = 0;
        space_dap(&script->space, &f.kite_id_array, (Id) 0);
        space_dap(&script->space, &block, f);
        block.frames_index = 1;
        Kite_Position kp = {.kite_id = 0, .position = stale_start, .angle = 0};
        space_dap(&script->space, &block.kite_frame_positions, kp);
        space_dap(&script->space, script, block);
    }
}

Test bake_fixes_stale_starts(void) {
    Test test = cassert_init_test("bake fixes stale starts without playing");
    Env *env = tkbc_init_env();
    Kite_State s0 = tkbc_init_kite();
    s0.kite_id = 0;
    s0.is_active = true;
    s0.is_script_kite = true;
    Vector2 start = {.x = 0, .y = 0};
    tkbc_center_rotation(s0.kite, &start, 0);
    s0.kite->old_center = s0.kite->center;
    s0.kite->old_angle = s0.kite->angle;
    tkbc_dap(&env->kite_array, s0);

    Script script = {0};
    bake_test_build_two_block_script(&script, (Vector2){.x = 999, .y = 0});

    // No play happened: block 1 still claims a stale start. The eager bake
    // (server after receive / offline client at load) must compute the true
    // chain end of block 0 instead.
    tkbc_bake_script_timeline(env, &script);
    cassert_bool_eq(fabsf(script.elements[0].kite_frame_positions.elements[0].position.x - 0.0f) < 0.01f, true);
    cassert_bool_eq(fabsf(script.elements[1].kite_frame_positions.elements[0].position.x - 60.0f) < 0.5f, true);

    // The env kites and the script runtime state are untouched by baking.
    cassert_float_eq(tkbc_get_kite_by_id(env, 0)->center.x, 0.0f);
    cassert_bool_eq(script.elements[0].elements[0].finished, false);

    space_free_space(&script.space);
    tkbc_destroy_env(env);
    return test;
}

Test bake_upscaled_slice_chain(void) {
    Test test = cassert_init_test("bake repairs upscaled slice starts");
    Script script = {0};
    Env *env = scrub_test_setup_env(&script);

    // Corrupt one slice start in the middle, then bake: the deterministic
    // simulation must restore the continuous chain without any play.
    script.elements[30].kite_frame_positions.elements[0].position.x = 999.0f;
    tkbc_bake_script_timeline(env, &script);
    cassert_bool_eq(fabsf(script.elements[30].kite_frame_positions.elements[0].position.x - 30.0f) < 0.5f, true);
    cassert_bool_eq(fabsf(script.elements[59].kite_frame_positions.elements[0].position.x - 59.0f) < 0.5f, true);

    space_free_space(&script.space);
    tkbc_destroy_env(env);
    return test;
}

static void quit_test_build_script(Script *script) {
    // One block: MOVE (0,0) -> (12,0) in 0.2s plus QUIT 0.5s. The QUIT is
    // timing-irrelevant next to motion (it finishes alongside it), so the
    // block must upscale from the MOVE duration only.
    memset(script, 0, sizeof(*script));
    space_init_capacity(&script->space, 128);
    Frames block = {0};
    Frame move = {0};
    move.kind = ACTION_KITE_MOVE;
    move.action.as_move.position = (Vector2){.x = 12, .y = 0};
    move.duration = 0.2f;
    move.original_duration = 0.2f;
    move.index = 0;
    space_dap(&script->space, &move.kite_id_array, (Id) 0);
    space_dap(&script->space, &block, move);
    Frame quit = {0};
    quit.kind = ACTION_KITE_QUIT;
    quit.duration = 0.5f;
    quit.original_duration = 0.5f;
    quit.index = 1;
    space_dap(&script->space, &block, quit);
    block.frames_index = 0;
    Kite_Position kp = {.kite_id = 0, .position = {.x = 0, .y = 0}, .angle = 0};
    space_dap(&script->space, &block.kite_frame_positions, kp);
    space_dap(&script->space, script, block);
}

static bool slice_contains_quit(const Frames *slice) {
    for (size_t i = 0; i < slice->count; ++i) {
        if (slice->elements[i].kind == ACTION_KITE_QUIT) {
            return true;
        }
    }
    return false;
}

Test upscale_quit_mixed_block(void) {
    Test test = cassert_init_test("upscale drops QUIT from slices");
    Env *env = tkbc_init_env();
    Kite_State s0 = tkbc_init_kite();
    s0.kite_id = 0;
    s0.is_active = true;
    s0.is_script_kite = true;
    Vector2 start = {.x = 0, .y = 0};
    tkbc_center_rotation(s0.kite, &start, 0);
    s0.kite->old_center = s0.kite->center;
    s0.kite->old_angle = s0.kite->angle;
    tkbc_dap(&env->kite_array, s0);

    Script script = {0};
    quit_test_build_script(&script);
    tkbc_upscale_script(env, &script, 60.0f);

    // 0.2s of motion, not 0.5s of QUIT: 12 slices, none a lone QUIT that
    // would share and exhaust the global quit countdown.
    cassert_size_t_eq(script.count, 12);
    for (size_t i = 0; i < script.count; ++i) {
        cassert_bool_eq(slice_contains_quit(&script.elements[i]), false);
    }

    space_free_space(&script.space);
    tkbc_destroy_env(env);
    return test;
}

Test upscale_quit_block_plays_through(void) {
    Test test = cassert_init_test("mixed QUIT block plays through");
    Env *env = tkbc_init_env();
    Kite_State s0 = tkbc_init_kite();
    s0.kite_id = 0;
    s0.is_active = true;
    s0.is_script_kite = true;
    Vector2 start = {.x = 0, .y = 0};
    tkbc_center_rotation(s0.kite, &start, 0);
    s0.kite->old_center = s0.kite->center;
    s0.kite->old_angle = s0.kite->angle;
    tkbc_dap(&env->kite_array, s0);

    Script script = {0};
    quit_test_build_script(&script);
    tkbc_upscale_script(env, &script, 60.0f);
    tkbc_bake_script_timeline(env, &script);

    // Play with realistic frame times: the script must run through all
    // slices to its natural end instead of dying early via the global quit.
    env->script = &script;
    env->frames = &script.elements[0];
    env->script_finished = false;
    for (int i = 0; i < 60 && !tkbc_script_finished(env); ++i) {
        tkbc_make_frame_time(TARGET_DT);
        tkbc_script_update_frames(env);
    }
    cassert_bool_eq(tkbc_script_finished(env), true);
    cassert_size_t_eq(env->frames->frames_index, script.count - 1);
    cassert_ptr_neq(env->script, NULL);
    cassert_ptr_neq(env->frames, NULL);

    space_free_space(&script.space);
    tkbc_destroy_env(env);
    return test;
}

Test quit_alone_acts_global(void) {
    Test test = cassert_init_test("lone QUIT acts global");
    Env *env = tkbc_init_env();
    Kite_State s0 = tkbc_init_kite();
    s0.kite_id = 0;
    s0.is_active = true;
    s0.is_script_kite = true;
    Vector2 start = {.x = 0, .y = 0};
    tkbc_center_rotation(s0.kite, &start, 0);
    s0.kite->old_center = s0.kite->center;
    s0.kite->old_angle = s0.kite->angle;
    tkbc_dap(&env->kite_array, s0);

    // Block 0: lone QUIT 0.3s arms the global timer. Blocks 1+2: long MOVEs.
    // The global timer must cut the script short mid-flight (index 1, never
    // reaching the natural end), while everything stays loaded.
    Script script = {0};
    space_init_capacity(&script.space, 128);
    {
        Frames block = {0};
        Frame quit = {0};
        quit.kind = ACTION_KITE_QUIT;
        quit.duration = 0.3f;
        quit.original_duration = 0.3f;
        quit.index = 0;
        space_dap(&script.space, &block, quit);
        block.frames_index = 0;
        space_dap(&script.space, &script, block);
    }
    for (int b = 1; b <= 2; ++b) {
        Frames block = {0};
        Frame move = {0};
        move.kind = ACTION_KITE_MOVE;
        move.action.as_move.position = (Vector2){.x = (float) (b * 1000), .y = 0};
        move.duration = 5.0f;
        move.original_duration = 5.0f;
        move.index = 0;
        space_dap(&script.space, &move.kite_id_array, (Id) 0);
        space_dap(&script.space, &block, move);
        block.frames_index = (size_t) b;
        Kite_Position kp = {.kite_id = 0, .position = start, .angle = 0};
        space_dap(&script.space, &block.kite_frame_positions, kp);
        space_dap(&script.space, &script, block);
    }
    tkbc_upscale_script(env, &script, 60.0f);

    // The lone QUIT block itself is kept as-is (global semantics preserved).
    cassert_int_eq(script.elements[0].elements[0].kind, ACTION_KITE_QUIT);

    env->script = &script;
    env->frames = &script.elements[0];
    env->script_finished = false;
    for (int i = 0; i < 80 && !tkbc_script_finished(env); ++i) {
        tkbc_make_frame_time(TARGET_DT);
        tkbc_script_update_frames(env);
    }
    // Cut short by the global timer mid-flight: finished, parked well
    // before the natural end, and still loaded.
    cassert_bool_eq(tkbc_script_finished(env), true);
    cassert_bool_eq(env->frames->frames_index == script.count - 1, false);
    cassert_ptr_neq(env->script, NULL);
    cassert_ptr_neq(env->frames, NULL);

    space_free_space(&script.space);
    tkbc_destroy_env(env);
    return test;
}

/**
 * @brief Run all script handler unit tests.
 *
 * @param tests Pointer to the Tests struct to register results in.
 */
void tkbc_test_script_handler(Tests *tests) {
    cassert_dap(tests, init_frame());
    cassert_dap(tests, get_kite_by_id());
    cassert_dap(tests, get_kite_state_by_id());
    cassert_dap(tests, contains_id());
    cassert_dap(tests, deep_copy_frame());
    cassert_dap(tests, deep_copy_frames());
    cassert_dap(tests, deep_copy_script());
    cassert_dap(tests, destroy_frames_internal_data());
    cassert_dap(tests, reset_frames_internal_data());
    cassert_dap(tests, calculate_script_byte_size_allocated());
    cassert_dap(tests, upscale_move_absolute());
    cassert_dap(tests, upscale_move_add());
    cassert_dap(tests, upscale_wait());
    cassert_dap(tests, upscale_rotation_add());
    cassert_dap(tests, scrub_stays_in_script_mode());
    cassert_dap(tests, finish_stays_loaded());
    cassert_dap(tests, toggle_at_end_clamps());
    cassert_dap(tests, bake_fixes_stale_starts());
    cassert_dap(tests, bake_upscaled_slice_chain());
    cassert_dap(tests, upscale_quit_mixed_block());
    cassert_dap(tests, upscale_quit_block_plays_through());
    cassert_dap(tests, quit_alone_acts_global());
}
