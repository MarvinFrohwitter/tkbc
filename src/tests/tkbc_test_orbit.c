#include "../../external/cassert/cassert.h"

#include "../../external/orbit/orbit.h"

/* Hands out pointers into a bigger arena, just like space_malloc() does.
 * free() must never be called on these pointers. */
static char orbit_test_arena[64 * 1024];
static size_t orbit_test_arena_offset = 0;

static void *orbit_test_arena_alloc(void *alloc_data, size_t size) {
    (void) alloc_data;
    orbit_test_arena_offset = (orbit_test_arena_offset + 15) & ~((size_t) 15);
    char *ptr = orbit_test_arena + orbit_test_arena_offset;
    orbit_test_arena_offset += size;
    return orbit_test_arena_offset <= sizeof(orbit_test_arena) ? ptr : NULL;
}

/**
 * @brief A hash table with a custom (arena) allocator must survive resizes.
 *
 * The resize path replaces the elements array. With the old code that array
 * was released with free() even when a custom allocator produced it, which
 * aborted with "free(): invalid size" for arena/mmap backed allocators.
 */
Test orbit_custom_allocator_resize(void) {
    Test test = cassert_init_test("orbit hash table resize with custom allocator");

    Orbit_HT(int, int) ids = {
        .init_size = 4,
        .allocator.alloc = orbit_test_arena_alloc,
        .allocator.alloc_data = NULL,
    };

    enum { COUNT = 200 };
    for (int i = 0; i < COUNT; ++i) {
        cassert_bool_eq(orbit_put(&ids, i, i * 10) != NULL, true);
    }
    cassert_size_t_eq(ids.count, COUNT);

    for (int i = 0; i < COUNT; ++i) {
        int *value = orbit_get(&ids, i);
        cassert_bool_eq(value != NULL, true);
        cassert_int_eq(*value, i * 10);
    }

    orbit_free(&ids);
    return test;
}

void tkbc_test_orbit(Tests *tests) {
    cassert_dap(tests, orbit_custom_allocator_resize());
}
