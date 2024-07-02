#ifndef TKBC_FFMPEG_H_
#define TKBC_FFMPEG_H_

#include "tkbc-types.h"

void tkbc_ffmpeg_handler(Env *env);
bool tkbc_ffmpeg_create_proc(Env *env);
bool tkbc_ffmpeg_end(Env *env);
bool tkbc_ffmpeg_wait(pid_t pid);
void tkbc_ffmpeg_write_image(Env *env);

#endif // TKBC_FFMPEG_H_

// ===========================================================================

#ifdef TKBC_FFMPEG_IMPLEMENTATION
#include <errno.h>
#include <stdint.h>

#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#ifndef TKBC_UTILS_IMPLEMENTATION
#define TKBC_UTILS_IMPLEMENTATION
#include <tkbc-utils.h>
#endif // TKBC_UTILS_IMPLEMENTATION

void tkbc_ffmpeg_handler(Env *env) {
  // The handler has to be carefully checked because the same key is used
  // multiple times and that can cause problems, with reinitializing the
  // ffmpeg child process where the old one is still running.
  if ((IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) &&
      IsKeyPressed(KEY_V)) {
    tkbc_ffmpeg_end(env);
  } else if (IsKeyPressed(KEY_V)) {
    tkbc_ffmpeg_create_proc(env);
  }

  tkbc_ffmpeg_write_image(env);
}

bool tkbc_ffmpeg_end(Env *env) {

  SetTraceLogLevel(LOG_ALL);
  int close_status = close(env->pipe);
  if (close_status < 0) {
    fprintf(stderr,
            "ERROR: The ffmpeg_end function could not close the pipe: %d: %s\n",
            env->pipe, strerror(errno));
  }
  env->recording = false;
  bool status = tkbc_ffmpeg_wait(env->pid);
  env->rendering = false;
  return status;
}

bool tkbc_ffmpeg_create_proc(Env *env) {
  SetTraceLogLevel(LOG_INFO);
  env->recording = true;
  env->rendering = true;
  int fildes[2];
  int pipe_status = pipe(fildes);
  if (-1 == pipe_status) {
    fprintf(stderr, "ERROR: Creating the pipe has failed with:%s\n",
            strerror(errno));
  }

  char resolution[32] = {0};
  snprintf(resolution, sizeof(resolution), "%dx%d", GetScreenWidth(),
           GetScreenHeight());
  char fps[32] = {0};
  snprintf(fps, sizeof(fps), "%d", env->fps);

  pid_t pid = fork();
  if (0 > pid) {
    fprintf(stderr, "[ERROR]: The fork was not possible:%s\n", strerror(errno));
    return false;
  }

  if (0 == pid) {
    if (dup2(fildes[0], STDIN_FILENO) < 0) {
      fprintf(stderr, "ERROR: Could not reopen pipe: %s\n", strerror(errno));
      exit(1);
    }

    // Close the write filed so ffmpeg can not write to the pipe back.
    int close_status = close(fildes[1]);
    if (close_status < 0) {
      fprintf(stderr,
              "ERROR: The ffmpeg_end function could not close the pipe field: "
              "%d: %s\n",
              fildes[1], strerror(errno));
    }

    int return_code;
    if (env->sound_file_name == NULL) {
      const char *ffmpeg_cmd[] = {"ffmpeg", "-loglevel", "verbose", "-y",

                                  "-f", "rawvideo", "-pix_fmt", "rgba", "-s",
                                  resolution,
                                  "-r",       fps,

                                  "-i", "pipe:0",

                                  "-c:v", "libx264", "-vb", "2500k", "-c:a",
                                  "aac", "-ab", "200k", "-pix_fmt", "yuv420p",

                                  "video.mp4", NULL};

      tkbc_print_cmd(ffmpeg_cmd);
      return_code = execvp("ffmpeg", (char *const *)ffmpeg_cmd);

    } else {
      const char *ffmpeg_cmd[] = {
          "ffmpeg",    "-loglevel", "verbose",  "-y",

          "-f",        "rawvideo",  "-pix_fmt", "rgba",
          "-s",        resolution,  "-r",       fps,

          "-i",        "pipe:0",    "-i",       env->sound_file_name,

          "-c:v",      "libx264",   "-vb",      "2500k",
          "-c:a",      "aac",       "-ab",      "200k",
          "-pix_fmt",  "yuv420p",

          "video.mp4", NULL};

      tkbc_print_cmd(ffmpeg_cmd);
      return_code = execvp("ffmpeg", (char *const *)ffmpeg_cmd);
    }

    if (return_code < 0) {
      fprintf(stderr, "[ERROR]: The execvp has failed with:%s\n",
              strerror(errno));
      exit(1);
    }
    assert(0 && "UNREACHABLE: Possible bug in kernel or libc!");
  }

  int close_status = close(fildes[0]);
  if (close_status < 0) {
    fprintf(stderr,
            "ERROR: The ffmpeg_end function could not close the pipe: %d: %s\n",
            fildes[0], strerror(errno));
  }
  env->pipe = fildes[1];
  env->pid = pid;
  return true;
}

bool tkbc_ffmpeg_wait(pid_t pid) {

  while (true) {
    int status = 0;
    if (0 > waitpid(pid, &status, 0)) {
      fprintf(stderr, "[ERROR]: Waiting on process %d has failed:%s\n", pid,
              strerror(errno));
      return false;
    }
    if (WIFEXITED(status)) {
      int exit_status = WEXITSTATUS(status);
      if (0 != exit_status) {
        fprintf(stderr, "[ERROR]: Process exited with exit code:%d\n",
                exit_status);
        return false;
      }
      break;
    }
    if (WIFSIGNALED(status)) {
      fprintf(stderr, "[ERROR]: Process was terminated by:%s\n",
              strsignal(WTERMSIG(status)));
      return false;
    }
  }
  return true;
}

void tkbc_ffmpeg_write_image(Env *env) {
  if (env->rendering) {
    Image image = LoadImageFromScreen();
    ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    for (int y = 1; y <= image.height; ++y) {
      int write_status =
          write(env->pipe, (uint32_t *)image.data + image.width * y,
                sizeof(uint32_t) * image.width);
      if (write_status < 0) {
        fprintf(stderr, "ERROR: Could not write to the pipe: %s\n",
                strerror(errno));
      }
    }

    UnloadImage(image); // This is very important otherwise it is memory leak.
  }
}

#endif // TKBC_FFMPEG_IMPLEMENTATION
