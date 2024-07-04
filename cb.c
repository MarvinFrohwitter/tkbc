#define CB_IMPLEMENTATION
#include "cb.h"

int main(void) {
  Cmd cmd = {0};
  cb_cmd_push(&cmd, "rm", "-rf", "build");
  if (!cb_run_sync(&cmd))
    return 1;

  cb_cmd_push(&cmd, "mkdir", "-p", "build");
  if (!cb_run_sync(&cmd))
    return 1;

  cb_cmd_push(&cmd, "make", "-C", "./external/raylib-5.0/src");
  if (!cb_run_sync(&cmd))
    return 1;

  cb_cmd_push(&cmd, "cc");
  INCLUDE(&cmd, "-I", "external/raylib-5.0/src");
  INCLUDE(&cmd, "-I", "src/");
  CFLAGS(&cmd, "-x", "c");
  CFLAGS(&cmd, "-O3", "-pedantic", "-Wall", "-Wextra", "-ggdb");
  LIBS(&cmd, "-lraylib", "-lm");
  cb_cmd_push(&cmd, "-o", "build/first.o", "-c", "tkbc_scripts/first.c");
  if (!cb_run_sync(&cmd))
    return 1;

  cb_cmd_push(&cmd, "cc");
  INCLUDE(&cmd, "-I", "external/raylib-5.0/src");
  INCLUDE(&cmd, "-I", "tkbc_scripts/", "-I", "src/");
  CFLAGS(&cmd, "-x", "c");
  CFLAGS(&cmd, "-O3", "-pedantic", "-Wall", "-Wextra", "-ggdb");
  LIBS(&cmd, "-lraylib", "-lm");
  cb_cmd_push(&cmd, "-o", "build/tkbc", "src/main.c", "src/tkbc.c");
  if (!cb_run_sync(&cmd))
    return 1;

  return 0;
}
