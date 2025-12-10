#ifndef SPACE_H_
#define SPACE_H_

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
} Space;

Planet *space_init_planet(Space *space, size_t size_in_bytes);
void space_free_planet(Space *space, Planet *planet);
void space_free_space(Space *space);
void space_reset_planet(Planet *planet);
void space_reset_planet_and_zero(Planet *planet);
bool space_reset_planet_id(Space *space, size_t id);
bool space_reset_planet_and_zero_id(Space *space, size_t id);
void space_reset_space(Space *space);
void space_reset_space_and_zero(Space *space);
void *space_malloc(Space *space, size_t size_in_bytes);
void *space_calloc(Space *space, size_t nmemb, size_t size);
void *space_realloc(Space *space, void *ptr, size_t old_size, size_t new_size);

void *space_malloc_planetid(Space *space, size_t size_in_bytes,
                            size_t *planet_id);
void *space_calloc_planetid(Space *space, size_t nmemb, size_t size,
                            size_t *planet_id);
void *space_realloc_planetid(Space *space, void *ptr, size_t old_size,
                             size_t new_size, size_t *planet_id);

bool space_init_capacity(Space *space, size_t size_in_bytes);
size_t space__find_planet_id_from_ptr(Space *space, void *ptr);
Planet *space__find_planet_from_ptr(Space *space, void *ptr);
bool try_to_expand_in_place(Space *space, void *ptr, size_t old_size,
                            size_t new_size, size_t *planet_id);

#define DAP_CAP 64
#define space_dap(space, dynamic_array, element)                               \
  do {                                                                         \
    if ((dynamic_array)->capacity <= (dynamic_array)->count) {                 \
      size_t old_capacity = (dynamic_array)->capacity;                         \
      if ((dynamic_array)->capacity == 0)                                      \
        (dynamic_array)->capacity = DAP_CAP;                                   \
      else                                                                     \
        (dynamic_array)->capacity = (dynamic_array)->capacity * 2;             \
                                                                               \
      (dynamic_array)->elements = space_realloc(                               \
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

#define space_dapc(space, dynamic_array, new_elements, new_elements_count)     \
  do {                                                                         \
    if ((new_elements) != NULL) {                                              \
      if ((dynamic_array)->capacity <                                          \
          (dynamic_array)->count + new_elements_count) {                       \
        size_t old_capacity = (dynamic_array)->capacity;                       \
        if ((dynamic_array)->capacity == 0) {                                  \
          (dynamic_array)->capacity = DAP_CAP;                                 \
        }                                                                      \
        while ((dynamic_array)->capacity <                                     \
               (dynamic_array)->count + new_elements_count) {                  \
          (dynamic_array)->capacity = (dynamic_array)->capacity * 2;           \
        }                                                                      \
        (dynamic_array)->elements = space_realloc(                             \
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

#define space_dapf(space, dynamic_array, fmt, ...)                             \
  do {                                                                         \
    int n = snprintf(NULL, 0, (fmt), ##__VA_ARGS__);                           \
    if (n == -1) {                                                             \
      assert(0 && "snprintf failed!");                                         \
    }                                                                          \
    n += 1;                                                                    \
    if ((dynamic_array)->capacity < (dynamic_array)->count + n) {              \
      size_t old_capacity = (dynamic_array)->capacity;                         \
      if ((dynamic_array)->capacity == 0) {                                    \
        (dynamic_array)->capacity = DAP_CAP;                                   \
      }                                                                        \
      while ((dynamic_array)->capacity < (dynamic_array)->count + n) {         \
        (dynamic_array)->capacity = (dynamic_array)->capacity * 2;             \
      }                                                                        \
      (dynamic_array)->elements = space_realloc(                               \
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

#endif // SPACE_H_

// ===========================================================================

#ifdef SPACE_IMPLEMENTATION

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
    planet->id = space->planet_count++;
  }

  return planet;
}

void space_free_planet(Space *space, Planet *planet) {
  if (!planet) {
    return;
  }
  if (planet->prev) {
    planet->prev->next = planet->next;
  }
  if (planet->next) {
    planet->next->prev = planet->prev;
  }

  free(planet->elements);
  planet->elements = NULL;
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
  for (Planet *next, *planet = space->sun; planet; planet = next) {
    next = planet->next;
    space_free_planet(space, planet);
  }
}

void space_reset_planet(Planet *planet) { planet->count = 0; }
void space_reset_planet_and_zero(Planet *planet) {
  memset(planet->elements, 0, planet->capacity);
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

void *space_malloc_planetid(Space *space, size_t size_in_bytes,
                            size_t *planet_id) {
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
    if (p->count + size_in_bytes > p->capacity) {
      prev = p;
      p = p->next;
      continue;
    }

    // TODO: Think about handling this by deleting the planet chunk in between.
    // We can't get the original pointer at this point anyway.
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
      // This is needed to achieve the free functionality that realloc provides.
      p->count = new_size;
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
    if (ptr) { // This is to ensure memcpy() does not copy from NULL, this is
               // undefended behavior and a memory corruption.
      memcpy(new_ptr, ptr, old_size);
    }
  }
  return new_ptr;
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
  size_t planet_id;
  if (space_malloc_planetid(space, size_in_bytes, &planet_id)) {
    space_reset_planet_id(space, planet_id);
    return true;
  }
  return false;
}

size_t space__find_planet_id_from_ptr(Space *space, void *ptr) {
  if (!ptr || !space) {
    return -1;
  }
  if (!space->planet_count) {
    return -1;
  }
  if (!space->sun || !space->sun->elements) {
    return -1;
  }
  if (ptr < space->sun->elements) {
    return -1;
  }

  for (Planet *p = space->sun; p; p = p->next) {
    if ((char *)p->elements + p->capacity - (char *)ptr >= 0) {
      return p->id;
    }
  }

  return -1;
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
  if (ptr < space->sun->elements) {
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
  if (!p || (p->count != old_size) || (p->capacity - old_size != p->count) ||
      (p->count + new_size > p->capacity)) {
    planet_id = NULL;
    return false;
  }

  p->count = new_size;
  *planet_id = p->id;
  return true;
}

#endif // SPACE_IMPLEMENTATION
