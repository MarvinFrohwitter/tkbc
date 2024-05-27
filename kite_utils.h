#ifndef KITE_UTILS_H_
#define KITE_UTILS_H_
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

#define DAP_CAP 64
#define kite_dap(dynamic_array, element)                                       \
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
        printf("The allocation for the dynamic array has failed in: %s: %d\n", \
               __FILE__, __LINE__);                                            \
      }                                                                        \
    }                                                                          \
                                                                               \
    (dynamic_array)->elements[(dynamic_array)->count] = (element);             \
    (dynamic_array)->count = (dynamic_array)->count + 1;                       \
  } while (0)

#define action_alloc(type)                                                     \
  do {                                                                         \
    action = calloc(1, sizeof(type));                                          \
    if (action == NULL) {                                                      \
      fprintf(stderr, "ERROR: No more memory can be allocated.\n");            \
      return NULL;                                                             \
    }                                                                          \
  } while (0)

#endif // KITE_UTILS_H_
