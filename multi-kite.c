#include "kite_utils.h"
#include "tkbc.h"
#include <stddef.h>

/**
 * @brief The function kite_array_start_pos() computes the spaced start
 * positions for the kite_array and set the kites back to the default state
 * values.
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
 * @param s1 The first complete state of a kite.
 * @param s2 The second complete state of a kite.
 * @param s3 The third complete state of a kite.
 * @param s4 The fourth complete state of a kite.
 */
void kite_gen_kites(Env *env, State *s1, State *s2, State *s3, State *s4) {

  s1 = kite_kite_init();
  s2 = kite_kite_init();
  s3 = kite_kite_init();
  s4 = kite_kite_init();
  s1->kite->body_color = BLUE;
  s2->kite->body_color = GREEN;
  s3->kite->body_color = PURPLE;
  s4->kite->body_color = RED;

  s1->id = 0;
  s2->id = 1;
  s3->id = 2;
  s4->id = 3;

  kite_da_append(env->kite_array, *s1);
  kite_da_append(env->kite_array, *s2);
  kite_da_append(env->kite_array, *s3);
  kite_da_append(env->kite_array, *s4);

  for (size_t i = 0; i < env->kite_array->count; ++i) {
    env->kite_array->items[i].id = i;
  }

  kite_array_start_pos(env);
}

/**
 * @brief The function kite_array_destroy_kites() frees all the kites that are
 * currently in the global kite_array.
 */
void kite_array_destroy_kites(Env *env) {
  for (size_t i = 0; i < env->kite_array->count; ++i) {
    kite_kite_destroy(&env->kite_array->items[i]);
  }
}

/**
 * @brief The function kite_draw_kite_array() draws every kite with its
 * corresponding position on the canvas.
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
 */
void kite_array_input_handler(Env *env) {

  if (IsKeyPressed(KEY_B)) {
    TakeScreenshot("1.png");
  }

  if (IsKeyPressed(KEY_ONE)) {
    env->kite_array->items[0].kite_input_handler_active =
        !env->kite_array->items[0].kite_input_handler_active;
  } else if (IsKeyPressed(KEY_TWO)) {
    env->kite_array->items[1].kite_input_handler_active =
        !env->kite_array->items[1].kite_input_handler_active;
  } else if (IsKeyPressed(KEY_THREE)) {
    env->kite_array->items[2].kite_input_handler_active =
        !env->kite_array->items[2].kite_input_handler_active;
  } else if (IsKeyPressed(KEY_FOUR)) {
    env->kite_array->items[3].kite_input_handler_active =
        !env->kite_array->items[3].kite_input_handler_active;
  }

  for (size_t i = 0; i < env->kite_array->count; ++i) {
    kite_input_handler(env, &env->kite_array->items[i]);
  }
}
