#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define TKBC_UTILS_IMPLEMENTATION
#include "tkbc-utils.h"

typedef struct {
  int socket_id;
  bool connected;
} Client;

typedef struct {
  Client *elements;
  size_t count;
  size_t capacity;
} Clients;


// static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

#define SERVER_CONNETCTIONS 64
pthread_t threads[SERVER_CONNETCTIONS];

Clients clients = {0};

uint16_t tkbc_port_parsing(char *port_check) {

  for (size_t i = 0; i < strlen(port_check); ++i) {
    if (!isdigit(port_check[i])) {
      fprintf(stderr, "ERROR: The given port [%s] is not valid.\n", port_check);
      exit(1);
    }
  }

  int port = atoi(port_check);
  if (port >= 65535 || port <= 0) {
    fprintf(stderr, "ERROR: The given port [%s] is not valid.\n", port_check);
    exit(1);
  }

  return (uint16_t)port;
}

void tkbc_usage(const char *program_name) {
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "       %s <PORT> \n", program_name);
}

void tkbc_commandline_check(int argc, const char *program_name) {
  if (argc > 1) {
    fprintf(stderr, "ERROR: To may arguments.\n");
    tkbc_usage(program_name);
    exit(1);
  }
  if (argc == 0) {
    fprintf(stderr, "ERROR: No arguments were provided.\n");
    tkbc_usage(program_name);
    exit(1);
  }
}

int tkbc_server_socket_creation(uint32_t addr, uint16_t port) {
  int socket_id = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_id == -1) {
    fprintf(stderr, "ERROR: %s\n", strerror(errno));
    exit(1);
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = addr;

  int bind_status =
      bind(socket_id, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (bind_status == -1) {
    fprintf(stderr, "ERROR: %s\n", strerror(errno));
    exit(1);
  }

  int listen_status = listen(socket_id, SERVER_CONNETCTIONS);
  if (listen_status == -1) {
    fprintf(stderr, "ERROR: %s\n", strerror(errno));
  }
  return socket_id;
}

void *client_handler(void *client) {
  Client *c = (Client *)client;

  while (true) {

    if (!c->connected) {
      c->connected = true;
      char *hello_msg = "Hello client from server!";
      int send_check = send(c->socket_id, hello_msg, strlen(hello_msg), 0);
      if (send_check == -1) {
        fprintf(stderr,
                "ERROR: Hello Message could not be send to client %d.\n",
                c->socket_id);
      }
    }
    // TODO: Handle other messages.
  }

  return NULL;
}

int main(int argc, char *argv[]) {
  char *program_name = tkbc_shift_args(&argc, &argv);
  tkbc_commandline_check(argc, program_name);

  char *port_check = tkbc_shift_args(&argc, &argv);
  uint16_t port = tkbc_port_parsing(port_check);

  int socket_id = tkbc_server_socket_creation(INADDR_ANY, port);

  while (true) {
    int client_socket = accept(socket_id, NULL, NULL);
    if (client_socket != -1) {
      Client client = {.socket_id = client_socket, .connected = false};
      tkbc_dap(&clients, client);
      fprintf(stderr, "Client %d has connected.\n", client_socket);

      assert(clients.count > 0);
      Client *c = &clients.elements[clients.count - 1];
      pthread_create(&threads[clients.count], NULL, client_handler, c);
    }
  }

  for (size_t i = 0; i < clients.count; ++i) {
    pthread_join(threads[i], NULL);
  }

  int iSetOption = 1;
  int sso = setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, (char *)&iSetOption,
                       sizeof(iSetOption));
  if (sso == -1) {
    fprintf(stderr, "ERROR: %s\n", strerror(errno));
  }

  close(socket_id);

  free(clients.elements);
  return 0;
}
