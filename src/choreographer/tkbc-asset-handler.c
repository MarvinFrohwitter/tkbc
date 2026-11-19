#include "../../external/space/space.h"
#include "../global/tkbc-types.h"
#include "../global/tkbc-utils.h"
#include "raylib.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern Assets assets;

static Image tkbc_copy_compressed_image(const unsigned char *data, int data_size, int width, int height);

#include "../../assets/Kite_111/Kites/111_Base_White.h"

#include "../../assets/Kite_111/Around/111_Skeleton.h"

#include "../../assets/Kite_111/Left/111_Left_01_1.h"
#include "../../assets/Kite_111/Left/111_Left_02_1.h"
#include "../../assets/Kite_111/Left/111_Left_03_1.h"
#include "../../assets/Kite_111/Left/111_Left_04_1.h"
#include "../../assets/Kite_111/Left/111_Left_05_1.h"
#include "../../assets/Kite_111/Left/111_Left_06_1.h"

#include "../../assets/Kite_111/Right/111_Right_01_2.h"
#include "../../assets/Kite_111/Right/111_Right_02_2.h"
#include "../../assets/Kite_111/Right/111_Right_03_2.h"
#include "../../assets/Kite_111/Right/111_Right_04_2.h"
#include "../../assets/Kite_111/Right/111_Right_05_2.h"
#include "../../assets/Kite_111/Right/111_Right_06_2.h"

#include "../../assets/CE_NG/Middle/Middle_05.h"
#include "../../assets/CE_NG/Middle/Middle_09.h"
#include "../../assets/CE_NG/Middle/Middle_12.h"

#include "../../assets/CE_NG/Left/Left_01_1.h"
#include "../../assets/CE_NG/Left/Left_02_1.h"
#include "../../assets/CE_NG/Left/Left_03_1.h"
#include "../../assets/CE_NG/Left/Left_04_1.h"
#include "../../assets/CE_NG/Left/Left_05_1.h"
#include "../../assets/CE_NG/Left/Left_06_1.h"
#include "../../assets/CE_NG/Left/Left_07_1.h"
#include "../../assets/CE_NG/Left/Left_08_1.h"
#include "../../assets/CE_NG/Left/Left_09_1.h"
#include "../../assets/CE_NG/Left/Left_10_1.h"
#include "../../assets/CE_NG/Left/Left_11_1.h"
#include "../../assets/CE_NG/Left/Left_12_1.h"
#include "../../assets/CE_NG/Left/Left_13_1.h"
#include "../../assets/CE_NG/Left/Left_14_1.h"
#include "../../assets/CE_NG/Left/Left_15_1.h"

#include "../../assets/CE_NG/Right/Right_01_2.h"
#include "../../assets/CE_NG/Right/Right_02_2.h"
#include "../../assets/CE_NG/Right/Right_03_2.h"
#include "../../assets/CE_NG/Right/Right_04_2.h"
#include "../../assets/CE_NG/Right/Right_05_2.h"
#include "../../assets/CE_NG/Right/Right_06_2.h"
#include "../../assets/CE_NG/Right/Right_07_2.h"
#include "../../assets/CE_NG/Right/Right_08_2.h"
#include "../../assets/CE_NG/Right/Right_09_2.h"
#include "../../assets/CE_NG/Right/Right_10_2.h"
#include "../../assets/CE_NG/Right/Right_11_2.h"
#include "../../assets/CE_NG/Right/Right_12_2.h"
#include "../../assets/CE_NG/Right/Right_13_2.h"
#include "../../assets/CE_NG/Right/Right_14_2.h"
#include "../../assets/CE_NG/Right/Right_15_2.h"

#include "../../assets/CE_NG/Around/Filled_Panel.h"
#include "../../assets/CE_NG/Around/Gaze.h"
#include "../../assets/CE_NG/Around/Leadingedge.h"
#include "../../assets/CE_NG/Around/Skeleton.h"
#include "../../assets/CE_NG/Around/Skeleton_Leadingedge.h"

#include "../../assets/CE_NG/Kites/CE_NG_1.h"
#include "../../assets/CE_NG/Kites/CE_NG_2.h"
#include "../../assets/CE_NG/Kites/CE_NG_3.h"
#include "../../assets/CE_NG/Kites/CE_NG_4.h"
#include "../../assets/CE_NG/Kites/CE_NG_5.h"

#include "../../assets/Logos/128x128_Logo.h"
#include "../../assets/Logos/16x16_Logo.h"
#include "../../assets/Logos/24x24_Logo.h"
#include "../../assets/Logos/256x256_Logo.h"
#include "../../assets/Logos/32x32_Logo.h"
#include "../../assets/Logos/48x48_Logo.h"
#include "../../assets/Logos/64x64_Logo.h"
#include "../../assets/Logos/96x96_Logo.h"

#include "tkbc-asset-handler.h"

// Save and load kite designs from config files.

/**
 * @brief The function returns the uuid that should be assigned to a newly
 * appended asset.
 *
 * While the baked in base assets are appended (see append_assets) the uuid is
 * derived from the build-order sequence. append_assets() runs in the exact
 * same order on every client and on the server, so this yields the very same
 * uuid for the same base asset everywhere. That is what keeps the already
 * known base assets - like IMAGE_1 .. IMAGE_4, KITE_COLORIZER and the panel
 * parts - from being treated as newly received assets on the other sides.
 *
 * Everything that is created afterwards (e.g. a colorizer result) gets a
 * random uuid instead, so that designs created by different users never
 * collide.
 *
 * @return The uuid for the newly appended asset.
 */
static inline UUID tkbc_generate_uuid_for_asset(void) {
    if (assets.default_assets_setup) {
        return tkbc_uuid_from_number(++assets.default_assets_seq);
    }
    return tkbc_uuid_generate();
}

/**
 * @brief The function appends a new kite image under the given uuid to the
 * global kite_images collection. The image data is copied and stored.
 *
 * @param data The raw image data.
 * @param width The width of the image.
 * @param height The height of the image.
 * @param format The pixel format of the image.
 * @param id The uuid the kite image is stored under.
 * @return The uuid of the newly appended kite image.
 */
UUID tkbc_append_kite_image_with_id(unsigned char *data, int width, int height, int format, UUID id,
                                    bool is_dual_line) {
    Image image_normal = {
        .data = data,
        .width = width,
        .height = height,
        .mipmaps = 1,
        .format = format,
    };

    image_normal = ImageCopy(image_normal);
    Kite_Image kite_image = (Kite_Image){
        .normal = image_normal,
        .compressed_size = 0,
    };

    space_dap(&assets.space, &assets,
              ((Asset){
                  .type = ASSETS_KITE_DESIGN,
                  .as.kite_image = kite_image,
                  .as.is_dual_line = is_dual_line,
                  .id = id,
              }));

    return id;
}

/**
 * @brief Appends a kite image whose data is still a PNG file.
 *
 * This is the server-side representation of transferred kite designs. The
 * server only needs the width, height and the compressed bytes to forward the
 * design to clients, so decoding it to raw RGBA would waste memory.
 *
 * @param data The PNG file bytes.
 * @param data_size The size of the PNG file in bytes.
 * @param width The image width.
 * @param height The image height.
 * @param id The uuid the image is stored under.
 * @return The uuid of the appended kite image.
 */
UUID tkbc_append_compressed_kite_image_with_id(const unsigned char *data, int data_size, int width, int height, UUID id,
                                               bool is_dual_line) {
    if (tkbc_uuid_is_nil(id)) {
        id = tkbc_uuid_generate();
    }

    Kite_Image kite_image = (Kite_Image){
        .normal = tkbc_copy_compressed_image(data, data_size, width, height),
        .compressed_size = data_size,
    };

    space_dap(&assets.space, &assets,
              ((Asset){
                  .type = ASSETS_KITE_DESIGN,
                  .as.kite_image = kite_image,
                  .as.is_dual_line = is_dual_line,
                  .id = id,
              }));

    return id;
}

/**
 * @brief The function appends a new kite image with a fresh uuid to the global
 * kite_images collection. Only used for assets that are created at runtime,
 * like a colorizer result.
 *
 * @param data The raw image data.
 * @param width The width of the image.
 * @param height The height of the image.
 * @param format The pixel format of the image.
 * @return The uuid assigned to the newly appended kite image.
 */
UUID tkbc_append_kite_image(unsigned char *data, int width, int height, int format, bool is_dual_line) {
    return tkbc_append_kite_image_with_id(data, width, height, format, tkbc_generate_uuid_for_asset(), is_dual_line);
}

/**
 * @brief The function appends a new asset image to the global assets
 * collection. The image data is copied and stored.
 *
 * @param data The raw image data.
 * @param width The width of the image.
 * @param height The height of the image.
 * @param format The pixel format of the image.
 * @return The uuid assigned to the newly appended asset.
 */
UUID tkbc_append_asset_image(unsigned char *data, int width, int height, int format) {
    Image image = {
        .data = data,
        .width = width,
        .height = height,
        .mipmaps = 1,
        .format = format,
    };

    image = ImageCopy(image);

    UUID id = tkbc_generate_uuid_for_asset();
    space_dap(&assets.space, &assets,
              ((Asset){
                  .type = ASSETS_IMAGE,
                  .as.image = image,
                  .id = id,
              }));

    return id;
}

#ifdef TKBC_SERVER
/**
 * @brief Reads the width and height from a PNG file header.
 *
 * @param data The PNG file bytes.
 * @param data_size The size of the PNG file in bytes.
 * @param width Pointer that receives the image width.
 * @param height Pointer that receives the image height.
 * @return True if a valid PNG IHDR was found, otherwise false.
 */
static bool tkbc_png_dimensions(const unsigned char *data, int data_size, int *width, int *height) {
    static const unsigned char signature[8] = {0x89, 'P', 'N', 'G', 0x0d, 0x0a, 0x1a, 0x0a};

    if (data == NULL || data_size < 24 || memcmp(data, signature, sizeof(signature)) != 0) {
        return false;
    }
    if (memcmp(data + 12, "IHDR", 4) != 0) {
        return false;
    }

    *width = (int) (((uint32_t) data[16] << 24) | ((uint32_t) data[17] << 16) | ((uint32_t) data[18] << 8) |
                    (uint32_t) data[19]);
    *height = (int) (((uint32_t) data[20] << 24) | ((uint32_t) data[21] << 16) | ((uint32_t) data[22] << 8) |
                     (uint32_t) data[23]);
    return *width > 0 && *height > 0;
}
#endif  // TKBC_SERVER

/**
 * @brief Copies PNG file bytes into an Image that keeps them compressed.
 *
 * @param data The PNG file bytes.
 * @param data_size The size of the PNG file in bytes.
 * @param width The image width.
 * @param height The image height.
 * @return An Image whose data is the PNG file, not decoded pixels.
 */
static Image tkbc_copy_compressed_image(const unsigned char *data, int data_size, int width, int height) {
    unsigned char *copy = (unsigned char *) MemAlloc((size_t) data_size);
    assert(copy != NULL && "Out of memory while storing a compressed image");
    if (copy == NULL) {
        return (Image){0};
    }
    memcpy(copy, data, (size_t) data_size);

    return (Image){
        .data = copy,
        .width = width,
        .height = height,
        .mipmaps = 1,
        .format = TKBC_IMAGE_FORMAT_COMPRESSED_PNG,
    };
}

#ifndef TKBC_SERVER
/**
 * @brief Decodes one of the baked in image assets.
 *
 * The generated asset headers store the original PNG bytes instead of raw
 * pixels. raylib decodes the compressed data from memory, which keeps the
 * generated headers and the resulting binaries small.
 *
 * @param data The compressed image file data.
 * @param data_size The size of the compressed image data in bytes.
 * @param file_type The file extension used to select the decoder, e.g. ".png".
 * @return The decoded image, or an invalid image on failure.
 */
static Image tkbc_load_compressed_asset_image(const unsigned char *data, int data_size, const char *file_type) {
    Image image = LoadImageFromMemory(file_type, data, data_size);
    if (!IsImageValid(image)) {
        tkbc_fprintf(stderr, "ERROR", "Could not load baked in image asset (%s).\n", file_type);
    }
    return image;
}
#endif  // !TKBC_SERVER

/**
 * @brief Appends a baked in asset image that is stored in a compressed format.
 *
 * @param data The compressed image file data.
 * @param data_size The size of the compressed image data in bytes.
 * @param file_type The file extension used to select the decoder, e.g. ".png".
 * @return The uuid assigned to the newly appended asset.
 */
static UUID tkbc_append_compressed_asset_image(const unsigned char *data, int data_size, const char *file_type) {
#ifdef TKBC_SERVER
    (void) file_type;
    int width = 0;
    int height = 0;
    if (!tkbc_png_dimensions(data, data_size, &width, &height)) {
        return tkbc_uuid_nil();
    }

    Image image = tkbc_copy_compressed_image(data, data_size, width, height);
    UUID id = tkbc_generate_uuid_for_asset();
    space_dap(&assets.space, &assets,
              ((Asset){
                  .type = ASSETS_IMAGE,
                  .as.image = image,
                  .id = id,
              }));
    return id;
#else
    Image image = tkbc_load_compressed_asset_image(data, data_size, file_type);
    if (!IsImageValid(image)) {
        return tkbc_uuid_nil();
    }

    UUID id = tkbc_append_asset_image(image.data, image.width, image.height, image.format);
    UnloadImage(image);
    return id;
#endif
}

/**
 * @brief Appends a baked in kite image that is stored in a compressed format.
 *
 * @param data The compressed image file data.
 * @param data_size The size of the compressed image data in bytes.
 * @param file_type The file extension used to select the decoder, e.g. ".png".
 * @return The uuid assigned to the newly appended kite image.
 */
static UUID tkbc_append_compressed_kite_image(const unsigned char *data, int data_size, const char *file_type,
                                              bool is_dual_line) {
#ifdef TKBC_SERVER
    (void) file_type;
    int width = 0;
    int height = 0;
    if (!tkbc_png_dimensions(data, data_size, &width, &height)) {
        return tkbc_uuid_nil();
    }
    return tkbc_append_compressed_kite_image_with_id(data, data_size, width, height, tkbc_generate_uuid_for_asset(),
                                                     is_dual_line);
#else
    Image image = tkbc_load_compressed_asset_image(data, data_size, file_type);
    if (!IsImageValid(image)) {
        return tkbc_uuid_nil();
    }

    UUID id = tkbc_append_kite_image(image.data, image.width, image.height, image.format, is_dual_line);
    UnloadImage(image);
    return id;
#endif
}

#define tkbc_append_baked_asset_image(asset_symbol)                                                                    \
    tkbc_append_compressed_asset_image((asset_symbol), (int) sizeof(asset_symbol), ".png")
#define tkbc_append_baked_kite_image(asset_symbol, is_dual_line)                                                       \
    tkbc_append_compressed_kite_image((asset_symbol), (int) sizeof(asset_symbol), ".png", (is_dual_line))

/**
 * @brief Append all kite image assets to the global asset list.
 *
 * Registers the predefined kite panel kite images, Skeleton, Filled Panel,
 * Leadingedge, Gaze, etc.
 */
void tkbc_append_ce_ng_kite_image_pannels() {
    tkbc_append_baked_asset_image(asset_image_skeleton);
    tkbc_append_baked_asset_image(asset_image_filled_panel);
    tkbc_append_baked_asset_image(asset_image_skeleton_leadingedge);
    tkbc_append_baked_asset_image(asset_image_leadingedge);
    tkbc_append_baked_asset_image(asset_image_gaze);

    tkbc_append_baked_asset_image(asset_image_left_01_1);
    tkbc_append_baked_asset_image(asset_image_left_02_1);
    tkbc_append_baked_asset_image(asset_image_left_03_1);
    tkbc_append_baked_asset_image(asset_image_left_04_1);
    tkbc_append_baked_asset_image(asset_image_left_05_1);
    tkbc_append_baked_asset_image(asset_image_left_06_1);
    tkbc_append_baked_asset_image(asset_image_left_07_1);
    tkbc_append_baked_asset_image(asset_image_left_08_1);
    tkbc_append_baked_asset_image(asset_image_left_09_1);
    tkbc_append_baked_asset_image(asset_image_left_10_1);
    tkbc_append_baked_asset_image(asset_image_left_11_1);
    tkbc_append_baked_asset_image(asset_image_left_12_1);
    tkbc_append_baked_asset_image(asset_image_left_13_1);
    tkbc_append_baked_asset_image(asset_image_left_14_1);
    tkbc_append_baked_asset_image(asset_image_left_15_1);

    tkbc_append_baked_asset_image(asset_image_right_01_2);
    tkbc_append_baked_asset_image(asset_image_right_02_2);
    tkbc_append_baked_asset_image(asset_image_right_03_2);
    tkbc_append_baked_asset_image(asset_image_right_04_2);
    tkbc_append_baked_asset_image(asset_image_right_05_2);
    tkbc_append_baked_asset_image(asset_image_right_06_2);
    tkbc_append_baked_asset_image(asset_image_right_07_2);
    tkbc_append_baked_asset_image(asset_image_right_08_2);
    tkbc_append_baked_asset_image(asset_image_right_09_2);
    tkbc_append_baked_asset_image(asset_image_right_10_2);
    tkbc_append_baked_asset_image(asset_image_right_11_2);
    tkbc_append_baked_asset_image(asset_image_right_12_2);
    tkbc_append_baked_asset_image(asset_image_right_13_2);
    tkbc_append_baked_asset_image(asset_image_right_14_2);
    tkbc_append_baked_asset_image(asset_image_right_15_2);

    tkbc_append_baked_asset_image(asset_image_middle_05);
    tkbc_append_baked_asset_image(asset_image_middle_09);
    tkbc_append_baked_asset_image(asset_image_middle_12);
}

void tkbc_append_111_kite_image_pannels() {

    tkbc_append_baked_asset_image(asset_image_111_skeleton);
    tkbc_append_baked_asset_image(asset_image_111_base_white);

    tkbc_append_baked_asset_image(asset_image_111_left_01_1);
    tkbc_append_baked_asset_image(asset_image_111_left_02_1);
    tkbc_append_baked_asset_image(asset_image_111_left_03_1);
    tkbc_append_baked_asset_image(asset_image_111_left_04_1);
    tkbc_append_baked_asset_image(asset_image_111_left_05_1);
    tkbc_append_baked_asset_image(asset_image_111_left_06_1);

    tkbc_append_baked_asset_image(asset_image_111_right_01_2);
    tkbc_append_baked_asset_image(asset_image_111_right_02_2);
    tkbc_append_baked_asset_image(asset_image_111_right_03_2);
    tkbc_append_baked_asset_image(asset_image_111_right_04_2);
    tkbc_append_baked_asset_image(asset_image_111_right_05_2);
    tkbc_append_baked_asset_image(asset_image_111_right_06_2);
}

/**
 * @brief The function loads all the predefined kite assets into the
 * asset collection. This includes left, right, middle, skeleton,
 * leading edge, and other predefined kite images.
 */
void append_assets(void) {
    // NOTE: This is needed when loading assets directly from files.
    // GetApplicationDirectory();

    // The baked in base assets get a uuid that is derived from the
    // build-order sequence. Since this function appends them in the exact same
    // order on every client and on the server, all sides end up with the same
    // uuid for the same base asset and therefore recognize e.g. IMAGE_1 as
    // already known instead of registering it as a newly received asset.
    assets.default_assets_setup = true;
    assets.default_assets_seq = 0;

    // This loading order has to be the exact same as in the
    // Asset_Kite_Design_Kind enum to correlate the place.
    // TODO: Make the asset loading depended on the enum.
    static_assert(ASSET_KITE_DESIGN_COUNT, "The static asset count has changed think about the order of appending");

    tkbc_append_baked_asset_image(asset_image_16x16_logo);
    tkbc_append_baked_asset_image(asset_image_24x24_logo);
    tkbc_append_baked_asset_image(asset_image_32x32_logo);
    tkbc_append_baked_asset_image(asset_image_48x48_logo);
    tkbc_append_baked_asset_image(asset_image_64x64_logo);
    tkbc_append_baked_asset_image(asset_image_96x96_logo);
    tkbc_append_baked_asset_image(asset_image_128x128_logo);
    tkbc_append_baked_asset_image(asset_image_256x256_logo);

    tkbc_append_ce_ng_kite_image_pannels();

    tkbc_append_111_kite_image_pannels();

#ifdef TKBC_SERVER
    // The colorizer starts as a copy of the filled panel. The server never
    // paints on it, so keep the baked in PNG bytes instead of decoding them.
    tkbc_append_compressed_kite_image(asset_image_filled_panel, (int) sizeof(asset_image_filled_panel), ".png", false);
    tkbc_append_compressed_kite_image(asset_image_111_base_white, (int) sizeof(asset_image_111_base_white), ".png",
                                      true);
#else
    Image colorizer_image_ce_ng = _tkbc_get_asset_image(IMAGE_CE_NG_FILLED_PANEL).as.image;
    tkbc_append_kite_image(colorizer_image_ce_ng.data, colorizer_image_ce_ng.width, colorizer_image_ce_ng.height,
                           colorizer_image_ce_ng.format, false);

    Image colorizer_image_111 = _tkbc_get_asset_image(IMAGE_111_FILLED_PANEL).as.image;
    tkbc_append_kite_image(colorizer_image_111.data, colorizer_image_111.width, colorizer_image_111.height,
                           colorizer_image_111.format, true);

#endif

    tkbc_append_baked_kite_image(asset_image_ce_ng_1, false);
    tkbc_append_baked_kite_image(asset_image_ce_ng_2, false);
    tkbc_append_baked_kite_image(asset_image_ce_ng_3, false);
    tkbc_append_baked_kite_image(asset_image_ce_ng_4, false);
    tkbc_append_baked_kite_image(asset_image_ce_ng_5, false);
    tkbc_append_baked_kite_image(asset_image_111_base_white, true);

    assets.default_assets_setup = false;
}

#ifndef TKBC_SERVER
/**
 * @brief The function creates and appends a new kite texture from the given
 * kite image. The texture is loaded from the image for both normal and flipped
 * orientations.
 *
 * @param kite_image The kite image from which the texture should be created.
 */
void tkbc_load_kite_texture_from_kite_image(Kite_Image kite_image, UUID asset_id) {
    Asset *asset = tkbc_find_asset_from_id(asset_id);
    if (!asset) {
        return;
    }

    assert(asset->type == ASSETS_KITE_DESIGN);
    asset->as.kite_texture = (Kite_Texture){
        .normal = LoadTextureFromImage(kite_image.normal),
    };
    GenTextureMipmaps(&asset->as.kite_texture.normal);
    // Kite textures are drawn at arbitrary (fractional) positions and scales.
    // With the default repeat wrap the sampling of the outermost edge can wrap
    // around to the opposite edge, which made the top pixel row of a texture
    // show up at its bottom. Clamping keeps the edge pixels in place.
    SetTextureWrap(asset->as.kite_texture.normal, TEXTURE_WRAP_CLAMP);
    SetTextureFilter(asset->as.kite_texture.normal, TEXTURE_FILTER_BILINEAR);
}

/**
 * @brief The function loads all kite images and creates textures from them.
 * This function should only be called on the client side, not on the server.
 */
void tkbc_load_assets(void) {

    // TODO: Make a type dependent image loader.
    append_assets();

    for (size_t i = 0; i < assets.count; ++i) {

        if (assets.elements[i].type == ASSETS_IMAGE) {
            if (!IsImageValid(_tkbc_get_asset_image(i).as.image)) {
                tkbc_fprintf(stderr, "ERROR", "Could not load image asset: %zu.\n", i);
            }
            continue;
        }

        if (!IsImageValid(_tkbc_get_asset_kite_design(i).as.kite_image.normal)) {
            tkbc_fprintf(stderr, "ERROR", "Could not load normal kite image: %zu.\n", i);
        }

        // The asset index correlates with the Asset_Id_Kind enum, the Asset.id
        // is a uuid and therefore not comparable to the enum values.
        if (i >= IMAGE_PANNEL_PARTS_BEGIN && i <= IMAGE_PANNEL_PARTS_END) {
            continue;
        }

        tkbc_load_kite_texture_from_kite_image(_tkbc_get_asset_kite_design(i).as.kite_image,
                                               _tkbc_get_asset_kite_design(i).id);

        if (!IsTextureValid(_tkbc_get_asset_kite_design(i).as.kite_texture.normal)) {
            tkbc_fprintf(stderr, "ERROR", "Could not load normal kite texture: %zu.\n", i);
        }
    }
}
#endif

/**
 * @brief The function can be used to destroy a given asset.
 *
 * @param asset The asset that should be destroyed.
 */
void tkbc_asset_destroy(Asset *asset) {
    switch (asset->type) {
    case ASSETS_IMAGE: UnloadImage(asset->as.image); break;
    case ASSETS_KITE_DESIGN: UnloadImage(asset->as.kite_image.normal);

#ifndef TKBC_SERVER
        UnloadTexture(asset->as.kite_texture.normal);
#endif
        break;
    case ASSETS_KIND_COUNT:
    default: assert(false && "tkbc_asset_destroy: UNREACHABLE");
    }
}

/**
 * @brief The function destroys all kite assets including images and textures.
 */
void tkbc_assets_destroy(void) {
    for (size_t i = 0; i < assets.count; ++i) {
        tkbc_asset_destroy(&assets.elements[i]);
    }
}

/**
 * @brief THis function can be used to remove a specified asset from the global assets array.
 *
 * @param uuid Id where the asset is identifiable by,
 */
void tkbc_removed_asset_by_id(UUID uuid) {
    for (size_t i = 0; i < assets.count; ++i) {
        if (tkbc_uuid_equals(assets.elements[i].id, uuid)) {
            tkbc_asset_destroy(&assets.elements[i]);

            if (i + 1 < assets.count) {
                size_t n = assets.count - i - 1;
                memmove(&assets.elements[i], &assets.elements[i + 1], n * sizeof(*assets.elements));
            }

            assets.count -= 1;
            break;
        }
    }
}

/**
 * @brief The function tries to find a specific asset by its uuid.
 *
 * @param id The uuid of the asset to search for.
 * @return The found asset, if not found NULL.
 */
Asset *tkbc_find_asset_from_id(UUID id) {
    for (size_t i = 0; i < assets.count; ++i) {
        if (tkbc_uuid_equals(assets.elements[i].id, id)) {
            return &assets.elements[i];
        }
    }
    return NULL;
}

/**
 * @brief Returns the colorizer asset for the given kite design type.
 *
 * Non dual line designs belong to the CE_NG colorizer, dual line designs to
 * the 111 colorizer.
 *
 * @param is_dual_line The type of kite design the colorizer is needed for.
 * @return The matching colorizer asset.
 */
Asset *tkbc_get_colorizer_asset(bool is_dual_line) {
    return &_tkbc_get_asset_kite_design(is_dual_line ? KITE_111_COLORIZER : KITE_CE_NG_COLORIZER);
}

/**
 * @brief Assigns a texture to a kite and mirrors the asset's is_dual_line flag
 * onto the kite.
 *
 * The asset is the single source of truth for the flag, so every texture
 * assignment derives the kite value from it. A missing id (or an id that does
 * not belong to a kite design) means there is no dual line information and
 * therefore resets the flag to false.
 *
 * @param kite The kite that receives the texture.
 * @param texture_id The uuid of the kite design asset to assign.
 */
void tkbc_set_kite_texture_id(Kite *kite, UUID texture_id) {
    assert(kite != NULL);

    kite->texture_id = texture_id;

    Asset *asset = tkbc_find_asset_from_id(texture_id);
    kite->is_dual_line = (asset != NULL && asset->type == ASSETS_KITE_DESIGN) ? asset->as.is_dual_line : false;
}

/**
 * @brief This function calculates the current number of assets that is related
 * to the kite designs.
 *
 * @return The count of the currently registered assets related to kite design.
 */
size_t tkbc_get_current_kite_design_count() {
    size_t result = 0;
    for (size_t i = 0; i < assets.count; ++i) {
        if (assets.elements[i].type == ASSETS_KITE_DESIGN) {
            result++;
        }
    }
    return result;
}

/**
 * @brief Appends a kite image under the given uuid to the asset list and loads
 * its texture.
 *
 * This is the path for kite images that arrive from another side. The uuid is
 * the one the originator assigned to the design, so storing it under that very
 * uuid makes the design known to this side. Otherwise the receiving side would
 * register the design a second time under a fresh uuid and the kite referring
 * to the original uuid could not be resolved.
 *
 * @param data The pixel data of the kite image, either raw or a PNG file.
 * @param data_size The size of data in bytes.
 * @param width The width of the image in pixels.
 * @param height The height of the image in pixels.
 * @param format The pixel format of the image data.
 * @param id The uuid the kite image is stored under.
 * @return The uuid of the appended kite image.
 */
UUID tkbc_append_kite_image_and_kite_texture_with_id(unsigned char *data, int data_size, int width, int height,
                                                     int format, UUID id, bool is_dual_line) {
    if (tkbc_uuid_is_nil(id)) {
        // The id comes from the network, so a faulty or hostile peer could
        // claim the nil uuid. Fall back to a fresh uuid rather than storing an
        // unidentifiable asset.
        id = tkbc_uuid_generate();
    }

    if (format == TKBC_IMAGE_FORMAT_COMPRESSED_PNG) {
#ifdef TKBC_SERVER
        tkbc_append_compressed_kite_image_with_id(data, data_size, width, height, id, is_dual_line);
#else
        Image image = LoadImageFromMemory(".png", data, data_size);
        if (!IsImageValid(image)) {
            return id;
        }
        if (image.format != PIXELFORMAT_UNCOMPRESSED_R8G8B8A8) {
            ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
        }
        tkbc_append_kite_image_with_id(image.data, image.width, image.height, image.format, id, is_dual_line);
        UnloadImage(image);
#endif
    } else {
        (void) data_size;
        tkbc_append_kite_image_with_id(data, width, height, format, id, is_dual_line);
    }
    // This is just for compilation the function is not used in
    // the server at all. Just the files in this dir are all
    // passed to the server compilations as well.
#ifndef TKBC_SERVER
    // NOTE: This is the last appended item but for future possible data structure change we find it clean one more
    // time.
    Asset *asset = tkbc_find_asset_from_id(id);
    assert(asset != NULL);
    assert(asset->type == ASSETS_KITE_DESIGN);
    tkbc_load_kite_texture_from_kite_image(asset->as.kite_image, id);
#endif
    return id;
}

/**
 * @brief The function checks if an asset with the same image data already
 * exists in the assets.
 *
 * @param image The image that gets searched for in the assets.
 * @param id A pointer where the uuid of the found asset gets stored.
 * @return Returns true if an asset with the same image was found, otherwise
 * false.
 */
bool tkbc_image_already_exitst_in_assets(Image image, UUID *id) {
    for (size_t i = KITE_DEFAULT_DESIGNS_BEGIN; i < assets.count; ++i) {
        if (tkbc_is_same_image(image, assets.elements[i].as.image)) {
            *id = assets.elements[i].id;
            return true;
        }
    }
    return false;
}

/**
 * @brief Checks whether the given uuid belongs to one of the baked in default
 * kite designs.
 *
 * @param assets The assets collection holding the default asset sequence
 * counter.
 * @param uuid The uuid to check.
 * @return True if the uuid is a default kite design, otherwise false.
 */
bool tkbc_is_default_kite_design(Assets *assets, UUID uuid) {
    for (size_t i = 1; i <= assets->default_assets_seq; ++i) {
        if (tkbc_uuid_equals(uuid, tkbc_uuid_from_number(i))) {
            return true;
        }
    }

    return false;
}
