#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "tkbc-client.h"
#include "tkbc-network-common.h"

#define TKBC_UTILS_IMPLEMENTATION
#include "../global/tkbc-utils.h"

#include "../choreographer/tkbc.h"

#include "../../external/lexer/tkbc-lexer.h"

#define WINDOW_SCALE 120
#define SCREEN_WIDTH 16 * WINDOW_SCALE
#define SCREEN_HEIGHT 9 * WINDOW_SCALE

Env *env = {0};
Message message_queue = {0};
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void tkbc_client_usage(const char *program_name) {
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "       %s <HOST> <PORT> \n", program_name);
}

void tkbc_client_commandline_check(int argc, const char *program_name) {
  if (argc > 2) {
    fprintf(stderr, "ERROR: To may arguments.\n");
    tkbc_client_usage(program_name);
    exit(1);
  }
  if (argc == 0) {
    fprintf(stderr, "ERROR: No arguments were provided.\n");
    tkbc_client_usage(program_name);
    exit(1);
  }
}

const char *tkbc_host_parsing(const char *host_check) {

  const char *host = "127.0.0.1";
  if (strcmp(host_check, "localhost") == 0) {
    return host;
  }

  for (size_t i = 0; i < strlen(host_check); ++i) {
    if (!isdigit(host_check[i]) && host_check[i] != '.') {
      fprintf(stderr, "ERROR: The given host [%s] is not supported.\n",
              host_check);
      exit(1);
    }
  }

  // TODO: Check for DNS-resolution.
  host = host_check;
  return host;
}

int tkbc_client_socket_creation(const char *addr, uint16_t port) {
  int client_socket = socket(AF_INET, SOCK_STREAM, 0);

  int option = 1;
  int sso = setsockopt(client_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&option,
                       sizeof(option));
  if (sso == -1) {
    fprintf(stderr, "ERROR: %s\n", strerror(errno));
  }

  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port);
  server_address.sin_addr.s_addr = inet_addr(addr);

  int connection_status =
      connect(client_socket, (struct sockaddr *)&server_address,
              sizeof(server_address));

  if (connection_status == -1) {
    fprintf(stderr, "ERROR: %s\n", strerror(errno));
    exit(1);
  }

  fprintf(stderr, "Connected to Server: %s:%hd\n",
          inet_ntoa(server_address.sin_addr), ntohs(server_address.sin_port));

  return client_socket;
}

bool tkbc_parse_message_client(Lexer *lexer, size_t *kite_id, float *x,
                               float *y, float *angle, Color *color) {
  Content buffer = {0};
  Token token;
  bool ok = true;
  token = lexer_next(lexer);
  if (token.kind != NUMBER) {
    check_return(false);
  }
  *kite_id = atoi(lexer_token_to_cstr(lexer, &token));
  token = lexer_next(lexer);
  if (token.kind != PUNCT_COLON) {
    check_return(false);
  }
  token = lexer_next(lexer);
  if (token.kind != PUNCT_LPAREN) {
    check_return(false);
  }

  token = lexer_next(lexer);
  if (token.kind != NUMBER && token.kind != PUNCT_SUB) {
    check_return(false);
  }
  if (token.kind == PUNCT_SUB) {
    tkbc_dapc(&buffer, token.content, token.size);
    token = lexer_next(lexer);
    if (token.kind != NUMBER) {
      check_return(false);
    }
  }
  tkbc_dapc(&buffer, token.content, token.size);
  tkbc_dap(&buffer, 0);
  *x = atoi(buffer.elements);
  buffer.count = 0;

  token = lexer_next(lexer);
  if (token.kind != PUNCT_COMMA) {
    check_return(false);
  }
  token = lexer_next(lexer);
  if (token.kind != NUMBER && token.kind != PUNCT_SUB) {
    check_return(false);
  }
  if (token.kind == PUNCT_SUB) {
    tkbc_dapc(&buffer, token.content, token.size);
    token = lexer_next(lexer);
    if (token.kind != NUMBER) {
      check_return(false);
    }
  }
  tkbc_dapc(&buffer, token.content, token.size);
  tkbc_dap(&buffer, 0);
  *y = atoi(lexer_token_to_cstr(lexer, &token));
  buffer.count = 0;

  token = lexer_next(lexer);
  if (token.kind != PUNCT_RPAREN) {
    check_return(false);
  }
  token = lexer_next(lexer);
  if (token.kind != PUNCT_COLON) {
    check_return(false);
  }
  token = lexer_next(lexer);
  if (token.kind != NUMBER && token.kind != PUNCT_SUB) {
    check_return(false);
  }
  if (token.kind == PUNCT_SUB) {
    tkbc_dapc(&buffer, token.content, token.size);
    token = lexer_next(lexer);
    if (token.kind != NUMBER) {
      check_return(false);
    }
  }
  tkbc_dapc(&buffer, token.content, token.size);
  tkbc_dap(&buffer, 0);
  *angle = atoi(lexer_token_to_cstr(lexer, &token));
  buffer.count = 0;

  token = lexer_next(lexer);
  if (token.kind != PUNCT_COLON) {
    check_return(false);
  }

  token = lexer_next(lexer);
  if (token.kind != NUMBER) {
    check_return(false);
  }
  size_t color_number = atoi(lexer_token_to_cstr(lexer, &token));
  *color = *(Color *)&color_number;
  token = lexer_next(lexer);
  if (token.kind != PUNCT_COLON) {
    check_return(false);
  }
check:
  if (buffer.elements) {
    free(buffer.elements);
  }
  return ok ? true : false;
}

void tkbc_register_kite_from_values(size_t kite_id, float x, float y,
                                    float angle, Color color) {
  Kite_State *kite_state = tkbc_init_kite();
  kite_state->kite_id = kite_id;
  kite_state->kite->center.x = x;
  kite_state->kite->center.y = y;
  kite_state->kite->angle = angle;
  kite_state->kite->body_color = color;
  tkbc_center_rotation(kite_state->kite, NULL, kite_state->kite->angle);
  tkbc_dap(env->kite_array, *kite_state);
}

void message_handler(void) {
  Message message = {0};
  Token token;
  if (pthread_mutex_lock(&mutex) != 0) {
    assert(0 && "ERROR:mutex lock");
  }
  if (message_queue.count == 0) {
    if (pthread_mutex_unlock(&mutex) != 0) {
      assert(0 && "ERROR:mutex unlock");
    }
    return;
  }
  tkbc_dapc(&message, message_queue.elements, message_queue.count);
  tkbc_dap(&message, 0);
  message_queue.count = 0;
  if (pthread_mutex_unlock(&mutex) != 0) {
    assert(0 && "ERROR:mutex unlock");
  }

  Lexer *lexer = lexer_new(__FILE__, message.elements, message.count, 0);
  do {
    token = lexer_next(lexer);
    if (token.kind == EOF_TOKEN) {
      break;
    }
    if (token.kind == INVALID) {
      goto err;
    }
    if (token.kind == ERROR) {
      goto err;
    }

    if (token.kind != NUMBER) {
      goto err;
    }

    int kind = atoi(lexer_token_to_cstr(lexer, &token));
    token = lexer_next(lexer);
    if (token.kind != PUNCT_COLON) {
      goto err;
    }

    assert(MESSAGE_COUNT == 4);
    switch (kind) {
    case MESSAGE_HELLO: {
      token = lexer_next(lexer);
      if (token.kind != STRINGLITERAL) {
        goto err;
      }

      const char *greeting = "\"Hello client from server!1.0\"";
      const char *compare = lexer_token_to_cstr(lexer, &token);
      if (strncmp(compare, greeting, strlen(greeting)) != 0) {
        fprintf(stderr, "ERROR: Hello Message failed!\n");
        assert(0 && "ERROR: Wrong protocol version!");
      }
      token = lexer_next(lexer);
      if (token.kind != PUNCT_COLON) {
        goto err;
      }

      fprintf(stderr, "[[MESSAGEHANDLER]] message = HELLO\n");
    } break;
    case MESSAGE_KITEADD: {
      size_t kite_id;
      float x, y, angle;
      Color color;
      if (!tkbc_parse_message_client(lexer, &kite_id, &x, &y, &angle, &color)) {
        goto err;
      }

      tkbc_register_kite_from_values(kite_id, x, y, angle, color);

      fprintf(stderr, "[[MESSAGEHANDLER]] message = KITEADD\n");
    } break;
    case MESSAGE_CLIENTKITES: {
      token = lexer_next(lexer);
      if (token.kind != NUMBER) {
        goto err;
      }
      size_t amount = atoi(lexer_token_to_cstr(lexer, &token));
      token = lexer_next(lexer);
      if (token.kind != PUNCT_COLON) {
        goto err;
      }
      for (size_t i = 0; i < amount; ++i) {
        size_t kite_id;
        float x, y, angle;
        Color color;
        if (!tkbc_parse_message_client(lexer, &kite_id, &x, &y, &angle,
                                       &color)) {
          goto err;
        }
        bool found = false;
        for (size_t j = 0; j < env->kite_array->count; ++j) {
          if (kite_id == env->kite_array->elements[j].kite_id) {
            env->kite_array->elements[j].kite->center.x = x;
            env->kite_array->elements[j].kite->center.y = y;
            env->kite_array->elements[j].kite->angle = angle;
            env->kite_array->elements[j].kite->body_color = color;
            found = true;
            break;
          }
        }

        if (!found) {
          // If the kite_id is not registered.
          tkbc_register_kite_from_values(kite_id, x, y, angle, color);
        }
      }

      fprintf(stderr, "[[MESSAGEHANDLER]] message = CLIENTKITES\n");
    } break;
    default:
      fprintf(stderr, "ERROR: Unknown KIND: %d\n", kind);
      exit(1);
    }
    continue;

  err: {
    char *rn = strstr(message.elements, "\r\n");
    if (rn != NULL) {
      int jump_length = rn + 2 - &lexer->content[lexer->position];
      lexer_chop_char(lexer, jump_length);
      continue;
    }
    fprintf(stderr, "message.elements = %s\n", message.elements);
    break;
  }
  } while (token.kind != EOF_TOKEN);

  lexer_del(lexer);
}

void *message_recieving(void *client) {
  int client_socket = *((int *)client);

  for (;;) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    // The recv call blocks.
    int message_ckeck =
        recv(client_socket, buffer, sizeof(buffer), MSG_NOSIGNAL);
    if (message_ckeck == -1) {
      fprintf(stderr, "ERROR: %s\n", strerror(errno));
      break;
    }

    if (pthread_mutex_lock(&mutex) != 0) {
      assert(0 && "ERROR:mutex lock");
    }

    // This assumes that the message is less than the buffer size and read
    // completely.
    tkbc_dapc(&message_queue, buffer, strlen(buffer));

    if (pthread_mutex_unlock(&mutex) != 0) {
      assert(0 && "ERROR:mutex unlock");
    }
  }

  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {

  char *program_name = tkbc_shift_args(&argc, &argv);
  tkbc_client_commandline_check(argc, program_name);

  const char *host_check = tkbc_shift_args(&argc, &argv);
  const char *host = tkbc_host_parsing(host_check);

  char *port_check = tkbc_shift_args(&argc, &argv);
  uint16_t port = tkbc_port_parsing(port_check);

  int client_socket = tkbc_client_socket_creation(host, port);

  env = tkbc_init_env();
  pthread_t thread;

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  pthread_create(&thread, &attr, message_recieving, (void *)&client_socket);
  pthread_attr_destroy(&attr);

  const char *title = "TEAM KITE BALLETT CHOREOGRAPHER CLIENT";
  SetTraceLogLevel(LOG_NONE);
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, title);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(TARGET_FPS);
  SetExitKey(KEY_ESCAPE);
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(SKYBLUE);
    message_handler();

    tkbc_draw_kite_array(env->kite_array);
    EndDrawing();
  };
  CloseWindow();

  pthread_cancel(thread);
  free(message_queue.elements);

  shutdown(client_socket, SHUT_WR);
  char buf[1024] = {0};
  int n;
  do {
    n = read(client_socket, buf, sizeof(buf));
  } while (n > 0);

  if (n == 0) {
    fprintf(stderr, "INFO: Could not read any more data.\n");
  }
  if (n < 0) {
    fprintf(stderr, "ERROR: reading failed: %s\n", strerror(errno));
  }
  if (close(client_socket) == -1) {
    fprintf(stderr, "ERROR: Could not close socket: %s\n", strerror(errno));
  }

  tkbc_destroy_env(env);
  return 0;
}
