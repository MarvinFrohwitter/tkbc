#include <string.h>
#define CB_IMPLEMENTATION
#include "cb.h"
Cmd cmd = {0};

#define shift(array, size) (assert(0 < (size)), (size)--, *(array)++)
#define CC "gcc"
#define BUILD_PATH "build/"
#define RAYLIB_PATH "external/raylib-6.0_linux_amd64/"
#define TESTS_PATH "src/tests/"
#define CHOREOGRAPHER_PATH "src/choreographer/"

bool str_compare(const char *s1, const char *s2) {
  if (!s1 || !s2) {
    return false;
  }

  return strcmp(s1, s2) == 0;
}

char *get_next_or_last(char ***array, int *size) {
  if (!size) {
    return NULL;
  }
  if (*size > 1) {
    return shift(*array, *size);
  } else {
    return **array;
  }
}

typedef struct {
  bool include_raylib;
  bool tkbc_server;

  bool print_operation_and_description;
  bool short_log;
} Define_Opts;

#define define(cmd, ...) define_opt((cmd), ((Define_Opts){__VA_ARGS__}))

void define_opt(Cmd *cmd, Define_Opts opts) {
  if (opts.tkbc_server) {
    CFLAGS(cmd, "-DTKBC_SERVER");
  }
  if (opts.include_raylib) {
    CFLAGS(cmd, "-DINCLUDE_RAYLIB");
  }
  if (opts.print_operation_and_description) {
    CFLAGS(cmd, "-DPRINT_OPERATION_AND_DESCRIPTION");
  }
  if (opts.short_log) {
    CFLAGS(cmd, "-DSHORT_LOG");
  }
}

void cflags(Cmd *cmd, bool sanitize) {
  CFLAGS(cmd, "-fPIC", "-O3", "-Wall", "-Wextra", "-ggdb");
  if (sanitize) {
    CFLAGS(cmd, "-fsanitize=address");
    CFLAGS(cmd, "-fsanitize=undefined");
    CFLAGS(cmd, "-fno-sanitize-recover=undefined");
  }
}

void include(Cmd *cmd, bool include_raylib) {
  INCLUDE(cmd, "-I", CHOREOGRAPHER_PATH);
  INCLUDE(cmd, "-I", "src/global/", "-I", "src/network/");
  INCLUDE(cmd, "-I", "src/network/");
  INCLUDE(cmd, "-I", "tkbc_scripts/");
  INCLUDE(cmd, "-I", BUILD_PATH);
  if (include_raylib) {
    INCLUDE(cmd, "-I", RAYLIB_PATH "include/");
  }
}

typedef struct {
  bool math;
  bool include_raylib;
  bool X11;
} Libs_Opts;

#define libs(cmd, ...) libs_opt((cmd), ((Libs_Opts){__VA_ARGS__}))
void libs_opt(Cmd *cmd, Libs_Opts opts) {
  if (opts.math) {
    INCLUDE(cmd, "-lm");
  }
  if (opts.include_raylib) {
    INCLUDE(cmd, "-L", RAYLIB_PATH "lib/");
    INCLUDE(cmd, "-l:libraylib.a");
  }
  if (opts.X11) {
    INCLUDE(cmd, "-lX11");
  }
}

void files_for_test(Cmd *cmd) {
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-script-api.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-team-figures-api.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-script-handler.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-keymaps.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-script-converter.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-parser.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-asset-handler.c");
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void clean(Cmd *cmd) {
  cb_cmd_push(cmd, "rm", "-rf", "build");
  if (!cb_run_sync(cmd))
    exit(EXIT_FAILURE);
}

typedef struct {
  bool normal;
  bool verbose;
  bool short_log;
} Tests_Opts;

#define tests(cmd, ...) tests_opt((cmd), ((Tests_Opts){__VA_ARGS__}))
void tests_opt(Cmd *cmd, Tests_Opts opts) {
  cb_cmd_push(cmd, CC);
  include(cmd, true);
  cflags(cmd, false);
  if (0) {
  } else if (opts.normal) {
    define(cmd, .include_raylib = true, .tkbc_server = true);
  } else if (opts.verbose) {
    define(cmd, .include_raylib = true, .tkbc_server = true,
           .print_operation_and_description = true);
  } else if (opts.short_log) {
    define(cmd, .include_raylib = true, .tkbc_server = true, .short_log = true);
  } else {
    exit(EXIT_FAILURE);
  }
  cb_cmd_push(cmd, "-o", BUILD_PATH "tests");
  cb_cmd_push(cmd, TESTS_PATH "tkbc_tests.c");
  files_for_test(cmd);
  libs(cmd, .include_raylib = true, .X11 = true, .math = true);

  if (!cb_run_sync(cmd))
    exit(EXIT_FAILURE);

  cb_cmd_push(cmd, "./" BUILD_PATH "tests");
  if (!cb_run_sync(cmd))
    exit(EXIT_FAILURE);
}

void make_build_dir(Cmd *cmd) {
  cb_cmd_push(cmd, "mkdir", "-p", "build");
  if (!cb_run_sync(cmd))
    exit(EXIT_FAILURE);
}

typedef struct {
  const char *prog_name;

  bool all;
  bool test;
  bool clean;
} Usage_Opts;

#define usage(...) usage_opt(((Usage_Opts){__VA_ARGS__}))
void usage_opt(Usage_Opts opts) {
  fprintf(stderr, "Usage:\n");
  if (opts.test || opts.all) {
    fprintf(stderr, "       <%s> <test> [verbose, short]\n", opts.prog_name);
  }
  if (opts.clean || opts.all) {
    fprintf(stderr, "       <%s> <clean>\n", opts.prog_name);
  }
  exit(EXIT_FAILURE);
}

static char *prog_name = NULL;
int main(int argc, char *argv[]) {
  prog_name = get_next_or_last(&argv, &argc);
  if (!prog_name) {
    usage(.prog_name = prog_name, .all = true);
  }
  char *flag = get_next_or_last(&argv, &argc);
  if (0) {
  } else if (str_compare("help", flag)) {
    usage(.prog_name = prog_name, .all = true);
  } else if (str_compare("build", flag)) {
    make_build_dir(&cmd);
  } else if (str_compare("clean", flag)) {
    make_build_dir(&cmd);
    void flag_clean(char *flag, char ***argv, int *argc);
    flag_clean(flag, &argv, &argc);
  } else if (str_compare("test", flag)) {
    make_build_dir(&cmd);
    void flag_test(char *flag, char ***argv, int *argc);
    flag_test(flag, &argv, &argc);
  } else {
    usage(.prog_name = prog_name, .all = true);
  }

  return 0;
}

void flag_clean(char *flag, char ***argv, int *argc) {
  clean(&cmd);
  char *prev_flag = flag;
  flag = get_next_or_last(argv, argc);
  if (prev_flag != flag) {
    usage(.prog_name = prog_name, .clean = true);
  }
}

void flag_test(char *flag, char ***argv, int *argc) {
  char *prev_flag = flag;
  flag = get_next_or_last(argv, argc);
  if (str_compare(flag, prev_flag)) {
    tests(&cmd, .normal = true);
  } else {
    char ***saved_argv = argv;
    int saved_argc = *argc;
    bool first = true;
  second:
    for (;;) {
      prev_flag = flag;
      if (0) {
      } else if (str_compare("verbose", flag)) {
        if (!first) {
          tests(&cmd, .verbose = true);
        }
      } else if (str_compare("short", flag)) {
        if (!first) {
          tests_opt((&cmd), ((Tests_Opts){.short_log = 1}));
        }
      } else {
        usage(.prog_name = prog_name, .test = true);
      }

      flag = get_next_or_last(argv, argc);
      if (prev_flag == flag) {
        if (first) {
          first = false;
          argv = saved_argv;
          *argc = saved_argc;
          goto second;
        }
        break;
      }
    }
  }
}
