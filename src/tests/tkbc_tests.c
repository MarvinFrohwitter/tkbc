#define SPACE_IMPLEMENTATION
#include "../../external/space/space.h"
#undef SPACE_IMPLEMENTATION

#define TKBC_UTILS_IMPLEMENTATION
#include "../global/tkbc-utils.h"
#undef TKBC_UTILS_IMPLEMENTATION

#define ORBIT_IMPLEMENTATION
#include "../../external/orbit/orbit.h"
#undef ORBIT_IMPLEMENTATION

#include "tkbc_test_geometrics.c"
#include "tkbc_test_kite_array.c"
#include "tkbc_test_script_handler.c"
#include "tkbc_test_script_api.c"
#include "tkbc_test_parser.c"
#include "tkbc_test_orbit.c"
#include "tkbc_test_utils.c"

#define eps 0.01
#define CASSERT_IMPLEMENTATION
#include "../../external/cassert/cassert.h"

#include "../choreographer/tkbc-asset-handler.h"
#include "../assets/CE_NG/Kites/CE_NG_1.h"
Assets assets = {0};
Env *env = {0};

/**
 * @brief The hash baked into a generated asset header must be the same one the
 * runtime computes for the decoded pixels. Duplicate design detection relies
 * on this and no longer hashes the 84 MB image during startup.
 */
Test baked_design_hash_matches_decoded_pixels(void) {
    Test test = cassert_init_test("baked kite design hash matches decoded pixels");

    Image decoded = LoadImageFromMemory(".png", asset_image_ce_ng_1, (int) sizeof(asset_image_ce_ng_1));
    cassert_bool_eq(IsImageValid(decoded), true);
    if (IsImageValid(decoded)) {
        cassert_uint64_t_eq(tkbc_image_content_hash(decoded), asset_image_ce_ng_1_hash);
        UnloadImage(decoded);
    }

    return test;
}

int main(void) {
    append_assets();
    cassert_tests {
        tkbc_test_utils(&tests);
        tkbc_test_geometrics(&tests);
        tkbc_test_kite_array(&tests);
        tkbc_test_script_handler(&tests);
        tkbc_test_script_api(&tests);
        tkbc_test_parser(&tests);
        tkbc_test_orbit(&tests);
        cassert_dap(&tests, baked_design_hash_matches_decoded_pixels());
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
