#include <stdio.h>
#include <string.h>
#define TKBC_UTILS_IMPLEMENTATION
#include "../src/global/tkbc-utils.h"

void usage(const char *program_name) {
    printf("Usage: %s <out_filepath> <filepath ...>\n", program_name);
}

bool append_file_contente(const char *filepath, Content *out_content) {
    Content file_content = {0};
    bool ok = true;
    if (tkbc_read_entire_file(filepath, &file_content) < 0) {
        check_return(false);
    }

    for (size_t i = 0; i < file_content.count; ++i) {
        if (i == 0) {
            tkbc_dap(out_content, '\"');
        }

        if (file_content.elements[i] == '\\' || file_content.elements[i] == '\"') {
            tkbc_dap(out_content, '\\');
        }

        if (file_content.elements[i] == '\n') {
            tkbc_dap(out_content, '\"');
        }

        tkbc_dap(out_content, file_content.elements[i]);

        if (file_content.elements[i] == '\n') {
            tkbc_dap(out_content, '\"');
        }

        if (i == file_content.count - 1) {
            tkbc_dap(out_content, '\"');
        }
    }

check:
    if (file_content.elements) {
        free(file_content.elements);
    }
    return ok;
}

int main(int argc, char *argv[]) {
    const char *program_name = shift(argv, argc);
    if (argc < 2) {
        printf("Error: no arguments are provided.\n");
        usage(program_name);
        return 1;
    }
    const char *out_filepath = shift(argv, argc);

    Content out_content = {0};
    const char *prefix = "static const char* global_api_header_string =\n";
    tkbc_dapc(&out_content, prefix, strlen(prefix));
    for (int i = 0; i < argc; ++i) {
        bool ok = true;
        ok = append_file_contente(argv[i], &out_content);
        if (!ok) {
            return 2;
        }
    }
    const char *postfix = ";\n";
    tkbc_dapc(&out_content, postfix, strlen(postfix));

    if (out_content.count) {
        int ok = tkbc_write_file(out_filepath, out_content.elements, out_content.count);
        if (ok < 0) {
            return 3;
        }
    }
    return 0;
}
