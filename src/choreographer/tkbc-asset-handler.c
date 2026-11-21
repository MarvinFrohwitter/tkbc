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
#ifndef TKBC_SERVER
static Image tkbc_load_compressed_asset_image(const unsigned char *data, int data_size, const char *file_type);
#endif
static UUID tkbc_append_owned_kite_image(unsigned char *data, int width, int height, int format, UUID id,
                                         uint64_t content_hash, bool is_dual_line);
#ifndef TKBC_SERVER
static UUID tkbc_append_colorizer_from_baked_png(const unsigned char *data, int data_size, uint64_t content_hash,
                                                 bool is_dual_line);
#endif

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
 * @brief Appends a raw kite image and takes ownership of its pixel buffer.
 *
 * The buffer must have been allocated with raylib's MemAlloc (as returned by
 * LoadImageFromMemory/ImageCopy), because it is eventually released with
 * UnloadImage(). This avoids an otherwise redundant full-size ImageCopy.
 */
static UUID tkbc_append_owned_kite_image(unsigned char *data, int width, int height, int format, UUID id,
                                         uint64_t content_hash, bool is_dual_line) {
    Kite_Image kite_image = (Kite_Image){
        .normal =
            {
                .data = data,
                .width = width,
                .height = height,
                .mipmaps = 1,
                .format = format,
            },
        .compressed_size = 0,
        .content_hash = content_hash,
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

#ifndef TKBC_SERVER
/**
 * @brief Decodes one of the baked in kite PNGs directly into the colorizer
 * buffer. Only the two colorizers keep raw pixels; all other designs stay
 * compressed.
 */
static UUID tkbc_append_colorizer_from_baked_png(const unsigned char *data, int data_size, uint64_t content_hash,
                                                 bool is_dual_line) {
    Image image = tkbc_load_compressed_asset_image(data, data_size, ".png");
    if (!IsImageValid(image)) {
        return tkbc_uuid_nil();
    }
    if (image.format != PIXELFORMAT_UNCOMPRESSED_R8G8B8A8) {
        ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    }
    return tkbc_append_owned_kite_image(image.data, image.width, image.height, image.format,
                                        tkbc_generate_uuid_for_asset(), content_hash, is_dual_line);
}
#endif  // !TKBC_SERVER

/**
 * @brief The function appends a new kite image under the given uuid to the
 * global kite_images collection. The image data is copied and stored.
 *
 * Callers that already own a raylib-allocated pixel buffer should use the
 * internal tkbc_append_owned_kite_image() instead to avoid a full-size copy.
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
    Image copy = ImageCopy((Image){
        .data = data,
        .width = width,
        .height = height,
        .mipmaps = 1,
        .format = format,
    });
    return tkbc_append_owned_kite_image(copy.data, copy.width, copy.height, copy.format, id, 0, is_dual_line);
}

/**
 * @brief Appends a kite image whose data is still a PNG file.
 *
 * This is the canonical representation of every non-editable kite design.
 * The PNG bytes are kept for network transfer; pixels are only decoded
 * temporarily when a GPU texture is created. That way one saved design costs
 * a few hundred kilobytes instead of ~84 MB of raw RGBA per client.
 *
 * @param data The PNG file bytes.
 * @param data_size The size of the PNG file in bytes.
 * @param width The image width.
 * @param height The image height.
 * @param content_hash Hash of the decoded RGBA pixels, or 0 if unknown.
 * @param id The uuid the image is stored under.
 * @return The uuid of the appended kite image.
 */
UUID tkbc_append_compressed_kite_image_with_id(const unsigned char *data, int data_size, int width, int height,
                                               uint64_t content_hash, UUID id, bool is_dual_line) {
    if (tkbc_uuid_is_nil(id)) {
        id = tkbc_uuid_generate();
    }

    Kite_Image kite_image = (Kite_Image){
        .normal = tkbc_copy_compressed_image(data, data_size, width, height),
        .compressed_size = data_size,
        .content_hash = content_hash,
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
 * collection. Ownership of the decoded pixel buffer is transferred to the
 * asset, so no extra full-size copy is made.
 *
 * @param data The raw image data (raylib allocated).
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

    UUID id = tkbc_generate_uuid_for_asset();
    space_dap(&assets.space, &assets,
              ((Asset){
                  .type = ASSETS_IMAGE,
                  .as.image = image,
                  .id = id,
              }));

    return id;
}

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

static uint32_t tkbc_mask_read_u32_le(const unsigned char *data) {
    return (uint32_t) data[0] | ((uint32_t) data[1] << 8) | ((uint32_t) data[2] << 16) | ((uint32_t) data[3] << 24);
}

/**
 * @brief Registers a baked in RLE alpha mask as ASSETS_PANEL_MASK.
 *
 * The compressed bytes live in the generated header (static read-only memory),
 * while the unpacked bit/alpha buffer is only allocated on first use.
 */
UUID tkbc_append_panel_mask(const unsigned char *data, int data_size) {
    if (data == NULL || data_size < TKBC_MASK_HEADER_SIZE || memcmp(data, "TMSK", 4) != 0 || data[4] != 1 ||
        (data[5] != TKBC_MASK_KIND_BINARY && data[5] != TKBC_MASK_KIND_ALPHA8)) {
        tkbc_fprintf(stderr, "ERROR", "Invalid baked in panel mask.\n");
        return tkbc_uuid_nil();
    }

    Panel_Mask mask = {
        .compressed = data,
        .compressed_size = data_size,
        .width = (int) tkbc_mask_read_u32_le(data + 6),
        .height = (int) tkbc_mask_read_u32_le(data + 10),
        .kind = data[5],
        .x0 = (int) tkbc_mask_read_u32_le(data + 14),
        .y0 = (int) tkbc_mask_read_u32_le(data + 18),
        .x1 = (int) tkbc_mask_read_u32_le(data + 22),
        .y1 = (int) tkbc_mask_read_u32_le(data + 26),
    };
    if (mask.width <= 0 || mask.height <= 0) {
        return tkbc_uuid_nil();
    }

    UUID id = tkbc_generate_uuid_for_asset();
    space_dap(&assets.space, &assets, ((Asset){
                                          .type = ASSETS_PANEL_MASK,
                                          .as.panel_mask = mask,
                                          .id = id,
                                      }));
    return id;
}

/** @brief Tests one bit of an already unpacked binary mask. */
static inline bool tkbc_panel_mask_bit_set(const Panel_Mask *mask, int x, int y) {
    const size_t index = (size_t) y * (size_t) mask->width + (size_t) x;
    return (mask->bits[index >> 3] & (1u << (index & 7))) != 0;
}

/**
 * @brief Builds a run list for a binary mask so painting can skip transparent
 * pixels without visiting them at all.
 */
static void tkbc_panel_mask_build_runs(Panel_Mask *mask) {
    if (mask->kind != TKBC_MASK_KIND_BINARY || mask->x1 < mask->x0 || mask->y1 < mask->y0) {
        return;
    }

    size_t count = 0;
    for (int y = mask->y0; y <= mask->y1; ++y) {
        bool in_run = false;
        for (int x = mask->x0; x <= mask->x1; ++x) {
            if (tkbc_panel_mask_bit_set(mask, x, y)) {
                if (!in_run) {
                    ++count;
                    in_run = true;
                }
            } else {
                in_run = false;
            }
        }
    }
    if (count == 0) {
        return;
    }

    mask->runs = (uint32_t *) MemAlloc(count * 3 * sizeof(uint32_t));
    assert(mask->runs != NULL && "Out of memory while building panel mask runs");
    if (mask->runs == NULL) {
        return;
    }
    mask->run_count = count;

    size_t r = 0;
    for (int y = mask->y0; y <= mask->y1; ++y) {
        int run_x0 = -1;
        for (int x = mask->x0; x <= mask->x1; ++x) {
            if (tkbc_panel_mask_bit_set(mask, x, y)) {
                if (run_x0 < 0) {
                    run_x0 = x;
                }
            } else if (run_x0 >= 0) {
                mask->runs[r * 3 + 0] = (uint32_t) y;
                mask->runs[r * 3 + 1] = (uint32_t) run_x0;
                mask->runs[r * 3 + 2] = (uint32_t) (x - 1);
                ++r;
                run_x0 = -1;
            }
        }
        if (run_x0 >= 0) {
            mask->runs[r * 3 + 0] = (uint32_t) y;
            mask->runs[r * 3 + 1] = (uint32_t) run_x0;
            mask->runs[r * 3 + 2] = (uint32_t) mask->x1;
            ++r;
        }
    }
    assert(r == count && "Panel mask run count mismatch");
}

/** @brief Expands the RLE stream into a directly indexable mask. */
void tkbc_panel_mask_unpack(Panel_Mask *mask) {
    if (mask == NULL || mask->bits != NULL) {
        return;
    }

    const size_t pixel_count = (size_t) mask->width * (size_t) mask->height;
    size_t output_size = mask->kind == TKBC_MASK_KIND_BINARY ? (pixel_count + 7) / 8 : pixel_count;
    mask->bits = (unsigned char *) MemAlloc(output_size);
    assert(mask->bits != NULL && "Out of memory while unpacking a panel mask");
    if (mask->bits == NULL) {
        return;
    }
    memset(mask->bits, 0, output_size);
    mask->bits_size = output_size;

    size_t pos = TKBC_MASK_HEADER_SIZE;
    size_t output = 0;
    while (pos < (size_t) mask->compressed_size && output < pixel_count) {
        uint64_t run = 0;
        unsigned int shift = 0;
        unsigned char byte = 0;
        do {
            byte = mask->compressed[pos++];
            run |= (uint64_t) (byte & 0x7f) << shift;
            shift += 7;
        } while ((byte & 0x80) != 0 && pos < (size_t) mask->compressed_size);

        if (pos >= (size_t) mask->compressed_size) {
            break;
        }
        const unsigned char value = mask->compressed[pos++];
        const size_t count = run > pixel_count - output ? pixel_count - output : (size_t) run;

        if (mask->kind == TKBC_MASK_KIND_BINARY) {
            if (value != 0) {
                for (size_t i = 0; i < count; ++i) {
                    const size_t bit = output + i;
                    mask->bits[bit / 8] |= (unsigned char) (1u << (bit % 8));
                }
            }
        } else {
            memset(mask->bits + output, value, count);
        }
        output += count;
    }
    assert(output == pixel_count && "Panel mask RLE stream is incomplete");
    tkbc_panel_mask_build_runs(mask);
}

/** @brief Returns the alpha value (0..255) of one mask pixel. */
unsigned char tkbc_panel_mask_alpha_at(Panel_Mask *mask, int x, int y) {
    assert(mask != NULL);
    // Hit-testing can be the first access to a mask, so make sure it is ready.
    tkbc_panel_mask_unpack(mask);
    assert(mask->bits != NULL);
    assert(x >= 0 && y >= 0 && x < mask->width && y < mask->height);
    if (x < 0 || y < 0 || x >= mask->width || y >= mask->height) {
        return 0;
    }

    const size_t index = (size_t) y * (size_t) mask->width + (size_t) x;
    if (mask->kind == TKBC_MASK_KIND_BINARY) {
        return (mask->bits[index / 8] & (1u << (index % 8))) != 0 ? 255 : 0;
    }
    return mask->bits[index];
}

/**
 * @brief Paints one horizontal opaque span and expands the dirty rectangle.
 *
 * @p changed is shared across all spans of a mask. The first written pixel of
 * the whole mask initializes the rectangle; every later span only expands it.
 * Using a per-span flag would overwrite the rectangle with the last span and
 * leave the rest of the painted panel invisible on the GPU.
 */
static void tkbc_panel_mask_paint_span(const Panel_Mask *mask, unsigned char *dst, Color replace, char alpha_threshold,
                                       bool overwrite, int y, int x0, int x1, bool *changed, int *dx0, int *dy0,
                                       int *dx1, int *dy1) {
    const size_t row = (size_t) y * (size_t) mask->width;
    for (int x = x0; x <= x1; ++x) {
        unsigned char *out = dst + (row + (size_t) x) * 4;
        if (!overwrite && out[3] > alpha_threshold) {
            continue;
        }
        out[0] = replace.r;
        out[1] = replace.g;
        out[2] = replace.b;
        out[3] = replace.a;

        if (!*changed) {
            *dx0 = *dx1 = x;
            *dy0 = *dy1 = y;
            *changed = true;
        } else {
            if (x < *dx0) *dx0 = x;
            if (x > *dx1) *dx1 = x;
            if (y < *dy0) *dy0 = y;
            if (y > *dy1) *dy1 = y;
        }
    }
}

/**
 * @brief Paints every opaque mask pixel into the RGBA buffer of the colorizer.
 *
 * This is the hot path of the colorizer. The opaque bounds come from the
 * generated header, the mask is unpacked only once, and binary masks are
 * painted through a precomputed run list so transparent areas are skipped
 * entirely. When @p overwrite is false, pixels that are already filled in the
 * destination are left alone.
 *
 * @return True if at least one pixel was written. The touched pixels are
 *         reported through the dirty rectangle pointers.
 */
bool tkbc_panel_mask_paint(Panel_Mask *mask, unsigned char *dst, Color replace, char alpha_threshold, bool overwrite,
                           int *dx0, int *dy0, int *dx1, int *dy1) {
    assert(mask != NULL && dst != NULL);
    if (mask->x1 < mask->x0 || mask->y1 < mask->y0) {
        return false;
    }

    tkbc_panel_mask_unpack(mask);
    assert(mask->bits != NULL);

    const unsigned char *bits = mask->bits;
    const int stride = mask->width;
    const bool binary = mask->kind == TKBC_MASK_KIND_BINARY;
    bool changed = false;

    if (binary && mask->runs != NULL) {
        for (size_t r = 0; r < mask->run_count; ++r) {
            const int y = (int) mask->runs[r * 3 + 0];
            const int x0 = (int) mask->runs[r * 3 + 1];
            const int x1 = (int) mask->runs[r * 3 + 2];
            tkbc_panel_mask_paint_span(mask, dst, replace, alpha_threshold, overwrite, y, x0, x1, &changed, dx0, dy0,
                                       dx1, dy1);
        }
        return changed;
    }

    // Alpha8 masks (anti-aliased edges) need the exact alpha value per pixel.
    for (int y = mask->y0; y <= mask->y1; ++y) {
        const size_t row = (size_t) y * (size_t) stride;
        for (int x = mask->x0; x <= mask->x1; ++x) {
            const size_t index = row + (size_t) x;
            if (bits[index] <= alpha_threshold) {
                continue;
            }

            unsigned char *out = dst + index * 4;
            if (!overwrite && out[3] > alpha_threshold) {
                continue;
            }
            out[0] = replace.r;
            out[1] = replace.g;
            out[2] = replace.b;
            out[3] = replace.a;

            if (!changed) {
                *dx0 = *dx1 = x;
                *dy0 = *dy1 = y;
                changed = true;
            } else {
                if (x < *dx0) *dx0 = x;
                if (x > *dx1) *dx1 = x;
                if (y < *dy0) *dy0 = y;
                if (y > *dy1) *dy1 = y;
            }
        }
    }
    return changed;
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

    // tkbc_append_asset_image() takes ownership of the decoded buffer, so no
    // UnloadImage() here and no redundant ImageCopy() inside the append.
    return tkbc_append_asset_image(image.data, image.width, image.height, image.format);
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
                                              uint64_t content_hash, bool is_dual_line) {
    (void) file_type;
    int width = 0;
    int height = 0;
    if (!tkbc_png_dimensions(data, data_size, &width, &height)) {
        return tkbc_uuid_nil();
    }
    return tkbc_append_compressed_kite_image_with_id(data, data_size, width, height, content_hash,
                                                     tkbc_generate_uuid_for_asset(), is_dual_line);
}

#define tkbc_append_baked_asset_image(asset_symbol)                                                                    \
    tkbc_append_compressed_asset_image((asset_symbol), (int) sizeof(asset_symbol), ".png")
#define tkbc_append_baked_panel_mask(asset_symbol)                                                                     \
    tkbc_append_panel_mask(asset_mask_##asset_symbol, (int) sizeof(asset_mask_##asset_symbol))
#define tkbc_append_baked_kite_image(asset_symbol, is_dual_line)                                                       \
    tkbc_append_compressed_kite_image((asset_symbol), (int) sizeof(asset_symbol), ".png", (asset_symbol##_hash),        \
                                      (is_dual_line))

/**
 * @brief Append all kite image assets to the global asset list.
 *
 * Registers the predefined kite panel kite images, Skeleton, Filled Panel,
 * Leadingedge, Gaze, etc.
 */
void tkbc_append_ce_ng_kite_image_pannels() {
    tkbc_append_baked_panel_mask(skeleton);
    // The filled panel is the template for reset/duplicate detection and needs
    // its RGB values, so it stays a decoded ASSETS_IMAGE.
    tkbc_append_baked_asset_image(asset_image_filled_panel);
    tkbc_append_baked_panel_mask(leadingedge);
    tkbc_append_baked_panel_mask(gaze);

    tkbc_append_baked_panel_mask(left_01_1);
    tkbc_append_baked_panel_mask(left_02_1);
    tkbc_append_baked_panel_mask(left_03_1);
    tkbc_append_baked_panel_mask(left_04_1);
    tkbc_append_baked_panel_mask(left_05_1);
    tkbc_append_baked_panel_mask(left_06_1);
    tkbc_append_baked_panel_mask(left_07_1);
    tkbc_append_baked_panel_mask(left_08_1);
    tkbc_append_baked_panel_mask(left_09_1);
    tkbc_append_baked_panel_mask(left_10_1);
    tkbc_append_baked_panel_mask(left_11_1);
    tkbc_append_baked_panel_mask(left_12_1);
    tkbc_append_baked_panel_mask(left_13_1);
    tkbc_append_baked_panel_mask(left_14_1);
    tkbc_append_baked_panel_mask(left_15_1);

    tkbc_append_baked_panel_mask(right_01_2);
    tkbc_append_baked_panel_mask(right_02_2);
    tkbc_append_baked_panel_mask(right_03_2);
    tkbc_append_baked_panel_mask(right_04_2);
    tkbc_append_baked_panel_mask(right_05_2);
    tkbc_append_baked_panel_mask(right_06_2);
    tkbc_append_baked_panel_mask(right_07_2);
    tkbc_append_baked_panel_mask(right_08_2);
    tkbc_append_baked_panel_mask(right_09_2);
    tkbc_append_baked_panel_mask(right_10_2);
    tkbc_append_baked_panel_mask(right_11_2);
    tkbc_append_baked_panel_mask(right_12_2);
    tkbc_append_baked_panel_mask(right_13_2);
    tkbc_append_baked_panel_mask(right_14_2);
    tkbc_append_baked_panel_mask(right_15_2);
}

void tkbc_append_111_kite_image_pannels() {

    tkbc_append_baked_panel_mask(111_skeleton);
    // Same as above: the base white panel doubles as the 111 colorizer
    // template, so it keeps its decoded pixels.
    tkbc_append_baked_asset_image(asset_image_111_base_white);

    tkbc_append_baked_panel_mask(111_left_01_1);
    tkbc_append_baked_panel_mask(111_left_02_1);
    tkbc_append_baked_panel_mask(111_left_03_1);
    tkbc_append_baked_panel_mask(111_left_04_1);
    tkbc_append_baked_panel_mask(111_left_05_1);
    tkbc_append_baked_panel_mask(111_left_06_1);

    tkbc_append_baked_panel_mask(111_right_01_2);
    tkbc_append_baked_panel_mask(111_right_02_2);
    tkbc_append_baked_panel_mask(111_right_03_2);
    tkbc_append_baked_panel_mask(111_right_04_2);
    tkbc_append_baked_panel_mask(111_right_05_2);
    tkbc_append_baked_panel_mask(111_right_06_2);
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
    tkbc_append_compressed_kite_image(asset_image_filled_panel, (int) sizeof(asset_image_filled_panel), ".png",
                                      asset_image_filled_panel_hash, false);
    tkbc_append_compressed_kite_image(asset_image_111_base_white, (int) sizeof(asset_image_111_base_white), ".png",
                                      asset_image_111_base_white_hash, true);
#else
    // Decode straight into the colorizer buffer instead of copying the already
    // decoded filled panel image once more (that cost another ~96 MB).
    tkbc_append_colorizer_from_baked_png(asset_image_filled_panel, (int) sizeof(asset_image_filled_panel),
                                         asset_image_filled_panel_hash, false);
    tkbc_append_colorizer_from_baked_png(asset_image_111_base_white, (int) sizeof(asset_image_111_base_white),
                                         asset_image_111_base_white_hash, true);
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

    Image image = kite_image.normal;
    bool temporary_decode = false;
    if (image.format == TKBC_IMAGE_FORMAT_COMPRESSED_PNG) {
        // Designs are stored as PNG bytes. Decode only long enough to upload
        // the texture, then release the pixels. Hashes are baked into the
        // generated asset headers for the default designs and computed lazily
        // for designs created at runtime.
        image = LoadImageFromMemory(".png", image.data, kite_image.compressed_size);
        if (!IsImageValid(image)) {
            tkbc_fprintf(stderr, "ERROR", "Could not decode kite design texture.\n");
            return;
        }
        if (image.format != PIXELFORMAT_UNCOMPRESSED_R8G8B8A8) {
            ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
        }
        temporary_decode = true;
    }

    asset->as.kite_texture = (Kite_Texture){
        .normal = LoadTextureFromImage(image),
    };
    GenTextureMipmaps(&asset->as.kite_texture.normal);
    // Kite textures are drawn at arbitrary (fractional) positions and scales.
    // With the default repeat wrap the sampling of the outermost edge can wrap
    // around to the opposite edge, which made the top pixel row of a texture
    // show up at its bottom. Clamping keeps the edge pixels in place.
    SetTextureWrap(asset->as.kite_texture.normal, TEXTURE_WRAP_CLAMP);
    SetTextureFilter(asset->as.kite_texture.normal, TEXTURE_FILTER_BILINEAR);

    if (temporary_decode) {
        UnloadImage(image);
    }
}

/**
 * @brief Returns the GPU texture of a kite design, creating it on first use.
 *
 * Decoding a default design uploads an 84 MB texture, so doing that for all of
 * them during startup made the program take about two seconds to appear. The
 * colorizers are still created up front because the UI always shows them.
 *
 * @param asset The kite design asset.
 * @return The design's texture.
 */
Texture2D tkbc_get_kite_texture(Asset *asset) {
    assert(asset != NULL && asset->type == ASSETS_KITE_DESIGN);
    if (!IsTextureValid(asset->as.kite_texture.normal)) {
        tkbc_load_kite_texture_from_kite_image(asset->as.kite_image, asset->id);
    }
    return asset->as.kite_texture.normal;
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

        // Panel masks have no GPU texture; they are expanded on demand by the
        // colorizer.
        if (assets.elements[i].type == ASSETS_PANEL_MASK) {
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

        // Default designs are decoded when they are first drawn or shown in
        // the design picker. Only the two colorizers are needed immediately.
        if (i > KITE_111_COLORIZER) {
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

#ifdef TKBC_SERVER
Texture2D tkbc_get_kite_texture(Asset *asset) {
    assert(asset != NULL && asset->type == ASSETS_KITE_DESIGN);
    return asset->as.kite_texture.normal;
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
    case ASSETS_PANEL_MASK:
        // compressed points into a static generated header; only the lazy
        // unpacked buffer and run list are owned by the asset.
        if (asset->as.panel_mask.bits != NULL) {
            MemFree(asset->as.panel_mask.bits);
            asset->as.panel_mask.bits = NULL;
        }
        if (asset->as.panel_mask.runs != NULL) {
            MemFree(asset->as.panel_mask.runs);
            asset->as.panel_mask.runs = NULL;
        }
        break;
    case ASSETS_KITE_DESIGN: UnloadImage(asset->as.kite_image.normal);

#ifndef TKBC_SERVER
        // Default designs may never have been drawn, so their texture can be
        // invalid here. Only unload textures that were actually created.
        if (IsTextureValid(asset->as.kite_texture.normal)) {
            UnloadTexture(asset->as.kite_texture.normal);
        }
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
        // The wire always carries PNG bytes. Keep them compressed on both
        // sides; the client decodes only while creating the GPU texture.
        tkbc_append_compressed_kite_image_with_id(data, data_size, width, height, 0, id, is_dual_line);
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
    return tkbc_image_already_exitst_in_assets_with_hash(image, tkbc_image_content_hash(image), id);
}

bool tkbc_image_already_exitst_in_assets_with_hash(Image image, uint64_t hash, UUID *id) {
    if (hash == 0) {
        return false;
    }

    for (size_t i = KITE_DEFAULT_DESIGNS_BEGIN; i < assets.count; ++i) {
        Asset *asset = &assets.elements[i];
        if (asset->type != ASSETS_KITE_DESIGN) {
            continue;
        }
        if (asset->as.kite_image.normal.width != image.width ||
            asset->as.kite_image.normal.height != image.height) {
            continue;
        }

        // The hash is filled when the texture is created. Compute it lazily if
        // that has not happened yet (e.g. for a design that was never drawn).
        if (asset->as.kite_image.content_hash == 0) {
            Kite_Image *design = &asset->as.kite_image;
            if (design->normal.format == TKBC_IMAGE_FORMAT_COMPRESSED_PNG) {
                Image decoded = LoadImageFromMemory(".png", design->normal.data, design->compressed_size);
                if (!IsImageValid(decoded)) {
                    continue;
                }
                if (decoded.format != PIXELFORMAT_UNCOMPRESSED_R8G8B8A8) {
                    ImageFormat(&decoded, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
                }
                design->content_hash = tkbc_image_content_hash(decoded);
                UnloadImage(decoded);
            } else {
                design->content_hash = tkbc_image_content_hash(design->normal);
            }
        }

        if (asset->as.kite_image.content_hash == hash) {
            *id = asset->id;
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
