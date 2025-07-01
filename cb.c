#include <string.h>
#define CB_IMPLEMENTATION
#include "cb.h"

#define CC "gcc"
#define RAYLIBPATH "external/raylib-5.5_linux_amd64"
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

void include(Cmd *cmd) { INCLUDE(cmd, "-I", RAYLIBPATH "/include/"); }

void cflags(Cmd *cmd) {
  CFLAGS(cmd, "-x", "c");
  CFLAGS(cmd, "-fPIC", "-O3", "-pedantic", "-Wall", "-Wextra", "-ggdb");
}

void linker(Cmd *cmd, bool link_dynamic) {
  LDFLAGS(cmd, "-L", RAYLIBPATH "/lib/");

  if (link_dynamic) {
    LDFLAGS(cmd, "-Wl,-rpath=" RAYLIBPATH "/lib/");
    LDFLAGS(cmd, "-Wl,-rpath=../" RAYLIBPATH "/lib/");
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

  const char *files[] = {
      "rcore",   "raudio", "rglfw",     "rmodels",
      "rshapes", "rtext",  "rtextures", "utils",
  };
  char buf1[64] = {0};
  char buf2[64] = {0};
  for (size_t i = 0; i < (sizeof files / sizeof files[0]); ++i) {
    cb_cmd_push(cmd, CC);
    cb_cmd_push(cmd, "-DPLATFORM_DESKTOP", "-DGRAPHICS_API_OPENGL_33");
    cb_cmd_push(cmd, "-D_GNU_SOURCE");
    CFLAGS(cmd, "-static", "-fPIC", "-O3", "-ggdb");
    INCLUDE(cmd, "-I", "./external/raylib-5.0/src/external/glfw/include/GLFW/");

    memset(buf1, 0, sizeof(buf1));
    cb_cmd_push(cmd, "-c");
    sprintf(buf1, "%s/%s.c", RAYLIBPATH, files[i]);
    // This is depend on space that is 0 in the buffer.
    cb_cmd_push(cmd, buf1);

    memset(buf2, 0, sizeof(buf2));
    cb_cmd_push(cmd, "-o");
    sprintf(buf2, "%s/%s.o", RAYLIBPATH, files[i]);
    // This is depend on space that is 0 in the buffer.
    cb_cmd_push(cmd, buf2);

    if (!cb_run_sync(cmd))
      exit(EXIT_FAILURE);
  }

  cb_cmd_push(cmd, "ar", "-r", "external/raylib-5.0/src/libraylib.a",
              "external/raylib-5.0/src/*.o");
  if (!cb_run_sync(cmd))
    exit(EXIT_FAILURE);
}

void kite_script(Cmd *cmd) {
  // Kite Script
  cb_cmd_push(cmd, CC);
  include(cmd);
  cflags(cmd);
  cb_cmd_push(cmd, "-o", "build/first.o", "-c", "./tkbc_scripts/first.c");
  if (!cb_run_sync(cmd))
    exit(EXIT_FAILURE);
}

void choreographer_files(Cmd *cmd) {
  cb_cmd_push(cmd, CHOREOGRAPHERPATH "tkbc.c");
  cb_cmd_push(cmd, CHOREOGRAPHERPATH "tkbc-ffmpeg.c");
  cb_cmd_push(cmd, CHOREOGRAPHERPATH "tkbc-input-handler.c");
  cb_cmd_push(cmd, CHOREOGRAPHERPATH "tkbc-script-api.c");
  cb_cmd_push(cmd, CHOREOGRAPHERPATH "tkbc-script-handler.c");
  cb_cmd_push(cmd, CHOREOGRAPHERPATH "tkbc-sound-handler.c");
  cb_cmd_push(cmd, CHOREOGRAPHERPATH "tkbc-team-figures-api.c");
  cb_cmd_push(cmd, CHOREOGRAPHERPATH "tkbc-ui.c");
  cb_cmd_push(cmd, CHOREOGRAPHERPATH "tkbc-keymaps.c");
  cb_cmd_push(cmd, CHOREOGRAPHERPATH "tkbc-parser.c");
  cb_cmd_push(cmd, CHOREOGRAPHERPATH "tkbc-script-converter.c");
}

void choreographer(Cmd *cmd, bool link_dynamic) {
  // Choreographer
  cb_cmd_push(cmd, CC);
  include(cmd);
  cflags(cmd);
  cb_cmd_push(cmd, "-o", "build/tkbc");
  cb_cmd_push(cmd, CHOREOGRAPHERPATH "main.c");
  choreographer_files(cmd);
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
  cb_cmd_push(cmd, "src/global/tkbc-popup.c");
  choreographer_files(cmd);
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
  // raylib(&cmd, linkoption);

  kite_script(&cmd);
  choreographer(&cmd, linkoption);
  client(&cmd, linkoption);

  return 0;
}
