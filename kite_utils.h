#ifndef KITE_UTILS_H_
#define KITE_UTILS_H_
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define kite_da_append(da, item)                                                    \
  do {                                                                         \
    if ((da)->count >= (da)->capacity) {                                       \
      (da)->capacity = (da)->capacity == 0 ? 512 : (da)->capacity * 2; \
      (da)->items =                                                            \
          realloc((da)->items, (da)->capacity * sizeof(*(da)->items));         \
      assert((da)->items != NULL &&                                            \
             "The allocation of a new array member has failed.");              \
    }                                                                          \
                                                                               \
    (da)->items[(da)->count++] = (item);                                       \
  } while (0)

// Append several items to a dynamic array
#define kite_da_append_multi(da, new_items, new_items_count)                   \
  do {                                                                         \
    if (new_items != NULL) {                                                   \
      if ((da)->count + new_items_count > (da)->capacity) {                    \
        if ((da)->capacity == 0) {                                             \
          (da)->capacity = 512;                                                \
        }                                                                      \
        while ((da)->count + new_items_count > (da)->capacity) {               \
          (da)->capacity *= 2;                                                 \
        }                                                                      \
        (da)->items =                                                          \
            realloc((da)->items, (da)->capacity * sizeof(*(da)->items));       \
        assert((da)->items != NULL &&                                          \
               "The allocation of a new array member has failed.");            \
      }                                                                        \
      memcpy((da)->items + (da)->count, new_items,                             \
             new_items_count * sizeof(*(da)->items));                          \
      (da)->count += new_items_count;                                          \
    }                                                                          \
  } while (0)

#endif // KITE_UTILS_H_
