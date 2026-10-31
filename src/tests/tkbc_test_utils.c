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
    const char *file_content = "tkbc_script_input {\n}\n";
    const char *expected = "prefix\ntkbc_script_input {\n}\n";

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
 * @brief Run all file utility unit tests.
 *
 * @param tests Pointer to the Tests struct to register results in.
 */
void tkbc_test_utils(Tests *tests) {
    cassert_dap(tests, read_entire_file_terminates_content());
}
