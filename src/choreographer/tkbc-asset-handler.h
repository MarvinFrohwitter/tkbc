#ifndef TKBC_ASSET_HANDLER_H_
#define TKBC_ASSET_HANDLER_H_

#include "../../external/space/space.h"
#include "../global/tkbc-uuid.h"
#include "raylib.h"
#include <assert.h>
#include <stdint.h>
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
    IMAGE_CE_NG_SKELETON = IMAGE_PANNEL_PARTS_BEGIN,
    IMAGE_CE_NG_FILLED_PANEL,
    IMAGE_CE_NG_LEADINGEDGE,
    IMAGE_CE_NG_GAZE,

    IMAGE_CE_NG_LEFT_01_1,
    IMAGE_CE_NG_LEFT_02_1,
    IMAGE_CE_NG_LEFT_03_1,
    IMAGE_CE_NG_LEFT_04_1,
    IMAGE_CE_NG_LEFT_05_1,
    IMAGE_CE_NG_LEFT_06_1,
    IMAGE_CE_NG_LEFT_07_1,
    IMAGE_CE_NG_LEFT_08_1,
    IMAGE_CE_NG_LEFT_09_1,
    IMAGE_CE_NG_LEFT_10_1,
    IMAGE_CE_NG_LEFT_11_1,
    IMAGE_CE_NG_LEFT_12_1,
    IMAGE_CE_NG_LEFT_13_1,
    IMAGE_CE_NG_LEFT_14_1,
    IMAGE_CE_NG_LEFT_15_1,

    IMAGE_CE_NG_RIGHT_01_2,
    IMAGE_CE_NG_RIGHT_02_2,
    IMAGE_CE_NG_RIGHT_03_2,
    IMAGE_CE_NG_RIGHT_04_2,
    IMAGE_CE_NG_RIGHT_05_2,
    IMAGE_CE_NG_RIGHT_06_2,
    IMAGE_CE_NG_RIGHT_07_2,
    IMAGE_CE_NG_RIGHT_08_2,
    IMAGE_CE_NG_RIGHT_09_2,
    IMAGE_CE_NG_RIGHT_10_2,
    IMAGE_CE_NG_RIGHT_11_2,
    IMAGE_CE_NG_RIGHT_12_2,
    IMAGE_CE_NG_RIGHT_13_2,
    IMAGE_CE_NG_RIGHT_14_2,
    IMAGE_CE_NG_RIGHT_15_2,

    IMAGE_CE_NG_PANNEL_PARTS_END = IMAGE_CE_NG_RIGHT_15_2,

    IMAGE_111_PANNEL_PARTS_BEGIN,
    IMAGE_111_SKELETON = IMAGE_111_PANNEL_PARTS_BEGIN,
    IMAGE_111_FILLED_PANEL,

    IMAGE_111_LEFT_01_1,
    IMAGE_111_LEFT_02_1,
    IMAGE_111_LEFT_03_1,
    IMAGE_111_LEFT_04_1,
    IMAGE_111_LEFT_05_1,
    IMAGE_111_LEFT_06_1,

    IMAGE_111_RIGHT_01_2,
    IMAGE_111_RIGHT_02_2,
    IMAGE_111_RIGHT_03_2,
    IMAGE_111_RIGHT_04_2,
    IMAGE_111_RIGHT_05_2,
    IMAGE_111_RIGHT_06_2,

    IMAGE_PANNEL_PARTS_END = IMAGE_111_RIGHT_06_2,

    KITE_CE_NG_COLORIZER,
    KITE_111_COLORIZER,

    KITE_DEFAULT_DESIGNS_BEGIN,
    IMAGE_CE_NG_1 = KITE_DEFAULT_DESIGNS_BEGIN,
    IMAGE_CE_NG_2,
    IMAGE_CE_NG_3,
    IMAGE_CE_NG_4,
    IMAGE_CE_NG_5,
    IMAGE_111_DEFAULT,
    KITE_DEFAULT_DESIGNS_END = IMAGE_111_DEFAULT,

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
    // Baked in panel parts. Only their alpha channel is needed by the
    // colorizer, so they are stored as compact RLE masks instead of full RGBA
    // images (see Panel_Mask below).
    ASSETS_PANEL_MASK,
    ASSETS_KITE_DESIGN,
    ASSETS_KIND_COUNT,
} Assets_Kind;

// Internal marker for an Image whose data is a PNG file instead of raw pixels.
// Both the server and the client store non-editable kite designs in this form;
// the client only decodes them temporarily while uploading a GPU texture.
#define TKBC_IMAGE_FORMAT_COMPRESSED_PNG 1001

// Mask kinds and header layout produced by metaprogramming/assets2h.c.
#define TKBC_MASK_HEADER_SIZE 30
#define TKBC_MASK_KIND_BINARY 1
#define TKBC_MASK_KIND_ALPHA8 2

typedef struct {
    // RLE compressed mask bytes from the generated asset header. This memory
    // is static read-only data and is therefore never freed.
    const unsigned char *compressed;
    int compressed_size;
    int width;
    int height;
    int kind;
    // Precomputed opaque bounds (inclusive). x1 < x0 for an empty mask.
    int x0, y0, x1, y1;

    // Unpacked representation, allocated lazily on first access. For binary
    // masks this is one bit per pixel (LSB first), for alpha8 masks one byte.
    unsigned char *bits;
    size_t bits_size;

    // For binary masks: flat list of opaque (y, x0, x1) triples, built once
    // from the unpacked bits. Painting iterates these runs instead of every
    // pixel in the opaque bounding box.
    uint32_t *runs;
    size_t run_count;
} Panel_Mask;

typedef struct {
    Image normal;
    // Byte count of normal.data when normal.format is
    // TKBC_IMAGE_FORMAT_COMPRESSED_PNG. Zero for raw pixel data.
    int compressed_size;
    // Hash of the decoded RGBA pixels. Used to detect duplicate designs
    // without keeping (or re-creating) full pixel buffers around.
    uint64_t content_hash;
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
        Panel_Mask panel_mask;

        struct {
            Kite_Image kite_image;
            Kite_Texture kite_texture;
            bool is_dual_line;
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

UUID tkbc_append_kite_image_with_id(unsigned char *data, int width, int height, int format, UUID id, bool is_dual_line);
UUID tkbc_append_kite_image(unsigned char *data, int width, int height, int format, bool is_dual_line);
UUID tkbc_append_compressed_kite_image_with_id(const unsigned char *data, int data_size, int width, int height,
                                               uint64_t content_hash, UUID id, bool is_dual_line);

UUID tkbc_append_panel_mask(const unsigned char *data, int data_size);
void tkbc_panel_mask_unpack(Panel_Mask *mask);
unsigned char tkbc_panel_mask_alpha_at(Panel_Mask *mask, int x, int y);
bool tkbc_panel_mask_paint(Panel_Mask *mask, unsigned char *dst, Color replace, char alpha_threshold, bool overwrite,
                           int *dx0, int *dy0, int *dx1, int *dy1);

Texture2D tkbc_get_kite_texture(Asset *asset);
#ifndef TKBC_SERVER
void tkbc_load_kite_texture_from_kite_image(Kite_Image kite_image, UUID asset_id);
void tkbc_load_assets(void);
#endif

void append_assets(void);
void tkbc_asset_destroy(Asset *asset);
void tkbc_assets_destroy(void);
void tkbc_removed_asset_by_id(UUID uuid);
Asset *tkbc_find_asset_from_id(UUID id);
Asset *tkbc_get_colorizer_asset(bool is_dual_line);
size_t tkbc_get_current_kite_design_count();
UUID tkbc_append_kite_image_and_kite_texture_with_id(unsigned char *data, int data_size, int width, int height,
                                                     int format, UUID id, bool is_dual_line);
bool tkbc_image_already_exitst_in_assets(Image image, UUID *id);
bool tkbc_image_already_exitst_in_assets_with_hash(Image image, uint64_t hash, UUID *id);
bool tkbc_is_default_kite_design(Assets *assets, UUID uuid);

#define _tkbc_get_asset_image(enum_kind)                                                                               \
    assets.elements[(assert(assets.count > 0), assert(assets.elements[enum_kind].type == ASSETS_IMAGE), enum_kind)]

#define _tkbc_get_asset_panel_mask(enum_kind)                                                                          \
    assets.elements[(assert(assets.count > 0), assert(assets.elements[enum_kind].type == ASSETS_PANEL_MASK), enum_kind)]

#define _tkbc_get_asset_kite_design(enum_kind)                                                                         \
    assets.elements[(assert(assets.count > 0), assert(assets.elements[(enum_kind)].type == ASSETS_KITE_DESIGN),        \
                     (enum_kind))]

#define _tkbc_get_asset(kind) assets.elements[(assert(assets.count > 0), kind)]

#endif  // TKBC_ASSET_HANDLER_H_
