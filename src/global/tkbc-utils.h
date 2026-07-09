#ifndef TKBC_UTILS_H_
#define TKBC_UTILS_H_

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <dirent.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef INCLUDE_RAYLIB
#include "raylib.h"
#endif

#ifdef INCLUDE_RAYLIB
#include "../choreographer/tkbc.h"
#endif

// ===========================================================================
// ========================== KITE UTILS =====================================
// ===========================================================================

typedef struct {
  char *elements;
  size_t count;
  size_t capacity;
} Content; // A representation of a file content.

typedef struct {
  char *name;
  char *full_path;
  char type;
} Dir_Entry;

typedef struct {
  Dir_Entry *elements;
  size_t count;
  size_t capacity;
} Dir_Entries;

#define TKBC_DT_UNKNOWN 0
#define TKBC_DT_DIR 4
#define TKBC_DT_REG 8

#define STR2(literal) #literal
#define STR(macro_literal) STR2(macro_literal)
#define shift(array, size) (assert(0 < (size)), (size)--, *(array)++)

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
        abort();                                                               \
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
          abort();                                                             \
        }                                                                      \
      }                                                                        \
      memcpy((dynamic_array)->elements + (dynamic_array)->count,               \
             (new_elements),                                                   \
             sizeof(*(dynamic_array)->elements) * (new_elements_count));       \
      (dynamic_array)->count = (dynamic_array)->count + (new_elements_count);  \
    }                                                                          \
  } while (0)

/**
 * @brief The macro pushes the given elements after processing it through the
 * format string to the end of the dynamic array.
 *
 * @param dynamic_array The given array by a pointer.
 * @param fmt The format string that is passed to the printf function.
 */
#define tkbc_dapf(dynamic_array, fmt, ...)                                     \
  do {                                                                         \
    int n = snprintf(NULL, 0, fmt, ##__VA_ARGS__);                             \
    if (n == -1) {                                                             \
      assert(0 && "snprintf failed!");                                         \
    }                                                                          \
    n += 1;                                                                    \
    if ((dynamic_array)->capacity < (dynamic_array)->count + n) {              \
      if ((dynamic_array)->capacity == 0) {                                    \
        (dynamic_array)->capacity = DAP_CAP;                                   \
      }                                                                        \
      while ((dynamic_array)->capacity < (dynamic_array)->count + n) {         \
        (dynamic_array)->capacity = (dynamic_array)->capacity * 2;             \
      }                                                                        \
      (dynamic_array)->elements = realloc((dynamic_array)->elements,           \
                                          sizeof(*(dynamic_array)->elements) * \
                                              (dynamic_array)->capacity);      \
      if ((dynamic_array)->elements == NULL) {                                 \
        fprintf(                                                               \
            stderr,                                                            \
            "The allocation for the dynamic array has failed in: %s: %d\n",    \
            __FILE__, __LINE__);                                               \
        abort();                                                               \
      }                                                                        \
    }                                                                          \
                                                                               \
    int err = snprintf((dynamic_array)->elements + (dynamic_array)->count, n,  \
                       (fmt), ##__VA_ARGS__);                                  \
    if (err == -1) {                                                           \
      assert(0 && "snprintf failed!");                                         \
    }                                                                          \
    (dynamic_array)->count += err;                                             \
  } while (0);

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

typedef enum {
  TYPE_SIZE_T,
  TYPE_INT,
  TYPE_LONG,
  TYPE_FLOAT,
  TYPE_DOUBLE,
} Types;

int tkbc_fprintf(FILE *stream, const char *level, const char *fmt, ...);
char *tkbc_ptoa(char *buffer, size_t buffer_size, void *number, Types type);
char *tkbc_shift_args(int *argc, char ***argv);

int tkbc_read_file(const char *filename, Content *content);
long tkbc_read_entire_file(const char *filename, Content *content);

int tkbc_write_file(const char *filename, const void *buffer, size_t size);
int tkbc_append_file(const char *filename, const void *buffer, size_t size);
int tkbc_write_file_mode(const char *filename, const void *buffer, size_t size,
                         const char *mode);
void tkbc_print_cmd(FILE *stream, const char *cmd[]);

int tkbc_get_screen_height(void);
int tkbc_get_screen_width(void);

char *tkbc_generate_file_name_with_time_stamp(const char *prefix,
                                              const char *postfix);
double tkbc_get_time(void);
void tkbc_make_frame_time(double target_dt);
float tkbc_get_frame_time(void);
#ifdef INCLUDE_RAYLIB
bool is_mouse_double_click(int mouse_button);
bool tkbc_is_same_image(Image a, Image b);
bool tkbc_vector2_equals_epsilon(Vector2 p, Vector2 q, float epsilon);
bool tkbc_is_rectangle_equal(Rectangle r1, Rectangle r2);
uint32_t tkbc_color_to_uint32_t(Color color);
Color tkbc_uint32_t_to_color(uint32_t color);

unsigned char *tkbc_get_position_in_image(Image image, int x, int y);
Vector2 tkbc_reduce_str_to_fit_box(Font font, const char *str, int *font_size,
                                   Rectangle bounding_box);
#endif

float tkbc_clamp(float z, float a, float b);
bool tkbc_float_equals_epsilon(float x, float y, float epsilon);
int tkbc_max(int x, int y);
char *tkbc_strtolower(char *str);
char *tkbc_strtoupper(char *str);

void free_dir_entrys(Dir_Entries dir_entrys);
bool read_dir_impl(const char *path, Dir_Entries *list);
bool read_dir(const char *path, Dir_Entries *list);
bool read_dir_recursive(const char *path, Dir_Entries *list);
bool tkbc_make_dir_if_not_existis(const char *path);
bool tkbc_make_dir_recursive_if_not_existis(const char *path);
char tkbc_get_file_type(const char *file_path);
bool tkbc_remove(const char *path);
bool tkbc_remove_recursive(const char *path);

#endif // TKBC_UTILS_H_

// ===========================================================================

#ifdef TKBC_UTILS_IMPLEMENTATION

// ========================== KITE UTILS =====================================

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
#ifndef TKBC_LOGGING_MESSAGEHANDLER
  if (strncmp(level, "MESSAGEHANDLER", 14) == 0) {
    return ret;
  }
#endif // TKBC_LOGGING_MESSAGEHANDLER

  va_list args;
  va_start(args, fmt);
  if (vsnprintf(NULL, 0, fmt, args) == 0) {
    va_end(args);
    return 0;
  }
  va_end(args);

  Content c = {0};
  if (level != NULL) {
    tkbc_dap(&c, '[');
    tkbc_dapc(&c, level, strlen(level));
    tkbc_dap(&c, ']');
    tkbc_dap(&c, ' ');
  }
  tkbc_dapc(&c, fmt, strlen(fmt));
  tkbc_dap(&c, 0);

  va_start(args, fmt);
  ret = vfprintf(stream, c.elements, args);
  va_end(args);
  free(c.elements);
  c.elements = NULL;
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
  if ((type) == TYPE_SIZE_T) {
    snprintf(buffer, buffer_size - 1, "%zu", *((size_t *)number));
  } else if ((type) == TYPE_INT) {
    snprintf(buffer, buffer_size - 1, "%d", *((int *)number));
  } else if ((type) == TYPE_FLOAT) {
    snprintf(buffer, buffer_size - 1, "%f", *((float *)number));
  } else if ((type) == TYPE_DOUBLE) {
    snprintf(buffer, buffer_size - 1, "%lf", *((double *)number));
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
  char *old_argv = NULL;

  if (*argc > 0) {
    old_argv = **argv;
    *argc = *argc - 1;
    *argv = *argv + 1;
  } else
    assert(*argc > 0 && "ERROR: No more arguments left!");

  return old_argv;
}

/**
 * @brief The function reads a file in 4KB steps into memory given by the
 * content structure. For errors the specific error is already logged into
 * stderr.
 *
 * @param filename The file path that should be read into memory.
 * @param content The resulting memory pointer that contains the file content
 * after reading.
 * @return 0 if no errors occurred, otherwise -1.
 */
int tkbc_read_file(const char *filename, Content *content) {
  int ok = 0;
  FILE *file = fopen(filename, "rb");
  if (file == NULL) {
    tkbc_fprintf(stderr, "ERROR", "%s:%d:%s\n", __FILE__, __LINE__,
                 strerror(errno));
    ok = -1;
    return ok;
  }

  size_t chunk_size = 4 * 1024;
  char chunk[chunk_size];
  while (feof(file) == 0) {
    size_t n = fread(chunk, sizeof(*chunk), chunk_size, file);
    if (chunk_size != n) {
      if (ferror(file) != 0) {
        ok = -1;
        break;
      }
    }
    tkbc_dapc(content, chunk, n);
  }
  if (content->count > 0) {
    tkbc_dap(content, '\0');
    content->count--;
  }

  if (fclose(file) == EOF) {
    tkbc_fprintf(stderr, "ERROR", "%s:%d:%s\n", __FILE__, __LINE__,
                 strerror(errno));
    ok = -1;
  }
  return ok;
}

/**
 * @brief The function returns the next nearest power of 2.
 *
 * @param number The number that should be expanded to the next power of 2.
 * @return The next nearest power of 2 relative to the number.
 */
static inline uint64_t tkbc_next_pow2(uint64_t number) {
  if (number == 0) {
    return number;
  }
  --number;
  number |= number >> 1;
  number |= number >> 2;
  number |= number >> 4;
  number |= number >> 8;
  number |= number >> 16;
  number |= number >> 32;
  return ++number;
}

/**
 * @brief The function reads the entire file in one attempt into memory
 * resulting in the content structure. For errors the specific error is already
 * logged into stderr.
 *
 * @param filename The file path that should be read into memory.
 * @param content The resulting memory pointer that contains the file content
 * after reading.
 * @return -1 if an error occurred,-2 if reading has failed, -4 if reading and
 * closing the file has failed, or the amount of bytes read.
 */
long tkbc_read_entire_file(const char *filename, Content *content) {
  long ok = 0;
  FILE *file = fopen(filename, "rb");
  if (file == NULL) {
    tkbc_fprintf(stderr, "ERROR", "%s:%s\n", filename, strerror(errno));
    ok = -1;
    goto error;
  }
  if (fseek(file, 0, SEEK_END) < 0) {
    tkbc_fprintf(stderr, "ERROR", "%s:%s\n", filename, strerror(errno));
    ok = -1;
    goto error;
  }
  long size = ftell(file);
  if (size < 0) {
    tkbc_fprintf(stderr, "ERROR", "%s:%s\n", filename, strerror(errno));
    ok = -1;
    goto error;
  }
  if (fseek(file, 0, SEEK_SET) < 0) {
    tkbc_fprintf(stderr, "ERROR", "%s:%s\n", filename, strerror(errno));
    ok = -1;
    goto error;
  }

  if (content->count + size > content->capacity) {
    long size_pow2 = tkbc_next_pow2(size);
    content->elements =
        realloc(content->elements, sizeof(*content->elements) * size_pow2);
    if (content->elements == NULL) {
      tkbc_fprintf(stderr, "ERROR", "%s:%d:allocation has failed for file %s\n",
                   __FILE__, __LINE__, filename);
      ok = -1;
      goto error;
    }
    content->capacity = size_pow2;
  }

  size_t read_count = fread(content->elements + content->count,
                            sizeof(*content->elements), size, file);
  if ((long)read_count != size) {
    if (feof(file) != 0) {
      content->count += read_count;
      ok = read_count;
      goto error;
    }

    if (ferror(file) != 0) {
      tkbc_fprintf(stderr, "ERROR", "%s:%d:reading %s has failed!\n", __FILE__,
                   __LINE__, filename);
      ok = -2;
      goto error;
    }
  }
  content->count += read_count;
  ok = read_count;

error:
  if (file) {
    if (fclose(file) == EOF) {
      tkbc_fprintf(stderr, "ERROR", "%s:%s\n", filename, strerror(errno));
      if (ok == -2) {
        return -4;
      }
      return -1;
    }
  }
  return ok;
}

/**
 * @brief The function writes the given content buffer to disk. For errors
 * the specific error is already logged into stderr.
 *
 * @param filename The file path where the content should be written too.
 * @param buffer The buffer that contains the content that should be written to
 * the filename.
 * @param size The size of the buffer in bytes.
 * @param mode The mode the file should open with.
 * @return 0 if no errors occurred, otherwise -1.
 */
int tkbc_write_file_mode(const char *filename, const void *buffer, size_t size,
                         const char *mode) {
  int ok = 0;
  FILE *file = fopen(filename, mode);
  if (file == NULL) {
    tkbc_fprintf(stderr, "ERROR", "%s:%s\n", filename, strerror(errno));
    ok = -1;
    return ok;
  }

  size_t err = fwrite(buffer, 1, size, file);
  if (err != size) {
    ok = -1;
    if (feof(file) != 0) {
      fprintf(stderr, "ERROR:%s:%d: while writing the file EOF has occurred.\n",
              __FILE__, __LINE__);
    } else if (ferror(file) != 0) {
      fprintf(stderr, "ERROR:%s:%d: while writing the file.\n", __FILE__,
              __LINE__);
    }
  }

  if (fclose(file) == EOF) {
    tkbc_fprintf(stderr, "ERROR", "%s:%s\n", filename, strerror(errno));
    ok = -1;
  }
  return ok;
}

/**
 * @brief The function writes the given content buffer to disk. For errors
 * the specific error is already logged into stderr.
 *
 * @param filename The file path where the content should be written too.
 * @param buffer The buffer that contains the content that should be written to
 * the filename.
 * @param size The size of the buffer in bytes.
 * @return 0 if no errors occurred, otherwise -1.
 */
int tkbc_write_file(const char *filename, const void *buffer, size_t size) {
  return tkbc_write_file_mode(filename, buffer, size, "wb");
}

/**
 * @brief The function appends the given content buffer to disk. For errors
 * the specific error is already logged into stderr.
 *
 * @param filename The file path where the content should be written too.
 * @param buffer The buffer that contains the content that should be written to
 * the filename.
 * @param size The size of the buffer in bytes.
 * @return 0 if no errors occurred, otherwise -1.
 */
int tkbc_append_file(const char *filename, const void *buffer, size_t size) {
  return tkbc_write_file_mode(filename, buffer, size, "a");
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
  cmd_string.elements = NULL;
}

#ifdef TKBC_SERVER
extern Env *env;
#endif

/**
 * @brief The function is a wrapper for the GetScreenHeight() that is not
 * available in the server computation.
 *
 * @return The screen height of the window.
 */
int tkbc_get_screen_height(void) {
#ifdef TKBC_SERVER
  if (env == NULL) {
    return 0;
  }
  return env->window_height;
#else
#ifdef INCLUDE_RAYLIB
  return GetScreenHeight();
#endif
#endif
  exit(0);
}

/**
 * @brief The function is a wrapper for the GetScreenWidth() that is not
 * available in the server computation.
 *
 * @return The screen width of the window.
 */
int tkbc_get_screen_width(void) {
#ifdef TKBC_SERVER
  if (env == NULL) {
    return 0;
  }
  return env->window_width;
#else
#ifdef INCLUDE_RAYLIB
  return GetScreenWidth();
#endif
#endif
  return 0;
  exit(0);
}

/**
 * @brief The function generates a file name with a time stamp between the
 * prefix and the postfix.
 *
 * @param prefix The prefix for the file name.
 * @param postfix The postfix for the file name.
 * @return A pointer to the generated file name on the heap or NULL on failure.
 */
char *tkbc_generate_file_name_with_time_stamp(const char *prefix,
                                              const char *postfix) {
  const time_t current_time = time(NULL);
  if (current_time == (time_t)-1) {
    tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
    return NULL;
  }

  size_t time_string_len = 26;
  char *time_string = ctime(&current_time);
  time_string = strdup(time_string);
  time_string[24] = '\0'; // This is to remove the '\n' from the time_string.
  size_t size = strlen(prefix) + time_string_len + strlen(postfix);
  char *mem = malloc(size * sizeof(char));
  int ret = snprintf(mem, size, "%s%s%s", prefix, time_string, postfix);
  free(time_string);
  if (ret != (int)size) {
    free(mem);
    return NULL;
  }
  return mem;
}

/**
 * @brief The function is a wrapper for the GetTime() that is not available
 * in the server computation, note that the GetTime() function calculate the
 * time since InitWindow() and the alternative uses CLOCK_MONOTONIC.
 *
 * @return The time since the program has initialized.
 */
double tkbc_get_time(void) {
#ifdef TKBC_SERVER
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
    tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
    exit(0);
  }
  return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
#else
#ifdef INCLUDE_RAYLIB
  return GetTime();
#endif
#endif
  exit(0);
}

static double tkbc_dt = 0;
/**
 * @brief The function sets the global tkbc_dt that represents a delta time
 * between the execution calls of this function. By setting the target_dt to 0
 * it can be used to profile the execution time between two calls of this
 * function. It also can be used to generate a delta time for fps related event
 * loop calls. By setting the target_dt to 1/TARGET_FPS, where TARGET_FPS
 * represents the upper limit if the event loop iteration per second.
 *
 * @param target_dt The delta time in seconds that should be reached before
 * continuing the execution.
 */
void tkbc_make_frame_time(double target_dt) {
  static double tkbc_last_frame_time = 0;

  double current_time = tkbc_get_time();
  tkbc_dt = (current_time - tkbc_last_frame_time);

  if (target_dt && tkbc_dt < target_dt) {
    double remaining = target_dt - tkbc_dt;

    const struct timespec rqtp = {
        .tv_sec = (long int)remaining,
        .tv_nsec = (remaining - (double)(long int)remaining) * 1e9,
    };

    nanosleep(&rqtp, NULL);
    current_time = tkbc_get_time();
    tkbc_dt = (current_time - tkbc_last_frame_time);
  }

  tkbc_last_frame_time = current_time;
}

/**
 * @brief The function is a wrapper for the GetFrameTime() that is not available
 * in the server computation.
 *
 * @return The delta time off a computation cycle.
 */
float tkbc_get_frame_time(void) {
#ifdef TKBC_SERVER
  return (float)tkbc_dt;
#else
#ifdef INCLUDE_RAYLIB
  return GetFrameTime();
#endif
#endif
  exit(0);
}

#ifdef INCLUDE_RAYLIB
#define TKBC_MAX_DOUBLECLICK_MS 400 // max time between two clicks
/**
 * @brief This function detects a double click.
 *
 * @param mouse_button The mouse_button that is pressed.
 * @return True if a double click is detected otherwise false:
 */
bool is_mouse_double_click(int mouse_button) {
#define MOUSE_BUTTON_COUNT 7
  static_assert(MOUSE_BUTTON_BACK == 6, "MOUSE_BUTTON positions changed");
  static_assert(MOUSE_BUTTON_COUNT == MOUSE_BUTTON_BACK + 1,
                "More mouse buttons need to be handled.");
  assert(mouse_button >= 0 && mouse_button < MOUSE_BUTTON_COUNT);
  switch (mouse_button) {
  case MOUSE_BUTTON_LEFT:
  case MOUSE_BUTTON_RIGHT:
  case MOUSE_BUTTON_MIDDLE:
  case MOUSE_BUTTON_SIDE:
  case MOUSE_BUTTON_EXTRA:
  case MOUSE_BUTTON_FORWARD:
  case MOUSE_BUTTON_BACK:
    break;
  default:
    assert(false && "UNREACHABLE");
  }

  static struct {
    int last_release_ms;
    bool waiting;
    bool cached;
    bool result;
  } state[MOUSE_BUTTON_COUNT] = {0};

  static float cached_frame_time = 0;
  float frame_time = tkbc_get_frame_time();
  if (frame_time != cached_frame_time) {
    cached_frame_time = frame_time;
    for (int i = 0; i < MOUSE_BUTTON_COUNT; i++) {
      state[i].cached = false;
    }
  }

  if (state[mouse_button].cached) {
    return state[mouse_button].result;
  }

  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
    tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
    exit(0);
  }
  double current_time = (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
  int now_ms = current_time * 1000;

  bool result = false;
  if (IsMouseButtonReleased(mouse_button)) {
    if (state[mouse_button].waiting) {
      int delta = now_ms - state[mouse_button].last_release_ms;
      state[mouse_button].waiting = false;
      if (delta >= 0 && delta <= TKBC_MAX_DOUBLECLICK_MS)
        result = true;
    }
    if (!result) {
      state[mouse_button].last_release_ms = now_ms;
      state[mouse_button].waiting = true;
    }
  } else if (state[mouse_button].waiting &&
             now_ms - state[mouse_button].last_release_ms >
                 TKBC_MAX_DOUBLECLICK_MS) {
    state[mouse_button].waiting = false;
  }

  state[mouse_button].cached = true;
  state[mouse_button].result = result;
  return result;
}

/**
 * @brief The function computes how may bytes per pixel a pixel format needs.
 *
 * @param format The pixel format representation from raylib.
 * @return How many bytes a particular format needs per pixel,
 * for uncompressed formats 0 is returned, in an unknown case -1 is returned.
 */
static int bytes_per_pixel_from_format(PixelFormat format) {
  switch (format) {
  case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
    return 1;
  case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
    return 2;
  case PIXELFORMAT_UNCOMPRESSED_R5G6B5:
    return 2;
  case PIXELFORMAT_UNCOMPRESSED_R8G8B8:
    return 3;
  case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
    return 2;
  case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:
    return 2;
  case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
    return 4;

  case PIXELFORMAT_UNCOMPRESSED_R32:
    return 4;
  case PIXELFORMAT_UNCOMPRESSED_R32G32B32:
    return 12;
  case PIXELFORMAT_UNCOMPRESSED_R32G32B32A32:
    return 16;

  case PIXELFORMAT_COMPRESSED_DXT1_RGB:
  case PIXELFORMAT_COMPRESSED_DXT1_RGBA:
  case PIXELFORMAT_COMPRESSED_DXT3_RGBA:
  case PIXELFORMAT_COMPRESSED_DXT5_RGBA:
  case PIXELFORMAT_COMPRESSED_ETC1_RGB:
  case PIXELFORMAT_COMPRESSED_ETC2_RGB:
  case PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA:
  case PIXELFORMAT_COMPRESSED_PVRT_RGB:
  case PIXELFORMAT_COMPRESSED_PVRT_RGBA:
  case PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA:
  case PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA:
    return 0;

  default:
    return -1;
  }
}

/**
 * @brief Checks if the given two images have the same data.
 *
 * @param a The first image.
 * @param b The second image.
 * @return True if the images are the same, otherwise false.
 */
bool tkbc_is_same_image(Image a, Image b) {
  if (a.data == NULL || b.data == NULL)
    return false;

  if (a.width != b.width)
    return false;
  if (a.height != b.height)
    return false;
  if (a.mipmaps != b.mipmaps)
    return false;
  if (a.format != b.format)
    return false;

  int bpp = bytes_per_pixel_from_format(a.format);
  if (bpp <= 0)
    return false;

  size_t total_bytes = 0;
  if (a.mipmaps <= 1) {
    total_bytes = a.width * a.height * bpp;
  } else {
    int w = a.width;
    int h = a.height;
    for (int level = 0; level < a.mipmaps; ++level) {
      if (w < 1)
        w = 1;
      if (h < 1)
        h = 1;
      total_bytes += w * h * bpp;
      w = w >> 1;
      h = h >> 1;
    }
  }

  if (total_bytes == 0)
    return false;

  return memcmp(a.data, b.data, total_bytes) == 0;
}

/**
 * @brief The function checks if tow Vector2s are equal to each other with a
 * custom epsilon.
 *
 * @param p The first value to compare.
 * @param q The second value to compare.
 * @param epsilon The value that represents the maximum difference  between x
 * and y.
 * @return True if x and y are the same in respect to epsilon, otherwise false.
 */
bool tkbc_vector2_equals_epsilon(Vector2 p, Vector2 q, float epsilon) {
  int result = ((fabsf(p.x - q.x)) <=
                (epsilon * fmaxf(1.0f, fmaxf(fabsf(p.x), fabsf(q.x))))) &&
               ((fabsf(p.y - q.y)) <=
                (epsilon * fmaxf(1.0f, fmaxf(fabsf(p.y), fabsf(q.y)))));

  return result;
}

/**
 * @brief This function check if two rectangles are the same.
 *
 * @param r1 The first rectangle.
 * @param r2 The second rectangle.
 * @return True if the given rectangles are the same, otherwise false.
 */
bool tkbc_is_rectangle_equal(Rectangle r1, Rectangle r2) {

  return r1.x == r2.x && r1.y == r2.y && r1.width == r2.width &&
         r1.height == r2.height;
}

/**
 * @brief The function converts a given Color struct to a uint32_t.
 *
 * @param color The color to convert.
 * @return The integer representation of the color packed in 4 bytes.
 */
uint32_t tkbc_color_to_uint32_t(Color color) {
  uint32_t c = ((uint32_t)color.r << 24) | ((uint32_t)color.g << 16) |
               ((uint32_t)color.b << 8) | (uint32_t)color.a;

  return c;
}

/**
 * @brief The function unpacks the given color from a uint32_t to the Color
 * type.
 *
 * @param color The color represented as a 4 byte integer.
 * @return The unpacked color information for the 4 bytes.
 */
Color tkbc_uint32_t_to_color(uint32_t color) {
  Color c;
  c.a = (color >> 0) & 0xFF;
  c.b = (color >> 8) & 0xFF;
  c.g = (color >> 16) & 0xFF;
  c.r = (color >> 24) & 0xFF;
  return c;
}

unsigned char *tkbc_get_position_in_image(Image image, int x, int y) {
  assert(image.format == PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
  return &((unsigned char *)image.data)[(x + y * (int)image.width) * 4];
}

/**
 * @brief The function computes the maximum font size that can be used to
 * display the given text inside a given bounding box.
 *
 * @param font The font that represents the glyph sizes.
 * @param str The text that should fit in the bounding_box.
 * @param font_size The font size the str has to be displayed at to fit the
 * bounding_box.
 * @param bounding_box The rectangle where the text str should fit in.
 * @return The text with and height that fits the bounding_box.
 */
Vector2 tkbc_reduce_str_to_fit_box(Font font, const char *str, int *font_size,
                                   Rectangle bounding_box) {
  Vector2 text_size = {0};

  text_size = MeasureTextEx(font, str, *font_size, 0);
  while (text_size.x > bounding_box.width / 2.0 &&
         text_size.y > bounding_box.height / 2.0) {
    *font_size -= 1;
    text_size = MeasureTextEx(font, str, *font_size, 2);
  }
  return text_size;
}

#endif

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

/**
 * @brief The function finds the maximum of two integers.
 *
 * @param x The fist integer.
 * @param y The second integer.
 * @return The bigger one of those.
 */
int tkbc_max(int x, int y) { return x > y ? x : y; }

/**
 * @brief This function lowercase the given string in place if a char in the
 * string can't be lowercase or it has no lowercase equivalent that it stays the
 * same.
 *
 * @param str The string that should be lowercase.
 * @return NULL if str was NULL, otherwise the string in lowercase.
 */
char *tkbc_strtolower(char *str) {
  if (str == NULL) {
    return NULL;
  }
  char *begin_str = str;
  for (size_t i = 0; str[i] != '\0'; ++i) {
    str[i] = tolower(str[i]);
  }
  return begin_str;
}

/**
 * @brief This function uppercase the given string in place if a char in the
 * string can't be uppercase or it has no uppercase equivalent that it stays the
 * same.
 *
 * @param str The string that should be uppercase.
 * @return NULL if str was NULL, otherwise the string in uppercase.
 */
char *tkbc_strtoupper(char *str) {
  if (str == NULL) {
    return NULL;
  }
  char *begin_str = str;
  for (size_t i = 0; str[i] != '\0'; ++i) {
    str[i] = toupper(str[i]);
  }
  return begin_str;
}

/**
 * @brief The function frees the memory allocated for the directory entries.
 *
 * @param dir_entrys The directory entries to free.
 */
void free_dir_entrys(Dir_Entries dir_entrys) {
  for (size_t i = 0; i < dir_entrys.count; ++i) {
    free(dir_entrys.elements[i].name);
    free(dir_entrys.elements[i].full_path);
  }
  free(dir_entrys.elements);
}

/**
 * @brief The function reads a directory and populates the list parameter with
 * its entries.
 *
 * @param path The path to the directory to read.
 * @param list The list to populate with the directory entries.
 * @return True if successful, otherwise false.
 */
bool read_dir_impl(const char *path, Dir_Entries *list) {
  char *d_name;
#ifdef _WIN32
  WIN32_FIND_DATAA lpFindFileData = {0};
  HANDLE dir = FindFirstFile(path, &lpFindFileData);
  if (dir == INVALID_HANDLE_VALUE) {
    fprintf(stderr, "Error: Could not open dir %s: %lu\n", path,
            GetLastError());
    return false;
  }
  goto read_entrys;
#else
  DIR *dir = opendir(path);
  if (!dir) {
    fprintf(stderr, "Error: Could not open dir %s: %s\n", path,
            strerror(errno));
    return false;
  }
#endif

  for (;;) {
#ifdef _WIN32
    if (FindNextFile(dir, &lpFindFileData) == 0) {
      DWORD error_code = GetLastError();
      if (error_code == ERROR_NO_MORE_FILES) {
        break;
      } else {
        printf("Error: Could not find next file in %s: %lu\n", path,
               error_code);
        BOOL ok = FindClose(dir);
        if (ok == 0) {
          printf("Error: Could not close dir %s: %lu\n", path, GetLastError());
        }
        return false;
      }
    }
  read_entrys:
    d_name = lpFindFileData.cFileName;
#else
    errno = 0;
    struct dirent *entry = readdir(dir);
    if (entry == NULL && errno == 0) {
      break;
    }
    // There should no error occur because EBADF can not happen.
    assert(errno == 0);
    d_name = entry->d_name;
#endif

    if (strcmp(d_name, ".") == 0) {
      continue;
    }
    if (strcmp(d_name, "..") == 0) {
      continue;
    }

#ifdef _WIN32
    const DWORD nBufferLength = MAX_PATH;
    char real_path_buf[nBufferLength];
    DWORD ok = GetFullPathName(path, nBufferLength, real_path_buf, NULL);
    if (ok == 0) {
      return false;
    }

    char *real_path = strdup(real_path_buf);
#else
    char *real_path = realpath(path, NULL);
#endif

#ifdef _WIN32
    int ret = snprintf(NULL, 0, "%s\\%s", real_path, d_name);
#else
    int ret = snprintf(NULL, 0, "%s/%s", real_path, d_name);
#endif
    if (ret < 0) {
      free(real_path);
      return false;
    }
    ret += 1;
    char *full_path = malloc(sizeof(char) * ret);
#ifdef _WIN32
    ret = snprintf(full_path, ret, "%s\\%s", real_path, d_name);
#else
    ret = snprintf(full_path, ret, "%s/%s", real_path, d_name);
#endif
    free(real_path);
    if (ret < 0) {
      return false;
    }

    char d_type = 0;
#ifdef _WIN32
    if (lpFindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      d_type = TKBC_DT_DIR;
    } else if (lpFindFileData.dwFileAttributes & FILE_ATTRIBUTE_NORMAL) {
      d_type = TKBC_DT_REG;
    } else {
      return false;
    }

#else
    d_type = entry->d_type;
#endif

    tkbc_dap(list, ((Dir_Entry){
                       .name = strdup(d_name),
                       .full_path = full_path,
                       .type = d_type,
                   }));
  }

#ifdef _WIN32
  BOOL ok = FindClose(dir);
  if (ok == 0) {
    printf("Error: Could not close dir %s: %lu\n", path, GetLastError());
    return false;
  }
#else
  int ok = closedir(dir);
  if (ok < 0) {
    printf("Error: Could not close dir %s: %s\n", path, strerror(errno));
    return false;
  }
#endif

  return true;
}

/**
 * @brief The function reads a directory and populates a list with its entries
 * including "." and "..".
 *
 * @param path The path to the directory to read.
 * @param list The list to populate with the directory entries.
 * @return True if successful, otherwise false.
 */
bool read_dir(const char *path, Dir_Entries *list) {
  if (!path) {
    return false;
  }

#ifdef _WIN32
  const DWORD nBufferLength = MAX_PATH;
  char real_path[nBufferLength];
  DWORD ok = GetFullPathName(path, nBufferLength, real_path, NULL);
  if (ok == 0) {
    return false;
  }
  char *parent_full_path = strdup(real_path);
#else
  char *parent_full_path = realpath(path, NULL);
#endif

  tkbc_dap(list, ((Dir_Entry){
                     .name = strdup("."),
                     .full_path = strdup(parent_full_path),
                     .type = TKBC_DT_DIR,
                 }));

#ifdef _WIN32
  char *filename;
  GetFullPathName(parent_full_path, nBufferLength, real_path, &filename);
  if (filename != NULL) {
    *(filename) = '\0';
  }
  parent_full_path = strdup(real_path);
#else
  parent_full_path = dirname(parent_full_path);
#endif

  tkbc_dap(list, ((Dir_Entry){
                     .name = strdup(".."),
                     .full_path = parent_full_path,
                     .type = TKBC_DT_DIR,
                 }));

  return read_dir_impl(path, list);
}

/**
 * @brief The function reads a directory and all its subdirectories recursively.
 *
 * @param path The path to the directory to read.
 * @param list The list to populate with the directory entries.
 * @return True if successful, otherwise false.
 */
bool read_dir_recursive(const char *path, Dir_Entries *list) {
  bool ok = true;
  ok = read_dir(path, list);
  if (!ok) {
    return false;
  }

  for (size_t i = 0; i < list->count; ++i) {
    if (list->elements[i].type != TKBC_DT_DIR) {
      continue;
    }
    if (strcmp(list->elements[i].name, ".") == 0) {
      continue;
    }
    if (strcmp(list->elements[i].name, "..") == 0) {
      continue;
    }

    ok = read_dir_impl(list->elements[i].full_path, list);
    if (!ok) {
      return false;
    }
  }

  return true;
}

/**
 * @brief The function creates a directory if it does not already exist.
 *
 * @param path The path of the directory to create.
 * @return True if successful or if the directory already exists, otherwise
 * false.
 */
bool tkbc_make_dir_if_not_existis(const char *path) {
#ifdef _WIN32
#include <direct.h>
  int ok = _mkdir(path);
#else
  int ok = mkdir(path, 0755);
#endif
  if (ok < 0) {
    if (errno == EEXIST) {
      tkbc_fprintf(stderr, "INFO", "The given path `%s` already exist.\n",
                   path);
    } else {
      tkbc_fprintf(stderr, "ERROR", "Failed to create directory: %s\n",
                   strerror(errno));
      return false;
    }
  }
  return true;
}

/**
 * @brief The function creates a directory and all its parent directories if
 * they do not already exist.
 *
 * @param path The path of the directory to create.
 * @return True if successful, otherwise false.
 */
bool tkbc_make_dir_recursive_if_not_existis(const char *path) {
  bool ok = true;
#ifdef _WIN32
  char separator = '\\';
#else
  char separator = '/';
  if (strcmp(path, "/") == 0) {
    return tkbc_make_dir_if_not_existis(path);
  }
#endif

  char *copy = strdup(path);
  char *next = strchr(copy, separator);
  while (true) {
    if (next == NULL) {
      ok = tkbc_make_dir_if_not_existis(copy);
      break;
    }
    *next = '\0';
    ok = tkbc_make_dir_if_not_existis(copy);
    *next = separator;
    if (!ok) {
      break;
    }
    next = strchr(next + 1, separator);
  }

  free(copy);
  return ok;
}

/**
 * @brief The function determines the file type of the given path.
 *
 * @param file_path The path to the file to check.
 * @return TKBC_DT_DIR if it is a directory, TKBC_DT_REG if it is a regular
 * file, otherwise TKBC_DT_UNKNOWN.
 */
char tkbc_get_file_type(const char *file_path) {
#ifdef _WIN32
  DWORD attributes = GetFileAttributes(file_path);

  if (attributes == INVALID_FILE_ATTRIBUTES) {
    tkbc_fprintf(stderr, "ERROR",
                 "Failed to get file information for %s: %lu\n", file_path,
                 GetLastError());
    return TKBC_DT_UNKNOWN;
  }

  if (attributes & FILE_ATTRIBUTE_DIRECTORY) {
    return TKBC_DT_DIR;
  }

  if (attributes & FILE_ATTRIBUTE_NORMAL ||
      !(attributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_DEVICE))) {
    return TKBC_DT_REG;
  }

#else

  struct stat statbuf;
  if (lstat(file_path, &statbuf) < 0) {
    tkbc_fprintf(stderr, "ERROR", "Failed to get file information for %s: %s\n",
                 file_path, strerror(errno));
    return TKBC_DT_UNKNOWN;
  }
  if (S_ISDIR(statbuf.st_mode)) {
    return TKBC_DT_DIR;
  }
  if (S_ISREG(statbuf.st_mode)) {
    return TKBC_DT_REG;
  }
#endif
  return TKBC_DT_UNKNOWN;
}

/**
 * @brief The function removes a file or an empty directory.
 *
 * @param path The path to remove.
 * @return True if successful, otherwise false.
 */
bool tkbc_remove(const char *path) {
  bool ok = true;
#ifdef _WIN32
  char type = tkbc_get_file_type(path);
  switch (type) {
  case TKBC_DT_DIR:
    if (RemoveDirectory(path) == 0) {
      tkbc_fprintf(stderr, "ERROR", "Failed to remove %s: %lu\n", path,
                   GetLastError());
      ok = false;
    }
    break;
  case TKBC_DT_REG:
    if (DeleteFile(path) == 0) {
      tkbc_fprintf(stderr, "ERROR", "Failed to remove %s: %lu\n", path,
                   GetLastError());
      ok = false;
    }
    break;
  default:
    assert("tkbc_remove_dir_recursive: Unsupported file type");
    ok = false;
  }

#else
  if (remove(path) < 0) {
    tkbc_fprintf(stderr, "ERROR", "Failed to remove %s: %s\n", path,
                 strerror(errno));
    ok = false;
  }
#endif
  return ok;
}

/**
 * @brief The function removes a directory and all its contents recursively.
 *
 * @param path The path to remove.
 * @return True if successful, otherwise false.
 */
bool tkbc_remove_recursive(const char *path) {
  Dir_Entries list = {0};
  bool ok = true;
  ok = read_dir_recursive(path, &list);
  if (!ok) {
    free(list.elements);
    return ok;
  }

  for (size_t k = list.count; k > 0; --k) {
    size_t index = k - 1;
    if (strcmp(list.elements[index].name, "..") == 0) {
      continue;
    }
    if (!tkbc_remove(list.elements[index].full_path)) {
      ok = false;
    }
  }

  free(list.elements);
  return ok;
}

#endif // TKBC_UTILS_IMPLEMENTATION
