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

#include "../choreographer/tkbc-ffmpeg.h"
#include "../choreographer/tkbc-input-handler.h"
#include "../choreographer/tkbc-script-api.h"
#include "../choreographer/tkbc-script-converter.h"
#include "../choreographer/tkbc-sound-handler.h"
#include "../choreographer/tkbc.h"

#include "../../external/lexer/tkbc-lexer.h"
#include "../../tkbc_scripts/first.c"

#define WINDOW_SCALE 120
#define SCREEN_WIDTH 16 * WINDOW_SCALE
#define SCREEN_HEIGHT 9 * WINDOW_SCALE

Env *env = {0};
Client client = {0};
Message receive_message_queue = {0};
Message send_message_queue = {0};
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

void sending_script_handler() {
  if (env->script_setup) {
    // For detection if the begin and end is called correctly.
    env->script_setup = false;
    tkbc_script_input(env);
    for (size_t i = 0; i < env->block_frames->count; ++i) {
      tkbc_print_script(stderr, &env->block_frames->elements[i]);
      char buf[32];
      sprintf(buf, "Script%zu.kite", i);
      tkbc_write_script_kite_from_mem(&env->block_frames->elements[i], buf);
    }
  }
}

void send_message_handler() {
  if (send_message_queue.count) {
    int n = send(client.socket_id, send_message_queue.elements,
                 send_message_queue.count, MSG_NOSIGNAL);
    if (n == 0) {
      fprintf(stderr, "ERROR no bytes where send to the server!\n");
      return;
    }
    if (n < 0) {
      tkbc_dap(&send_message_queue, 0);
      fprintf(stderr, "ERROR: Could not broadcast message: %s\n",
              send_message_queue.elements);
      fprintf(stderr, "ERROR: %s\n", strerror(errno));
      return;
    }
  }
  send_message_queue.count = 0;
}

bool received_message_handler(bool first) {
  Message message = {0};
  Token token;
  bool ok = true;
  if (pthread_mutex_lock(&mutex) != 0) {
    assert(0 && "ERROR:mutex lock");
  }
  if (receive_message_queue.count == 0) {
    if (pthread_mutex_unlock(&mutex) != 0) {
      assert(0 && "ERROR:mutex unlock");
    }
    check_return(true);
  }
  tkbc_dapc(&message, receive_message_queue.elements,
            receive_message_queue.count);
  tkbc_dap(&message, 0);
  receive_message_queue.count = 0;
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

    assert(MESSAGE_COUNT == 5);
    switch (kind) {
    case MESSAGE_HELLO: {
      token = lexer_next(lexer);
      if (token.kind != STRINGLITERAL) {
        check_return(false);
      }

      const char *greeting = "\"Hello client from server!1.0\"";
      const char *compare = lexer_token_to_cstr(lexer, &token);
      if (strncmp(compare, greeting, strlen(greeting)) != 0) {
        fprintf(stderr, "ERROR: Hello Message failed!\n");
        fprintf(stderr, "ERROR: Wrong protocol version!");
        check_return(false);
      }
      token = lexer_next(lexer);
      if (token.kind != PUNCT_COLON) {
        check_return(false);
      }

      {
        char buf[64] = {0};
        snprintf(buf, sizeof(buf), "%d", MESSAGE_HELLO);
        tkbc_dapc(&send_message_queue, buf, strlen(buf));
        tkbc_dap(&send_message_queue, ':');

        tkbc_dapc(&send_message_queue, "\"", 1);
        const char *m = "Hello server from client!";
        tkbc_dapc(&send_message_queue, m, strlen(m));

        tkbc_dapc(&send_message_queue, PROTOCOL_VERSION,
                  strlen(PROTOCOL_VERSION));
        tkbc_dapc(&send_message_queue, "\"", 1);
        tkbc_dap(&send_message_queue, ':');
        tkbc_dapc(&send_message_queue, "\r\n", 2);
      }

      fprintf(stderr, "[[MESSAGEHANDLER]] message = HELLO\n");
    } break;
    case MESSAGE_KITEADD: {
      size_t kite_id;
      float x, y, angle;
      Color color;
      if (!tkbc_parse_message_kite_value(lexer, &kite_id, &x, &y, &angle,
                                         &color)) {
        goto err;
      }

      tkbc_register_kite_from_values(kite_id, x, y, angle, color);
      // This assumes the server sends the first KITEADD to the client, that
      // contains his own kite;
      if (first) {
        client.kite_id = kite_id;
        first = false;
      }

      fprintf(stderr, "[[MESSAGEHANDLER]] message = KITEADD\n");
    } break;
    case MESSAGE_KITEVALUE: {
      size_t kite_id;
      float x, y, angle;
      Color color;
      if (!tkbc_parse_message_kite_value(lexer, &kite_id, &x, &y, &angle,
                                         &color)) {
        goto err;
      }

      for (size_t i = 0; i < env->kite_array->count; ++i) {
        if (kite_id == env->kite_array->elements[i].kite_id) {
          Kite *kite = env->kite_array->elements[i].kite;
          kite->center.x = x;
          kite->center.y = y;
          kite->angle = angle;
          kite->body_color = color;
          tkbc_center_rotation(kite, NULL, kite->angle);
        }
      }

      fprintf(stderr, "[[MESSAGEHANDLER]] message = KITEVALUE\n");
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
        if (!tkbc_parse_message_kite_value(lexer, &kite_id, &x, &y, &angle,
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
    check_return(false);
  }
  } while (token.kind != EOF_TOKEN);

  lexer_del(lexer);
check:
  return ok ? true : false;
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
    tkbc_dapc(&receive_message_queue, buffer, strlen(buffer));

    if (pthread_mutex_unlock(&mutex) != 0) {
      assert(0 && "ERROR:mutex unlock");
    }
  }

  pthread_exit(NULL);
}

void tkbc_client_input_handler_kite() {
  for (size_t i = 0; i < env->kite_array->count; ++i) {
    if (env->kite_array->elements[i].kite_id == client.kite_id) {
      Kite_State *kite_state = &env->kite_array->elements[i];
      kite_state->kite_input_handler_active = true;
      tkbc_input_handler(env, kite_state);

      char buf[64] = {0};
      snprintf(buf, sizeof(buf), "%d", MESSAGE_KITEVALUE);
      tkbc_dapc(&send_message_queue, buf, strlen(buf));
      tkbc_dap(&send_message_queue, ':');
      tkbc_message_append_clientkite(client.kite_id, &send_message_queue);
      tkbc_dapc(&send_message_queue, "\r\n", 2);
      return;
    }
  }
}

int main(int argc, char *argv[]) {

  char *program_name = tkbc_shift_args(&argc, &argv);
  tkbc_client_commandline_check(argc, program_name);

  const char *host_check = tkbc_shift_args(&argc, &argv);
  const char *host = tkbc_host_parsing(host_check);

  char *port_check = tkbc_shift_args(&argc, &argv);
  uint16_t port = tkbc_port_parsing(port_check);

  int client_socket = tkbc_client_socket_creation(host, port);
  int first = true;

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
  tkbc_init_sound(40);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(SKYBLUE);
    if (!received_message_handler(first)) {
      break;
    }
    send_message_handler();
    sending_script_handler();

    tkbc_draw_kite_array(env->kite_array);
    tkbc_draw_ui(env);
    EndDrawing();

    tkbc_input_sound_handler(env);
    tkbc_client_input_handler_kite();
    // TODO: Handle the messages for the script.
    // That should contain of requests of starting/stopping the script,
    // switching to the next and generally all of the functionality in the
    // function below.
    tkbc_input_handler_script(env);
    tkbc_ffmpeg_handler(env, "output.mp4");
  };
  CloseWindow();

  pthread_cancel(thread);
  if (receive_message_queue.elements) {
    free(receive_message_queue.elements);
  }
  if (send_message_queue.elements) {
    free(send_message_queue.elements);
  }

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

  tkbc_sound_destroy(env->sound);
  tkbc_destroy_env(env);
  return 0;
}
