#include <string.h>
#define CB_IMPLEMENTATION
#include "cb.h"
Cmd cmd = {0};

#define shift(array, size) (assert(0 < (size)), (size)--, *(array)++)
#define CC "gcc"
#define BUILD_PATH "build/"
#define RAYLIB_PATH "external/raylib-6.0_linux_amd64/"
#define TESTS_PATH "src/tests/"
#define SCRIPT_PATH "tkbc_scripts/"
#define CHOREOGRAPHER_PATH "src/choreographer/"
#define NETWORK_PATH "src/network/"
#define GLOBAL_PATH "src/global/"
#define MESSAGES_PATH "src/network/messages/"

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
  bool raylib;
  bool X11;
} Libs_Opts;

#define libs(cmd, ...) libs_opt((cmd), ((Libs_Opts){__VA_ARGS__}))
void libs_opt(Cmd *cmd, Libs_Opts opts) {
  if (opts.math) {
    INCLUDE(cmd, "-lm");
  }
  if (opts.raylib) {
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

void files_for_choreographer(Cmd *cmd) {
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-script-api.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-team-figures-api.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-asset-handler.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-keymaps.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-sound-handler.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-ui.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-ffmpeg.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-input-handler.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-script-handler.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-parser.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-script-converter.c");
}

void files_for_client(Cmd *cmd) {
  files_for_choreographer(cmd);

  cb_cmd_push(cmd, GLOBAL_PATH "tkbc-popup.c");
  cb_cmd_push(cmd, NETWORK_PATH "tkbc-network-common.c");

  cb_cmd_push(cmd, MESSAGES_PATH "tkbc-messages-hello-verification.c");
  cb_cmd_push(cmd, MESSAGES_PATH "tkbc-messages-send-texture.c");
  cb_cmd_push(cmd, MESSAGES_PATH "tkbc-messages-get-texture.c");
  cb_cmd_push(cmd, MESSAGES_PATH "tkbc-messages-send-texture-id.c");
  cb_cmd_push(cmd, MESSAGES_PATH "tkbc-messages-script-meta-data.c");
  cb_cmd_push(cmd, MESSAGES_PATH "tkbc-messages-single-kite-add.c");
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void clean(Cmd *cmd) {
  cb_cmd_push(cmd, "rm", "-rf", "build");
  if (!cb_run_sync(cmd))
    exit(EXIT_FAILURE);
}

void first_o(Cmd *cmd) {
  cb_cmd_push(cmd, CC);
  include(cmd, false);
  cflags(cmd, false);
  cb_cmd_push(cmd, "-c");
  cb_cmd_push(cmd, "-o", BUILD_PATH "first.o");
  cb_cmd_push(cmd, SCRIPT_PATH "first.c");

  if (!cb_run_sync(cmd))
    exit(EXIT_FAILURE);
}

void tkbc(Cmd *cmd) {
  first_o(cmd);

  cb_cmd_push(cmd, CC);
  include(cmd, true);
  cflags(cmd, false);
  define(cmd, .include_raylib = true);

  cb_cmd_push(cmd, "-o", BUILD_PATH "tkbc");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "main.c");
  files_for_choreographer(cmd);
  libs(cmd, .raylib = true, .X11 = true, .math = true);

  if (!cb_run_sync(cmd))
    exit(EXIT_FAILURE);
}

void client(Cmd *cmd) {
  first_o(cmd);
  cb_cmd_push(cmd, CC);
  include(cmd, true);
  cflags(cmd, false);
  define(cmd, .include_raylib = true);

  cb_cmd_push(cmd, "-o", BUILD_PATH "client");
  cb_cmd_push(cmd, NETWORK_PATH "tkbc-client.c");
  files_for_client(cmd);
  libs(cmd, .raylib = true, .X11 = true, .math = true);

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
  libs(cmd, .raylib = true, .X11 = true, .math = true);

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
  bool build_dir;
  bool clean;
  bool first_o;
  bool test;

  bool tkbc;
  bool client;
} Usage_Opts;

#define FLAG_HELP "help"
#define FLAG_BUILD_DIR "build-dir"
#define FLAG_CLEAN "clean"
#define FLAG_FIRST_O "first.0"
#define FLAG_TEST "test"
#define FLAG_TEST_VERBOSE "verbose"
#define FLAG_TEST_SHORT "short"
#define FLAG_TKBC "tkbc"
#define FLAG_CLIENT "client"

#define usage(...) usage_opt(((Usage_Opts){__VA_ARGS__}))
void usage_opt(Usage_Opts opts) {
  fprintf(stderr, "Usage:\n");
  if (opts.build_dir || opts.all) {
    fprintf(stderr, "       <%s> <%s>\n", opts.prog_name, FLAG_BUILD_DIR);
  }
  if (opts.clean || opts.all) {
    fprintf(stderr, "       <%s> <%s>\n", opts.prog_name, FLAG_CLEAN);
  }
  if (opts.first_o || opts.all) {
    fprintf(stderr, "       <%s> <%s>\n", opts.prog_name, FLAG_FIRST_O);
  }
  if (opts.test || opts.all) {
    fprintf(stderr, "       <%s> <%s> [%s, %s]\n", opts.prog_name, FLAG_TEST,
            FLAG_TEST_VERBOSE, FLAG_TEST_SHORT);
  }
  if (opts.tkbc || opts.all) {
    fprintf(stderr, "       <%s> <%s>\n", opts.prog_name, FLAG_TKBC);
  }
  if (opts.client || opts.all) {
    fprintf(stderr, "       <%s> <%s>\n", opts.prog_name, FLAG_CLIENT);
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
  } else if (str_compare(FLAG_HELP, flag)) {
    usage(.prog_name = prog_name, .all = true);
  } else if (str_compare(FLAG_BUILD_DIR, flag)) {
    void flag_build(char *flag, char ***argv, int *argc);
    flag_build(flag, &argv, &argc);
  } else if (str_compare(FLAG_CLEAN, flag)) {
    void flag_clean(char *flag, char ***argv, int *argc);
    flag_clean(flag, &argv, &argc);
  } else if (str_compare(FLAG_FIRST_O, flag)) {
    make_build_dir(&cmd);
    void flag_first_o(char *flag, char ***argv, int *argc);
    flag_first_o(flag, &argv, &argc);
  } else if (str_compare(FLAG_TEST, flag)) {
    make_build_dir(&cmd);
    void flag_test(char *flag, char ***argv, int *argc);
    flag_test(flag, &argv, &argc);
  } else if (str_compare(FLAG_TKBC, flag)) {
    make_build_dir(&cmd);
    void flag_tkbc(char *flag, char ***argv, int *argc);
    flag_tkbc(flag, &argv, &argc);
  } else if (str_compare(FLAG_CLIENT, flag)) {
    make_build_dir(&cmd);
    void flag_client(char *flag, char ***argv, int *argc);
    flag_client(flag, &argv, &argc);
  } else {
    usage(.prog_name = prog_name, .all = true);
  }

  return 0;
}

void flag_build(char *flag, char ***argv, int *argc) {
  make_build_dir(&cmd);
  char *prev_flag = flag;
  flag = get_next_or_last(argv, argc);
  if (prev_flag != flag) {
    usage(.prog_name = prog_name, .build_dir = true);
  }
}

void flag_clean(char *flag, char ***argv, int *argc) {
  clean(&cmd);
  char *prev_flag = flag;
  flag = get_next_or_last(argv, argc);
  if (prev_flag != flag) {
    usage(.prog_name = prog_name, .clean = true);
  }
}

void flag_first_o(char *flag, char ***argv, int *argc) {
  first_o(&cmd);
  char *prev_flag = flag;
  flag = get_next_or_last(argv, argc);
  if (prev_flag != flag) {
    usage(.prog_name = prog_name, .first_o = true);
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
      } else if (str_compare(FLAG_TEST_VERBOSE, flag)) {
        if (!first) {
          tests(&cmd, .verbose = true);
        }
      } else if (str_compare(FLAG_TEST_SHORT, flag)) {
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

void flag_tkbc(char *flag, char ***argv, int *argc) {
  tkbc(&cmd);
  char *prev_flag = flag;
  flag = get_next_or_last(argv, argc);
  if (prev_flag != flag) {
    usage(.prog_name = prog_name, .tkbc = true);
  }
}

void flag_client(char *flag, char ***argv, int *argc) {
  client(&cmd);
  char *prev_flag = flag;
  flag = get_next_or_last(argv, argc);
  if (prev_flag != flag) {
    usage(.prog_name = prog_name, .client = true);
  }
}
