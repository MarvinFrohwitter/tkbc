#include "raylib.h"
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WINUSER_
#define _WINGDI_
#define _IMM_
#define _WINCON_
#include <windows.h>

struct Process {
  HANDLE pipe; // The pipe the ffmpeg process receives data through.
  HANDLE pid;  // The process number the ffmpeg child process gets.
};

#else
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

struct Process {
  int pipe;  // The pipe the ffmpeg process receives data through.
  pid_t pid; // The process number the ffmpeg child process gets.
};
#endif // _WIN32

//////////////////////////////////////////////////////////////////////////////
// The common part.
//////////////////////////////////////////////////////////////////////////////

#include "../global/tkbc-utils.h"
#include "tkbc-ffmpeg.h"
#include "tkbc-keymaps.h"

/**
 * @brief The function controls the keyboard input of the start and stop video
 * capturing.
 *
 * @param env The global state of the application.
 * @param output_file_path The file path of the output video.
 */
void tkbc_ffmpeg_handler(Env *env, const char *output_file_path) {
  // KEY_B
  if (IsKeyPressed(tkbc_hash_to_key(*env->keymaps, 1027))) {
    TakeScreenshot("1.png");
  }
  // The handler has to be carefully checked because the same key is used
  // multiple times and that can cause problems, with reinitializing the
  // ffmpeg child process where the old one is still running.
  KeyMap keymap = tkbc_hash_to_keymap(*env->keymaps, 1009);
  // KEY_V && KEY_LEFT_SHIFT && KEY_RIGHT_SHIFT
  if (IsKeyPressed(keymap.key) &&
      (IsKeyDown(keymap.mod_key) || IsKeyDown(keymap.mod_co_key))) {
    tkbc_ffmpeg_end(env, false);
  } else if (IsKeyPressed(tkbc_hash_to_key(*env->keymaps, 1008))) {
    if (!env->rendering) {
      if (!tkbc_ffmpeg_create_proc(env, output_file_path)) {
        env->recording = false;
        env->rendering = false;
      }
    }
  }

  if (env->rendering) {
    int err = tkbc_ffmpeg_write_image(env);
    if (err == -1) {
      tkbc_ffmpeg_end(env, true);
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
// The implementation for Windows.
//////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
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
  // _WIN32
  env->ffmpeg = calloc(1, sizeof(Process));
  if (env->ffmpeg == NULL) {
    tkbc_fprintf(stderr, "ERROR", "No more memory can be allocated.\n");
    return NULL;
  }
  env->recording = true;
  env->rendering = true;

  HANDLE wpipe;
  HANDLE rpipe;

  SECURITY_ATTRIBUTES security_attributes = {0};
  security_attributes.bInheritHandle = TRUE;
  security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);

  if (!CreatePipe(&rpipe, &wpipe, &security_attributes, 0)) {
    tkbc_fprintf(stderr, "ERROR", "Creating the pipe has failed with:%d\n",
                 GetLastError());
    return false;
  }

  if (!SetHandleInformation(wpipe, HANDLE_FLAG_INHERIT, 0)) {
    tkbc_fprintf(stderr, "ERROR",
                 "The write pipe could not be set to HANDLE_FLAG_INHERIT:%d\n",
                 GetLastError());
    return false;
  }

  STARTUPINFO startup_info;
  ZeroMemory(&startup_info, sizeof(startup_info));
  startup_info.cb = sizeof(STARTUPINFO);
  startup_info.dwFlags |= STARTF_USESTDHANDLES;
  startup_info.hStdInput = rpipe;
  startup_info.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
  if (startup_info.hStdOutput == INVALID_HANDLE_VALUE) {
    tkbc_fprintf(stderr, "ERROR",
                 "Could not get STD_OUTPUT_HANDLE for ffmpeg: %d\n",
                 GetLastError());
    return false;
  }
  startup_info.hStdError = GetStdHandle(STD_ERROR_HANDLE);
  if (startup_info.hStdError == INVALID_HANDLE_VALUE) {
    tkbc_fprintf(stderr, "ERROR",
                 "Could not get STD_ERROR_HANDLE for ffmpeg: %d\n",
                 GetLastError());
    return false;
  }

  // Information injection to the cmd.
  // ---------------------
  char resolution[32] = {0};
  snprintf(resolution, sizeof(resolution), "%zux%zu", env->window_width,
           env->window_height);
  char fps[32] = {0};
  snprintf(fps, sizeof(fps), "%d", env->fps);
  // ---------------------

  PROCESS_INFORMATION process_information;
  ZeroMemory(&process_information, sizeof(PROCESS_INFORMATION));
  if (env->sound_file_name == NULL) {
    char ffmpeg_cmd[512] = {0};
    snprintf(ffmpeg_cmd, sizeof(ffmpeg_cmd),
             "ffmpeg -loglevel verbose -y -f rawvideo -pix_fmt rgba -s %s -r "
             "%s -i pipe:0 -c:v libx264 -vb  2500k -c:a aac -ab 200k -pix_fmt "
             "yuv420p %s",
             resolution, fps, output_file_path);

    tkbc_fprintf(stderr, "INFO", "%s %s\n", "[CMD]", ffmpeg_cmd);
    if (!CreateProcess(NULL, ffmpeg_cmd, NULL, NULL, TRUE, 0, NULL, NULL,
                       &startup_info, &process_information)) {
      tkbc_fprintf(stderr, "ERROR",
                   "Creation of the child process was not possible:%d\n",
                   GetLastError());
      CloseHandle(wpipe);
      CloseHandle(rpipe);
      return false;
    }

  } else {
    char ffmpeg_cmd[512] = {0};
    snprintf(ffmpeg_cmd, sizeof(ffmpeg_cmd),
             "ffmpeg -loglevel verbose -y -f rawvideo -pix_fmt rgba -s %s -r "
             "%s -i pipe:0 -i %s -c:v libx264 -vb 2500k -c:a aac -ab 200k "
             "-pix_fmt yuv420p %s",
             resolution, env->sound_file_name, fps, output_file_path);

    tkbc_fprintf(stderr, "INFO", "%s %s\n", "[CMD]", ffmpeg_cmd);
    if (!CreateProcess(NULL, ffmpeg_cmd, NULL, NULL, TRUE, 0, NULL, NULL,
                       &startup_info, &process_information)) {
      tkbc_fprintf(stderr, "ERROR",
                   "Creation of the child process was not possible:%d\n",
                   GetLastError());
      CloseHandle(wpipe);
      CloseHandle(rpipe);
      return false;
    }
  }

  CloseHandle(rpipe);
  CloseHandle(process_information.hThread);

  env->ffmpeg->pipe = wpipe;
  env->ffmpeg->pid = process_information.hProcess;
  return true;
}

/**
 * @brief The function is responsible for closing the recording state of ffmpeg.
 *
 * @param env The global state of the application.
 * @param is_kill_foreced Determines if the process is fore killed.
 * @return True if the ffmpeg process has finished successfully, otherwise
 * false.
 */
bool tkbc_ffmpeg_end(Env *env, bool is_kill_foreced) {
  BOOL close_status = CloseHandle(env->ffmpeg->pipe);
  if (close_status == 0) {
    tkbc_fprintf(stderr, "ERROR",
                 "The ffmpeg_end() function could not close the pipe: %d: "
                 "Error code: %d\n",
                 env->ffmpeg->pipe, GetLastError());
  }
  FlushFileBuffers(env->ffmpeg->pipe);

  env->recording = false;
  bool status = tkbc_ffmpeg_wait(*env->ffmpeg, is_kill_foreced);
  env->rendering = false;
  free(env->ffmpeg);
  return status;
}

/**
 * @brief The function checks the status of the ffmpeg process. If there is any
 * misbehavior it will be logged to the stderr as well as any occurring error.
 *
 * @param process The process that contains the pid of the previously created
 * ffmpeg child process.
 * @param is_kill_foreced Determines if the process is fore killed.
 * @return True if the ffmpeg process has finished successfully, otherwise
 * false.
 */
bool tkbc_ffmpeg_wait(Process process, bool is_kill_foreced) {
  if (is_kill_foreced) {
    TerminateProcess(process.pid, 1);
  }

  bool ok = true;
  if (WaitForSingleObject(process.pid, INFINITE) == WAIT_FAILED) {
    tkbc_fprintf(stderr, "ERROR",
                 "Waiting on process %d has failed: Error code:%d\n",
                 process.pid, GetLastError());
    check_return(false);
  }

  DWORD status;
  if (GetExitCodeProcess(process.pid, &status) == 0) {
    tkbc_fprintf(stderr, "ERROR",
                 "Could not get exit code for %d! Error code:%d\n", process.pid,
                 GetLastError());
    check_return(false);
  }

  if (status != 0) {
    tkbc_fprintf(stderr, "ERROR",
                 "The exit code of the ffmpeg process %d was: %d!\n",
                 process.pid, status);
    check_return(false);
  }

check:
  CloseHandle(process.pid);
  return ok;
}

/**
 * @brief The function can be used to write the current state of screen image
 * data to the ffmpeg process.
 *
 * @param env The global state of the application.
 * @return 1 If no image was send to ffmpeg. 0 if the sending was successful,
 * otherwise if an error occurred while sending the image -1 is returned.
 */
int tkbc_ffmpeg_write_image(Env *env) {
  if (env->rendering) {
    bool ok = 0;
    Image image = LoadImageFromScreen();
    ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    for (int y = 1; y <= image.height; ++y) {
      LPDWORD amount = NULL;
      // ERROR_IO_PENDING is not handled.
      int write_status =
          WriteFile(env->ffmpeg->pipe, (uint32_t *)image.data + image.width * y,
                    sizeof(uint32_t) * image.width, amount, NULL);

      if (!write_status) {
        DWORD err = GetLastError();
        if (err == ERROR_IO_PENDING) {
          tkbc_fprintf(stderr, "ERROR",
                       "Process: ffmpeg: pending write! Amount written: %d\n",
                       *amount);
          continue;
        }
        tkbc_fprintf(stderr, "ERROR",
                     "Could not write to the pipe: Error code: %d\n", err);

        check_return(-1);
      }
    }

  check:
    UnloadImage(image); // This is very important otherwise it is memory leak.
    return ok;
  }
  return 1;
}

#else
//////////////////////////////////////////////////////////////////////////////
// The implementation for Linux.
//////////////////////////////////////////////////////////////////////////////
/**
 * @brief The function is responsible for closing the recording state of ffmpeg.
 *
 * @param env The global state of the application.
 * @param is_kill_foreced Determines if the process is fore killed.
 * @return True if the ffmpeg process has finished successfully, otherwise
 * false.
 */
bool tkbc_ffmpeg_end(Env *env, bool is_kill_foreced) {

  int close_status = close(env->ffmpeg->pipe);
  if (close_status < 0) {
    tkbc_fprintf(stderr, "ERROR",
                 "The ffmpeg_end() function could not close the pipe: %d: %s\n",
                 env->ffmpeg->pipe, strerror(errno));
  }
  env->recording = false;
  bool status = tkbc_ffmpeg_wait(*env->ffmpeg, is_kill_foreced);
  env->rendering = false;
  free(env->ffmpeg);
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
  env->ffmpeg = calloc(1, sizeof(Process));
  if (env->ffmpeg == NULL) {
    tkbc_fprintf(stderr, "ERROR", "No more memory can be allocated.\n");
    return NULL;
  }

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
  env->ffmpeg->pipe = fildes[1];
  env->ffmpeg->pid = pid;
  return true;
}

/**
 * @brief The function checks the status of the ffmpeg process. If there is any
 * misbehavior it will be logged to the stderr as well as any occurring error.
 *
 * @param process The process that contains the pid of the previously created
 * ffmpeg child process.
 * @param is_kill_foreced Determines if the process is fore killed.
 * @return True if the ffmpeg process has finished successfully, otherwise
 * false.
 */
bool tkbc_ffmpeg_wait(Process process, bool is_kill_foreced) {
  if (is_kill_foreced) {
    kill(process.pid, SIGKILL);
  }

  while (true) {
    int status = 0;
    if (0 > waitpid(process.pid, &status, 0)) {
      tkbc_fprintf(stderr, "ERROR", "Waiting on process %d has failed:%s\n",
                   process.pid, strerror(errno));
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
 * @return 1 If no image was send to ffmpeg. 0 if the sending was successful,
 * otherwise if an error occurred while sending the image -1 is returned.
 */
int tkbc_ffmpeg_write_image(Env *env) {
  if (env->rendering) {
    bool ok = 0;
    Image image = LoadImageFromScreen();
    ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    for (int y = 1; y <= image.height; ++y) {
      int write_status =
          write(env->ffmpeg->pipe, (uint32_t *)image.data + image.width * y,
                sizeof(uint32_t) * image.width);
      if (write_status < 0) {
        tkbc_fprintf(stderr, "ERROR", "Could not write to the pipe: %s\n",
                     strerror(errno));
        check_return(-1);
      }
    }

  check:
    UnloadImage(image); // This is very important otherwise it is memory leak.
    return ok;
  }
  return 1;
}
#endif // _WIN32
