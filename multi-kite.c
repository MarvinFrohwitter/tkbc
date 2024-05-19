#include <stddef.h>
#include "tkbc.h"

#define KITE_ARRAY_LEN 4
State kite_array[KITE_ARRAY_LEN];

/**
 * @brief The function kite_array_start_pos() computes the spaced start
 * positions for the kite_array and set the kites back to the default state
 * values.
 */
void kite_array_start_pos() {
  int kite_width = kite_array[0].kite->width;
  int kite_heigt = kite_array[0].kite->height;

  int viewport_padding = kite_width > kite_heigt ? kite_width / 2 : kite_heigt;

  Vector2 start_pos = {.y = GetScreenHeight() - 2 * viewport_padding,
                       .x = GetScreenWidth() / 2.0f -
                            KITE_ARRAY_LEN * kite_width + kite_width / 2.0f};

  for (size_t i = 0; i < KITE_ARRAY_LEN; ++i) {
    kite_set_state_defaults(&kite_array[i]);
    kite_set_kite_defaults(kite_array[i].kite, false);

    kite_center_rotation(kite_array[i].kite, &start_pos, 0);
    start_pos.x += 2 * kite_width;
  }
}

/**
 * @brief The function kite_gen_kites() initializes the amount of kites that are
 * provided in the arguments and inserts them in the global kite_array. It also
 * sets a different color for each kite, rather than the default color.
 *
 * @param s1 The first complete state of a kite.
 * @param s2 The second complete state of a kite.
 * @param s3 The third complete state of a kite.
 * @param s4 The fourth complete state of a kite.
 */
void kite_gen_kites(State *s1, State *s2, State *s3, State *s4) {
  s1 = kite_init();
  s2 = kite_init();
  s3 = kite_init();
  s4 = kite_init();
  kite_array[0] = *s1;
  kite_array[1] = *s2;
  kite_array[2] = *s3;
  kite_array[3] = *s4;

  kite_array_start_pos();

  s1->kite->body_color = BLUE;
  s2->kite->body_color = GREEN;
  s3->kite->body_color = PURPLE;
  s4->kite->body_color = RED;
}

/**
 * @brief The function kite_array_destroy_kites() frees all the kites that are
 * currently in the global kite_array.
 */
void kite_array_destroy_kites() {
  for (size_t i = 0; i < KITE_ARRAY_LEN; ++i) {
    kite_destroy(&kite_array[i]);
  }
}

/**
 * @brief The function kite_draw_kite_array() draws every kite with its
 * corresponding position on the canvas.
 */
void kite_draw_kite_array() {
  for (size_t i = 0; i < KITE_ARRAY_LEN; ++i) {
    kite_draw_kite(kite_array[i].kite);
  }
}

/**
 * @brief The function kite_array_check_interrupt_script() checks if one of the
 * kites that are currently in the kite_array has interrupt_script set to true.
 *
 * @return boolean Returns true if one of the kites of the global kite_array has
 * set the value interrupt_script, otherwise false.
 */
bool kite_array_check_interrupt_script() {
  for (size_t i = 0; i < KITE_ARRAY_LEN; ++i) {
    if (kite_array[i].interrupt_script) {
      return true;
    }
  }
  return false;
}

/**
 * @brief The function kite_array_input_handler() handles the kite switching and
 * calls the kite_input_handler() for each kite in the global kite_array.
 */
void kite_array_input_handler() {

  if (IsKeyPressed(KEY_B)) {
    TakeScreenshot("1.png");
  }

  if (IsKeyPressed(KEY_ONE)) {
    kite_array[0].kite_input_handler_active =
        !kite_array[0].kite_input_handler_active;
  } else if (IsKeyPressed(KEY_TWO)) {
    kite_array[1].kite_input_handler_active =
        !kite_array[1].kite_input_handler_active;
  } else if (IsKeyPressed(KEY_THREE)) {
    kite_array[2].kite_input_handler_active =
        !kite_array[2].kite_input_handler_active;
  } else if (IsKeyPressed(KEY_FOUR)) {
    kite_array[3].kite_input_handler_active =
        !kite_array[3].kite_input_handler_active;
  }

  for (size_t i = 0; i < KITE_ARRAY_LEN; ++i) {
    kite_input_handler(&kite_array[i]);
  }
}
