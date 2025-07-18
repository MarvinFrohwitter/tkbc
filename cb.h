#ifndef CB_H_
#define CB_H_

#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>

typedef struct {
  const char **elements;
  size_t count;
  size_t capacity;

} Cmd;

typedef struct {
  char *elements;
  size_t count;
  size_t capacity;

} Cmd_String;

#define DAP_CAP 64
#define dap(dynamic_array, element)                                            \
  do {                                                                         \
    if ((dynamic_array)->capacity <= (dynamic_array)->count) {                 \
      if ((dynamic_array)->capacity == 0)                                      \
        (dynamic_array)->capacity = DAP_CAP;                                   \
      else                                                                     \
        (dynamic_array)->capacity = (dynamic_array)->capacity * 2;             \
                                                                               \
      (dynamic_array)->elements = realloc((dynamic_array)->elements,           \
                                          sizeof(*(dynamic_array)->elements) * \
                                              (dynamic_array)->capacity);      \
                                                                               \
      if ((dynamic_array)->elements == NULL) {                                 \
        fprintf(                                                               \
            stderr,                                                            \
            "The allocation for the dynamic array has failed in: %s: %d\n",    \
            __FILE__, __LINE__);                                               \
        abort();                                                               \
      }                                                                        \
    }                                                                          \
                                                                               \
    (dynamic_array)->elements[(dynamic_array)->count] = (element);             \
    (dynamic_array)->count = (dynamic_array)->count + 1;                       \
  } while (0)

#define dapc(dynamic_array, new_elements, new_elements_count)                  \
  do {                                                                         \
    if ((new_elements) != NULL) {                                              \
      if ((dynamic_array)->capacity <                                          \
          (dynamic_array)->count + new_elements_count) {                       \
        if ((dynamic_array)->capacity == 0) {                                  \
          (dynamic_array)->capacity = DAP_CAP;                                 \
        }                                                                      \
        while ((dynamic_array)->capacity <                                     \
               (dynamic_array)->count + new_elements_count) {                  \
          (dynamic_array)->capacity = (dynamic_array)->capacity * 2;           \
        }                                                                      \
        (dynamic_array)->elements = realloc(                                   \
            (dynamic_array)->elements,                                         \
            sizeof(*(dynamic_array)->elements) * (dynamic_array)->capacity);   \
        if ((dynamic_array)->elements == NULL) {                               \
          fprintf(                                                             \
              stderr,                                                          \
              "The allocation for the dynamic array has failed in: %s: %d\n",  \
              __FILE__, __LINE__);                                             \
          abort();                                                             \
        }                                                                      \
      }                                                                        \
      memcpy((dynamic_array)->elements + (dynamic_array)->count,               \
             (new_elements),                                                   \
             sizeof(*(dynamic_array)->elements) * (new_elements_count));       \
      (dynamic_array)->count = (dynamic_array)->count + (new_elements_count);  \
    }                                                                          \
  } while (0)

void cb__cmd_push(Cmd *cmd, ...);
pid_t cb_run_async(Cmd *cmd);
bool cb_wait(pid_t pid);
bool cb_run_sync(Cmd *cmd);
bool check_char_is_safe(char c);

#define cb_cmd_push(cmd, ...) cb__cmd_push(cmd, __VA_ARGS__, NULL)
#define LIBS(cmd, ...) cb_cmd_push(cmd, __VA_ARGS__)
#define CFLAGS(cmd, ...) cb_cmd_push(cmd, __VA_ARGS__)
#define LDFLAGS(cmd, ...) cb_cmd_push(cmd, __VA_ARGS__)
#define INCLUDE(cmd, ...) cb_cmd_push(cmd, __VA_ARGS__)

#endif // CB_H_

// ===========================================================================

#ifdef CB_IMPLEMENTATION
#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

void cb__cmd_push(Cmd *cmd, ...) {

  va_list args;
  va_start(args, cmd);
  const char *arg = va_arg(args, const char *);
  while (arg != NULL) {
    dap(cmd, arg);
    arg = va_arg(args, const char *);
  }
  va_end(args);
}

pid_t cb_run_async(Cmd *cmd) {
  Cmd_String cmd_string = {0};
  for (size_t i = 0; i < cmd->count; ++i) {
    dapc(&cmd_string, cmd->elements[i], strlen(cmd->elements[i]));
    dap(&cmd_string, ' ');
  }
  dap(&cmd_string, '\0');

  fprintf(stderr, "[INFO] %s\n", cmd_string.elements);

  pid_t pid = fork();
  if (0 > pid) {
    fprintf(stderr, "[ERROR]: The fork of process %s was not possible:%s\n",
            cmd_string.elements, strerror(errno));
    return -1;
  }

  Cmd out_cmd = {0};
  dapc(&out_cmd, cmd->elements, cmd->count);
  dap(&out_cmd, NULL);

  if (0 == pid) {
    int ret = execvp(out_cmd.elements[0], (char *const *)out_cmd.elements);
    if (ret < 0) {
      fprintf(stderr, "[ERROR]: The execvp has failed with:%s\n",
              strerror(errno));
      exit(1);
    }
    assert(0 && "UNREACHABLE: Possible bug in kernel or libc!");
  } else {
    // If some time you don't want to reset disable it.
    cmd->count = 0;
  }

  return pid;
}

bool cb_wait(pid_t pid) {

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

bool cb_run_sync(Cmd *cmd) {
  pid_t pid = cb_run_async(cmd);
  if (-1 == pid) {
    return false;
  }
  return cb_wait(pid);
}

bool check_char_is_safe(char c) {
  char *unsafe_chars = "$_+=:,.@%/";

  while (*unsafe_chars) {
    if (c == *unsafe_chars) {
      return false;
    }
    unsafe_chars++;
  }
  return true;
}

#endif // CB_IMPLEMENTATION
