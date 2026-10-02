#define SPACE_IMPLEMENTATION
#include "../../external/space/space.h"
#undef SPACE_IMPLEMENTATION

#define TKBC_UTILS_IMPLEMENTATION
#include "../global/tkbc-utils.h"
#undef TKBC_UTILS_IMPLEMENTATION

#include "tkbc_test_geometrics.c"
#include "tkbc_test_script_handler.c"
#include "tkbc_test_script_api.c"
#include "tkbc_test_parser.c"

#define eps 0.01
#define CASSERT_IMPLEMENTATION
#include "../../external/cassert/cassert.h"

#include "../choreographer/tkbc-asset-handler.h"
Assets assets = {0};
Env *env = {0};

/**
 * @brief Test program entry point.
 *
 * Initialises global kite data, runs geometric, script handler, script
 * api and parser tests, prints results, then cleans up.
 *
 * @return 0 on success.
 */
int main(void) {
    append_assets();
    cassert_tests {
        tkbc_test_geometrics(&tests);
        tkbc_test_script_handler(&tests);
        tkbc_test_script_api(&tests);
        tkbc_test_parser(&tests);
    }

#ifdef SHORT_LOG
    cassert_short_print_tests(&tests);
#else
    cassert_print_tests(&tests);
#endif  // SHORT_LOG

    cassert_free_tests(&tests);
    tkbc_assets_destroy();
    return 0;
}
