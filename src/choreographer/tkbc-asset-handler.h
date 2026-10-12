#ifndef TKBC_ASSET_HANDLER_H_
#define TKBC_ASSET_HANDLER_H_

#include "../../external/space/space.h"
#include "../global/tkbc-uuid.h"
#include "raylib.h"
#include <assert.h>
#include <stdio.h>

typedef enum {
    _16X16_LOGO = 0,
    _24X24_LOGO,
    _32X32_LOGO,
    _48X48_LOGO,
    _64X64_LOGO,
    _96X96_LOGO,
    _128X128_LOGO,
    _256X256_LOGO,

    IMAGE_PANNEL_PARTS_BEGIN,
    IMAGE_SKELETON = IMAGE_PANNEL_PARTS_BEGIN,
    IMAGE_FILLED_PANEL,
    IMAGE_SKELETON_LEADINGEDGE,
    IMAGE_LEADINGEDGE,
    IMAGE_GAZE,

    IMAGE_LEFT_02_1,
    IMAGE_LEFT_03_1,
    IMAGE_LEFT_04_1,
    IMAGE_LEFT_05_1,
    IMAGE_LEFT_06_1,
    IMAGE_LEFT_07_1,
    IMAGE_LEFT_08_1,
    IMAGE_LEFT_09_1,
    IMAGE_LEFT_10_1,
    IMAGE_LEFT_11_1,
    IMAGE_LEFT_12_1,
    IMAGE_LEFT_13_1,
    IMAGE_LEFT_14_1,
    IMAGE_LEFT_15_1,
    IMAGE_LEFT_16_1,

    IMAGE_RIGHT_02_2,
    IMAGE_RIGHT_03_2,
    IMAGE_RIGHT_04_2,
    IMAGE_RIGHT_05_2,
    IMAGE_RIGHT_06_2,
    IMAGE_RIGHT_07_2,
    IMAGE_RIGHT_08_2,
    IMAGE_RIGHT_09_2,
    IMAGE_RIGHT_10_2,
    IMAGE_RIGHT_11_2,
    IMAGE_RIGHT_12_2,
    IMAGE_RIGHT_13_2,
    IMAGE_RIGHT_14_2,
    IMAGE_RIGHT_15_2,
    IMAGE_RIGHT_16_2,

    IMAGE_MIDDLE_06,
    IMAGE_MIDDLE_10,
    IMAGE_MIDDLE_13,

    IMAGE_PANNEL_PARTS_END = IMAGE_MIDDLE_13,

    KITE_COLORIZER,

    KITE_DEFAULT_DESIGNS_BEGIN,
    IMAGE_1 = KITE_DEFAULT_DESIGNS_BEGIN,
    IMAGE_2,
    IMAGE_3,
    IMAGE_4,
    KITE_DEFAULT_DESIGNS_END = IMAGE_4,

    // --- New generated kite designs from the colorizer.
    //
    // Everything from this index on is created at runtime and therefore has to
    // be transferred to the sides that do not know it yet. The baked in assets
    // above get the same deterministic uuid everywhere.
    KITE_NEW_DESIGNS_BEGIN,

    ASSET_KITE_DESIGN_COUNT,
} Asset_Id_Kind;

typedef enum {
    ASSETS_IMAGE,
    ASSETS_KITE_DESIGN,
    ASSETS_KIND_COUNT,
} Assets_Kind;

typedef struct {
    Image normal;
} Kite_Image;

typedef struct {
    Texture2D normal;
} Kite_Texture;

typedef struct {
    Assets_Kind type;
    UUID id;  // The unique uuid of the asset. The baked in base assets get a
              // deterministic uuid that is derived from the build-order
              // sequence (see Assets.default_assets_seq), so every client and
              // the server hold the same uuid for e.g. IMAGE_1. Assets that
              // are created at runtime (e.g. a colorizer result) get a random
              // uuid so designs of different users never collide.

    union {
        Image image;

        struct {
            Kite_Image kite_image;
            Kite_Texture kite_texture;
        };
    } as;

} Asset;

typedef struct {
    Asset *elements;
    size_t count;
    size_t capacity;

    bool default_assets_setup;  // The indication the baked in base assets are
                                // currently being appended. Only they get a
                                // deterministic uuid so that every side holds
                                // the same uuid for the same base asset.
    size_t default_assets_seq;  // Build-order sequence for the deterministic
                                // base asset uuids. Reset to 0 before each
                                // base build; incremented per base asset so
                                // the uuids stay stable.

    Space space;
} Assets;

UUID tkbc_append_kite_image_with_id(unsigned char *data, int width, int height, int format, UUID id);
UUID tkbc_append_kite_image(unsigned char *data, int width, int height, int format);

#ifndef TKBC_SERVER
void tkbc_load_kite_texture_from_kite_image(Kite_Image kite_image, UUID asset_id);
void tkbc_load_assets(void);
#endif

void append_assets(void);
void tkbc_asset_destroy(Asset *asset);
void tkbc_assets_destroy(void);
void tkbc_removed_asset_by_id(UUID uuid);
Asset *tkbc_find_asset_from_id(UUID id);
size_t tkbc_get_current_kite_design_count();
UUID tkbc_append_kite_image_and_kite_texture_with_id(unsigned char *data, int width, int height, int format, UUID id);
bool tkbc_image_already_exitst_in_assets(Image image, UUID *id);
bool tkbc_is_default_kite_design(Assets *assets, UUID uuid);

#define _tkbc_get_asset_image(enum_kind)                                                                               \
    assets.elements[(assert(assets.count > 0), assert(assets.elements[enum_kind].type == ASSETS_IMAGE), enum_kind)]

#define _tkbc_get_asset_kite_design(enum_kind)                                                                         \
    assets.elements[(assert(assets.count > 0), assert(assets.elements[(enum_kind)].type == ASSETS_KITE_DESIGN),        \
                     (enum_kind))]

#define _tkbc_get_asset(kind) assets.elements[(assert(assets.count > 0), kind)]

#endif  // TKBC_ASSET_HANDLER_H_
