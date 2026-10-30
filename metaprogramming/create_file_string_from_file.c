#include <stdio.h>
#include <string.h>
#define TKBC_UTILS_IMPLEMENTATION
#include "../src/global/tkbc-utils.h"

#define CB_IMPLEMENTATION
#include "../cb.h"

char *compiler = "gcc";

struct {
    const char **elements;
    size_t count;
    size_t capacity;
} compiler_defines;

void usage(const char *program_name) {
    printf("Usage: %s [-cc <compiler>] [-D<macro>[=<value>] ...] -o <out_filepath> <filepath ...>\n", program_name);
}

bool append_file_contente(const char *filepath, Content *out_content) {
    Content file_content = {0};
    bool ok = true;

    Cmd cmd = {0};
    const char *temp_path = "___temp_file_string.h";
    cb_cmd_push(&cmd, compiler, "-E", "-fdirectives-only", "-std=c11", "-I./external/raylib-6.0_linux_amd64/include/");
    for (size_t i = 0; i < compiler_defines.count; ++i) {
        cb_cmd_push(&cmd, compiler_defines.elements[i]);
    }
    cb_cmd_push(&cmd, filepath, "-o", temp_path);
    if (!cb_run_sync(&cmd)) {
        check_return(false);
    }

    if (tkbc_read_entire_file(temp_path, &file_content) < 0) {
        check_return(false);
    }
    tkbc_remove(temp_path);

    for (size_t i = 0; i < file_content.count; ++i) {
        if (i == 0) {
            tkbc_dap(out_content, '\"');
        }

        if (file_content.elements[i] == '\\' || file_content.elements[i] == '\"') {
            tkbc_dap(out_content, '\\');
        }

        if (file_content.elements[i] == '\n') {
            tkbc_dapc(out_content, "\\n", 2);
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
    if (argc == 0) {
        printf("Error: no arguments are provided.\n");
        usage(program_name);
        return 1;
    }

    char *out_filepath = NULL;
    while (argc > 0) {
        char *arg = *argv;
        if (strcmp(arg, "-cc") == 0) {
            if (argc < 2) {
                printf("Error: -cc requires a compiler.\n");
                usage(program_name);
                return 1;
            }
            shift(argv, argc);
            compiler = shift(argv, argc);
        } else if (strcmp(arg, "-o") == 0) {
            if (argc < 2) {
                printf("Error: -o requires an output filepath.\n");
                usage(program_name);
                return 1;
            }
            shift(argv, argc);
            out_filepath = shift(argv, argc);
        } else if (strncmp(arg, "-D", 2) == 0) {
            tkbc_dap(&compiler_defines, shift(argv, argc));
        } else {
            break;
        }
    }

    if (out_filepath == NULL) {
        printf("Error: no output filepath is provided (use -o <filepath>).\n");
        usage(program_name);
        return 1;
    }

    if (argc == 0) {
        printf("Error: no input filepath is provided.\n");
        usage(program_name);
        return 1;
    }

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
