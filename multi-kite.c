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
  float kite_width = env->kite_array->elements[0].kite->width;
  float kite_heigt = env->kite_array->elements[0].kite->height;
  int viewport_padding = kite_width > kite_heigt ? kite_width / 2 : kite_heigt;

  Vector2 start_pos = {.y = GetScreenHeight() - 2 * viewport_padding,
                       .x = GetScreenWidth() / 2.0f - kites_count * kite_width +
                            kite_width / 2.0f};

  for (size_t i = 0; i < kites_count; ++i) {
    kite_set_state_defaults(&env->kite_array->elements[i]);
    kite_set_kite_defaults(env->kite_array->elements[i].kite, false);
    kite_center_rotation(env->kite_array->elements[i].kite, &start_pos, 0);
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
    kite_dap(env->kite_array, *kite_kite_init());
    env->kite_array->elements[i].id = i;
    env->kite_array->elements[i].kite->body_color =
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
    kite_kite_destroy(&env->kite_array->elements[i]);
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
    kite_draw_kite(env->kite_array->elements[i].kite);
  }
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

      for (size_t j = 0; j < env->frames->count; ++j) {

        Kite_Indexs new_kite_index_array = {0};
        Frame *frame = &env->frames->elements[j];

        if (frame->kite_index_array == NULL) {
          continue;
        }

        for (size_t k = 0; k < frame->kite_index_array->count; ++k) {
          if (i - 1 != frame->kite_index_array->elements[k].index) {
            kite_dap(&new_kite_index_array,
                     frame->kite_index_array->elements[k]);
          }
        }

        if (new_kite_index_array.count != 0) {
          // If there are kites left in the frame

          frame->kite_index_array->count = 0;
          kite_dapc(frame->kite_index_array, new_kite_index_array.elements,
                    new_kite_index_array.count);

          free(new_kite_index_array.elements);
        } else {
          // If there are no kites left in the frame
          // for the cases KITE_MOVE, KITE_ROTATION, KITE_TIP_ROTATION
          frame->finished = true;
          frame->kite_index_array = NULL;
        }
      }

      env->kite_array->elements[i - 1].kite_input_handler_active =
          !env->kite_array->elements[i - 1].kite_input_handler_active;
    }
  }

  // To handle all of the kites currently registered in the kite array.
  for (size_t i = 0; i < env->kite_array->count; ++i) {
    kite_input_handler(env, &env->kite_array->elements[i]);
  }
}
