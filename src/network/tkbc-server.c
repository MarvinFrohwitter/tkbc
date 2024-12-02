#include "tkbc-server.h"
#include "tkbc-network-common.h"
#include "tkbc-server-client-handler.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void (*signal(int sig, void (*func)(int)))(int);

#include "../global/tkbc-types.h"

Env *env;
Clients *clients;
int server_socket;

pthread_t threads[SERVER_CONNETCTIONS];

#include "../choreographer/tkbc.h"

#define TKBC_UTILS_IMPLEMENTATION
#include "../global/tkbc-utils.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

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
  int option = 1;
  int sso = setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, (char *)&option,
                       sizeof(option));
  if (sso == -1) {
    fprintf(stderr, "ERROR: %s\n", strerror(errno));
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
  fprintf(stderr, "INFO: Listening to port: %hu\n", port);

  return socket_id;
}

Clients *tkbc_init_clients(void) {
  Clients *clients = calloc(1, sizeof(*clients));
  if (clients == NULL) {
    fprintf(stderr, "ERROR: No more memory can be allocated.\n");
    return NULL;
  }
  return clients;
}

int main(int argc, char *argv[]) {
  signal(SIGABRT, signalhandler);
  signal(SIGINT, signalhandler);
  signal(SIGTERM, signalhandler);

  char *program_name = tkbc_shift_args(&argc, &argv);
  tkbc_server_commandline_check(argc, program_name);

  char *port_check = tkbc_shift_args(&argc, &argv);
  uint16_t port = tkbc_port_parsing(port_check);

  server_socket = tkbc_server_socket_creation(INADDR_ANY, port);
  fprintf(stderr, "INFO:Server socket: %d\n", server_socket);
  env = tkbc_init_env();
  clients = tkbc_init_clients();
  size_t clients_visited = 0;

  for (;;) {
    if (clients_visited > SERVER_CONNETCTIONS) {
      signalhandler(SIGINT);
      return 1;
    }
    struct sockaddr_in client_address = {0};
    socklen_t address_length = 0;
    int client_socket_id = accept(
        server_socket, (struct sockaddr *)&client_address, &address_length);
    if (client_socket_id != -1) {
      Client client = {
          .index = clients->count,
          .socket_id = client_socket_id,
          .client_address = client_address,
          .client_address_length = address_length,
      };
      tkbc_dap(clients, client);
      fprintf(stderr, "INFO: Client " CLIENT_FMT " has connected.\n",
              CLIENT_ARG(client));

      assert(clients->count > 0);
      Client *c = &clients->elements[clients->count - 1];
      pthread_create(&threads[clients_visited++], NULL, tkbc_client_handler, c);
    } else {
      fprintf(stderr, "ERROR: %s\n", strerror(errno));
      assert(0 && "accept error");
    }
  }

  return 0;
}

void signalhandler(int signal) {
  (void)signal;
  for (size_t i = 0; i < clients->count; ++i) {
    pthread_join(threads[i], NULL);
  }

  fprintf(stderr, "INFO: Closing...\n");
  if (close(server_socket) == -1) {
    fprintf(stderr, "ERROR: Main Server Socket: %s\n", strerror(errno));
  }
  free(clients->elements);
  exit(EXIT_SUCCESS);
}
