#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "tkbc-network-common.h"
#include "tkbc-server-client-handler.h"
#include "tkbc-server.h"

#define TKBC_UTILS_IMPLEMENTATION
#include "../global/tkbc-utils.h"

// static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

#define SERVER_CONNETCTIONS 64
pthread_t threads[SERVER_CONNETCTIONS];

Clients clients = {0};

void tkbc_server_usage(const char *program_name) {
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "       %s <PORT> \n", program_name);
}

void tkbc_server_commandline_check(int argc, const char *program_name) {
  if (argc > 1) {
    fprintf(stderr, "ERROR: To may arguments.\n");
    tkbc_server_usage(program_name);
    exit(1);
  }
  if (argc == 0) {
    fprintf(stderr, "ERROR: No arguments were provided.\n");
    tkbc_server_usage(program_name);
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
    exit(1);
  }
  printf("Listening to port: %hu\n", port);

  int iSetOption = 1;
  int sso = setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, (char *)&iSetOption,
                       sizeof(iSetOption));
  if (sso == -1) {
    fprintf(stderr, "ERROR: %s\n", strerror(errno));
  }

  return socket_id;
}

int main(int argc, char *argv[]) {
  char *program_name = tkbc_shift_args(&argc, &argv);
  tkbc_server_commandline_check(argc, program_name);

  char *port_check = tkbc_shift_args(&argc, &argv);
  uint16_t port = tkbc_port_parsing(port_check);

  int server_socket = tkbc_server_socket_creation(INADDR_ANY, port);

  for (int i = 0;; ++i) {

    struct sockaddr_in client_address;
    socklen_t address_length;
    int client_socket_id = accept(
        server_socket, (struct sockaddr *)&client_address, &address_length);
    if (client_socket_id != -1) {
      Client client = {.index = clients.count,
                       .socket_id = client_socket_id,
                       .client_address = client_address,
                       .client_address_length = address_length,
                       .connected = false};
      tkbc_dap(&clients, client);
      printf("Client %d has connected.\n", client_socket_id);

      assert(clients.count > 0);
      Client *c = &clients.elements[clients.count - 1];
      pthread_create(&threads[clients.count - 1], NULL, client_handler, c);
    }
  }

  for (size_t i = 0; i < clients.count; ++i) {
    pthread_join(threads[i], NULL);
  }

  close(server_socket);
  free(clients.elements);
  return 0;
}
