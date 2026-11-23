#include "../../external/cassert/cassert.h"

#include "../global/tkbc-utils.h"

/**
 * @brief Ensures that reading a file into a non-empty Content buffer always
 * NUL terminates the result. The drag and drop compilation passes the buffer
 * to tcc_compile_string() which requires a NUL terminated string.
 */
Test read_entire_file_terminates_content(void) {
    Test test = cassert_init_test("tkbc_read_entire_file() terminates content");

    const char *path = "build/tkbc_read_entire_file_test.tmp";
    const char *prefix = "prefix\n";
    const char *file_content = "skystep_script {\n}\n";
    const char *expected = "prefix\nskystep_script {\n}\n";

    cassert_eq(tkbc_write_file(path, file_content, strlen(file_content)), 0);

    Content content = {0};
    tkbc_dapc(&content, prefix, strlen(prefix));

    // Simulate dirty spare capacity as it can happen when the heap reuses
    // memory from earlier allocations.
    memset(content.elements + content.count, 'X', content.capacity - content.count);

    long read_count = tkbc_read_entire_file(path, &content);

    cassert_long_eq(read_count, (long) strlen(file_content));
    cassert_size_t_eq(content.count, strlen(expected));
    cassert_char_eq(content.elements[content.count], '\0');
    cassert_string_eq(content.elements, expected);

    free(content.elements);
    cassert_bool_eq(tkbc_remove(path), true);

    return test;
}

/**
 * @brief The fast PNG writer must produce a file that reloads losslessly.
 */
Test fast_png_export_roundtrip(void) {
    Test test = cassert_init_test("tkbc_export_image_png_fast() roundtrip");

    const int width = 7;
    const int height = 5;
    const size_t pixel_count = (size_t) width * (size_t) height * 4;
    unsigned char *pixels = (unsigned char *) MemAlloc((unsigned int) pixel_count);
    for (size_t i = 0; i < pixel_count; ++i) {
        pixels[i] = (unsigned char) ((i * 37u + (i / 4u) * 11u) & 0xffu);
    }

    Image source = (Image){
        .data = pixels,
        .width = width,
        .height = height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
    };

    int png_size = 0;
    unsigned char *png = tkbc_export_image_png_fast(source, &png_size);
    cassert_bool_eq(png != NULL && png_size > 0, true);

    Image decoded = LoadImageFromMemory(".png", png, png_size);
    cassert_bool_eq(IsImageValid(decoded), true);
    if (IsImageValid(decoded)) {
        cassert_int_eq(decoded.width, width);
        cassert_int_eq(decoded.height, height);
        cassert_int_eq(memcmp(decoded.data, pixels, pixel_count), 0);
        UnloadImage(decoded);
    }

    MemFree(png);
    MemFree(pixels);
    return test;
}

/**
 * @brief The LZ77 match path of the fast PNG writer must also be lossless.
 */
Test fast_png_export_roundtrip_matches(void) {
    Test test = cassert_init_test("tkbc_export_image_png_fast() roundtrip with matches");

    const int width = 64;
    const int height = 32;
    const size_t pixel_count = (size_t) width * (size_t) height * 4;
    unsigned char *pixels = (unsigned char *) MemAlloc((unsigned int) pixel_count);
    const unsigned char pattern[16] = {0, 17, 34, 51, 68, 85, 102, 119,
                                       136, 153, 170, 187, 204, 221, 238, 255};
    for (size_t i = 0; i < pixel_count; ++i) {
        pixels[i] = i < pixel_count / 2 ? 0x5a : pattern[i % sizeof(pattern)];
    }

    Image source = (Image){
        .data = pixels,
        .width = width,
        .height = height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
    };

    int png_size = 0;
    unsigned char *png = tkbc_export_image_png_fast(source, &png_size);
    cassert_bool_eq(png != NULL && png_size > 0, true);

    Image decoded = LoadImageFromMemory(".png", png, png_size);
    cassert_bool_eq(IsImageValid(decoded), true);
    if (IsImageValid(decoded)) {
        cassert_int_eq(decoded.width, width);
        cassert_int_eq(decoded.height, height);
        cassert_int_eq(memcmp(decoded.data, pixels, pixel_count), 0);
        UnloadImage(decoded);
    }

    MemFree(png);
    MemFree(pixels);
    return test;
}

/**
 * @brief Run all file utility unit tests.
 *
 * @param tests Pointer to the Tests struct to register results in.
 */
void tkbc_test_utils(Tests *tests) {
    cassert_dap(tests, read_entire_file_terminates_content());
    cassert_dap(tests, fast_png_export_roundtrip());
    cassert_dap(tests, fast_png_export_roundtrip_matches());
}
