#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
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

#define WINDOW_SCALE 120
#define SCREEN_WIDTH 16 * WINDOW_SCALE
#define SCREEN_HEIGHT 9 * WINDOW_SCALE

Env *env = {0};
Message message = {0};

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

  // TODO: Think about the decoding when ip and port are available through
  // arguments.
  fprintf(stderr, "Connected to Server: %s:%hd\n",
          inet_ntoa(server_address.sin_addr), ntohs(server_address.sin_port));

  return client_socket;
}

void message_handler(void) {
  char *token = strtok(message.elements, ":");
  if (token == NULL) {
    fprintf(stderr, "message.elements = %s\n", message.elements);
    assert(0 && "ERROR: token is NULL");
  }

  int kind = atoi(token);
  assert(MESSAGE_COUNT == 3);
  switch (kind) {
  case MESSAGE_HELLO: {
    token = strtok(NULL, ":");

    if (token == NULL) {
      fprintf(stderr, "message.elements = %s\n", message.elements);
      assert(0 && "ERROR: token is NULL");
    }

    fprintf(stderr, "[[MESSAGEHANDLER]] message = HELLO\n");
  } break;
  case MESSAGE_KITEADD: {
    token = strtok(NULL, ":");
    if (token == NULL) {
      fprintf(stderr, "message.elements = %s\n", message.elements);
      assert(0 && "ERROR: token is NULL");
    }
    size_t kite_id = atoi(token);

    token = strtok(NULL, ":");
    if (token == NULL) {
      fprintf(stderr, "message.elements = %s\n", message.elements);
      assert(0 && "ERROR: token is NULL");
    }
    size_t color_number = atoi(token);
    Color color = *(Color *)&color_number;

    token = strtok(NULL, ":");

    fprintf(stderr, "----------------------------\n");
    fprintf(stderr, "%s\n", token);
    fprintf(stderr, "----------------------------\n");

    if (token == NULL) {
      fprintf(stderr, "message.elements = %s\n", message.elements);
      assert(0 && "ERROR: token is NULL");
    }
    if (token != NULL) {
      fprintf(stderr, "message.elements = %s\n", token);
      assert(0 && "ERROR: invalid token.");
    }

    Kite_State *kite_state = tkbc_init_kite();
    tkbc_dap(env->kite_array, *kite_state);
    Index index = env->kite_array->count - 1;
    env->kite_array->elements[index].kite_id = kite_id;
    env->kite_array->elements[index].kite->body_color = color;

    fprintf(stderr, "[[MESSAGEHANDLER]] message = KITEADD\n");
  } break;
  default:
    fprintf(stderr, "ERROR: Unknown KIND: %d\n", kind);
    exit(1);
  }
  message.count = 0;
}

void *message_recieving(void *client) {
  int client_socket = *((int *)client);

  char tmp[1024];
  size_t tmp_count = 0;
  ushort rn_len = 2;
  // For init;
  tkbc_dap(&message, 0);

  for (;;) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    // The recv call blocks.
    int message_ckeck = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (message_ckeck == -1) {
      fprintf(stderr, "ERROR: %s\n", strerror(errno));
      break;
    }

    char *ptr = strstr(buffer, "\r\n");
    if (ptr == NULL) {
      if (strlen(buffer) > 0) {
        fprintf(stderr, "[[MESSAGERECIEVER]] %s", buffer);
        assert(0 && "ERROR:MESSAGE is bigger than 1024bytes");
      }
      continue;
    }

    if (tmp_count) {
      tkbc_dapc(&message, tmp, tmp_count);
      tmp_count = 0;
    }

    size_t firstbytes_len = ptr - buffer - 1;
    tmp_count = sizeof(buffer) - firstbytes_len - rn_len;
    memcpy(tmp, ptr + rn_len, tmp_count);
    memset(ptr, 0, tmp_count);

    assert(firstbytes_len <= sizeof(buffer));
    message.count = 0;
    tkbc_dapc(&message, buffer, strlen(buffer));
    tkbc_dap(&message, 0);

    // TODO: recv blocks so just the first message is send to the
    // message_handler and then the next message if more than on is in the read
    // buffer is going to the temp but not executed. If the next message is read
    // the handler will execute again?
    fprintf(stderr, "[Client] Message: [%s]\n", message.elements);
    // TODO: Check for return code.
  }
  free(message.elements);

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
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, title);
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(SKYBLUE);
    message_handler();

    tkbc_draw_kite_array(env->kite_array);
    EndDrawing();
  };

  close(client_socket);
  tkbc_destroy_env(env);
  CloseWindow();
  return 0;
}
