#ifndef SPACE_H_
#define SPACE_H_

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define SPACE_METHOD_MALLOC (0x10)
#define SPACE_METHOD_MMAP (0x20)
#define SPACE_METHOD_VIRTUAL_ALLOC (0x40)

#ifndef SPACE_ALLOC_METHOD
#define SPACE_ALLOC_METHOD (SPACE_METHOD_MALLOC)
// #define SPACE_ALLOC_METHOD (SPACE_METHOD_MMAP)
// #define SPACE_ALLOC_METHOD (SPACE_METHOD_MALLOC | SPACE_METHOD_MMAP)
// #define SPACE_ALLOC_METHOD (SPACE_METHOD_MALLOC | SPACE_METHOD_VIRTUAL_ALLOC)
#endif

#ifndef SPACE_METHOD_DEFAULT
#if SPACE_ALLOC_METHOD & SPACE_METHOD_MALLOC
#define SPACE_METHOD_DEFAULT SPACE_METHOD_MALLOC
#elif SPACE_ALLOC_METHOD & SPACE_METHOD_MMAP
#define SPACE_METHOD_DEFAULT SPACE_METHOD_MMAP
#elif SPACE_ALLOC_METHOD & SPACE_METHOD_VIRTUAL_ALLOC
#ifdef _WIN32
#define SPACE_METHOD_DEFAULT SPACE_METHOD_VIRTUAL_ALLOC
#else
#define SPACE_ALLOC_METHOD -1
#endif
#else
#define SPACE_METHOD_DEFAULT -1
#endif
#endif // SPACE_METHOD_DEFAULT
//
//
//
#if SPACE_ALLOC_METHOD & SPACE_METHOD_MALLOC
#include <stdlib.h>
#endif

#if SPACE_ALLOC_METHOD & SPACE_METHOD_MMAP
#include <sys/mman.h>
#endif

#if SPACE_ALLOC_METHOD & SPACE_METHOD_VIRTUAL_ALLOC
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#undef SPACE_ALLOC_METHOD
#define SPACE_ALLOC_METHOD -1
#endif
#endif

#ifndef SPACE_ALLOC_METHOD
#error "No specified alloc method"
#endif

#ifndef SPACEDECL
#define SPACEDECL static inline
#endif // SPACEDECL

#ifndef SPACEDEF
#define SPACEDEF static inline
#endif // SPACEDEF

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

  unsigned char alloc_method;
} Space;

SPACEDECL Planet *space_init_planet(Space *space, size_t size_in_bytes);
SPACEDECL void space_free_planet(Space *space, Planet *planet);
SPACEDECL void space_free_planet_optional_freeing_data(Space *space,
                                                       Planet *planet,
                                                       bool free_data);
SPACEDECL void space_free_space(Space *space);
SPACEDECL void space_free_space_internals_without_freeing_data(Space *space);

SPACEDECL void space_reset_planet(Planet *planet);
SPACEDECL bool space_reset_planet_id(Space *space, size_t id);

// WARNING: Dangerous to use:
// These functions sets the pointer to NULL so memory ownership is passed to the
// caller. That means the caller should free the allocated data.
SPACEDECL void space_reset_planet_and_zero(Planet *planet);
SPACEDECL bool space_reset_planet_and_zero_id(Space *space, size_t id);
SPACEDECL void space_reset_space_and_zero(Space *space);

SPACEDECL void space_reset_space(Space *space);
SPACEDECL void *space_malloc(Space *space, size_t size_in_bytes);
SPACEDECL void *space_calloc(Space *space, size_t nmemb, size_t size);
SPACEDECL void *space_realloc(Space *space, void *ptr, size_t old_size,
                              size_t new_size);
SPACEDECL void *space_alloc_planetid(Space *space, size_t size_in_bytes,
                                     size_t *planet_id, bool force_new_planet);

SPACEDECL void *space_malloc_planetid(Space *space, size_t size_in_bytes,
                                      size_t *planet_id);
SPACEDECL void *space_calloc_planetid(Space *space, size_t nmemb, size_t size,
                                      size_t *planet_id);
SPACEDECL void *space_realloc_planetid(Space *space, void *ptr, size_t old_size,
                                       size_t new_size, size_t *planet_id);

SPACEDECL void *space_malloc_force_new_planet(Space *space,
                                              size_t size_in_bytes);
SPACEDECL void *space_calloc_force_new_planet(Space *space, size_t nmemb,
                                              size_t size);
SPACEDECL void *space_realloc_force_new_planet(Space *space, void *ptr,
                                               size_t old_size,
                                               size_t new_size);

SPACEDECL void *space_malloc_planetid_force_new_planet(Space *space,
                                                       size_t size_in_bytes,
                                                       size_t *planet_id);
SPACEDECL void *space_calloc_planetid_force_new_planet(Space *space,
                                                       size_t nmemb,
                                                       size_t size,
                                                       size_t *planet_id);
SPACEDECL void *space_realloc_planetid_force_new_planet(Space *space, void *ptr,
                                                        size_t old_size,
                                                        size_t new_size,
                                                        size_t *planet_id);

SPACEDECL bool space_init_capacity(Space *space, size_t size_in_bytes);
SPACEDECL bool space_init_capacity_in_count_plantes(Space *space,
                                                    size_t size_in_bytes,
                                                    size_t count);
SPACEDECL size_t space_find_planet_id_from_ptr(Space *space, void *ptr);
SPACEDECL Planet *space_find_planet_from_ptr(Space *space, void *ptr);
SPACEDECL bool space_try_to_expand_in_place(Space *space, void *ptr,
                                            size_t old_size, size_t new_size,
                                            size_t *planet_id);

typedef struct {
  size_t planet_count;
  size_t allocated_capacity;
  size_t allocated_count;
} Space_Report;

SPACEDECL bool space_report_allocations(Space *space, Space_Report *report);

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
  } while (0)

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

SPACEDECL void *space_printf(Space *space, const char *fmt, ...);
SPACEDECL void *space_snprintf(Space *space, int n, const char *fmt, ...);
#define space_sprintf(space, fmt, ...) space_printf(space, fmt, ##__VA_ARGS__)
SPACEDECL void *space_catf(Space *space, const void *first, size_t first_len,
                           const char *fmt, ...);
SPACEDECL void *space_strcat(Space *space, const char *first,
                             const char *second);
SPACEDECL void *space_strdup(Space *space, const char *buf);
SPACEDECL void *space_strcpy(Space *space, const char *buf);
SPACEDECL void *space_strncpy(Space *space, const char *buf, size_t n);
SPACEDECL void *space_stpcpy(Space *space, const char *buf);
SPACEDECL void *space_stpncpy(Space *space, const char *buf, size_t n);
SPACEDECL void *space_memcpy(Space *space, const void *buf, size_t n);
SPACEDECL void *space_memmove(Space *space, const void *buf, size_t n);

#define space_strcatf(space, first_str, fmt, ...)                              \
  space_catf(space, first_str, first_str ? strlen(first_str) : 0, (fmt),       \
             ##__VA_ARGS__)
SPACEDECL void *space_vstrcat_impl(Space *space, const char *first, ...);
#define space_vstrcat(space, first, ...)                                       \
  space_vstrcat_impl(space, first, ##__VA_ARGS__, NULL)
SPACEDECL void *space_vcat_impl(Space *space, ...);
#define space_vcat(space, ...) space_vcat_impl(space, ##__VA_ARGS__, NULL)

SPACEDECL bool space__is_ptr_last_allocation_in_planet(Planet *p, void *ptr,
                                                       size_t ptr_size);
SPACEDECL size_t space_align(size_t alignment, size_t value);
SPACEDECL size_t space_align_power2(size_t alignment, size_t value);

// Temporary space allocator =================================================
SPACEDECL Space *space_get_tspace(void);

#define space_free_tspace() space_free_space(space_get_tspace())
#define space_reset_tspace() space_reset_space(space_get_tspace())
#define space_tmalloc(size_in_bytes)                                           \
  space_malloc(space_get_tspace(), size_in_bytes)
#define space_tcalloc(nmemb, size) space_calloc(space_get_tspace(), nmemb, size)
#define space_trealloc(ptr, old_size, new_size)                                \
  space_realloc(space_get_tspace(), ptr, old_size, new_size)

#define space_talloc_planetid(size_in_bytes, planet_id, force_new_planet)      \
  space_alloc_planetid(space_get_tspace(), size_in_bytes, planet_id,           \
                       force_new_planet)
#define space_tmalloc_planetid(size_in_bytes, planet_id)                       \
  space_malloc_planetid(space_get_tspace(), size_in_bytes, planet_id)
#define space_tcalloc_planetid(nmemb, size, planet_id)                         \
  space_calloc_planetid(space_get_tspace(), nmemb, size, planet_id)
#define space_trealloc_planetid(ptr, old_size, new_size, planet_id)            \
  space_realloc_planetid(space_get_tspace(), ptr, old_size, new_size, planet_id)
#define space_tmalloc_force_new_planet(size_in_bytes)                          \
  space_malloc_force_new_planet(space_get_tspace(), size_in_bytes)
#define space_tcalloc_force_new_planet(nmemb, size)                            \
  space_calloc_force_new_planet(space_get_tspace(), nmemb, size)
#define space_trealloc_force_new_planet(ptr, old_size, new_size)               \
  space_realloc_force_new_planet(space_get_tspace(), ptr, old_size, new_size)
#define space_tmalloc_planetid_force_new_planet(size_in_bytes, planet_id)      \
  space_malloc_planetid_force_new_planet(space_get_tspace(), size_in_bytes,    \
                                         planet_id)
#define space_tcalloc_planetid_force_new_planet(nmemb, size, planet_id)        \
  space_calloc_planetid_force_new_planet(space_get_tspace(), nmemb, size,      \
                                         planet_id)
#define space_trealloc_planetid_force_new_planet(ptr, old_size, new_size,      \
                                                 planet_id)                    \
  space_realloc_planetid_force_new_planet(space_get_tspace(), ptr, old_size,   \
                                          new_size, planet_id)

#define space_tdap(dynamic_array, element)                                     \
  space_dap_impl(space_get_tspace(), space_realloc, dynamic_array, element)
#define space_tndap(dynamic_array, element)                                    \
  space_dap_impl(space_get_tspace(), space_realloc_force_new_planet,           \
                 dynamic_array, element)
#define space_tdapc(dynamic_array, new_elements, new_elements_count)           \
  space_dapc_impl(space_get_tspace(), space_realloc, dynamic_array,            \
                  new_elements, new_elements_count)
#define space_tndapc(dynamic_array, new_elements, new_elements_count)          \
  space_dapc_impl(space_get_tspace(), space_realloc_force_new_planet,          \
                  dynamic_array, new_elements, new_elements_count)
#define space_tdapf(dynamic_array, fmt, ...)                                   \
  space_dapf_impl(space_get_tspace(), space_realloc, dynamic_array, fmt,       \
                  ##__VA_ARGS__)
#define space_tndapf(dynamic_array, fmt, ...)                                  \
  space_dapf_impl(space_get_tspace(), space_realloc_force_new_planet,          \
                  dynamic_array, fmt, ##__VA_ARGS__)

#define space_tprintf(fmt, ...)                                                \
  space_printf(space_get_tspace(), fmt, ##__VA_ARGS__)
#define space_tsprintf(fmt, ...)                                               \
  space_sprintf(space_get_tspace(), fmt, ##__VA_ARGS__)
#define space_tsnprintf(n, fmt, ...)                                           \
  space_snprintf(space_get_tspace(), n, fmt, ##__VA_ARGS__)
#define space_tcatf(first, first_len, fmt, ...)                                \
  space_catf(space_get_tspace(), first, first_len, fmt, ##__VA_ARGS__)
#define space_tstrcat(first, second)                                           \
  space_strcat(space_get_tspace(), first, second)
#define space_tstrdup(buf) space_strdup(space_get_tspace(), buf)
#define space_tstrcpy(buf) space_strcpy(space_get_tspace(), buf)
#define space_tstrncpy(buf, n) space_strncpy(space_get_tspace(), buf, n)
#define space_tstpcpy(buf) space_stpcpy(space_get_tspace(), buf)
#define space_tstpncpy(buf, n) space_stpncpy(space_get_tspace(), buf, n)
#define space_tmemcpy(buf, n) space_memcpy(space_get_tspace(), buf, n)
#define space_tmemmove(buf, n) space_memmove(space_get_tspace(), buf, n)

#define space_tstrcatf(first_str, fmt, ...)                                    \
  space_catf(space_get_tspace(), first_str, first_str ? strlen(first_str) : 0, \
             (fmt), ##__VA_ARGS__)
#define space_tvstrcat(first, ...)                                             \
  space_vstrcat_impl(space_get_tspace(), first, ##__VA_ARGS__, NULL)
#define space_tvcat(...)                                                       \
  space_vcat_impl(space_get_tspace(), ##__VA_ARGS__, NULL)

// ===========================================================================

#endif // SPACE_H_

// ===========================================================================

#ifdef SPACE_IMPLEMENTATION

/**
 * @brief Gets a pointer to the thread-local temporary space allocator.
 *
 * This function returns a static Space instance that can be used for
 * temporary allocations without needing to create and manage a Space
 * structure manually. The temporary space persists across calls and should
 * be reset or freed when no longer needed using space_reset_tspace() or
 * space_free_tspace().
 *
 * @return Pointer to the static temporary Space structure.
 */
SPACEDEF Space *space_get_tspace(void) {
  thread_local static Space space = {0};
  return &space;
}

/**
 * @brief Checks whether a given pointer represents the last allocation in a
 * planet.
 *
 * This internal function determines if a pointer points to the most recently
 * allocated memory block within a planet. This is useful for operations like
 * in-place reallocation or string concatenation that can modify memory directly
 * if the pointer is at the end of the used space.
 *
 * @param p Pointer to the Planet to check.
 * @param ptr The pointer to verify.
 * @param ptr_size The size of the memory block pointed to by ptr.
 * @return true if ptr is the last allocation in the planet and can be modified
 * in place, false otherwise.
 */
SPACEDEF bool space__is_ptr_last_allocation_in_planet(Planet *p, void *ptr,
                                                      size_t ptr_size) {
  if (!p || !ptr || ptr_size > p->count) {
    return false;
  }
  return (char *)p->elements + p->count - ptr_size == ptr;
}

/**
 * @brief Allocates memory in the space and writes a formatted string into it.
 *
 * This function works similarly to snprintf but allocates memory from
 * the space allocator instead of a static buffer. The formatted string is
 * stored in allocated memory that will be freed when the space is reset or
 * freed.

 * @param space Pointer to the Space structure used for allocation.
 * @param n Maximum number of characters that should be part of the
 * null-terminated resulting string.
 * @param fmt A null-terminated format string following printf conventions.
 * @return Pointer to the allocated formatted string, or NULL on failure.
 */
SPACEDEF void *space_snprintf(Space *space, int n, const char *fmt, ...) {
  if (!space || !fmt || n == 0) {
    return NULL;
  }

  va_list args;
  va_start(args, fmt);
  int sn = vsnprintf(NULL, 0, fmt, args);
  va_end(args);
  if (sn == -1) {
    return NULL;
  }
  if (sn < n) {
    n = sn;
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

/**
 * @brief Allocates memory in the space and writes a formatted string into it.
 *
 * This function works similarly to sprintf/snprintf but allocates memory from
 * the space allocator instead of a static buffer. The formatted string is
 * stored in allocated memory that will be freed when the space is reset or
 * freed.
 *
 * @param space Pointer to the Space structure used for allocation.
 * @param fmt A null-terminated format string following printf conventions.
 * @return Pointer to the allocated formatted string, or NULL on failure.
 */
SPACEDEF void *space_printf(Space *space, const char *fmt, ...) {
  if (!space || !fmt) {
    return NULL;
  }

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

/**
 * @brief Concatenates multiple buffers with explicit lengths in the space.
 *
 * This is a variadic helper function that takes pointers to buffers followed by
 * their lengths. It attempts to append subsequent buffers to the first buffer
 * in-place if there is sufficient capacity, otherwise it allocates new memory.
 * This is used by the space_vcat macro.
 *
 * @param space Pointer to the Space structure used for allocation.
 * @return Pointer to the concatenated buffer, or NULL on failure.
 */
SPACEDEF void *space_vcat_impl(Space *space, ...) {
  if (!space) {
    return NULL;
  }

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

  Planet *p = space_find_planet_from_ptr(space, (void *)first);
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

      memcpy(((char *)p->elements) + p->count, arg, arg_len);
      p->count += arg_len;

      arg = va_arg(args, char *);
    }
    va_end(args);
    return (void *)first;
  }

alloc: {}
  size_t count = first_len;
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
  if (ptr == NULL) {
    return NULL;
  }
  p = space_find_planet_from_ptr(space, ptr);
  p->count -= count;

  memcpy(ptr, first, first_len);
  p->count += first_len;

  va_start(args, space);
  first = va_arg(args, char *);
  first_len = va_arg(args, size_t);
  char *arg = va_arg(args, char *);
  while (arg != NULL) {
    size_t arg_len = va_arg(args, size_t);
    memcpy((char *)p->elements + p->count, arg, arg_len);
    p->count += arg_len;
    arg = va_arg(args, char *);
  }
  va_end(args);

  return ptr;
}

/**
 * @brief Concatenates multiple null-terminated strings in the space.
 *
 * This is a variadic helper function that takes null-terminated string
 * pointers. It calculates string lengths automatically using strlen. It
 * attempts to append subsequent strings to the first string in-place if there
 * is sufficient capacity in the planet, otherwise it allocates new memory. This
 * is used by the space_vstrcat macro.
 *
 * @param space Pointer to the Space structure used for allocation.
 * @param first The first null-terminated string to concatenate.
 * @return Pointer to the concatenated null-terminated string, or NULL on
 * failure.
 */
SPACEDEF void *space_vstrcat_impl(Space *space, const char *first, ...) {
  if (!space) {
    return NULL;
  }
  size_t first_len = first ? strlen(first) : 0;
  va_list args;

  Planet *p = space_find_planet_from_ptr(space, (void *)first);
  if (p && space__is_ptr_last_allocation_in_planet(p, (void *)first,
                                                   first_len + 1)) {
    size_t save = p->count;

    va_start(args, first);
    char *arg = va_arg(args, char *);

    // This is needed to ensure the first arg can be part of the va_args.
    // And the call in va_args does not expand with the previous state of
    // concatenation.
    char *place = (char *)first + first_len;
    char replace = '\0';
    if (arg) {
      replace = arg[0];
    }

    while (arg != NULL) {
      size_t arg_len = strlen(arg);
      if (p->count + arg_len > p->capacity) {
        // Restore the old allocation size to be consistent with the
        // space_strcat function.
        p->count = save;
        va_end(args);
        goto alloc;
      }
      memmove((char *)p->elements + p->count - (first ? 1 : 0), arg,
              arg_len + 1);

      *place = '\0';
      p->count += arg_len;

      arg = va_arg(args, char *);
    }
    *place = replace;

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

  void *ptr = NULL;
  if (count == 1) {
    return ptr;
  } else {
    ptr = space_malloc(space, count);
    if (ptr == NULL) {
      return NULL;
    }
    p = space_find_planet_from_ptr(space, ptr);
    p->count -= count;
  }

  if (first) {
    memcpy(ptr, first, first_len + 1);
    p->count += first_len;
  }

  va_start(args, first);
  char *arg = va_arg(args, char *);
  while (arg != NULL) {
    size_t arg_len = strlen(arg); // This is slow to compute the length again.
    memmove((char *)p->elements + p->count, arg, arg_len + 1);
    p->count += arg_len;
    arg = va_arg(args, char *);
  }
  p->count += 1;
  va_end(args);
  return ptr;
}

/**
 * @brief Concatenates an existing buffer with a formatted string in the space.
 *
 * This function appends a formatted string to an existing buffer pointer.
 * If the original pointer is at the end of a planet's used memory and there
 * is sufficient capacity, it may extend the memory in place. Otherwise, it
 * allocates new combined memory.
 *
 * @param space Pointer to the Space structure used for allocation.
 * @param first Pointer to the existing buffer to append to.
 * @param first_len Length of the first buffer.
 * @param fmt Format string to create the string to append.
 * @return Pointer to the concatenated buffer, or NULL on failure.
 */
SPACEDEF void *space_catf(Space *space, const void *first, size_t first_len,
                          const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int n = vsnprintf(NULL, 0, fmt, args);
  if (n == -1) {
    return NULL;
  }
  va_end(args);

  size_t max_count = n + 1;
  Planet *p = space_find_planet_from_ptr(space, (void *)first);
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
  if (combind_size == 1) {
    return NULL;
  }
  void *ptr = space_malloc(space, combind_size);
  if (ptr == NULL) {
    return NULL;
  }
  if (first) {
    memcpy(ptr, first, first_len);
  }
  va_start(args, fmt);
  int err = vsnprintf((char *)ptr + first_len, max_count, fmt, args);
  va_end(args);
  if (err == -1) {
    return NULL;
  }
  return ptr;
}

/**
 * @brief Concatenates two null-terminated strings in the space.
 *
 * This function combines two strings into a single string allocated from the
 * space. If the first string pointer is at the end of a planet's used memory
 * and there is sufficient capacity, it may append in place. Otherwise, it
 * allocates new combined memory.
 *
 * @param space Pointer to the Space structure used for allocation.
 * @param first The first null-terminated string.
 * @param second The second null-terminated string to append to first.
 * @return Pointer to the concatenated null-terminated string, or NULL on
 * failure.
 */
SPACEDEF void *space_strcat(Space *space, const char *first,
                            const char *second) {
  void *ptr = NULL;
  if (!space || (!first && !second)) {
    return ptr;
  }
  size_t first_len = first ? strlen(first) : 0;
  size_t second_len = second ? strlen(second) : 0;

  Planet *p = space_find_planet_from_ptr(space, (void *)first);
  if (p && (p->count + second_len <= p->capacity) &&
      space__is_ptr_last_allocation_in_planet(p, (void *)first, first_len)) {

    memmove((char *)first + first_len, second, second_len + 1);
    p->count += second_len;
    return (void *)first;
  }

  int n = first_len + second_len + 1;
  ptr = space_malloc(space, n);
  if (ptr == NULL) {
    return NULL;
  }
  if (first) {
    memcpy(ptr, first, first_len + 1);
  }
  if (second) {
    memmove((char *)ptr + first_len, second, second_len + 1);
  }
  return ptr;
}

/**
 * @brief Creates a copy of a string in the space.
 *
 * This function allocates memory in the space and copies the entire string
 * including its null terminator into the newly allocated memory. The original
 * string remains unchanged.
 *
 * @param space Pointer to the Space structure used for allocation.
 * @param buf The null-terminated string to duplicate.
 * @return Pointer to the newly allocated copy of the string, or NULL on
 * failure.
 */
SPACEDEF void *space_strdup(Space *space, const char *buf) {
  if (!space || !buf) {
    return NULL;
  }
  size_t n = strlen(buf) + 1;
  return space_memcpy(space, buf, n);
}

/**
 * @brief Copies a null-terminated string into newly allocated space memory.
 *
 * This function allocates enough memory in the space to hold the string
 * including its null terminator, then copies the entire string into the
 * allocated memory. This is similar to strdup but implemented using
 * space_memcpy internally.
 *
 * @param space Pointer to the Space structure used for allocation.
 * @param buf The null-terminated string to copy.
 * @return Pointer to the newly allocated copy of the string, or NULL on
 * failure.
 */
SPACEDEF void *space_strcpy(Space *space, const char *buf) {
  if (!space || !buf) {
    return NULL;
  }
  size_t n = strlen(buf) + 1;
  return space_memcpy(space, buf, n);
}

/**
 * @brief Copies up to n characters into newly allocated space memory.
 *
 * This function works like the standard strncpy function but allocates memory
 * from the space allocator instead. It copies at most n characters from the
 * source string. If the source is longer or equal than n, the result will not
 * be null-terminated. If the source is shorter than n, then the resulting
 * buffer will be just the length of buf plus the null byte.
 *
 * @param space Pointer to the Space structure used for allocation.
 * @param buf The null-terminated string to copy.
 * @param n Maximum number of characters to copy from buf.
 * @return Pointer to the newly allocated copy of the string, or NULL on
 * failure.
 */
SPACEDEF void *space_strncpy(Space *space, const char *buf, size_t n) {
  if (!space || !buf || n == 0) {
    return NULL;
  }
  size_t nn = strlen(buf) + 1;
  size_t len = nn > n ? n : nn;
  return space_memcpy(space, buf, len);
}

/**
 * @brief Copies a string into the space and returns a pointer to its null
 * terminator.
 *
 * This function works like the standard stpcpy function but allocates memory
 * from the space allocator. It copies the entire source string including the
 * null terminator and returns a pointer to the null terminator of the new
 * string, allowing for immediate string continuation.
 *
 * @param space Pointer to the Space structure used for allocation.
 * @param buf The null-terminated string to copy.
 * @return Pointer to the null terminator of the newly allocated string, or NULL
 * on failure.
 */
SPACEDEF void *space_stpcpy(Space *space, const char *buf) {
  if (!space || !buf) {
    return NULL;
  }
  size_t n = strlen(buf) + 1;
  char *result = space_strncpy(space, buf, n);
  if (!result) {
    return NULL;
  }
  return result + n - 1;
}

/**
 * @brief Copies up to n characters into the space and returns a pointer to the
 * end.
 *
 * This function works like the standard stpncpy function but allocates memory
 * from the space allocator. It copies at most n characters from the source
 * string and returns a pointer to either the null terminator (if reached) or
 * n characters into the destination.
 *
 * @param space Pointer to the Space structure used for allocation.
 * @param buf The null-terminated string to copy.
 * @param n Maximum number of characters to copy from buf.
 * @return Pointer to the null terminator (or n characters if no null
 * terminator), or NULL on failure.
 */
SPACEDEF void *space_stpncpy(Space *space, const char *buf, size_t n) {
  if (!space || !buf || n == 0) {
    return NULL;
  }
  size_t nn = strlen(buf) + 1;
  size_t len = nn > n ? n : nn;
  char *result = space_strncpy(space, buf, len);
  if (!result) {
    return NULL;
  }
  if (nn > n) {
    return result + len;
  }
  return result + len - 1;
}

/**
 * @brief Allocates memory in the space and copies raw bytes into it.
 *
 * This function allocates the specified number of bytes in the space and
 * copies the given memory content into the newly allocated buffer. This is
 * the most general-purpose function for copying any type of data into space
 * memory, similar to the standard memcpy function.
 *
 * @param space Pointer to the Space structure used for allocation.
 * @param buf Pointer to the memory to copy.
 * @param n Number of bytes to copy from buf.
 * @return Pointer to the newly allocated copy of the memory, or NULL on
 * failure.
 */
SPACEDEF void *space_memcpy(Space *space, const void *buf, size_t n) {
  if (!space || !buf || n == 0) {
    return NULL;
  }
  void *ptr = space_malloc(space, n);
  if (ptr == NULL) {
    return NULL;
  }
  memcpy(ptr, buf, n);
  return ptr;
}

/**
 * @brief Allocates memory in the space and copies bytes, handling overlapping
 * regions.
 *
 * This function allocates the specified number of bytes in the space and copies
 * the given memory content into the newly allocated buffer. Unlike memcpy,
 * this function correctly handles cases where the source and destination
 * memory regions overlap.
 *
 * @param space Pointer to the Space structure used for allocation.
 * @param buf Pointer to the memory to copy.
 * @param n Number of bytes to copy from buf.
 * @return Pointer to the newly allocated copy of the memory, or NULL on
 * failure.
 */
SPACEDEF void *space_memmove(Space *space, const void *buf, size_t n) {
  if (!space || !buf || n == 0) {
    return NULL;
  }
  void *ptr = space_malloc(space, n);
  if (ptr == NULL) {
    return NULL;
  }
  memmove(ptr, buf, n);
  return ptr;
}

/**
 * @brief Creates and initializes a new planet memory area with the specified
 * capacity.
 *
 * A planet is a memory area that can hold multiple allocations. This function
 * allocates a new planet structure and allocates memory for its data buffer
 * with the specified capacity. The planet is assigned a unique ID that can be
 * used to identify allocations from this planet.
 *
 * @param space Pointer to the Space structure that will contain this planet.
 * @param size_in_bytes The initial capacity of the planet's data buffer in
 * bytes.
 * @return Pointer to the newly created Planet, or NULL on allocation failure.
 */
SPACEDEF Planet *space_init_planet(Space *space, size_t size_in_bytes) {
  Planet *planet = NULL;
method_rerun:
  switch (space->alloc_method) {
  case 0:
    space->alloc_method = SPACE_METHOD_DEFAULT;
    goto method_rerun;
#if SPACE_ALLOC_METHOD & SPACE_METHOD_MALLOC
  case SPACE_METHOD_MALLOC:
    planet = malloc(sizeof(*planet));
    if (planet) {
      memset(planet, 0, sizeof(*planet));
      planet->capacity = size_in_bytes;
      planet->count = 0;
      planet->elements = malloc(planet->capacity);
      if (!planet->elements) {
        free(planet);
        return NULL;
      }

      // The '1+' is needed because 0 is an invalid id and
      // space_find_planet_id_from_ptr() returns 0 if it could not be found.
      // This allows to use size_t and still return an error value.
      planet->id = 1 + space->id_counter++;
    }
    break;
#endif
#if SPACE_ALLOC_METHOD & SPACE_METHOD_MMAP
  case SPACE_METHOD_MMAP:
    planet = mmap(NULL, sizeof(*planet), PROT_READ | PROT_WRITE,
                  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (planet != MAP_FAILED) {
      memset(planet, 0, sizeof(*planet));
      planet->capacity = size_in_bytes;
      planet->count = 0;
      planet->elements = mmap(NULL, planet->capacity, PROT_READ | PROT_WRITE,
                              MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
      if (planet->elements == MAP_FAILED) {
        munmap(planet, sizeof(*planet));
        return NULL;
      }

      // The '1+' is needed because 0 is an invalid id and
      // space_find_planet_id_from_ptr() returns 0 if it could not be found.
      // This allows to use size_t and still return an error value.
      planet->id = 1 + space->id_counter++;
    }
    break;
#endif
#if SPACE_ALLOC_METHOD & SPACE_METHOD_VIRTUAL_ALLOC
  case SPACE_METHOD_VIRTUAL_ALLOC:
    planet = VirtualAllocEx(GetCurrentProcess(), NULL, sizeof(*planet),
                            MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!INV_HANDLE(planet)) {
      memset(planet, 0, sizeof(*planet));
      planet->capacity = size_in_bytes;
      planet->count = 0;
      planet->elements =
          VirtualAllocEx(GetCurrentProcess(), NULL, planet->capacity,
                         MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
      if (INV_HANDLE(planet->elements)) {
        VirtualFreeEx(GetCurrentProcess(), (LPVOID)planet, sizeof(*planet),
                      MEM_RELEASE);
        return NULL;
      }

      // The '1+' is needed because 0 is an invalid id and
      // space_find_planet_id_from_ptr() returns 0 if it could not be found.
      // This allows to use size_t and still return an error value.
      planet->id = 1 + space->id_counter++;
    }

    break;
#endif
  default:
    assert(false && "UNREACHABLE: This allocation method is not supported");
  }

  return planet;
}

/**
 * @brief Frees a planet and all its allocated data from the space.
 *
 * This function removes the planet from the space's linked list and frees both
 * the planet structure and its data buffer. All allocations within this planet
 * become invalid after this call.
 *
 * @param space Pointer to the Space structure containing the planet.
 * @param planet Pointer to the Planet to free.
 */
SPACEDEF void space_free_planet(Space *space, Planet *planet) {
  space_free_planet_optional_freeing_data(space, planet, true);
}

/**
 * @brief Frees a planet with control over whether to free the underlying data
 * buffer.
 *
 * This function removes the planet from the space's linked list. The free_data
 * parameter controls whether the planet's data buffer is also freed. When
 * false, only the planet structure is freed, leaving the data buffer allocated
 * - useful for scenarios where the caller wants to take ownership of the data.
 *
 * @param space Pointer to the Space structure containing the planet.
 * @param planet Pointer to the Planet to free.
 * @param free_data If true, frees both the planet structure and its data
 * buffer; if false, only frees the planet structure itself.
 */
SPACEDEF void space_free_planet_optional_freeing_data(Space *space,
                                                      Planet *planet,
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
  method_rerun1:
    switch (space->alloc_method) {
    case 0:
      space->alloc_method = SPACE_METHOD_DEFAULT;
      goto method_rerun1;
#if SPACE_ALLOC_METHOD & SPACE_METHOD_MALLOC
    case SPACE_METHOD_MALLOC:
      free(planet->elements);
      break;
#endif
#if SPACE_ALLOC_METHOD & SPACE_METHOD_MMAP
    case SPACE_METHOD_MMAP:
      munmap(planet->elements, planet->capacity);
      break;
#endif
#if SPACE_ALLOC_METHOD & SPACE_METHOD_VIRTUAL_ALLOC
    case SPACE_METHOD_VIRTUAL_ALLOC:
      VirtualFreeEx(GetCurrentProcess(), (LPVOID)planet->elements,
                    planet->capacity, MEM_RELEASE);
      break;
#endif
    default:
      assert(false && "UNREACHABLE: This allocation method is not supported");
    }

    planet->elements = NULL;
  }
  planet->count = 0;
  planet->capacity = 0;
  planet->next = NULL;
  planet->prev = NULL;
  planet->id = 0;

method_rerun2:
  switch (space->alloc_method) {
  case 0:
    space->alloc_method = SPACE_METHOD_DEFAULT;
    goto method_rerun2;
#if SPACE_ALLOC_METHOD & SPACE_METHOD_MALLOC
  case SPACE_METHOD_MALLOC:
    free(planet);
    break;
#endif
#if SPACE_ALLOC_METHOD & SPACE_METHOD_MMAP
  case SPACE_METHOD_MMAP:
    munmap(planet, sizeof(*planet));
    break;
#endif
#if SPACE_ALLOC_METHOD & SPACE_METHOD_VIRTUAL_ALLOC
  case SPACE_METHOD_VIRTUAL_ALLOC:
    VirtualFreeEx(GetCurrentProcess(), (LPVOID)planet, sizeof(*planet),
                  MEM_RELEASE);
    break;
#endif
  default:
    assert(false && "UNREACHABLE: This allocation method is not supported");
  }
  planet = NULL;
  space->planet_count--;
}

/**
 * @brief Frees all planets in the space and releases all allocated memory.
 *
 * This function iterates through all planets in the space, freeing each planet
 * and its data buffer. After this call, all memory managed by the space is
 * released and the space is left in an empty state with zero planets.
 *
 * @param space Pointer to the Space structure to free.
 */
SPACEDEF void space_free_space(Space *space) {
  while (space->sun) {
    space_free_planet(space, space->sun);
  }
  assert(space->planet_count == 0);
  assert(space->sun == NULL);
}

/**
 * @brief Frees the internal structures of the space without freeing the
 * allocated data.
 *
 * The function is useful if you have to unit test a function that depends on
 * the space allocator, but you want to be sure to track every memory
 * allocated manually, so you free manually. But as a result calling
 * space_free_space() would result in double free so just free the structure.
 *
 * @param space Pointer to the Space structure.
 */
SPACEDEF void space_free_space_internals_without_freeing_data(Space *space) {
  while (space->sun) {
    space_free_planet_optional_freeing_data(space, space->sun, false);
  }
  assert(space->planet_count == 0);
  assert(space->sun == NULL);
}

/**
 * @brief Resets a planet's used count to zero, making all its memory available
 * for reuse.
 *
 * This function resets the planet's internal counter to zero without freeing
 * any memory. All previously allocated data in the planet becomes inaccessible
 * but the memory remains allocated. This is useful for reusing a planet's
 * capacity without the overhead of reallocation.
 *
 * @param planet Pointer to the Planet to reset.
 */
SPACEDEF void space_reset_planet(Planet *planet) { planet->count = 0; }

/**
 * @brief Resets a planet and zeros out all its memory, passing ownership to the
 * caller.
 *
 * WARNING: This function zeros all memory in the planet and sets internal
 * pointers to NULL so that memory ownership is effectively transferred to the
 * caller. After calling this function, the caller is responsible for freeing
 * the allocated data that was previously managed by the space. This is useful
 * for taking snapshots or when you need to manually manage memory lifecycle.
 *
 * @param planet Pointer to the Planet to reset and zero.
 */
SPACEDEF void space_reset_planet_and_zero(Planet *planet) {
  if (!planet) {
    return;
  }
  if (planet->elements) {
    memset(planet->elements, 0, planet->capacity);
  }
  planet->count = 0;
}

/**
 * @brief Resets a planet by looking it up using its unique identifier.
 *
 * This function searches the space for a planet with the specified ID and
 * resets its used count to zero, making all its memory available for reuse.
 * The planet retains its capacity and allocated data buffer.
 *
 * @param space Pointer to the Space structure to search.
 * @param id The unique identifier of the planet to reset.
 * @return true if a planet with the given ID was found and reset; false
 * otherwise.
 */
SPACEDEF bool space_reset_planet_id(Space *space, size_t id) {
  for (Planet *planet = space->sun; planet; planet = planet->next) {
    if (planet->id == id) {
      space_reset_planet(planet);
      return true;
    }
  }
  return false;
}

/**
 * @brief Resets and zeros a planet by its ID.
 *
 * WARNING: This sets the pointer to NULL so memory ownership is passed to the
 * caller. The caller should free the allocated data.
 *
 * @param space Pointer to the Space structure.
 * @param id The ID of the planet to reset and zero.
 * @return true if the planet was found and reset, false otherwise.
 */
SPACEDEF bool space_reset_planet_and_zero_id(Space *space, size_t id) {
  for (Planet *planet = space->sun; planet; planet = planet->next) {
    if (planet->id == id) {
      space_reset_planet_and_zero(planet);
      return true;
    }
  }
  return false;
}

/**
 * @brief Resets all planets in the space, making all memory available for
 * reuse.
 *
 * This function iterates through all planets in the space and resets each one's
 * used count to zero. This makes all previously allocated memory in all planets
 * available for new allocations without freeing and reallocating. The data in
 * the planets is not cleared but becomes inaccessible until overwritten.
 *
 * @param space Pointer to the Space structure to reset.
 */
SPACEDEF void space_reset_space(Space *space) {
  for (Planet *p = space->sun; p; p = p->next) {
    space_reset_planet(p);
  }
}

/**
 * @brief Resets all planets in the space and zeros out all their memory.
 *
 * WARNING: This function zeros all memory in all planets and sets internal
 * pointers to NULL so that memory ownership is effectively transferred to the
 * caller. After calling this function, the caller is responsible for freeing
 * all data that was previously managed by the space. This is useful for taking
 * snapshots or when you need to manually manage the memory lifecycle of all
 * allocations.
 *
 * @param space Pointer to the Space structure to reset and zero.
 */
SPACEDEF void space_reset_space_and_zero(Space *space) {
  for (Planet *p = space->sun; p; p = p->next) {
    space_reset_planet_and_zero(p);
  }
}

/**
 * @brief Core allocation function that allocates memory and optionally returns
 * the planet ID.
 *
 * This is the main internal allocation function that handles memory allocation
 * from the space. It searches existing planets for sufficient free space, and
 * if found, allocates from there. If force_new_planet is true or no existing
 * planet has enough space, a new planet is created. The planet ID is useful for
 * tracking which planet an allocation came from.
 *
 * @param space Pointer to the Space structure.
 * @param size_in_bytes The number of bytes to allocate.
 * @param planet_id Pointer to store the ID of the planet where memory was
 * allocated; this value is set to 0 on failure.
 * @param force_new_planet If true, always allocates in a new planet; if false,
 *                         attempts to use existing planet space first.
 * @return Pointer to the allocated memory, or NULL on failure.
 */
SPACEDEF void *space_alloc_planetid(Space *space, size_t size_in_bytes,
                                    size_t *planet_id, bool force_new_planet) {
  if (!space) {
    *planet_id = 0;
    return NULL;
  }

  if (!space->planet_count) {
    space->sun = space_init_planet(space, size_in_bytes);
    if (!space->sun) {
      *planet_id = 0;
      return NULL;
    }
    space->sun->count = size_in_bytes;
    space->planet_count++;
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

    size_t align_pcount = space_align_power2(8, p->count);
    if (align_pcount + size_in_bytes > p->capacity) {
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

    p->count = align_pcount;
    void *place = &((char *)p->elements)[p->count];
    p->count += size_in_bytes;
    *planet_id = p->id;
    return place;
  }

  p = space_init_planet(space, size_in_bytes);
  if (!p) {
    *planet_id = 0;
    return NULL;
  }
  p->count = size_in_bytes;
  space->planet_count++;
  *planet_id = p->id;

  p->prev = prev;
  p->next = NULL;
  prev->next = p;
  return prev->next->elements;
}

/**
 * @brief Allocates memory in a brand new planet, bypassing existing planet
 * capacity.
 *
 * This function always creates a new planet for the allocation rather than
 * using available space in existing planets. This is useful when you need to
 * isolate an allocation into its own memory area, which can be freed
 * independently.
 *
 * @param space Pointer to the Space structure.
 * @param size_in_bytes The number of bytes to allocate.
 * @param planet_id Pointer to store the ID of the newly created planet.
 * @return Pointer to the allocated memory, or NULL on failure.
 */
SPACEDEF void *space_malloc_planetid_force_new_planet(Space *space,
                                                      size_t size_in_bytes,
                                                      size_t *planet_id) {
  return space_alloc_planetid(space, size_in_bytes, planet_id, true);
}

/**
 * @brief Allocates zero-initialized memory array in a brand new planet.
 *
 * This function allocates an array of nmemb elements, each of the specified
 * size, in a newly created planet. All bytes are initialized to zero, making
 * this useful for allocating arrays, structures, or any data that needs to be
 * initially zeroed.
 *
 * @param space Pointer to the Space structure.
 * @param nmemb Number of elements to allocate.
 * @param size Size of each element in bytes.
 * @param planet_id Pointer to store the ID of the newly created planet.
 * @return Pointer to the allocated zero-initialized memory, or NULL on failure.
 */
SPACEDEF void *space_calloc_planetid_force_new_planet(Space *space,
                                                      size_t nmemb, size_t size,
                                                      size_t *planet_id) {
  size_t size_in_bytes = nmemb * size;
  void *ptr =
      space_malloc_planetid_force_new_planet(space, size_in_bytes, planet_id);
  if (ptr) {
    memset(ptr, 0, size_in_bytes);
  }
  return ptr;
}

/**
 * @brief Reallocates memory in a new planet, copying data if necessary.
 *
 * @param space Pointer to the Space structure.
 * @param ptr Pointer to the existing memory to reallocate.
 * @param old_size The size of the existing memory.
 * @param new_size The new size to allocate.
 * @param planet_id Pointer to store the ID of the newly created planet.
 * @return Pointer to the reallocated memory, or NULL on failure.
 */
SPACEDEF void *space_realloc_planetid_force_new_planet(Space *space, void *ptr,
                                                       size_t old_size,
                                                       size_t new_size,
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

/**
 * @brief Allocates memory in the space and returns the planet ID for tracking.
 *
 * This function allocates memory from the space, attempting to use existing
 * planet capacity first before creating new planets. The returned planet ID
 * can be used to identify which planet the allocation came from, which is
 * useful for debugging or memory management.
 *
 * @param space Pointer to the Space structure.
 * @param size_in_bytes The number of bytes to allocate.
 * @param planet_id Pointer to store the ID of the planet where memory was
 * allocated; this value is set to 0 on failure.
 * @return Pointer to the allocated memory, or NULL on failure.
 */
SPACEDEF void *space_malloc_planetid(Space *space, size_t size_in_bytes,
                                     size_t *planet_id) {
  return space_alloc_planetid(space, size_in_bytes, planet_id, false);
}

/**
 * @brief Allocates a zero-initialized array in the space and returns the planet
 * ID.
 *
 * This function allocates an array of nmemb elements, each of the specified
 * size, from the space. All bytes are initialized to zero. This is useful for
 * allocating arrays, structures, or any data that needs to start in a zeroed
 * state.
 *
 * @param space Pointer to the Space structure.
 * @param nmemb Number of elements to allocate.
 * @param size Size of each element in bytes.
 * @param planet_id Pointer to store the ID of the planet where memory was
 * allocated; this value is set to 0 on failure.
 * @return Pointer to the allocated zero-initialized memory, or NULL on failure.
 */
SPACEDEF void *space_calloc_planetid(Space *space, size_t nmemb, size_t size,
                                     size_t *planet_id) {
  size_t size_in_bytes = nmemb * size;
  void *ptr = space_malloc_planetid(space, size_in_bytes, planet_id);
  if (ptr) {
    memset(ptr, 0, size_in_bytes);
  }
  return ptr;
}

/**
 * @brief Changes the size of an existing allocation, potentially moving it.
 *
 * This function attempts to resize an existing allocation to a new size. If the
 * current allocation is at the end of a planet with enough free space following
 * it, it may be expanded in place. Otherwise, a new allocation is made and the
 * old data is copied. This is similar to the standard realloc function.
 *
 * @param space Pointer to the Space structure.
 * @param ptr Pointer to the existing memory to resize.
 * @param old_size The size of the existing memory block.
 * @param new_size The new desired size.
 * @param planet_id Pointer to store the ID of the planet where memory is
 * allocated; this value is set to 0 on failure.
 * @return Pointer to the resized memory, or NULL on failure.
 */
SPACEDEF void *space_realloc_planetid(Space *space, void *ptr, size_t old_size,
                                      size_t new_size, size_t *planet_id) {
  if (old_size >= new_size) {
    Planet *p = space_find_planet_from_ptr(space, ptr);
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
    *planet_id = 0;
    return NULL;
  }

  if (space_try_to_expand_in_place(space, ptr, old_size, new_size, planet_id)) {
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
        if (ptr == new_ptr) {
          memmove(new_ptr, ptr, old_size);
        } else {
          memcpy(new_ptr, ptr, old_size);
        }
      }
    }
  }
  return new_ptr;
}

/**
 * @brief Allocates memory in a newly created planet, isolating the allocation.
 *
 * This function always creates a new planet for the allocation rather than
 * using available space in existing planets. This ensures the allocation is
 * isolated in its own memory area, which can be useful for independent
 * lifetime management.
 *
 * @param space Pointer to the Space structure.
 * @param size_in_bytes The number of bytes to allocate.
 * @return Pointer to the allocated memory, or NULL on failure.
 */
SPACEDEF void *space_malloc_force_new_planet(Space *space,
                                             size_t size_in_bytes) {
  size_t id;
  return space_malloc_planetid_force_new_planet(space, size_in_bytes, &id);
}

/**
 * @brief Allocates a zero-initialized array in a newly created planet.
 *
 * This function allocates an array of nmemb elements in a new planet, with all
 * bytes initialized to zero. This is useful for allocating arrays or structures
 * that need to start in a zeroed state while keeping the allocation isolated.
 *
 * @param space Pointer to the Space structure.
 * @param nmemb Number of elements to allocate.
 * @param size Size of each element in bytes.
 * @return Pointer to the allocated zero-initialized memory, or NULL on failure.
 */
SPACEDEF void *space_calloc_force_new_planet(Space *space, size_t nmemb,
                                             size_t size) {
  size_t id;
  return space_calloc_planetid_force_new_planet(space, nmemb, size, &id);
}

/**
 * @brief Resizes an allocation by creating a new planet and copying data.
 *
 * This function always allocates new memory in a fresh planet rather than
 * trying to expand in place or use existing planet space. The old data is
 * copied to the new location. This ensures the allocation is isolated in its
 * own area.
 *
 * @param space Pointer to the Space structure.
 * @param ptr Pointer to the existing memory to resize.
 * @param old_size The size of the existing memory block.
 * @param new_size The new desired size.
 * @return Pointer to the resized memory, or NULL on failure.
 */
SPACEDEF void *space_realloc_force_new_planet(Space *space, void *ptr,
                                              size_t old_size,
                                              size_t new_size) {
  size_t id;
  return space_realloc_planetid_force_new_planet(space, ptr, old_size, new_size,
                                                 &id);
}

/**
 * @brief Allocates raw memory from the space allocator.
 *
 * This is the primary allocation function for the space allocator. It searches
 * existing planets for available space and uses that first, creating a new
 * planet only when necessary. The allocated memory is uninitialized and
 * contains garbage values, similar to the standard malloc function.
 *
 * @param space Pointer to the Space structure.
 * @param size_in_bytes The number of bytes to allocate.
 * @return Pointer to the allocated uninitialized memory, or NULL on failure.
 */
SPACEDEF void *space_malloc(Space *space, size_t size_in_bytes) {
  size_t id;
  return space_malloc_planetid(space, size_in_bytes, &id);
}

/**
 * @brief Allocates a zero-initialized array from the space.
 *
 * This function allocates an array of nmemb elements, each of the specified
 * size, from the space allocator. All bytes are initialized to zero. This is
 * useful for allocating arrays, structures, or any data that needs to start in
 * a cleared state.
 *
 * @param space Pointer to the Space structure.
 * @param nmemb Number of elements to allocate.
 * @param size Size of each element in bytes.
 * @return Pointer to the allocated zero-initialized memory, or NULL on failure.
 */
SPACEDEF void *space_calloc(Space *space, size_t nmemb, size_t size) {
  size_t id;
  return space_calloc_planetid(space, nmemb, size, &id);
}

/**
 * @brief Resizes an existing allocation within the space.
 *
 * This function changes the size of a previously allocated memory block. If the
 * current allocation is at the end of a planet with enough free space, it may
 * be expanded in place without copying. Otherwise, new memory is allocated and
 * the old data is copied over. This behaves similarly to the standard realloc.
 *
 * @param space Pointer to the Space structure.
 * @param ptr Pointer to the existing memory to resize.
 * @param old_size The size of the existing memory block.
 * @param new_size The new desired size.
 * @return Pointer to the resized memory, or NULL on failure.
 */
SPACEDEF void *space_realloc(Space *space, void *ptr, size_t old_size,
                             size_t new_size) {
  size_t id;
  return space_realloc_planetid(space, ptr, old_size, new_size, &id);
}

/**
 * @brief Pre-allocates a planet with the specified capacity in the space.
 *
 * This function creates a new planet with enough space for the specified number
 * of bytes, then immediately resets it to make the memory available for use.
 * This is useful for preparing the space with a known capacity before
 * performing multiple allocations, which can improve performance by reducing
 * reallocation.
 *
 * @param space Pointer to the Space structure.
 * @param size_in_bytes The capacity to pre-allocate in bytes.
 * @return true if capacity was successfully initialized, false otherwise.
 */
SPACEDEF bool space_init_capacity(Space *space, size_t size_in_bytes) {
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

/**
 * @brief Pre-allocates multiple planets with the specified capacity.
 *
 * This function creates multiple planets, each with the specified capacity,
 * then immediately resets them to make all the memory available. This is useful
 * for preparing the space with a known number of planet before performing
 * multiple allocations, which can improve performance by reducing reallocation.
 *
 * @param space Pointer to the Space structure.
 * @param size_in_bytes The capacity for each planet in bytes.
 * @param count The number of planets to create and initialize.
 * @return true if all planets were successfully initialized, false otherwise.
 */
SPACEDEF bool space_init_capacity_in_count_plantes(Space *space,
                                                   size_t size_in_bytes,
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
    void *ids = NULL;
    size_t size_to_alloc = sizeof(*(size_t *)ids) * count;
  method_rerun1:
    switch (space->alloc_method) {
    case 0:
      space->alloc_method = SPACE_METHOD_DEFAULT;
      goto method_rerun1;
#if SPACE_ALLOC_METHOD & SPACE_METHOD_MALLOC
    case SPACE_METHOD_MALLOC:
      ids = malloc(size_to_alloc);
      if (!ids) {
        return false;
      }
      break;
#endif
#if SPACE_ALLOC_METHOD & SPACE_METHOD_MMAP
    case SPACE_METHOD_MMAP:
      ids = mmap(NULL, size_to_alloc, PROT_READ | PROT_WRITE,
                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
      if (ids == MAP_FAILED) {
        return false;
      }
      break;
#endif
#if SPACE_ALLOC_METHOD & SPACE_METHOD_VIRTUAL_ALLOC
    case SPACE_METHOD_VIRTUAL_ALLOC:
      ids = VirtualAllocEx(GetCurrentProcess(), NULL, size_to_alloc,
                           MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

      if (INV_HANDLE(ids)) {
        return false;
      }
      break;
#endif
    default:
      assert(false && "UNREACHABLE: This allocation method is not supported");
    }

    for (size_t i = 0; i < count; ++i) {
      if (!space_malloc_planetid(space, size_in_bytes, &((size_t *)ids)[i])) {
      method_rerun2:
        switch (space->alloc_method) {
        case 0:
          space->alloc_method = SPACE_METHOD_DEFAULT;
          goto method_rerun2;
#if SPACE_ALLOC_METHOD & SPACE_METHOD_MALLOC
        case SPACE_METHOD_MALLOC:
          free(ids);
          return false;
#endif
#if SPACE_ALLOC_METHOD & SPACE_METHOD_MMAP
        case SPACE_METHOD_MMAP:
          munmap(ids, size_to_alloc);
          return false;
#endif
#if SPACE_ALLOC_METHOD & SPACE_METHOD_VIRTUAL_ALLOC
        case SPACE_METHOD_VIRTUAL_ALLOC:
          VirtualFreeEx(GetCurrentProcess(), (LPVOID)ids, size_to_alloc,
                        MEM_RELEASE);
          return false;
#endif
        default:
          assert(false &&
                 "UNREACHABLE: This allocation method is not supported");
        }
      }
    }
    for (size_t i = 0; i < count; ++i) {
      if (!space_reset_planet_id(space, ((size_t *)ids)[i])) {
      method_rerun3:
        switch (space->alloc_method) {
        case 0:
          space->alloc_method = SPACE_METHOD_DEFAULT;
          goto method_rerun3;
#if SPACE_ALLOC_METHOD & SPACE_METHOD_MALLOC
        case SPACE_METHOD_MALLOC:
          free(ids);
          return false;
#endif
#if SPACE_ALLOC_METHOD & SPACE_METHOD_MMAP
        case SPACE_METHOD_MMAP:
          munmap(ids, size_to_alloc);
          return false;
#endif
#if SPACE_ALLOC_METHOD & SPACE_METHOD_VIRTUAL_ALLOC
        case SPACE_METHOD_VIRTUAL_ALLOC:
          VirtualFreeEx(GetCurrentProcess(), (LPVOID)ids, size_to_alloc,
                        MEM_RELEASE);
          return false;
#endif
        default:
          assert(false &&
                 "UNREACHABLE: This allocation method is not supported");
        }
      }
    }

  method_rerun4:
    switch (space->alloc_method) {
    case 0:
      space->alloc_method = SPACE_METHOD_DEFAULT;
      goto method_rerun4;
#if SPACE_ALLOC_METHOD & SPACE_METHOD_MALLOC
    case SPACE_METHOD_MALLOC:
      free(ids);
      break;
#endif
#if SPACE_ALLOC_METHOD & SPACE_METHOD_MMAP
    case SPACE_METHOD_MMAP:
      munmap(ids, size_to_alloc);
      break;
#endif
#if SPACE_ALLOC_METHOD & SPACE_METHOD_VIRTUAL_ALLOC
    case SPACE_METHOD_VIRTUAL_ALLOC:
      VirtualFreeEx(GetCurrentProcess(), (LPVOID)ids, size_to_alloc,
                    MEM_RELEASE);
      break;
#endif
    default:
      assert(false && "UNREACHABLE: This allocation method is not supported");
    }
  }

  return true;
}

/**
 * @brief Looks up which planet a pointer belongs to and returns its ID.
 *
 * This function searches through all planets in the space to find which one
 * contains the given pointer. It checks if the pointer falls within the memory
 * range of any planet's data buffer. Returns 0 if the pointer is not found
 * in any planet.
 *
 * @param space Pointer to the Space structure to search.
 * @param ptr Pointer to the memory address to find the planet for.
 * @return The unique ID of the planet containing the pointer, or 0 if not
 * found.
 */
SPACEDEF size_t space_find_planet_id_from_ptr(Space *space, void *ptr) {
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
    if ((char *)p->elements <= (char *)ptr &&
        (char *)p->elements + p->capacity >= (char *)ptr) {
      return p->id;
    }
  }

  return 0;
}

/**
 * @brief Looks up and returns the planet structure containing a pointer.
 *
 * This function searches through all planets in the space to find which one
 * contains the given pointer. It checks if the pointer falls within the memory
 * range of any planet's data buffer. Returns NULL if the pointer is not found.
 *
 * @param space Pointer to the Space structure to search.
 * @param ptr Pointer to the memory address to find the planet for.
 * @return Pointer to the Planet structure containing the pointer, or NULL if
 * not found.
 */
SPACEDEF Planet *space_find_planet_from_ptr(Space *space, void *ptr) {
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
    if ((char *)p->elements <= (char *)ptr &&
        (char *)p->elements + p->capacity >= (char *)ptr) {
      return p;
    }
  }

  return NULL;
}

/**
 * @brief Attempts to expand a memory allocation in place without moving it.
 *
 * This function checks if an existing allocation can be extended without
 * reallocating and moving the data. It succeeds only if the pointer is at
 * the end of a planet's used memory and there is enough free capacity following
 * it. This is more efficient than a full reallocation when possible.
 *
 * @param space Pointer to the Space structure.
 * @param ptr Pointer to the existing memory to expand.
 * @param old_size The current size of the allocation in bytes.
 * @param new_size The desired new size in bytes.
 * @param planet_id Pointer to store the planet ID if expansion succeeds;
 *                 set to 0 if expansion fails.
 * @return true if the memory was successfully expanded in place; false
 * otherwise.
 */
SPACEDEF bool space_try_to_expand_in_place(Space *space, void *ptr,
                                           size_t old_size, size_t new_size,
                                           size_t *planet_id) {

  Planet *p = space_find_planet_from_ptr(space, ptr);
  if (!p || !space__is_ptr_last_allocation_in_planet(p, ptr, old_size) ||
      (p->count - old_size + new_size > p->capacity)) {
    *planet_id = 0;
    return false;
  }

  p->count = p->count - old_size + new_size;
  *planet_id = p->id;
  return true;
}

/**
 * @brief Gathers statistics about current memory usage in the space.
 *
 * This function collects information about all planets in the space, including
 * the total number of planets, the total allocated capacity across all planets,
 * and the total used memory (count). This is useful for debugging, logging,
 * or implementing memory usage tracking.
 *
 * @param space Pointer to the Space structure to report on.
 * @param report Pointer to the Space_Report structure that will be filled
 *               with the current memory statistics.
 * @return true if the report was successfully generated; false if space is NULL
 *         or if the statistics would overflow.
 */
SPACEDEF bool space_report_allocations(Space *space, Space_Report *report) {
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

/**
 * @brief Aligns a value up to the specified alignment boundary.
 *
 * This function rounds up the given value to the next multiple of the
 * alignment. For example, with alignment of 8, a value of 5 becomes 8, 8
 * becomes 8, and 9 becomes 16. This is useful for ensuring memory or index
 * alignment.
 *
 * @param alignment The alignment boundary to use (must be > 0).
 * @param value The value to align.
 * @return The smallest value >= the input value that is a multiple of
 * alignment.
 */
SPACEDEF size_t space_align(size_t alignment, size_t value) {
  if (alignment == 0) {
    return value;
  }
  return ((value + alignment - 1) / alignment) * alignment;
}

/**
 * @brief Aligns a value to a power-of-two alignment boundary using bitwise
 * operations.
 *
 * This function efficiently rounds up the value to the next multiple of a
 * power-of-two alignment using bitwise AND. This is faster than division-based
 * alignment for power-of-two values. The alignment must be a power of 2.
 *
 * @param alignment The power-of-two alignment boundary to use.
 * @param value The value to align.
 * @return The smallest value >= the input value that is a multiple of
 * alignment.
 */
SPACEDEF size_t space_align_power2(size_t alignment, size_t value) {
  if (alignment == 0) {
    return value;
  }
  assert(alignment % 2 == 0 && "INCORRECT ALLIMENT VALUE");
  return (value + alignment - 1) & ~(alignment - 1);
}

#endif // SPACE_IMPLEMENTATION
