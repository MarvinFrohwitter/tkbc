#include "../../external/cassert/cassert.h"

#include "../choreographer/tkbc-script-api.h"
#include "../choreographer/tkbc-script-handler.h"
#include "../choreographer/tkbc.h"
#include "../global/tkbc-types.h"
#include "../global/tkbc-utils.h"
#include <stdlib.h>
#include <string.h>

#define TKBC_UTILS_IMPLEMENTATION
#include "../global/tkbc-utils.h"

Test init_frame() {
  Test test = cassert_init_test("tkbc_init_frame()");

  uintptr_t stack = 0;
  Frame *frame;
  cassert_ptr_neq(&frame, &stack + 0);
  cassert_set_last_cassert_description(
      &test, "A stack variable and the next one should not have the same stack "
             "address.");

  cassert_ptr_eq(&frame, &stack + 1);

  void *frame_before = frame;
  cassert_ptr_eq(frame, frame_before);

  frame = tkbc_init_frame();
  if (!frame) {
    free(frame);
    return test;
  }
  cassert_ptr_neq(frame, frame_before);
  cassert_set_last_cassert_description(
      &test, "Frame before and after should not be the same.");

  free(frame);
  return test;
}

Test get_kite_by_id() {
  Test test = cassert_init_test("tkbc_get_kite_by_id()");
  Env *env = tkbc_init_env();
  Kite_State kite_state0 = tkbc_init_kite();
  kite_state0.kite_id = 0;
  Kite_State kite_state1 = tkbc_init_kite();
  kite_state1.kite_id = 1;
  Kite_State kite_state2 = tkbc_init_kite();
  kite_state2.kite_id = 2;
  tkbc_dap(env->kite_array, kite_state0);
  tkbc_dap(env->kite_array, kite_state1);
  tkbc_dap(env->kite_array, kite_state2);
  cassert_size_t_eq(env->kite_array->count, 3);

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

Test get_kite_state_by_id() {
  Test test = cassert_init_test("tkbc_get_kite_state_by_id()");
  Env *env = tkbc_init_env();
  Kite_State kite_state0 = tkbc_init_kite();
  kite_state0.kite_id = 0;
  Kite_State kite_state1 = tkbc_init_kite();
  kite_state1.kite_id = 1;
  Kite_State kite_state2 = tkbc_init_kite();
  kite_state2.kite_id = 2;
  tkbc_dap(env->kite_array, kite_state0);
  tkbc_dap(env->kite_array, kite_state1);
  tkbc_dap(env->kite_array, kite_state2);
  cassert_size_t_eq(env->kite_array->count, 3);

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

Test contains_id() {
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

Test deep_copy_frame() {
  Test test = cassert_init_test("tkbc_deep_copy_frame()");
  Frame *frame = tkbc_init_frame();
  frame->kind = KITE_WAIT;
  frame->action.as_wait.starttime = 123.456789f;
  Kite_Ids kite_ids = tkbc_indexs_range(0, 8);
  tkbc_dapc(&frame->kite_id_array, kite_ids.elements, kite_ids.count);
  Frame frame_copy = tkbc_deep_copy_frame(frame);
  cassert_size_t_eq(frame->kite_id_array.count, 8);

  cassert_ptr_neq(frame, &frame_copy);
  cassert_ptr_neq(frame->kite_id_array.elements,
                  &frame_copy.kite_id_array.elements);

  cassert_bool_eq(frame->finished, frame_copy.finished);
  cassert_int_eq(frame->kind, frame_copy.kind);
  cassert_float_eq(frame->duration, frame_copy.duration);
  cassert_size_t_eq(frame->index, frame_copy.index);
  cassert_double_eq(frame->action.as_wait.starttime,
                    frame_copy.action.as_wait.starttime);

  free(kite_ids.elements);
  free(frame_copy.kite_id_array.elements);
  free(frame->kite_id_array.elements);
  free(frame);
  return test;
}

Test deep_copy_frames() {
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
  frame->kind = KITE_MOVE;
  frame->action.as_move.position = (Vector2){.x = 300, .y = 400};
  cassert_dap(frames, *frame);
  cassert_dap(
      &frames->kite_frame_positions,
      ((Kite_Position){
          .kite_id = 1, .position.x = 100, .position.y = 200, .angle = 90}));
  cassert_dap(
      &frames->kite_frame_positions,
      ((Kite_Position){
          .kite_id = 2, .position.x = 150, .position.y = 250, .angle = 180}));

  Frames new_frames = tkbc_deep_copy_frames(frames);

  cassert_ptr_neq(frames, &new_frames);
  cassert_int_eq(frames->count, new_frames.count);
  cassert_int_eq(frames->capacity, new_frames.capacity);
  cassert_int_eq(frames->frames_index, new_frames.frames_index);
  cassert_ptr_neq(&frames->kite_frame_positions,
                  &new_frames.kite_frame_positions);

  cassert_ptr_neq(frames->kite_frame_positions.elements,
                  new_frames.kite_frame_positions.elements);
  cassert_int_eq(frames->kite_frame_positions.elements->kite_id,
                 new_frames.kite_frame_positions.elements->kite_id);
  cassert_float_eq(frames->kite_frame_positions.elements->position.x,
                   new_frames.kite_frame_positions.elements->position.x);
  cassert_float_eq(frames->kite_frame_positions.elements->position.y,
                   new_frames.kite_frame_positions.elements->position.y);
  cassert_float_eq(frames->kite_frame_positions.elements->angle,
                   new_frames.kite_frame_positions.elements->angle);

  cassert_ptr_neq(frames->elements, new_frames.elements);
  cassert_ptr_neq(&frames->elements->kite_id_array,
                  &new_frames.elements->kite_id_array);
  cassert_ptr_neq(frames->elements->kite_id_array.elements,
                  new_frames.elements->kite_id_array.elements);

  cassert_int_eq(frames->elements->kite_id_array.count,
                 new_frames.elements->kite_id_array.count);
  cassert_int_eq(frames->elements->kite_id_array.capacity,
                 new_frames.elements->kite_id_array.capacity);

  cassert_bool_eq(frames->elements->finished, new_frames.elements->finished);
  cassert_int_eq(frames->elements->index, new_frames.elements->index);
  cassert_int_eq(frames->elements->kind, new_frames.elements->kind);
  cassert_float_eq(frames->elements->duration, new_frames.elements->duration);
  cassert_float_eq(frames->elements->action.as_move.position.x,
                   new_frames.elements->action.as_move.position.x);
  cassert_float_eq(frames->elements->action.as_move.position.y,
                   new_frames.elements->action.as_move.position.y);

  free(frame);

  free(frames->elements->kite_id_array.elements);
  free(frames->elements);
  free(frames->kite_frame_positions.elements);
  free(frames);

  free(new_frames.elements->kite_id_array.elements);
  free(new_frames.elements);
  free(new_frames.kite_frame_positions.elements);
  return test;
}

Test deep_copy_script() {
  Test test = cassert_init_test("deep_copy_script()");
  Script script = {0};
  Frames frames = {0};
  Frame frame = {0};

  frame.kite_id_array = tkbc_indexs_range(0, 3);
  frame.kind = KITE_MOVE;
  frame.action.as_move.position = (Vector2){.x = 300, .y = 400};
  cassert_dap(&frames, frame);

  cassert_dap(
      &frames.kite_frame_positions,
      ((Kite_Position){
          .kite_id = 1, .position.x = 100, .position.y = 200, .angle = 90}));
  cassert_dap(
      &frames.kite_frame_positions,
      ((Kite_Position){
          .kite_id = 2, .position.x = 150, .position.y = 250, .angle = 180}));

  cassert_dap(&script, frames);

  script new_script = tkbc_deep_copy_script(&script);
  cassert_ptr_neq(&script, &new_script);
  cassert_int_eq(script.count, new_script.count);
  cassert_int_eq(script.capacity, new_script.capacity);
  cassert_int_eq(script.script_id, new_script.script_id);
  cassert_ptr_neq(&script.elements, &new_script.elements);

  cassert_ptr_neq(script.elements, new_script.elements);

  for (size_t i = 0; i < script.count; ++i) {
    tkbc_destroy_frames_internal_data(&script.elements[i]);
  }
  for (size_t i = 0; i < new_script.count; ++i) {
    tkbc_destroy_frames_internal_data(&new_script.elements[i]);
  }
  script.name = NULL;
  new_script.name = NULL;

  free(script.elements);
  free(new_script.elements);
  script.elements = NULL;
  new_script.elements = NULL;
  cassert_ptr_eq(script.elements, NULL);
  cassert_ptr_eq(new_script.elements, NULL);

  return test;
}

Test destroy_frames_internal_data() {
  Test test = cassert_init_test("tkbc_destroy_frames_internal_data()");
  Frames frames = {0};
  Frame frame = {0};

  frame.kite_id_array = tkbc_indexs_range(0, 3);
  frame.kind = KITE_MOVE;
  frame.action.as_move.position = (Vector2){.x = 300, .y = 400};
  cassert_dap(&frames, frame);

  cassert_dap(
      &frames.kite_frame_positions,
      ((Kite_Position){
          .kite_id = 1, .position.x = 100, .position.y = 200, .angle = 90}));
  cassert_dap(
      &frames.kite_frame_positions,
      ((Kite_Position){
          .kite_id = 2, .position.x = 150, .position.y = 250, .angle = 180}));

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

Test reset_frames_internal_data() {
  Test test = cassert_init_test("tkbc_reset_frames_internal_data()");
  Frames frames = {0};
  Frame frame = {0};

  frame.kite_id_array = tkbc_indexs_range(0, 3);
  frame.kind = KITE_MOVE;
  frame.action.as_move.position = (Vector2){.x = 300, .y = 400};
  cassert_dap(&frames, frame);

  cassert_dap(
      &frames.kite_frame_positions,
      ((Kite_Position){
          .kite_id = 1, .position.x = 100, .position.y = 200, .angle = 90}));
  cassert_dap(
      &frames.kite_frame_positions,
      ((Kite_Position){
          .kite_id = 2, .position.x = 150, .position.y = 250, .angle = 180}));

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

  tkbc_reset_frames_internal_data(&frames);

  cassert_ptr_neq(ids, NULL);
  cassert_ptr_eq(ids->elements, NULL);
  cassert_size_t_neq(ids->count, 3);
  cassert_size_t_eq(ids->capacity, 0);

  cassert_ptr_neq(frames.elements, NULL);
  cassert_size_t_eq(frames.count, 0);
  cassert_size_t_neq(frames.capacity, 0);

  cassert_ptr_neq(frames.kite_frame_positions.elements, NULL);
  cassert_size_t_eq(frames.kite_frame_positions.count, 0);
  cassert_size_t_neq(frames.kite_frame_positions.capacity, 0);

  tkbc_destroy_frames_internal_data(&frames);
  return test;
}

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
}
