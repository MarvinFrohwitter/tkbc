// clang bug with size_t, variadics, optimization and compiler caching
// gcc is broken when using a format string that just includes "%s" longer ones
// are fine like "Hello %s" with NULL.
// printf("%s", NULL);

#include <string.h>
#define CB_IMPLEMENTATION
#include "cb.h"
Cmd cmd = {0};

#define shift(array, size) (assert(0 < (size)), (size)--, *(array)++)
// #define CC "gcc"
#define CC "clang"
#define BUILD_PATH "build/"
#define ASSETS_PATH "assets/"
#define RAYLIB_PATH_LINUX "external/raylib-6.0_linux_amd64/"
#define RAYLIB_PATH_WINDOWS "external/raylib-6.0_win64_mingw-w64/"
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
  bool release;
  bool ndebug;

  bool include_raylib;
  bool tkbc_server;

  bool print_operation_and_description;
  bool short_log;
} Define_Opts;

#define define(cmd, ...) define_opt((cmd), ((Define_Opts){__VA_ARGS__}))
void define_opt(Cmd *cmd, Define_Opts opts) {

  if (opts.release) {
    CFLAGS(cmd, "-DRELEASE");
    CFLAGS(cmd, "-DNDEBUG");
  }
  if (opts.ndebug) {
    CFLAGS(cmd, "-DNDEBUG");
  }

  if (opts.include_raylib) {
    CFLAGS(cmd, "-DINCLUDE_RAYLIB");
  }
  if (opts.tkbc_server) {
    CFLAGS(cmd, "-DTKBC_SERVER");
  }
  if (opts.print_operation_and_description) {
    CFLAGS(cmd, "-DPRINT_OPERATION_AND_DESCRIPTION");
  }
  if (opts.short_log) {
    CFLAGS(cmd, "-DSHORT_LOG");
  }
}

typedef struct {
  bool sanitize;
  bool WINDOWS;
} Cflags_Opts;

#define cflags(cmd, ...) cflags_opt(cmd, ((Cflags_Opts){__VA_ARGS__}))
void cflags_opt(Cmd *cmd, Cflags_Opts opts) {
  CFLAGS(cmd, "-fPIC", "-O0", "-Wall", "-Wextra", "-ggdb", "-std=gnu23");
  if (opts.WINDOWS) {
    CFLAGS(cmd, "-static");
    CFLAGS(cmd, "-mwindows");
  }

  if (opts.sanitize) {
    CFLAGS(cmd, "-fsanitize=address");
    CFLAGS(cmd, "-fsanitize=undefined");
    CFLAGS(cmd, "-fno-sanitize-recover=undefined");
  }
}

typedef struct {
  bool raylib;
  bool LINUX;
  bool WINDOWS;
} Include_Opts;

#define include(cmd, ...) include_opt(cmd, ((Include_Opts){__VA_ARGS__}))
void include_opt(Cmd *cmd, Include_Opts opts) {
  INCLUDE(cmd, "-I", CHOREOGRAPHER_PATH);
  INCLUDE(cmd, "-I", "src/global/");
  INCLUDE(cmd, "-I", "src/network/");
  INCLUDE(cmd, "-I", "tkbc_scripts/");
  INCLUDE(cmd, "-I", BUILD_PATH);
  if (opts.raylib) {
    if (0) {
    } else if (opts.LINUX) {
      INCLUDE(cmd, "-I", RAYLIB_PATH_LINUX "include/");
    } else if (opts.WINDOWS) {
      INCLUDE(cmd, "-I", RAYLIB_PATH_WINDOWS "include/");
    } else {
      exit(EXIT_FAILURE);
    }
  }
}

typedef struct {
  bool math;
  bool raylib;
  bool X11;

  bool LINUX;
  bool WINDOWS;

  bool network;
} Libs_Opts;

#define libs(cmd, ...) libs_opt((cmd), ((Libs_Opts){__VA_ARGS__}))
void libs_opt(Cmd *cmd, Libs_Opts opts) {
  if (opts.math) {
    LIBS(cmd, "-lm");
  }
  if (opts.raylib) {
    if (0) {
    } else if (opts.LINUX) {
      LDFLAGS(cmd, "-L", RAYLIB_PATH_LINUX "lib/");
      LIBS(cmd, "-l:libraylib.a");
      if (opts.X11) {
        LIBS(cmd, "-lX11");
      }
    } else if (opts.WINDOWS) {
      LDFLAGS(cmd, "-L", RAYLIB_PATH_WINDOWS "lib/");
      // Linking order is important because of collisions.
      LIBS(cmd, "-l:libraylib.a");
      LIBS(cmd, "-lwinmm");
      LIBS(cmd, "-lgdi32");

      if (opts.network) {
        LIBS(cmd, "-lws2_32");
      }
    } else {
      exit(EXIT_FAILURE);
    }
  }
}

void files_for_test(Cmd *cmd) {
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-script-api.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-team-figures-api.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-script-handler.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-keymaps.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-asset-handler.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-parser.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-script-converter.c");
}

void files_for_choreographer(Cmd *cmd) {
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-script-api.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-team-figures-api.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-asset-handler.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-keymaps.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-script-handler.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-parser.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-script-converter.c");
}

void files_for_tkbc(Cmd *cmd) {
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "main.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-ui.c");

  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-sound-handler.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-ffmpeg.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-input-handler.c");

  files_for_choreographer(cmd);
}

void files_for_client(Cmd *cmd) {
  cb_cmd_push(cmd, NETWORK_PATH "tkbc-client.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-ui.c");

  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-sound-handler.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-ffmpeg.c");
  cb_cmd_push(cmd, CHOREOGRAPHER_PATH "tkbc-input-handler.c");

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

void files_for_server(Cmd *cmd) {
  cb_cmd_push(cmd, NETWORK_PATH "poll-server.c");

  files_for_choreographer(cmd);

  cb_cmd_push(cmd, NETWORK_PATH "tkbc-network-common.c");

  cb_cmd_push(cmd, MESSAGES_PATH "tkbc-messages-hello-verification.c");
  cb_cmd_push(cmd, MESSAGES_PATH "tkbc-messages-send-texture.c");
  cb_cmd_push(cmd, MESSAGES_PATH "tkbc-messages-get-texture.c");

  cb_cmd_push(cmd, MESSAGES_PATH "tkbc-messages-get-texture-id.c");
  cb_cmd_push(cmd, MESSAGES_PATH "tkbc-messages-script-scrub.c");
  cb_cmd_push(cmd, MESSAGES_PATH "tkbc-messages-script-next.c");
  cb_cmd_push(cmd, MESSAGES_PATH "tkbc-messages-script.c");
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void clean(Cmd *cmd) {
  cb_cmd_push(cmd, "rm", "-rf", "build");
  if (!cb_run_sync(cmd))
    exit(EXIT_FAILURE);
}

void assets(Cmd *cmd) {
  cb_cmd_push(cmd, CC);
  cflags(cmd, .sanitize = true);

  cb_cmd_push(cmd, "-o", BUILD_PATH "assets2h");
  cb_cmd_push(cmd, ASSETS_PATH "assets2h.c");
  libs(cmd, .math = true);

  if (!cb_run_sync(cmd))
    exit(EXIT_FAILURE);

  cb_cmd_push(cmd, "./" BUILD_PATH "assets2h", ASSETS_PATH);

  if (!cb_run_sync(cmd))
    exit(EXIT_FAILURE);
}

typedef struct {
  bool LINUX;
  bool WINDOWS;
} OS_Opts;

#define first_o(cmd, ...) first_o_opt(cmd, ((OS_Opts){__VA_ARGS__}))
void first_o_opt(Cmd *cmd, OS_Opts os) {
  cb_cmd_push(cmd, CC);
  if (0) {
  } else if (os.LINUX) {
    include(cmd, .raylib = true, .LINUX = true);
  } else if (os.WINDOWS) {
    include(cmd, .raylib = true, .WINDOWS = true);
  } else {
    exit(EXIT_FAILURE);
  }

  cflags(cmd); // TODO Think about windows cflags(cmd, .WINDOWS = true);
  cb_cmd_push(cmd, "-c");
  cb_cmd_push(cmd, "-o", BUILD_PATH "first.o");
  cb_cmd_push(cmd, SCRIPT_PATH "first.c");

  if (!cb_run_sync(cmd))
    exit(EXIT_FAILURE);
}

#define tkbc(cmd, ...) tkbc_opt(cmd, ((OS_Opts){__VA_ARGS__}))
void tkbc_opt(Cmd *cmd, OS_Opts os) {

  if (0) {
  } else if (os.LINUX) {
    first_o(cmd, .LINUX = true);
    cb_cmd_push(cmd, CC);
    include(cmd, .raylib = true, .LINUX = true);
    cflags(cmd);
    define(cmd, .include_raylib = true);
    cb_cmd_push(cmd, "-o", BUILD_PATH "tkbc");
  } else if (os.WINDOWS) {
    first_o(cmd, .WINDOWS = true);
    cb_cmd_push(cmd, "x86_64-w64-mingw32-gcc");
    include(cmd, .raylib = true, .WINDOWS = true);
    define(cmd, .include_raylib = true, .release = true, .ndebug = false);
    cflags(cmd, .WINDOWS = true);
    cb_cmd_push(cmd, "-o", BUILD_PATH "tkbc-win64");
  } else {
    exit(EXIT_FAILURE);
  }

  files_for_tkbc(cmd);

  if (0) {
  } else if (os.LINUX) {
    libs(cmd, .raylib = true, .X11 = true, .math = true, .LINUX = true);
  } else if (os.WINDOWS) {
    libs(cmd, .raylib = true, .WINDOWS = true);
  } else {
    exit(EXIT_FAILURE);
  }

  if (!cb_run_sync(cmd))
    exit(EXIT_FAILURE);
}

#define client(cmd, ...) client_opt(cmd, ((OS_Opts){__VA_ARGS__}))
void client_opt(Cmd *cmd, OS_Opts os) {
  if (0) {
  } else if (os.LINUX) {
    first_o(cmd, .LINUX = true);
    cb_cmd_push(cmd, CC);
    include(cmd, .raylib = true, .LINUX = true);
    cflags(cmd);
    define(cmd, .include_raylib = true);
    cb_cmd_push(cmd, "-o", BUILD_PATH "client");
  } else if (os.WINDOWS) {
    first_o(cmd, .WINDOWS = true);
    cb_cmd_push(cmd, "x86_64-w64-mingw32-gcc");
    include(cmd, .raylib = true, .WINDOWS = true);
    cflags(cmd, .WINDOWS = true);
    define(cmd, .include_raylib = true, .release = true, .ndebug = false);
    cb_cmd_push(cmd, "-o", BUILD_PATH "client-win64");
  } else {
    exit(EXIT_FAILURE);
  }

  files_for_client(cmd);

  if (0) {
  } else if (os.LINUX) {
    libs(cmd, .raylib = true, .X11 = true, .math = true, .LINUX = true);
  } else if (os.WINDOWS) {
    libs(cmd, .raylib = true, .WINDOWS = true, .network = true);
  } else {
    exit(EXIT_FAILURE);
  }

  if (!cb_run_sync(cmd))
    exit(EXIT_FAILURE);
}

#define server(cmd, ...) server_opt(cmd, ((OS_Opts){__VA_ARGS__}))
void server_opt(Cmd *cmd, OS_Opts os) {
  if (0) {
  } else if (os.LINUX) {
    cb_cmd_push(cmd, CC);
    include(cmd, .raylib = true, .LINUX = true);
    cflags(cmd);
    define(cmd, .include_raylib = true, .tkbc_server = true);
    cb_cmd_push(cmd, "-o", BUILD_PATH "server");
  } else if (os.WINDOWS) {
    cb_cmd_push(cmd, "x86_64-w64-mingw32-gcc");
    include(cmd, .raylib = true, .WINDOWS = true);
    cflags(cmd, .WINDOWS = true);
    define(cmd, .include_raylib = true, .tkbc_server = true, .release = true);
    cb_cmd_push(cmd, "-o", BUILD_PATH "server-win64");
  } else {
    exit(EXIT_FAILURE);
  }

  files_for_server(cmd);

  if (0) {
  } else if (os.LINUX) {
    libs(cmd, .raylib = true, .X11 = true, .math = true, .LINUX = true);
  } else if (os.WINDOWS) {
    libs(cmd, .raylib = true, .WINDOWS = true, .network = true);
  } else {
    exit(EXIT_FAILURE);
  }

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
  include(cmd, .raylib = true, .LINUX = true);
  cflags(cmd, .sanitize = true);
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
  libs(cmd, .raylib = true, .X11 = true, .math = true, .LINUX = true);

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
  bool assets;

  bool tkbc;
  bool client;
  bool server;
} Usage_Opts;

#define FLAG_HELP "help"
#define FLAG_BUILD_DIR "build-dir"
#define FLAG_CLEAN "clean"
#define FLAG_FIRST_O "first.o"
#define FLAG_TEST "test"
#define FLAG_ASSETS "assets"
#define FLAG_TEST_VERBOSE "verbose"
#define FLAG_TEST_SHORT "short"
#define FLAG_TKBC "tkbc"
#define FLAG_CLIENT "client"
#define FLAG_SERVER "server"
#define FLAG_LINUX "linux"
#define FLAG_WINDOWS "windows"

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
    fprintf(stderr, "       <%s> <%s> [%s|%s]\n", opts.prog_name, FLAG_TEST,
            FLAG_TEST_VERBOSE, FLAG_TEST_SHORT);
  }
  if (opts.assets || opts.all) {
    fprintf(stderr, "       <%s> <%s>\n", opts.prog_name, FLAG_ASSETS);
  }
  if (opts.tkbc || opts.all) {
    fprintf(stderr, "       <%s> <%s>\n", opts.prog_name, FLAG_TKBC);
  }
  if (opts.client || opts.all) {
    fprintf(stderr, "       <%s> <%s>\n", opts.prog_name, FLAG_CLIENT);
  }
  if (opts.server || opts.all) {
    fprintf(stderr, "       <%s> <%s>\n", opts.prog_name, FLAG_SERVER);
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
  } else if (str_compare(FLAG_ASSETS, flag)) {
    make_build_dir(&cmd);
    void flag_assets(char *flag, char ***argv, int *argc);
    flag_assets(flag, &argv, &argc);
  } else if (str_compare(FLAG_TKBC, flag)) {
    make_build_dir(&cmd);
    void flag_tkbc(char *flag, char ***argv, int *argc);
    flag_tkbc(flag, &argv, &argc);
  } else if (str_compare(FLAG_CLIENT, flag)) {
    make_build_dir(&cmd);
    void flag_client(char *flag, char ***argv, int *argc);
    flag_client(flag, &argv, &argc);
  } else if (str_compare(FLAG_SERVER, flag)) {
    make_build_dir(&cmd);
    void flag_server(char *flag, char ***argv, int *argc);
    flag_server(flag, &argv, &argc);
  } else {
    usage(.prog_name = prog_name, .all = true);
  }

  return 0;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

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
  char *prev_flag = flag;
  flag = get_next_or_last(argv, argc);
  if (str_compare(flag, prev_flag)) {
    // defaults to linux when no other argument is specified.
    first_o(&cmd, .LINUX = true);
  } else {
    char ***saved_argv = argv;
    int saved_argc = *argc;
    bool first = true;
  second:
    for (;;) {
      prev_flag = flag;
      if (0) {
      } else if (str_compare(FLAG_LINUX, flag)) {
        if (!first) {
          first_o(&cmd, .LINUX = true);
        }
      } else if (str_compare(FLAG_WINDOWS, flag)) {
        if (!first) {
          first_o(&cmd, .WINDOWS = true);
        }
      } else {
        usage(.prog_name = prog_name, .first_o = true);
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

void flag_assets(char *flag, char ***argv, int *argc) {
  assets(&cmd);
  char *prev_flag = flag;
  flag = get_next_or_last(argv, argc);
  if (prev_flag != flag) {
    usage(.prog_name = prog_name, .assets = true);
  }
}

void flag_tkbc(char *flag, char ***argv, int *argc) {
  char *prev_flag = flag;
  flag = get_next_or_last(argv, argc);
  if (str_compare(flag, prev_flag)) {
    // defaults to linux when no other argument is specified.
    tkbc(&cmd, .LINUX = true);
  } else {
    char ***saved_argv = argv;
    int saved_argc = *argc;
    bool first = true;
  second:
    for (;;) {
      prev_flag = flag;
      if (0) {
      } else if (str_compare(FLAG_LINUX, flag)) {
        if (!first) {
          tkbc(&cmd, .LINUX = true);
        }
      } else if (str_compare(FLAG_WINDOWS, flag)) {
        if (!first) {
          tkbc(&cmd, .WINDOWS = true);
        }
      } else {
        usage(.prog_name = prog_name, .tkbc = true);
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

void flag_client(char *flag, char ***argv, int *argc) {
  char *prev_flag = flag;
  flag = get_next_or_last(argv, argc);
  if (str_compare(flag, prev_flag)) {
    // defaults to linux when no other argument is specified.
    client(&cmd, .LINUX = true);
  } else {
    char ***saved_argv = argv;
    int saved_argc = *argc;
    bool first = true;
  second:
    for (;;) {
      prev_flag = flag;
      if (0) {
      } else if (str_compare(FLAG_LINUX, flag)) {
        if (!first) {
          client(&cmd, .LINUX = true);
        }
      } else if (str_compare(FLAG_WINDOWS, flag)) {
        if (!first) {
          client(&cmd, .WINDOWS = true);
        }
      } else {
        usage(.prog_name = prog_name, .client = true);
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

void flag_server(char *flag, char ***argv, int *argc) {
  char *prev_flag = flag;
  flag = get_next_or_last(argv, argc);
  if (str_compare(flag, prev_flag)) {
    // defaults to linux when no other argument is specified.
    server(&cmd, .LINUX = true);
  } else {
    char ***saved_argv = argv;
    int saved_argc = *argc;
    bool first = true;
  second:
    for (;;) {
      prev_flag = flag;
      if (0) {
      } else if (str_compare(FLAG_LINUX, flag)) {
        if (!first) {
          server(&cmd, .LINUX = true);
        }
      } else if (str_compare(FLAG_WINDOWS, flag)) {
        if (!first) {
          server(&cmd, .WINDOWS = true);
        }
      } else {
        usage(.prog_name = prog_name, .server = true);
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
