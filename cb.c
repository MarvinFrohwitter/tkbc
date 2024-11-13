#include <string.h>
#define CB_IMPLEMENTATION
#include "cb.h"

#define CC "gcc"
#define RAYLIBPATH "external/raylib-5.0/src/"
#define CHOREOGRAPHERPATH "src/choreographer/"

char *shift_args(int *argc, char ***argv) {
  char *old_argv = **argv;

  if (*argc > 0) {
    *argc = *argc - 1;
    *argv = *argv + 1;
  } else
    assert(*argc > 0 && "ERROR: No more arguments left!");

  return old_argv;
}

void include(Cmd *cmd) {
  INCLUDE(cmd, "-I", RAYLIBPATH);
  INCLUDE(cmd, "-I", "src/global/");
  INCLUDE(cmd, "-I", "src/choreographer/");
  INCLUDE(cmd, "-I", "src/network/");
  INCLUDE(cmd, "-I", "tkbc_scripts/");
  INCLUDE(cmd, "-I", "build/");
}

void cflags(Cmd *cmd) {
  CFLAGS(cmd, "-x", "c");
  CFLAGS(cmd, "-O3", "-pedantic", "-Wall", "-Wextra", "-ggdb");
}

void linker(Cmd *cmd, bool link_dynamic) {

  LDFLAGS(cmd, "-L", RAYLIBPATH);
  // LDFLAGS(cmd, "-L", "src/global/");
  // LDFLAGS(cmd, "-L", "src/choreographer/");
  // LDFLAGS(cmd, "-L", "src/network/");
  // LDFLAGS(cmd, "-L", "tkbc_scripts/");
  // LDFLAGS(cmd, "-L", "build/");

  if (link_dynamic) {
    LDFLAGS(cmd, "-Wl,-rpath=" RAYLIBPATH);
    LDFLAGS(cmd, "-Wl,-rpath=../" RAYLIBPATH);
    LIBS(cmd, "-l:libraylib.so", "-lm");
  } else {
    LIBS(cmd, "-l:libraylib.a", "-lm");
  }
}

void pre_build(Cmd *cmd) {
  cb_cmd_push(cmd, "rm", "-rf", "build");
  if (!cb_run_sync(cmd))
    exit(EXIT_FAILURE);

  cb_cmd_push(cmd, "mkdir", "-p", "build");
  if (!cb_run_sync(cmd))
    exit(EXIT_FAILURE);
}

void raylib(Cmd *cmd, bool link_dynamic) {
  // Build raylib
  if (link_dynamic) {
    cb_cmd_push(cmd, "make", "-e", "RAYLIB_LIBTYPE=SHARED", "-C", RAYLIBPATH);
  } else {
    cb_cmd_push(cmd, "make", "-C", RAYLIBPATH);
  }
  if (!cb_run_sync(cmd))
    exit(EXIT_FAILURE);
}

void kite_script(Cmd *cmd) {
  // Kite Script
  cb_cmd_push(cmd, CC);
  include(cmd);
  cflags(cmd);
  cb_cmd_push(cmd, "-o", "build/first.o", "-c", "tkbc_scripts/first.c");
  if (!cb_run_sync(cmd))
    exit(EXIT_FAILURE);
}

void choreographer(Cmd *cmd, bool link_dynamic) {
  // Choreographer
  cb_cmd_push(cmd, CC);
  include(cmd);
  cflags(cmd);
  cb_cmd_push(cmd, "-o", "build/tkbc");
  cb_cmd_push(cmd, CHOREOGRAPHERPATH "main.c");
  cb_cmd_push(cmd, CHOREOGRAPHERPATH "tkbc.c");
  cb_cmd_push(cmd, CHOREOGRAPHERPATH "tkbc-ffmpeg.c");
  cb_cmd_push(cmd, CHOREOGRAPHERPATH "tkbc-input-handler.c");
  cb_cmd_push(cmd, CHOREOGRAPHERPATH "tkbc-script-api.c");
  cb_cmd_push(cmd, CHOREOGRAPHERPATH "tkbc-script-handler.c");
  cb_cmd_push(cmd, CHOREOGRAPHERPATH "tkbc-sound-handler.c");
  cb_cmd_push(cmd, CHOREOGRAPHERPATH "tkbc-team-figures-api.c");
  cb_cmd_push(cmd, CHOREOGRAPHERPATH "tkbc-ui.c");
  linker(cmd, link_dynamic);
  if (!cb_run_sync(cmd))
    exit(EXIT_FAILURE);
}

void server(Cmd *cmd, bool link_dynamic) {
  // Server
  cb_cmd_push(cmd, CC);
  include(cmd);
  cflags(cmd);
  cb_cmd_push(cmd, "-o", "build/server");
  cb_cmd_push(cmd, "src/network/tkbc-server.c");
  cb_cmd_push(cmd, "src/network/tkbc-server-client-handler.c");
  cb_cmd_push(cmd, "src/network/tkbc-network-common.c");
  cb_cmd_push(cmd, "src/choreographer/tkbc.c");
  cb_cmd_push(cmd, "src/choreographer/tkbc-script-handler.c");
  linker(cmd, link_dynamic);
  if (!cb_run_sync(cmd))
    exit(EXIT_FAILURE);
}

void client(Cmd *cmd, bool link_dynamic) {
  // Client
  cb_cmd_push(cmd, CC);
  include(cmd);
  cflags(cmd);
  cb_cmd_push(cmd, "-o", "build/client");
  cb_cmd_push(cmd, "src/network/tkbc-client.c");
  cb_cmd_push(cmd, "src/network/tkbc-network-common.c");
  cb_cmd_push(cmd, "src/choreographer/tkbc.c");
  cb_cmd_push(cmd, "src/choreographer/tkbc-script-handler.c");
  linker(cmd, link_dynamic);
  if (!cb_run_sync(cmd))
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  char *prog_name = shift_args(&argc, &argv);
  char *ldd = shift_args(&argc, &argv);

  int linkoption = 0;
  if (strncmp(ldd, "static", 6) == 0) {
    linkoption = 0;
  } else if (strncmp(ldd, "dynamic", 7) == 0) {
    linkoption = 1;
  }

  Cmd cmd = {0};
  pre_build(&cmd);
  raylib(&cmd, linkoption);
  kite_script(&cmd);
  choreographer(&cmd, linkoption);
  server(&cmd, linkoption);
  client(&cmd, linkoption);

  return 0;
}
