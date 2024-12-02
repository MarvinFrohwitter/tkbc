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

/**
 * @brief The macro gives the actual size of the given array x back.
 */
#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

/** @brief The initial capacity of the dynamic arrays. */
#define DAP_CAP 64

/**
 * @brief The macro pushes the new array element at the end of the dynamic
 * array.
 *
 * @param dynamic_array The given array by a pointer.
 * @param element The given new element by value of the same type the array
 * holds elements.
 */
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

/**
 * @brief The macro pushes the given amount of new elements to the end of the
 * dynamic array.
 *
 * @param dynamic_array The given array by a pointer.
 * @param new_element The given new elements by pointer of the same type the
 * array holds elements.
 * @param new_elements_count The amount of elements to add to the array.
 */
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

/**
 * The macro can be used to do additional calls at the end of the function. For
 * example freeing some memory.
 * @param return_code The return code that the outer function should return.
 */
#define check_return(return_code)                                              \
  do {                                                                         \
    ok = return_code;                                                          \
    goto check;                                                                \
  } while (0)

/**
 * The macro allocates according to the given action type the action on the
 * heap.
 * @param type The action type definition to allocate.
 */
#define action_alloc(type)                                                     \
  do {                                                                         \
    action = calloc(1, sizeof(type));                                          \
    if (action == NULL) {                                                      \
      fprintf(stderr, "ERROR: No more memory can be allocated.\n");            \
      return NULL;                                                             \
    }                                                                          \
  } while (0)

char *tkbc_shift_args(int *argc, char ***argv);
void *tkbc_move_action_to_heap(void *raw_action, Action_Kind kind,
                               bool isgenerated);
int tkbc_read_file(char *filename, Content *content);
void tkbc_print_cmd(FILE *stream, const char *cmd[]);
float tkbc_clamp(float z, float a, float b);

#endif // TKBC_UTILS_H_

// ===========================================================================

#ifdef TKBC_UTILS_IMPLEMENTATION

// ========================== KITE UTILS =====================================

#include "errno.h"

/**
 * @brief The function cuts of the first argument of a given list.
 *
 * @param argc The argument count.
 * @param argv The arguments.
 * @return The pointer to the first argument.
 */
char *tkbc_shift_args(int *argc, char ***argv) {
  char *old_argv = **argv;

  if (*argc > 0) {
    *argc = *argc - 1;
    *argv = *argv + 1;
  } else
    assert(*argc > 0 && "ERROR: No more arguments left!");

  return old_argv;
}

/**
 * @brief The function creates a new heap copy of the given action. If the
 * provided one is already on the heap a second one will be created on the heap,
 * else the stack allocation will be moved to heap and can be dropped after this
 * function call.
 *
 * @param raw_action The action that will be moved to the heap.
 * @param kind The action kind that identifies the type of the action.
 * @param isgenerated Indicates the specific use for the initialization of a
 * frame, where the quit and wait actions are handled separate, differently than
 * in the deep copy function.
 */
void *tkbc_move_action_to_heap(void *raw_action, Action_Kind kind,
                               bool isgenerated) {

  void *action = NULL;
  assert(ACTION_KIND_COUNT == 9 && "NOT ALL THE Action_Kinds ARE IMPLEMENTED");
  switch (kind) {
  case KITE_QUIT:
  case KITE_WAIT: {
    if (!isgenerated) {
      action_alloc(Wait_Action);
      ((Wait_Action *)action)->starttime =
          ((Wait_Action *)raw_action)->starttime;
    }
  } break;
  case KITE_MOVE:
  case KITE_MOVE_ADD: {
    action_alloc(Move_Action);
    ((Move_Action *)action)->position.x =
        ((Move_Action *)raw_action)->position.x;
    ((Move_Action *)action)->position.y =
        ((Move_Action *)raw_action)->position.y;

  } break;
  case KITE_ROTATION:
  case KITE_ROTATION_ADD: {

    action_alloc(Rotation_Action);
    ((Rotation_Action *)action)->angle = ((Rotation_Action *)raw_action)->angle;

  } break;
  case KITE_TIP_ROTATION:
  case KITE_TIP_ROTATION_ADD: {

    action_alloc(Tip_Rotation_Action);
    ((Tip_Rotation_Action *)action)->tip =
        ((Tip_Rotation_Action *)raw_action)->tip;

    ((Tip_Rotation_Action *)action)->angle =
        ((Tip_Rotation_Action *)raw_action)->angle;

  } break;
  default: {
    fprintf(stderr, "KIND = %d\n", kind);
    assert(0 && "Unsupported Kite Action");
  } break;
  }
  return action;
}

int tkbc_read_file(char *filename, Content *content) {

  FILE *f = fopen(filename, "rb");
  if (f == NULL) {
    fprintf(stderr, "Error:%s:%d:%s\n", __FILE__, __LINE__, strerror(errno));
    return -1;
  }

  size_t buf_size = 4 * 1024;
  char *buf = malloc(sizeof(char) * buf_size);
  if (buf == NULL) {
    fprintf(stderr,
            "The allocation for the internal buffer has failed in: %s: %d\n",
            __FILE__, __LINE__);
  }
  size_t n;

  while (feof(f) == 0) {
    memset(buf, 0, buf_size);
    n = fread(buf, buf_size, 1, f);

    if (n != buf_size) {
      if (ferror(f) == -1) {
        return -1;
      }
    }
    tkbc_dapc(content, buf, buf_size);
    tkbc_dap(content, '\0');
    content->count = strlen(content->elements);
  }
  // just override the last NULL-terminator to update the count.
  // dap(content, '\0');

  free(buf);
  int fret = fclose(f);
  if (fret == EOF) {
    fprintf(stderr, "Error:%s:%d:%s\n", __FILE__, __LINE__, strerror(errno));
    return -1;
  }

  return 0;
}

/**
 * @brief The function can print any command line that is in form of an array of
 * strings.
 *
 * @param cmd The command line that should be printed to given stream.
 * @param stream The FILE stream where the output should be directed.
 */
void tkbc_print_cmd(FILE *stream, const char *cmd[]) {
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

  // Remove the extra space at the end.
  cmd_string.count--;
  tkbc_dap(&cmd_string, '\0');

  fprintf(stream, "[INFO] [CMD] %s\n", cmd_string.elements);
  free(cmd_string.elements);
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

#endif // TKBC_UTILS_IMPLEMENTATION
