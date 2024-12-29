#include "raylib.h"
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../global/tkbc-utils.h"
#include "tkbc-ffmpeg.h"

/**
 * @brief The function controls the keyboard input of the start and stop video
 * capturing.
 *
 * @param env The global state of the application.
 * @param output_file_path The file path of the output video.
 */
void tkbc_ffmpeg_handler(Env *env, const char *output_file_path) {
  if (IsKeyPressed(KEY_B)) {
    TakeScreenshot("1.png");
  }
  // The handler has to be carefully checked because the same key is used
  // multiple times and that can cause problems, with reinitializing the
  // ffmpeg child process where the old one is still running.
  if ((IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) &&
      IsKeyPressed(KEY_V)) {
    tkbc_ffmpeg_end(env);
  } else if (IsKeyPressed(KEY_V)) {
    if (!env->rendering) {
      tkbc_ffmpeg_create_proc(env, output_file_path);
    }
  }

  tkbc_ffmpeg_write_image(env);
}

/**
 * @brief The function is responsible for closing the recording state of ffmpeg.
 *
 * @param env The global state of the application.
 * @return True if the ffmpeg process has finished successfully, otherwise
 * false.
 */
bool tkbc_ffmpeg_end(Env *env) {

  int close_status = close(env->pipe);
  if (close_status < 0) {
    tkbc_fprintf(stderr, "ERROR",
                 "The ffmpeg_end() function could not close the pipe: %d: %s\n",
                 env->pipe, strerror(errno));
  }
  env->recording = false;
  bool status = tkbc_ffmpeg_wait(env->pid);
  env->rendering = false;
  return status;
}

/**
 * @brief The function can be used to create a ffmpeg process for capturing the
 * current display of the application. The creation will be made with sound if
 * the sound_file_name was set in the env state, else the video capture is
 * without sound.
 *
 * @param env The global state of the application.
 * @param output_file_path The file path of the output video.
 * @return True if the ffmpeg child process creation was successful, otherwise
 * false.
 */
bool tkbc_ffmpeg_create_proc(Env *env, const char *output_file_path) {
  env->recording = true;
  env->rendering = true;
  int fildes[2];
  int pipe_status = pipe(fildes);
  if (-1 == pipe_status) {
    tkbc_fprintf(stderr, "ERROR", "Creating the pipe has failed with:%s\n",
                 strerror(errno));
  }

  char resolution[32] = {0};
  snprintf(resolution, sizeof(resolution), "%zux%zu", env->window_width,
           env->window_height);
  char fps[32] = {0};
  snprintf(fps, sizeof(fps), "%d", env->fps);

  pid_t pid = fork();
  if (0 > pid) {
    tkbc_fprintf(stderr, "ERROR", "The fork was not possible:%s\n",
                 strerror(errno));
    return false;
  }

  if (0 == pid) {
    if (dup2(fildes[0], STDIN_FILENO) < 0) {
      tkbc_fprintf(stderr, "ERROR", "Could not reopen pipe: %s\n",
                   strerror(errno));
      exit(1);
    }

    // Close the write filed so ffmpeg can not write to the pipe back.
    int close_status = close(fildes[1]);
    if (close_status < 0) {
      tkbc_fprintf(stderr, "ERROR",
                   "The ffmpeg_end() function could not close the pipe field: "
                   "%d: %s\n",
                   fildes[1], strerror(errno));
    }

    int return_code;
    if (env->sound_file_name == NULL) {
      const char *ffmpeg_cmd[] = {"ffmpeg",
                                  "-loglevel",
                                  "verbose",
                                  "-y",

                                  "-f",
                                  "rawvideo",
                                  "-pix_fmt",
                                  "rgba",
                                  "-s",
                                  resolution,
                                  "-r",
                                  fps,

                                  "-i",
                                  "pipe:0",

                                  "-c:v",
                                  "libx264",
                                  "-vb",
                                  "2500k",
                                  "-c:a",
                                  "aac",
                                  "-ab",
                                  "200k",
                                  "-pix_fmt",
                                  "yuv420p",

                                  output_file_path,
                                  NULL};

      tkbc_print_cmd(stderr, ffmpeg_cmd);
      return_code = execvp("ffmpeg", (char *const *)ffmpeg_cmd);

    } else {
      const char *ffmpeg_cmd[] = {"ffmpeg",
                                  "-loglevel",
                                  "verbose",
                                  "-y",

                                  "-f",
                                  "rawvideo",
                                  "-pix_fmt",
                                  "rgba",
                                  "-s",
                                  resolution,
                                  "-r",
                                  fps,

                                  "-i",
                                  "pipe:0",
                                  "-i",
                                  env->sound_file_name,

                                  "-c:v",
                                  "libx264",
                                  "-vb",
                                  "2500k",
                                  "-c:a",
                                  "aac",
                                  "-ab",
                                  "200k",
                                  "-pix_fmt",
                                  "yuv420p",

                                  output_file_path,
                                  NULL};

      tkbc_print_cmd(stderr, ffmpeg_cmd);
      return_code = execvp("ffmpeg", (char *const *)ffmpeg_cmd);
    }

    if (return_code < 0) {
      tkbc_fprintf(stderr, "ERROR", "The execvp has failed with:%s\n",
                   strerror(errno));
      exit(1);
    }
    assert(0 && "UNREACHABLE: Possible bug in kernel or libc!");
  }

  int close_status = close(fildes[0]);
  if (close_status < 0) {
    tkbc_fprintf(stderr, "ERROR",
                 "The ffmpeg_end() function could not close the pipe: %d: %s\n",
                 fildes[0], strerror(errno));
  }
  env->pipe = fildes[1];
  env->pid = pid;
  return true;
}

/**
 * @brief The function checks the status of the ffmpeg process. If there is any
 * misbehavior it will be logged to the stderr as well as any occurring error.
 *
 * @param pid The process id of the previously created ffmpeg child process.
 * @return True if the ffmpeg process has finished successfully, otherwise
 * false.
 */
bool tkbc_ffmpeg_wait(pid_t pid) {

  while (true) {
    int status = 0;
    if (0 > waitpid(pid, &status, 0)) {
      tkbc_fprintf(stderr, "ERROR", "Waiting on process %d has failed:%s\n",
                   pid, strerror(errno));
      return false;
    }
    if (WIFEXITED(status)) {
      int exit_status = WEXITSTATUS(status);
      if (0 != exit_status) {
        tkbc_fprintf(stderr, "ERROR", "Process exited with exit code:%d\n",
                     exit_status);
        return false;
      }
      break;
    }
    if (WIFSIGNALED(status)) {
      tkbc_fprintf(stderr, "ERROR", "Process was terminated by:%s\n",
                   strsignal(WTERMSIG(status)));
      return false;
    }
  }
  return true;
}

/**
 * @brief The function can be used to write the current state of screen image
 * data to the ffmpeg process.
 *
 * @param env The global state of the application.
 */
void tkbc_ffmpeg_write_image(Env *env) {
  if (env->rendering) {
    Image image = LoadImageFromScreen();
    ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    for (int y = 1; y <= image.height; ++y) {
      int write_status =
          write(env->pipe, (uint32_t *)image.data + image.width * y,
                sizeof(uint32_t) * image.width);
      if (write_status < 0) {
        tkbc_fprintf(stderr, "ERROR", "Could not write to the pipe: %s\n",
                     strerror(errno));
      }
    }

    UnloadImage(image); // This is very important otherwise it is memory leak.
  }
}
