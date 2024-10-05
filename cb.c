#include <string.h>
#define CB_IMPLEMENTATION
#include "cb.h"

#define RAYLIBPATH "external/raylib-5.0/src"

char *shift_args(int *argc, char ***argv) {
  char *old_argv = **argv;

  if (*argc > 0) {
    *argc = *argc - 1;
    *argv = *argv + 1;
  } else
    assert(*argc > 0 && "Error: No more arguments left!");

  return old_argv;
}

int main(int argc, char *argv[]) {

  char *prog_name = shift_args(&argc, &argv);
  char *ldd = shift_args(&argc, &argv);

  int dynamic = 0;
  if (strncmp(ldd, "static", 6) == 0) {
    dynamic = 0;
  } else if (strncmp(ldd, "dynamic", 7) == 0) {
    dynamic = 1;
  }

  Cmd cmd = {0};
  cb_cmd_push(&cmd, "rm", "-rf", "build");
  if (!cb_run_sync(&cmd))
    return 1;

  cb_cmd_push(&cmd, "mkdir", "-p", "build");
  if (!cb_run_sync(&cmd))
    return 1;

  if (dynamic) {
    cb_cmd_push(&cmd, "make", "-e", "RAYLIB_LIBTYPE=SHARED", "-C", RAYLIBPATH);
  } else {
    cb_cmd_push(&cmd, "make", "-C", RAYLIBPATH);
  }

  if (!cb_run_sync(&cmd))
    return 1;

  cb_cmd_push(&cmd, "cc");
  INCLUDE(&cmd, "-I", RAYLIBPATH);
  INCLUDE(&cmd, "-I", "src/");
  CFLAGS(&cmd, "-x", "c");
  CFLAGS(&cmd, "-O3", "-pedantic", "-Wall", "-Wextra", "-ggdb");
  cb_cmd_push(&cmd, "-o", "build/first.o", "-c", "tkbc_scripts/first.c");
  if (!cb_run_sync(&cmd))
    return 1;

  cb_cmd_push(&cmd, "cc");
  INCLUDE(&cmd, "-I", RAYLIBPATH);
  INCLUDE(&cmd, "-I", "tkbc_scripts/", "-I", "src/");
  CFLAGS(&cmd, "-x", "c");
  CFLAGS(&cmd, "-O3", "-pedantic", "-Wall", "-Wextra", "-ggdb");
  cb_cmd_push(&cmd, "-o", "build/tkbc", "src/main.c", "src/tkbc.c",
              "src/tkbc-ui.c");

  LDFLAGS(&cmd, "-L", RAYLIBPATH);
  if (dynamic) {
    LDFLAGS(&cmd, "-Wl,-rpath="RAYLIBPATH);
    LDFLAGS(&cmd, "-Wl,-rpath=../"RAYLIBPATH);
    LIBS(&cmd, "-l:libraylib.so", "-lm");
  } else {
    LIBS(&cmd, "-l:libraylib.a", "-lm");
  }

  if (!cb_run_sync(&cmd))
    return 1;

  return 0;
}
