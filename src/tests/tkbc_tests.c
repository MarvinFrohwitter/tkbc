#define SPACEDEF inline
#define SPACE_IMPLEMENTATION
#include "../../external/space/space.h"
#undef SPACE_IMPLEMENTATION

#define TKBC_UTILS_IMPLEMENTATION
#include "../global/tkbc-utils.h"
#undef TKBC_UTILS_IMPLEMENTATION

#include "tkbc_test_geometrics.c"
#include "tkbc_test_script_handler.c"

#define eps 0.01
#define CASSERT_IMPLEMENTATION
#include "../../external/cassert/cassert.h"

Assets assets = {0};
Env *env = {0};

/**
 * @brief Test program entry point.
 *
 * Initialises global kite data, runs geometric and script handler
 * tests, prints results, then cleans up.
 *
 * @return 0 on success.
 */
int main(void) {
  cassert_tests {
    tkbc_test_geometrics(&tests);
    tkbc_test_script_handler(&tests);
  }

#ifdef SHORT_LOG
  cassert_short_print_tests(&tests);
#else
  cassert_print_tests(&tests);
#endif // SHORT_LOG

  cassert_free_tests(&tests);
  return 0;
}
