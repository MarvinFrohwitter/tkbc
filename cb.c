#include <stddef.h>
void raylib(Cmd *cmd) {

  const char *files[] = {
      "rcore",   "raudio", "rglfw",     "rmodels",
      "rshapes", "rtext",  "rtextures", "utils",
  };
  char buf1[64] = {0};
  char buf2[64] = {0};
  for (size_t i = 0; i < (sizeof files / sizeof files[0]); ++i) {
    cb_cmd_push(cmd, "gcc");
    cb_cmd_push(cmd, "-DPLATFORM_DESKTOP", "-DGRAPHICS_API_OPENGL_33");
    cb_cmd_push(cmd, "-D_GNU_SOURCE");
    CFLAGS(cmd, "-static", "-fPIC", "-O3", "-ggdb");
    INCLUDE(cmd, "-I", "./external/raylib-5.0/src/external/glfw/include/GLFW/");

    memset(buf1, 0, sizeof(buf1));
    cb_cmd_push(cmd, "-c");
    sprintf(buf1, "%s/%s.c", "external/raylib-5.5_linux_amd64", files[i]);
    // This is depend on space that is 0 in the buffer.
    cb_cmd_push(cmd, buf1);

    memset(buf2, 0, sizeof(buf2));
    cb_cmd_push(cmd, "-o");
    sprintf(buf2, "%s/%s.o", "external/raylib-5.5_linux_amd64", files[i]);
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
