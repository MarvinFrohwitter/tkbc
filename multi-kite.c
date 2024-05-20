#include "kite_utils.h"
#include "tkbc.h"
#include <stddef.h>

/**
 * @brief The function kite_array_start_pos() computes the spaced start
 * positions for the kite_array and set the kites back to the default state
 * values.
 *
 * @param env The global state of the application.
 */
void kite_array_start_pos(Env *env) {

  assert(env->kite_array->count != 0);

  size_t kites_count = env->kite_array->count;
  float kite_width = env->kite_array->items[0].kite->width;
  float kite_heigt = env->kite_array->items[0].kite->height;
  int viewport_padding = kite_width > kite_heigt ? kite_width / 2 : kite_heigt;

  Vector2 start_pos = {.y = GetScreenHeight() - 2 * viewport_padding,
                       .x = GetScreenWidth() / 2.0f - kites_count * kite_width +
                            kite_width / 2.0f};

  for (size_t i = 0; i < kites_count; ++i) {
    kite_set_state_defaults(&env->kite_array->items[i]);
    kite_set_kite_defaults(env->kite_array->items[i].kite, false);
    kite_center_rotation(env->kite_array->items[i].kite, &start_pos, 0);
    start_pos.x += 2 * kite_width;
  }
}

/**
 * @brief The function kite_gen_kites() initializes the amount of kites that are
 * provided in the arguments and inserts them in the global kite_array. It also
 * sets a different color for each kite, rather than the default color.
 *
 * @param env The global state of the application.
 */
void kite_gen_kites(Env *env, size_t kite_count) {

  Color color_array[] = {BLUE, GREEN, PURPLE, RED, TEAL};

  for (size_t i = 0; i < kite_count; ++i) {
    kite_da_append(env->kite_array, *kite_kite_init());
    env->kite_array->items[i].id = i;
    env->kite_array->items[i].kite->body_color =
        color_array[i % ARRAY_LENGTH(color_array)];
  }

  kite_array_start_pos(env);
}

/**
 * @brief The function kite_array_destroy_kites() frees all the kites that are
 * currently in the global kite_array.
 *
 * @param env The global state of the application.
 */
void kite_array_destroy_kites(Env *env) {
  for (size_t i = 0; i < env->kite_array->count; ++i) {
    kite_kite_destroy(&env->kite_array->items[i]);
  }
}

/**
 * @brief The function kite_draw_kite_array() draws every kite with its
 * corresponding position on the canvas.
 *
 * @param env The global state of the application.
 */
void kite_draw_kite_array(Env *env) {
  for (size_t i = 0; i < env->kite_array->count; ++i) {
    kite_draw_kite(env->kite_array->items[i].kite);
  }
}

/**
 * @brief The function kite_array_check_interrupt_script() checks if one of the
 * kites that are currently in the kite_array has interrupt_script set to true.
 *
 *
 * @param env The global state of the application.
 * @return boolean Returns true if one of the kites of the global kite_array has
 * set the value interrupt_script, otherwise false.
 */
bool kite_array_check_interrupt_script(Env *env) {
  for (size_t i = 0; i < env->kite_array->count; ++i) {
    if (env->kite_array->items[i].interrupt_script) {
      return true;
    }
  }
  return false;
}

/**
 * @brief The function kite_array_input_handler() handles the kite switching and
 * calls the kite_input_handler() for each kite in the global kite_array.
 *
 * @param env The global state of the application.
 */
void kite_array_input_handler(Env *env) {

  if (IsKeyPressed(KEY_B)) {
    TakeScreenshot("1.png");
  }

  // To only handle 9 kites controlable by the keyboard.
  for (size_t i = 1; i <= 9; ++i) {
    if (IsKeyPressed(i + 48)) {
      env->kite_array->items[i - 1].kite_input_handler_active =
          !env->kite_array->items[i - 1].kite_input_handler_active;
    }
  }

  // To handle all of the kites currently registered in the kite array.
  for (size_t i = 0; i < env->kite_array->count; ++i) {
    kite_input_handler(env, &env->kite_array->items[i]);
  }
}
