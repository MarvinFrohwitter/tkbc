#ifndef SPACE_H_
#define SPACE_H_

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Planet Planet;
struct Planet {
  void *elements;
  size_t count;
  size_t capacity;
  size_t id;

  Planet *prev;
  Planet *next;
};

typedef struct {
  Planet *sun;
  size_t planet_count;
  size_t id_counter;
} Space;

Planet *space_init_planet(Space *space, size_t size_in_bytes);
void space_free_planet(Space *space, Planet *planet);
void space_free_planet_optional_freeing_data(Space *space, Planet *planet,
                                             bool free_data);
void space_free_space(Space *space);
void space_free_space_internals_without_freeing_data(Space *space);

void space_reset_planet(Planet *planet);
bool space_reset_planet_id(Space *space, size_t id);

// WARNING: Dangerous to use:
// These functions sets the pointer to NULL so memory ownership is passed to the
// caller. That means the caller should free the allocated data.
void space_reset_planet_and_zero(Planet *planet);
bool space_reset_planet_and_zero_id(Space *space, size_t id);
void space_reset_space_and_zero(Space *space);

void space_reset_space(Space *space);
void *space_malloc(Space *space, size_t size_in_bytes);
void *space_calloc(Space *space, size_t nmemb, size_t size);
void *space_realloc(Space *space, void *ptr, size_t old_size, size_t new_size);
void *space_alloc_planetid(Space *space, size_t size_in_bytes,
                           size_t *planet_id, bool force_new_planet);

void *space_malloc_planetid(Space *space, size_t size_in_bytes,
                            size_t *planet_id);
void *space_calloc_planetid(Space *space, size_t nmemb, size_t size,
                            size_t *planet_id);
void *space_realloc_planetid(Space *space, void *ptr, size_t old_size,
                             size_t new_size, size_t *planet_id);

void *space_malloc_force_new_planet(Space *space, size_t size_in_bytes);
void *space_calloc_force_new_planet(Space *space, size_t nmemb, size_t size);
void *space_realloc_force_new_planet(Space *space, void *ptr, size_t old_size,
                                     size_t new_size);

void *space_malloc_planetid_force_new_planet(Space *space, size_t size_in_bytes,
                                             size_t *planet_id);
void *space_calloc_planetid_force_new_planet(Space *space, size_t nmemb,
                                             size_t size, size_t *planet_id);
void *space_realloc_planetid_force_new_planet(Space *space, void *ptr,
                                              size_t old_size, size_t new_size,
                                              size_t *planet_id);

bool space_init_capacity(Space *space, size_t size_in_bytes);
bool space_init_capacity_in_count_plantes(Space *space, size_t size_in_bytes,
                                          size_t count);
size_t space__find_planet_id_from_ptr(Space *space, void *ptr);
Planet *space__find_planet_from_ptr(Space *space, void *ptr);
bool try_to_expand_in_place(Space *space, void *ptr, size_t old_size,
                            size_t new_size, size_t *planet_id);

typedef struct {
  size_t planet_count;
  size_t allocated_capacity;
  size_t allocated_count;
} Space_Report;

bool space_report_allocations(Space *space, Space_Report *report);

#define SPACE_DAP_CAP 64
#define space_dap_impl(space, realloc_function, dynamic_array, element)        \
  do {                                                                         \
    if ((dynamic_array)->capacity <= (dynamic_array)->count) {                 \
      size_t old_capacity = (dynamic_array)->capacity;                         \
      if ((dynamic_array)->capacity == 0)                                      \
        (dynamic_array)->capacity = SPACE_DAP_CAP;                             \
      else                                                                     \
        (dynamic_array)->capacity = (dynamic_array)->capacity * 2;             \
                                                                               \
      (dynamic_array)->elements = realloc_function(                            \
          (space), (dynamic_array)->elements,                                  \
          sizeof(*(dynamic_array)->elements) * old_capacity,                   \
          sizeof(*(dynamic_array)->elements) * (dynamic_array)->capacity);     \
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

#define space_dapc_impl(space, realloc_function, dynamic_array, new_elements,  \
                        new_elements_count)                                    \
  do {                                                                         \
    if ((new_elements) != NULL) {                                              \
      if ((dynamic_array)->capacity <                                          \
          (dynamic_array)->count + new_elements_count) {                       \
        size_t old_capacity = (dynamic_array)->capacity;                       \
        if ((dynamic_array)->capacity == 0) {                                  \
          (dynamic_array)->capacity = SPACE_DAP_CAP;                           \
        }                                                                      \
        while ((dynamic_array)->capacity <                                     \
               (dynamic_array)->count + new_elements_count) {                  \
          (dynamic_array)->capacity = (dynamic_array)->capacity * 2;           \
        }                                                                      \
        (dynamic_array)->elements = realloc_function(                          \
            (space), (dynamic_array)->elements,                                \
            sizeof(*(dynamic_array)->elements) * old_capacity,                 \
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
      (dynamic_array)->count = (dynamic_array)->count + new_elements_count;    \
    }                                                                          \
  } while (0)

#define space_dapf_impl(space, realloc_function, dynamic_array, fmt, ...)      \
  do {                                                                         \
    int n = snprintf(NULL, 0, (fmt), ##__VA_ARGS__);                           \
    if (n == -1) {                                                             \
      assert(0 && "snprintf failed!");                                         \
    }                                                                          \
    n += 1;                                                                    \
    if ((dynamic_array)->capacity < (dynamic_array)->count + n) {              \
      size_t old_capacity = (dynamic_array)->capacity;                         \
      if ((dynamic_array)->capacity == 0) {                                    \
        (dynamic_array)->capacity = SPACE_DAP_CAP;                             \
      }                                                                        \
      while ((dynamic_array)->capacity < (dynamic_array)->count + n) {         \
        (dynamic_array)->capacity = (dynamic_array)->capacity * 2;             \
      }                                                                        \
      (dynamic_array)->elements = realloc_function(                            \
          (space), (dynamic_array)->elements,                                  \
          sizeof(*(dynamic_array)->elements) * old_capacity,                   \
          sizeof(*(dynamic_array)->elements) * (dynamic_array)->capacity);     \
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

#define space_dap(space, dynamic_array, element)                               \
  space_dap_impl(space, space_realloc, dynamic_array, element)

#define space_ndap(space, dynamic_array, element)                              \
  space_dap_impl(space, space_realloc_force_new_planet, dynamic_array, element)

#define space_dapc(space, dynamic_array, new_elements, new_elements_count)     \
  space_dapc_impl(space, space_realloc, dynamic_array, new_elements,           \
                  new_elements_count)

#define space_ndapc(space, dynamic_array, new_elements, new_elements_count)    \
  space_dapc_impl(space, space_realloc_force_new_planet, dynamic_array,        \
                  new_elements, new_elements_count)

#define space_dapf(space, dynamic_array, fmt, ...)                             \
  space_dapf_impl(space, space_realloc, dynamic_array, fmt, ##__VA_ARGS__)

#define space_ndapf(space, dynamic_array, fmt, ...)                            \
  space_dapf_impl(space, space_realloc_force_new_planet, dynamic_array, fmt,   \
                  ##__VA_ARGS__)

void *space_printf(Space *space, const char *fmt, ...);
void *space_strcat(Space *space, const char *first, const char *second);
void *space_strdup(Space *space, const char *buf);
void *space_strcpy(Space *space, const char *buf);
void *space_strncpy(Space *space, const char *buf, size_t n);
void *space_stpcpy(Space *space, const char *buf);
void *space_stpncpy(Space *space, const char *buf, size_t n);
void *space_memcpy(Space *space, const void *buf, size_t n);
void *space_memmove(Space *space, const void *buf, size_t n);

bool space__is_ptr_last_allocation_in_planet(Planet *p, void *ptr,
                                             size_t ptr_size);

void *space_vstrcat_impl(Space *space, const char *first, ...);
void *space_catf(Space *space, const void *first, size_t first_len,
                 const char *fmt, ...);
void *space_strcat(Space *space, const char *first, const char *second);

#define space_strcatf(space, first_str, fmt, ...)                              \
  space_catf(space, first_str, strlen(first_str), (fmt), __VA_ARGS__)

#define space_vstrcat(space, first, ...)                                       \
  space_vstrcat_impl(space, first, __VA_ARGS__, NULL)

void *space_vcat_impl(Space *space, ...);
#define space_vcat(space, ...) space_vcat_impl(space, __VA_ARGS__, NULL)

bool space__is_ptr_last_allocation_in_planet(Planet *p, void *ptr,
                                             size_t ptr_size);
#endif // SPACE_H_

// ===========================================================================

#ifdef SPACE_IMPLEMENTATION

bool space__is_ptr_last_allocation_in_planet(Planet *p, void *ptr,
                                             size_t ptr_size) {
  return (char *)p->elements + p->count - ptr_size == ptr;
}

void *space_printf(Space *space, const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  int n = vsnprintf(NULL, 0, fmt, args);
  va_end(args);
  if (n == -1) {
    return NULL;
  }

  n += 1;

  char *ptr = space_malloc(space, sizeof(*ptr) * n);
  if (ptr == NULL) {
    return NULL;
  }

  va_start(args, fmt);
  int err = vsnprintf(ptr, n, fmt, args);
  va_end(args);
  if (err == -1) {
    return NULL;
  }

  return ptr;
}

void *space_vcat_impl(Space *space, ...) {
  va_list args;
  va_start(args, space);
  char *first = va_arg(args, char *);
  if (first == NULL) {
    return NULL;
  }
  size_t first_len = va_arg(args, size_t);
  if (first_len == 0) {
    return NULL;
  }
  va_end(args);

  Planet *p = space__find_planet_from_ptr(space, (void *)first);
  if (p &&
      space__is_ptr_last_allocation_in_planet(p, (void *)first, first_len)) {
    size_t save = p->count;

    va_start(args, space);
    char *arg = va_arg(args, char *); // The first one
    size_t arg_len = va_arg(args, size_t);
    arg = va_arg(args, char *);
    while (arg != NULL) {
      arg_len = va_arg(args, size_t);

      if (p->count + arg_len > p->capacity) {
        // Restore the old allocation size to be consistent with the
        // space_strcat function.
        p->count = save;
        va_end(args);
        goto alloc;
      }

      memcpy(((char *)p->elements) + p->count - 1, arg, arg_len + 1);
      p->count += arg_len;

      arg = va_arg(args, char *);
    }

    ((char *)p->elements)[p->count - 1] = '\0';
    va_end(args);
    return (void *)first;
  }

alloc: {}
  size_t count = first_len + 1;
  {
    va_start(args, space);
    first = va_arg(args, char *);
    first_len = va_arg(args, size_t);
    char *arg = va_arg(args, char *);
    while (arg != NULL) {
      count += va_arg(args, size_t);
      arg = va_arg(args, char *);
    }
    va_end(args);
  }

  void *ptr = space_malloc(space, count);
  p = space__find_planet_from_ptr(space, ptr);
  p->count -= count;

  memcpy(ptr, first, first_len);
  p->count += first_len;

  va_start(args, space);
  first = va_arg(args, char *);
  first_len = va_arg(args, size_t);
  char *arg = va_arg(args, char *);
  while (arg != NULL) {
    size_t arg_len = va_arg(args, size_t);
    memcpy((char *)p->elements + p->count - 1, arg, arg_len);
    p->count += arg_len;
    arg = va_arg(args, char *);
  }
  va_end(args);
  ((char *)p->elements)[p->count - 1] = '\0';

  return ptr;
}

void *space_vstrcat_impl(Space *space, const char *first, ...) {
  size_t first_len = strlen(first);
  va_list args;

  Planet *p = space__find_planet_from_ptr(space, (void *)first);
  if (p &&
      space__is_ptr_last_allocation_in_planet(p, (void *)first, first_len)) {
    size_t save = p->count;

    va_start(args, first);
    char *arg = va_arg(args, char *);
    while (arg != NULL) {
      size_t arg_len = strlen(arg);
      if (p->count + arg_len > p->capacity) {
        // Restore the old allocation size to be consistent with the
        // space_strcat function.
        p->count = save;
        va_end(args);
        goto alloc;
      }

      memcpy(((char *)p->elements) + p->count - 1, arg, arg_len + 1);
      p->count += arg_len;

      arg = va_arg(args, char *);
    }

    ((char *)p->elements)[p->count - 1] = '\0';
    va_end(args);
    return (void *)first;
  }

alloc: {}
  size_t count = first_len + 1;
  {
    va_start(args, first);
    char *arg = va_arg(args, char *);
    while (arg != NULL) {
      size_t arg_len = strlen(arg);
      count += arg_len;
      arg = va_arg(args, char *);
    }
    va_end(args);
  }

  void *ptr = space_malloc(space, count);
  p = space__find_planet_from_ptr(space, ptr);
  p->count -= count;

  memcpy(ptr, first, first_len);
  p->count += first_len;

  va_start(args, first);
  char *arg = va_arg(args, char *);
  while (arg != NULL) {
    size_t arg_len = strlen(arg); // This is slow to compute the length again.
    memcpy((char *)p->elements + p->count - 1, arg, arg_len);
    p->count += arg_len;
    arg = va_arg(args, char *);
  }
  va_end(args);
  ((char *)p->elements)[p->count - 1] = '\0';

  return ptr;
}

void *space_catf(Space *space, const void *first, size_t first_len,
                 const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int n = vsnprintf(NULL, 0, fmt, args);
  if (n == -1) {
    return NULL;
  }
  va_end(args);

  size_t max_count = n + 1;
  Planet *p = space__find_planet_from_ptr(space, (void *)first);
  if (p && (p->count + n <= p->capacity) &&
      space__is_ptr_last_allocation_in_planet(p, (void *)first, first_len)) {
    va_start(args, fmt);
    int err = vsnprintf((char *)first + first_len, max_count, fmt, args);
    va_end(args);
    if (err == -1) {
      return NULL;
    }

    p->count += err;
    return (void *)first;
  }

  size_t combind_size = max_count + first_len;
  void *ptr = space_malloc(space, combind_size);
  memcpy(ptr, first, first_len);
  va_start(args, fmt);
  int err = vsnprintf((char *)ptr + first_len, max_count, fmt, args);
  va_end(args);
  if (err == -1) {
    return NULL;
  }
  return ptr;
}

void *space_strcat(Space *space, const char *first, const char *second) {
  size_t first_len = strlen(first);
  size_t second_len = strlen(second);

  Planet *p = space__find_planet_from_ptr(space, (void *)first);
  if (p && (p->count + second_len <= p->capacity) &&
      space__is_ptr_last_allocation_in_planet(p, (void *)first, first_len)) {

    memcpy((char *)first + first_len, second, second_len + 1);
    p->count += second_len;
    return (void *)first;
  }

  int n = first_len + second_len + 1;
  void *ptr = space_malloc(space, n);
  memcpy(ptr, first, first_len);
  memcpy((char *)ptr + first_len, second, second_len + 1);
  return ptr;
}

void *space_strdup(Space *space, const char *buf) {
  size_t n = strlen(buf) + 1;
  return space_memcpy(space, buf, n);
}

void *space_strcpy(Space *space, const char *buf) {
  size_t n = strlen(buf) + 1;
  return space_memcpy(space, buf, n);
}

void *space_strncpy(Space *space, const char *buf, size_t n) {
  size_t nn = strlen(buf) + 1;
  size_t len = nn > n ? n : nn;
  return space_memcpy(space, buf, len);
}

void *space_stpcpy(Space *space, const char *buf) {
  size_t n = strlen(buf) + 1;
  char *result = space_strncpy(space, buf, n);
  return result + n - 1;
}

void *space_stpncpy(Space *space, const char *buf, size_t n) {
  size_t nn = strlen(buf) + 1;
  size_t len = nn > n ? n : nn;
  char *result = space_strncpy(space, buf, len);
  return result + len - 1;
}

void *space_memcpy(Space *space, const void *buf, size_t n) {
  void *ptr = space_malloc(space, n);
  if (ptr == NULL) {
    return NULL;
  }
  memcpy(ptr, buf, n);
  return ptr;
}

void *space_memmove(Space *space, const void *buf, size_t n) {
  void *ptr = space_malloc(space, n);
  if (ptr == NULL) {
    return NULL;
  }
  memmove(ptr, buf, n);
  return ptr;
}

Planet *space_init_planet(Space *space, size_t size_in_bytes) {
  Planet *planet = malloc(sizeof(*planet));
  if (planet) {
    memset(planet, 0, sizeof(*planet));
    planet->capacity = size_in_bytes;
    planet->count = size_in_bytes;
    planet->elements = malloc(planet->capacity);
    if (!planet->elements) {
      free(planet);
      return NULL;
    }

    // The '1+' is needed because 0 is an invalid id and
    // space__find_planet_id_from_ptr() returns 0 if it could not be found.
    // This allows to use size_t and still return an error value.
    planet->id = 1 + space->id_counter++;
    space->planet_count++;
  }

  return planet;
}

void space_free_planet(Space *space, Planet *planet) {
  space_free_planet_optional_freeing_data(space, planet, true);
}

void space_free_planet_optional_freeing_data(Space *space, Planet *planet,
                                             bool free_data) {
  if (!planet || !space || !space->sun) {
    return;
  }

  if (space->sun == planet) {
    space->sun = planet->next;
  }

  if (planet->prev) {
    planet->prev->next = planet->next;
  }

  if (planet->next) {
    planet->next->prev = planet->prev;
  }

  if (free_data) {
    free(planet->elements);
    planet->elements = NULL;
  }
  planet->count = 0;
  planet->capacity = 0;
  planet->next = NULL;
  planet->prev = NULL;
  planet->id = 0;
  free(planet);
  planet = NULL;

  space->planet_count--;
}

void space_free_space(Space *space) {
  while (space->sun) {
    space_free_planet(space, space->sun);
  }
  assert(space->planet_count == 0);
  assert(space->sun == NULL);
}

// The function is useful if you have to unit test a function that depends on
// the space allocator, but you want to be sure to track every memory
// allocated manually, so you free manually. But as a result calling
// space_free_space() would result in double free so just free the structure.
void space_free_space_internals_without_freeing_data(Space *space) {
  while (space->sun) {
    space_free_planet_optional_freeing_data(space, space->sun, false);
  }
  assert(space->planet_count == 0);
  assert(space->sun == NULL);
}

void space_reset_planet(Planet *planet) { planet->count = 0; }
void space_reset_planet_and_zero(Planet *planet) {
  if (!planet) {
    return;
  }
  if (planet->elements) {
    memset(planet->elements, 0, planet->capacity);
  }
  planet->count = 0;
}

bool space_reset_planet_id(Space *space, size_t id) {
  for (Planet *planet = space->sun; planet; planet = planet->next) {
    if (planet->id == id) {
      space_reset_planet(planet);
      return true;
    }
  }
  return false;
}

bool space_reset_planet_and_zero_id(Space *space, size_t id) {
  for (Planet *planet = space->sun; planet; planet = planet->next) {
    if (planet->id == id) {
      space_reset_planet_and_zero(planet);
      return true;
    }
  }
  return false;
}

void space_reset_space(Space *space) {
  for (Planet *p = space->sun; p; p = p->next) {
    space_reset_planet(p);
  }
}

void space_reset_space_and_zero(Space *space) {
  for (Planet *p = space->sun; p; p = p->next) {
    space_reset_planet_and_zero(p);
  }
}

void *space_alloc_planetid(Space *space, size_t size_in_bytes,
                           size_t *planet_id, bool force_new_planet) {
  if (!space) {
    planet_id = NULL;
    return NULL;
  }

  if (!space->planet_count) {
    space->sun = space_init_planet(space, size_in_bytes);
    if (!space->sun) {
      planet_id = NULL;
      return NULL;
    }
    *planet_id = space->sun->id;
    space->sun->next = NULL;
    space->sun->prev = NULL;
    return space->sun->elements;
  }

  Planet *p = space->sun;
  Planet *prev = space->sun;
  while (p) {
    // This is slow finding the end of the linked list.
    // TODO: Maybe find the end by storing it in sun->prev.
    if (force_new_planet) {
      prev = p;
      p = p->next;
      continue;
    }

    if (p->count + size_in_bytes > p->capacity) {
      prev = p;
      p = p->next;
      continue;
    }

    // TODO: Think about handling this by deleting the planet chunk in
    // between. We can't get the original pointer at this point anyway.
    // Recovery is outside the traditional behavior of this lib, which is
    // freeing the complete space at once and be sure that every allocated
    // memory is freed.
    //
    // We can't distinguish between an actual free call or destroying our
    // reference by setting it to NULL.
    //
    // Marvin Frohwitter 01.12.2025
    assert(p->elements &&
           "ERROR:SPACE: Memory inside a space was freed or set to NULL"
           "by an external call outside the space api!");
    void *place = &((char *)p->elements)[p->count];
    p->count += size_in_bytes;
    *planet_id = p->id;
    return place;
  }

  p = space_init_planet(space, size_in_bytes);
  if (!p) {
    planet_id = NULL;
    return NULL;
  }
  *planet_id = p->id;

  p->prev = prev;
  p->next = NULL;
  prev->next = p;
  return prev->next->elements;
}

void *space_malloc_planetid_force_new_planet(Space *space, size_t size_in_bytes,
                                             size_t *planet_id) {
  return space_alloc_planetid(space, size_in_bytes, planet_id, true);
}

void *space_calloc_planetid_force_new_planet(Space *space, size_t nmemb,
                                             size_t size, size_t *planet_id) {
  size_t size_in_bytes = nmemb * size;
  void *ptr =
      space_malloc_planetid_force_new_planet(space, size_in_bytes, planet_id);
  memset(ptr, 0, size_in_bytes);
  return ptr;
}

void *space_realloc_planetid_force_new_planet(Space *space, void *ptr,
                                              size_t old_size, size_t new_size,
                                              size_t *planet_id) {

  char *new_ptr =
      space_malloc_planetid_force_new_planet(space, new_size, planet_id);
  if (new_ptr) {
    // This is to ensure memcpy() does not copy from NULL, this is undefended
    // behavior and a memory corruption.
    if (ptr) {
      // This is need if the this function shrinks the size to 0 and then the
      // caller wants to realloc a lager pointer. In the mean time another
      // allocation has taken the plant and a new planet was allocated to
      // provided the requested space.
      if (old_size) {
        memcpy(new_ptr, ptr, old_size > new_size ? new_size : old_size);
      }
    }
  }
  return new_ptr;
}

void *space_malloc_planetid(Space *space, size_t size_in_bytes,
                            size_t *planet_id) {
  return space_alloc_planetid(space, size_in_bytes, planet_id, false);
}

void *space_calloc_planetid(Space *space, size_t nmemb, size_t size,
                            size_t *planet_id) {
  size_t size_in_bytes = nmemb * size;
  void *ptr = space_malloc_planetid(space, size_in_bytes, planet_id);
  memset(ptr, 0, size_in_bytes);
  return ptr;
}

void *space_realloc_planetid(Space *space, void *ptr, size_t old_size,
                             size_t new_size, size_t *planet_id) {
  if (old_size >= new_size) {
    Planet *p = space__find_planet_from_ptr(space, ptr);
    if (p) {
      //
      // This is needed to ensure just this one allocation is in the planet.
      // If there is more than just one allocation the shrinking is not
      // possible, without keeping better track of the resulting holes and in
      // general the allocator assumes freeing all at once and not partial.
      //
      if (p->count == old_size) {
        // This is needed to achieve the free functionality that realloc
        // provides.
        p->count = new_size;
      }
      *planet_id = p->id;
      return ptr;
    }
    planet_id = NULL;
    return NULL;
  }

  if (try_to_expand_in_place(space, ptr, old_size, new_size, planet_id)) {
    return ptr;
  }

  char *new_ptr = space_malloc_planetid(space, new_size, planet_id);
  if (new_ptr) {
    // This is to ensure memcpy() does not copy from NULL, this is undefended
    // behavior and a memory corruption.
    if (ptr) {
      // This is need if the this function shrinks the size to 0 and then the
      // caller wants to realloc a lager pointer. In the mean time another
      // allocation has taken the plant and a new planet was allocated to
      // provided the requested space.
      if (old_size) {
        memcpy(new_ptr, ptr, old_size);
      }
    }
  }
  return new_ptr;
}

void *space_malloc_force_new_planet(Space *space, size_t size_in_bytes) {
  size_t id;
  return space_malloc_planetid_force_new_planet(space, size_in_bytes, &id);
}

void *space_calloc_force_new_planet(Space *space, size_t nmemb, size_t size) {
  size_t id;
  return space_calloc_planetid_force_new_planet(space, nmemb, size, &id);
}

void *space_realloc_force_new_planet(Space *space, void *ptr, size_t old_size,
                                     size_t new_size) {
  size_t id;
  return space_realloc_planetid_force_new_planet(space, ptr, old_size, new_size,
                                                 &id);
}

void *space_malloc(Space *space, size_t size_in_bytes) {
  size_t id;
  return space_malloc_planetid(space, size_in_bytes, &id);
}

void *space_calloc(Space *space, size_t nmemb, size_t size) {
  size_t id;
  return space_calloc_planetid(space, nmemb, size, &id);
}

void *space_realloc(Space *space, void *ptr, size_t old_size, size_t new_size) {
  size_t id;
  return space_realloc_planetid(space, ptr, old_size, new_size, &id);
}

bool space_init_capacity(Space *space, size_t size_in_bytes) {
  if (!space || size_in_bytes == 0) {
    return false;
  }

  size_t planet_id;
  if (space_malloc_planetid(space, size_in_bytes, &planet_id)) {
    space_reset_planet_id(space, planet_id);
    return true;
  }
  return false;
}

bool space_init_capacity_in_count_plantes(Space *space, size_t size_in_bytes,
                                          size_t count) {
  if (!space || count == 0 || size_in_bytes == 0) {
    return false;
  }

  if (count <= 16) {
    size_t ids[count];
    for (size_t i = 0; i < count; ++i) {
      if (!space_malloc_planetid(space, size_in_bytes, &ids[i])) {
        return false;
      }
    }
    for (size_t i = 0; i < count; ++i) {
      if (!space_reset_planet_id(space, ids[i])) {
        return false;
      }
    }
  } else {

    size_t *ids = malloc(sizeof(*ids) * count);
    if (!ids) {
      return false;
    }
    for (size_t i = 0; i < count; ++i) {
      if (!space_malloc_planetid(space, size_in_bytes, &ids[i])) {
        free(ids);
        return false;
      }
    }
    for (size_t i = 0; i < count; ++i) {
      if (!space_reset_planet_id(space, ids[i])) {
        free(ids);
        return false;
      }
    }

    free(ids);
  }

  return true;
}

size_t space__find_planet_id_from_ptr(Space *space, void *ptr) {
  if (!ptr || !space) {
    return 0;
  }
  if (!space->planet_count) {
    return 0;
  }
  if (!space->sun || !space->sun->elements) {
    return 0;
  }

  for (Planet *p = space->sun; p; p = p->next) {
    if ((char *)p->elements + p->capacity - (char *)ptr >= 0) {
      return p->id;
    }
  }

  return 0;
}

Planet *space__find_planet_from_ptr(Space *space, void *ptr) {
  if (!ptr || !space) {
    return NULL;
  }
  if (!space->planet_count) {
    return NULL;
  }
  if (!space->sun || !space->sun->elements) {
    return NULL;
  }

  for (Planet *p = space->sun; p; p = p->next) {
    if ((char *)p->elements + p->capacity - (char *)ptr >= 0) {
      return p;
    }
  }

  return NULL;
}

bool try_to_expand_in_place(Space *space, void *ptr, size_t old_size,
                            size_t new_size, size_t *planet_id) {

  Planet *p = space__find_planet_from_ptr(space, ptr);
  if (!p || (p->count != old_size) ||
      (p->capacity - old_size != p->count) /*This seems wrong why?*/ ||
      (p->count + new_size > p->capacity)) {
    planet_id = NULL;
    return false;
  }

  p->count = new_size;
  *planet_id = p->id;
  return true;
}

bool space_report_allocations(Space *space, Space_Report *report) {
  if (!space) {
    return false;
  }
  report->planet_count = space->planet_count;
  Planet *p = space->sun;
  while (p) {
    if (report->allocated_capacity + p->capacity > SIZE_MAX ||
        report->allocated_count + p->count > SIZE_MAX) {
      return false;
    }

    report->allocated_capacity += p->capacity;
    report->allocated_count += p->count;
    p = p->next;
  }

  return true;
}
#endif // SPACE_IMPLEMENTATION
