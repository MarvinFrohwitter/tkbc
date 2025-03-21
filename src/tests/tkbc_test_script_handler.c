#include "../../external/cassert/cassert.h"

#include "../choreographer/tkbc-script-api.h"
#include "../choreographer/tkbc-script-handler.h"
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
  cassert_int_eq(frames->block_index, new_frames.block_index);
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

Test deep_copy_block_frame() {
  Test test = cassert_init_test("deep_copy_block_frame()");
  Block_Frame block_frame = {0};
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

  cassert_dap(&block_frame, frames);

  Block_Frame new_block_frame = tkbc_deep_copy_block_frame(&block_frame);
  cassert_ptr_neq(&block_frame, &new_block_frame);
  cassert_int_eq(block_frame.count, new_block_frame.count);
  cassert_int_eq(block_frame.capacity, new_block_frame.capacity);
  cassert_int_eq(block_frame.script_id, new_block_frame.script_id);
  cassert_ptr_neq(&block_frame.elements, &new_block_frame.elements);

  cassert_ptr_neq(block_frame.elements, new_block_frame.elements);

  for (size_t i = 0; i < block_frame.count; ++i) {
    tkbc_destroy_frames_internal_data(&block_frame.elements[i]);
  }
  for (size_t i = 0; i < new_block_frame.count; ++i) {
    tkbc_destroy_frames_internal_data(&new_block_frame.elements[i]);
  }

  free(block_frame.elements);
  free(new_block_frame.elements);
  block_frame.elements = NULL;
  new_block_frame.elements = NULL;
  cassert_ptr_eq(block_frame.elements, NULL);
  cassert_ptr_eq(new_block_frame.elements, NULL);

  return test;
}

void tkbc_test_script_handler(Tests *tests) {
  cassert_dap(tests, init_frame());
  cassert_dap(tests, deep_copy_frames());
  cassert_dap(tests, deep_copy_block_frame());
}
