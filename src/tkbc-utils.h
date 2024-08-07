#ifndef TKBC_UTILS_H_
#define TKBC_UTILS_H_

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "tkbc-types.h"

// ===========================================================================
// ========================== KITE UTILS =====================================
// ===========================================================================

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

#define DAP_CAP 64
#define tkbc_dap(dynamic_array, element)                                       \
  do {                                                                         \
    if ((dynamic_array)->capacity <= (dynamic_array)->count) {                 \
      if ((dynamic_array)->capacity == 0)                                      \
        (dynamic_array)->capacity = DAP_CAP;                                   \
      else                                                                     \
        (dynamic_array)->capacity = (dynamic_array)->capacity * 2;             \
                                                                               \
      (dynamic_array)->elements = realloc((dynamic_array)->elements,           \
                                          sizeof(*(dynamic_array)->elements) * \
                                              (dynamic_array)->capacity);      \
                                                                               \
      if ((dynamic_array)->elements == NULL) {                                 \
        fprintf(                                                               \
            stderr,                                                            \
            "The allocation for the dynamic array has failed in: %s: %d\n",    \
            __FILE__, __LINE__);                                               \
      }                                                                        \
    }                                                                          \
                                                                               \
    (dynamic_array)->elements[(dynamic_array)->count] = (element);             \
    (dynamic_array)->count = (dynamic_array)->count + 1;                       \
  } while (0)

#define tkbc_dapc(dynamic_array, new_elements, new_elements_count)             \
  do {                                                                         \
    if (new_elements != NULL) {                                                \
      if ((dynamic_array)->capacity <                                          \
          (dynamic_array)->count + new_elements_count) {                       \
        if ((dynamic_array)->capacity == 0) {                                  \
          (dynamic_array)->capacity = DAP_CAP;                                 \
        }                                                                      \
        while ((dynamic_array)->capacity <                                     \
               (dynamic_array)->count + new_elements_count) {                  \
          (dynamic_array)->capacity = (dynamic_array)->capacity * 2;           \
        }                                                                      \
        (dynamic_array)->elements = realloc(                                   \
            (dynamic_array)->elements,                                         \
            sizeof(*(dynamic_array)->elements) * (dynamic_array)->capacity);   \
        if ((dynamic_array)->elements == NULL) {                               \
          fprintf(                                                             \
              stderr,                                                          \
              "The allocation for the dynamic array has failed in: %s: %d\n",  \
              __FILE__, __LINE__);                                             \
        }                                                                      \
      }                                                                        \
      memcpy((dynamic_array)->elements + (dynamic_array)->count, new_elements, \
             sizeof(*(dynamic_array)->elements) * new_elements_count);         \
      (dynamic_array)->count = (dynamic_array)->count + new_elements_count;    \
    }                                                                          \
  } while (0)

#define action_alloc(type)                                                     \
  do {                                                                         \
    action = calloc(1, sizeof(type));                                          \
    if (action == NULL) {                                                      \
      fprintf(stderr, "ERROR: No more memory can be allocated.\n");            \
      return NULL;                                                             \
    }                                                                          \
  } while (0)

#define tkbc_ra_setup(type)                                                    \
  do {                                                                         \
    for (size_t i = 0; i < frame->kite_index_array->count; ++i) {              \
      type *ra = frame->action;                                                \
      Kite *kite =                                                             \
          env->kite_array->elements[frame->kite_index_array->elements[i]]      \
              .kite;                                                           \
                                                                               \
      kite->segment_size += fabsf(ra->angle / frame->duration);                \
      kite->remaining_angle += ra->angle;                                      \
      kite->angle_sum += ra->angle;                                            \
      kite->old_angle = kite->center_rotation;                                 \
      kite->old_center = kite->center;                                         \
    }                                                                          \
  } while (0)

void *tkbc_move_action_to_heap(void *raw_action, Action_Kind kind, bool copy);
void tkbc_print_cmd(const char *cmd[]);
int tkbc_check_boundary(Kite *kite, ORIENTATION orientation);
float tkbc_clamp(float z, float a, float b);
float tkbc_lerp(float a, float b, float t);
int tkbc_max(int a, int b);

#endif // TKBC_UTILS_H_

// ===========================================================================

#ifdef TKBC_UTILS_IMPLEMENTATION

// ========================== KITE UTILS =====================================

void *tkbc_move_action_to_heap(void *raw_action, Action_Kind kind, bool copy) {

  void *action = NULL;
  switch (kind) {
  case KITE_WAIT:
  case KITE_QUIT: {
    if (copy) {
      action_alloc(Wait_Action);
      ((Wait_Action *)action)->starttime =
          ((Wait_Action *)raw_action)->starttime;
    }
  } break;
  case KITE_MOVE_ADD:
  case KITE_MOVE: {
    action_alloc(Move_Action);
    ((Move_Action *)action)->position.x =
        ((Move_Action *)raw_action)->position.x;
    ((Move_Action *)action)->position.y =
        ((Move_Action *)raw_action)->position.y;

  } break;
  case KITE_ROTATION_ADD: {

    action_alloc(Rotation_Action);
    ((Rotation_Action *)action)->angle = ((Rotation_Action *)raw_action)->angle;

  } break;
  case KITE_TIP_ROTATION: {

    action_alloc(Tip_Rotation_Action);
    ((Tip_Rotation_Action *)action)->tip =
        ((Tip_Rotation_Action *)raw_action)->tip;

    ((Tip_Rotation_Action *)action)->angle =
        ((Tip_Rotation_Action *)raw_action)->angle;

  } break;
  default: {
    assert(0 && "Unsupported Kite Action");
  } break;
  }
  return action;
}

void tkbc_print_cmd(const char *cmd[]) {
  struct {
    char *elements;
    size_t count;
    size_t capacity;

  } cmd_string = {0};

  size_t i = 0;
  while (cmd[i] != NULL) {

    tkbc_dapc(&cmd_string, cmd[i], strlen(cmd[i]));
    tkbc_dap(&cmd_string, ' ');
    i++;
  }
  tkbc_dap(&cmd_string, '\0');

  fprintf(stderr, "[INFO] [CMD] %s\n", cmd_string.elements);
  free(cmd_string.elements);
}

/**
 * @brief The function checks if the kite is still in the displayed window in
 * the given orientation of the kite.
 *
 * @param kite The kite that is going to be modified.
 * @param orientation The orientation of the kite, to determine where the tips
 * are.
 * @return True if the kite is in the window, otherwise false.
 */
int tkbc_check_boundary(Kite *kite, ORIENTATION orientation) {
  size_t width = GetScreenWidth();
  size_t height = GetScreenHeight();
  float x = kite->center.x;
  float y = kite->center.y;
  size_t padding = kite->width / 2;

  switch (orientation) {
  case KITE_X:
    return x < width - padding && x > 0 + padding;
  case KITE_Y:
    return y < height - padding && y > 0 + padding;
  default:
    assert(0 && "UNREACHABLE");
  }
  return 0;
}

/**
 * @brief The function checks if the point z is between the range a and b and
 * if not returns the closer point of thous.
 *
 * @param z The value to check for the range a and b.
 * @param a The minimum range value.
 * @param b The maximum range value.
 * @return float The given value z if is in between the range a and b,
 * otherwise if the value of z is less than a, the value of a will be returned
 * or b if the value is lager than to b.
 */
float tkbc_clamp(float z, float a, float b) {

  float s = z < a ? a : z;
  return s < b ? s : b;
}

float tkbc_lerp(float a, float b, float t) { return a + (t * (b - a)); }

int tkbc_max(int a, int b) { return a <= b ? b : a; }

#endif // TKBC_UTILS_IMPLEMENTATION
