#ifndef TKBC_UTILS_H_
#define TKBC_UTILS_H_

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
      tkbc_fprintf(stderr, "ERROR", "No more memory can be allocated.\n");     \
      return NULL;                                                             \
    }                                                                          \
  } while (0)

typedef enum {
  SIZE_T,
  INT,
  LONG,
  FLOAT,
  DOUBLE,
} Types;

int tkbc_fprintf(FILE *stream, const char *level, const char *fmt, ...);
char *tkbc_ptoa(char *buffer, size_t buffer_size, void *number, Types type);
char *tkbc_shift_args(int *argc, char ***argv);
void *tkbc_move_action_to_heap(void *raw_action, Action_Kind kind,
                               bool isgenerated);
int tkbc_read_file(char *filename, Content *content);
void tkbc_print_cmd(FILE *stream, const char *cmd[]);
int tkbc_get_screen_height();
int tkbc_get_screen_width();
double tkbc_get_time();
float tkbc_get_frame_time();
float tkbc_clamp(float z, float a, float b);
bool tkbc_float_equals_epsilon(float x, float y, float epsilon);

#endif // TKBC_UTILS_H_

// ===========================================================================

#ifdef TKBC_UTILS_IMPLEMENTATION

// ========================== KITE UTILS =====================================

#include "raylib.h"
#include <errno.h>
#include <math.h>

/**
 * @brief The function provides a simple logging capability that supports a
 * level.
 *
 * @param stream The FILE stream where the output should be directed.
 * @param level The logging level that should be displayed in the brackets, or
 * NULL if the function should act like fprintf().
 * @param fmt The format string that is passed to the print.
 * @return The return code of the printf() function family and 0 if the logging
 * level macro is not defined.
 */
int tkbc_fprintf(FILE *stream, const char *level, const char *fmt, ...) {
  int ret = 0;
#ifdef TKBC_LOGGING
#ifndef TKBC_LOGGING_ERROR
  if (strncmp(level, "ERROR", 5) == 0) {
    return ret;
  }
#endif // TKBC_LOGGING_ERROR
#ifndef TKBC_LOGGING_INFO
  if (strncmp(level, "INFO", 4) == 0) {
    return ret;
  }
#endif // TKBC_LOGGING_INFO
#ifndef TKBC_LOGGING_WARNING
  if (strncmp(level, "WARNING", 7) == 0) {
    return ret;
  }
#endif // TKBC_LOGGING_WARNING

  Content c = {0};
  if (level != NULL) {
    tkbc_dap(&c, '[');
    tkbc_dapc(&c, level, strlen(level));
    tkbc_dap(&c, ']');
    tkbc_dap(&c, ' ');
  }
  tkbc_dapc(&c, fmt, strlen(fmt));
  tkbc_dap(&c, 0);

  va_list args;
  va_start(args, fmt);
  ret = vfprintf(stream, c.elements, args);
  va_end(args);
  free(c.elements);
#endif // TKBC_LOGGING
  (void)level;
  (void)fmt;
  (void)stream;
  return ret;
}

/**
 * @brief The function converts a given number by a pointer to a c-string.
 *
 * @param buffer The c-stings final location.
 * @param buffer_size The complete size of the buffer the string is stored in.
 * @param number The pointer to the number that should be converted.
 * @param type The type of the number in capital letters.
 * @return The pointer to the given buffer.
 */
char *tkbc_ptoa(char *buffer, size_t buffer_size, void *number, Types type) {
  memset(buffer, 0, buffer_size);
  if ((type) == SIZE_T) {
    snprintf(buffer, buffer_size - 1, "%zu", *((size_t *)number));
  } else if ((type) == INT) {
    snprintf(buffer, buffer_size - 1, "%d", *((int *)number));
  } else if ((type) == FLOAT) {
    snprintf(buffer, buffer_size - 1, "%f", *((float *)number));
  } else if ((type) == DOUBLE) {
    snprintf(buffer, buffer_size - 1, "%f", *((double *)number));
  }
  buffer[buffer_size - 1] = 0;
  return buffer;
}

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
    tkbc_fprintf(stderr, "ERROR", "KIND: %d\n", kind);
    assert(0 && "Unsupported Kite Action");
  } break;
  }
  return action;
}

int tkbc_read_file(char *filename, Content *content) {
  FILE *file = fopen(filename, "rb");
  if (file == NULL) {
    tkbc_fprintf(stderr, "ERROR", "%s:%d:%s\n", __FILE__, __LINE__,
                 strerror(errno));
    return -1;
  }

  size_t chunk_size = 4 * 1024;
  char chunk[chunk_size];
  while (feof(file) == 0) {
    memset(chunk, 0, chunk_size);
    if (chunk_size != fread(chunk, chunk_size, 1, file)) {
      if (ferror(file) == -1) {
        return -1;
      }
    }
    tkbc_dapc(content, chunk, chunk_size);
    tkbc_dap(content, '\0');
    content->count = strlen(content->elements);
  }

  if (fclose(file) == EOF) {
    tkbc_fprintf(stderr, "ERROR", "%s:%d:%s\n", __FILE__, __LINE__,
                 strerror(errno));
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

  tkbc_fprintf(stream, "INFO", "%s %s\n", "[CMD]", cmd_string.elements);
  free(cmd_string.elements);
}

#ifdef TKBC_SERVER
extern Env *env;
#endif // PROTOCOL_VERSION
/**
 * @brief The function is a wrapper for the GetScreenHeight() that is not
 * available in the server computation.
 *
 * @return The screen height of the window.
 */
int tkbc_get_screen_height() {
#ifdef TKBC_SERVER
  if (env == NULL) {
    return 0;
  }
  return env->window_height;
#else
  return GetScreenHeight();
#endif // PROTOCOL_VERSION
}

/**
 * @brief The function is a wrapper for the GetScreenWidth() that is not
 * available in the server computation.
 *
 * @return The screen width of the window.
 */
int tkbc_get_screen_width() {
#ifdef TKBC_SERVER
  if (env == NULL) {
    return 0;
  }
  return env->window_width;
#else
  return GetScreenWidth();
#endif // PROTOCOL_VERSION
}

/**
 * @brief The function is a wrapper for the GetTime() that is not available
 * in the server computation.
 *
 * @return The time since the program has initialized.
 */
double tkbc_get_time() {
#ifdef TKBC_SERVER
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
    tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
    exit(0);
  }
  return (double)((uint64_t)ts.tv_sec * (uint64_t)1e9 + (uint64_t)ts.tv_nsec);
#else
  return GetTime();
#endif // PROTOCOL_VERSION
}

#ifdef TKBC_SERVER
static double tkbc_last_frame_time = 0;
#endif // PROTOCOL_VERSION
/**
 * @brief The function is a wrapper for the GetFrameTime() that is not available
 * in the server computation.
 *
 * @return The delta time off a computation cycle.
 */
float tkbc_get_frame_time() {
#ifdef TKBC_SERVER
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
    tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
    exit(0);
  }
  double current_time =
      (float)((uint64_t)ts.tv_sec * (uint64_t)1e9 + (uint64_t)ts.tv_nsec);

  double dt = current_time - tkbc_last_frame_time;
  tkbc_last_frame_time = current_time;
  return (float)dt;
#else
  return GetFrameTime();
#endif // PROTOCOL_VERSION
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

/**
 * @brief The function checks if tow floats are equal to another with a custom
 * epsilon.
 *
 * @param x The first value to compare.
 * @param y The second value to compare.
 * @param epsilon The value that represents the maximum difference  between x
 * and y.
 * @return True if x and y are the same in respect to epsilon, otherwise false.
 */
bool tkbc_float_equals_epsilon(float x, float y, float epsilon) {
  int result =
      (fabsf(x - y)) <= (epsilon * fmaxf(1.0f, fmaxf(fabsf(x), fabsf(y))));

  return result;
}
#endif // TKBC_UTILS_IMPLEMENTATION
