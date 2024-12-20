#include "tkbc-server.h"
#include "../choreographer/tkbc-script-api.h"
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

#include "../global/tkbc-types.h"

Env *env = {0};
Clients *clients = {0};
int server_socket;

pthread_t threads[SERVER_CONNETCTIONS];
pthread_t execution_thread;

#include "../choreographer/tkbc.h"

#define TKBC_UTILS_IMPLEMENTATION
#include "../global/tkbc-utils.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void tkbc_server_usage(const char *program_name) {
  tkbc_logger(stderr, "Usage:\n");
  tkbc_logger(stderr, "       %s <PORT> \n", program_name);
}

void tkbc_server_commandline_check(int argc, const char *program_name) {
  if (argc > 1) {
    tkbc_logger(stderr, "ERROR: To may arguments.\n");
    tkbc_server_usage(program_name);
    exit(1);
  }
  if (argc == 0) {
    tkbc_logger(stderr, "ERROR: No arguments were provided.\n");
    tkbc_server_usage(program_name);
    exit(1);
  }
}

int tkbc_server_socket_creation(uint32_t addr, uint16_t port) {
  int socket_id = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_id == -1) {
    tkbc_logger(stderr, "ERROR: %s\n", strerror(errno));
    exit(1);
  }
  int option = 1;
  int sso = setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, (char *)&option,
                       sizeof(option));
  if (sso == -1) {
    tkbc_logger(stderr, "ERROR: %s\n", strerror(errno));
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = addr;

  int bind_status =
      bind(socket_id, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (bind_status == -1) {
    tkbc_logger(stderr, "ERROR: %s\n", strerror(errno));
    exit(1);
  }

  int listen_status = listen(socket_id, SERVER_CONNETCTIONS);
  if (listen_status == -1) {
    tkbc_logger(stderr, "ERROR: %s\n", strerror(errno));
    exit(1);
  }
  tkbc_logger(stderr, "INFO: Listening to port: %hu\n", port);

  return socket_id;
}

Clients *tkbc_init_clients(void) {
  Clients *clients = calloc(1, sizeof(*clients));
  if (clients == NULL) {
    tkbc_logger(stderr, "ERROR: No more memory can be allocated.\n");
    return NULL;
  }
  return clients;
}

int main(int argc, char *argv[]) {
  signal(SIGABRT, signalhandler);
  signal(SIGINT, signalhandler);
  signal(SIGTERM, signalhandler);
  char *start = "The server has started.";
  tkbc_fprintf(stderr, "INFO", "%s\n", start);

  // char *program_name = tkbc_shift_args(&argc, &argv);
  // tkbc_server_commandline_check(argc, program_name);

  // char *port_check = tkbc_shift_args(&argc, &argv);
  // uint16_t port = tkbc_port_parsing(port_check);

  server_socket = tkbc_server_socket_creation(INADDR_ANY, 8080);
  tkbc_logger(stderr, "INFO:Server socket: %d\n", server_socket);
  env = tkbc_init_env();
  clients = tkbc_init_clients();
  size_t clients_visited = 0;

  pthread_create(&execution_thread, NULL, tkbc_script_execution_handler, NULL);

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
      if (pthread_mutex_lock(&mutex) != 0) {
        assert(0 && "ERROR:mutex lock");
      }
      Client client = {
          .kite_id = env->kite_id_counter++,
          .thread_id = clients_visited,
          .socket_id = client_socket_id,
          .client_address = client_address,
          .client_address_length = address_length,
      };
      tkbc_dap(clients, client);
      tkbc_logger(stderr, "INFO: Client:" CLIENT_FMT " has connected.\n",
                  CLIENT_ARG(client));

      assert(clients->count > 0);
      Client c = clients->elements[clients->count - 1];
      if (pthread_mutex_unlock(&mutex) != 0) {
        assert(0 && "ERROR:mutex unlock");
      }
      pthread_create(&threads[clients_visited++], NULL, tkbc_client_handler,
                     &c);
    } else {
      tkbc_logger(stderr, "ERROR: %s\n", strerror(errno));
      assert(0 && "accept error");
    }
  }

  signalhandler(SIGINT);
  return 0;
}

void *tkbc_script_execution_handler() {
  for (;;) {
    pthread_mutex_lock(&mutex);
    if (env->block_frames->count > 0) {
      if (!tkbc_script_finished(env)) {
        size_t index = env->frames->block_index;
        tkbc_script_update_frames(env);
        if (env->frames->block_index != index) {
          tkbc_message_srcipt_block_frames_value();
        }
        Clients cs = {0};
        if (!tkbc_message_clientkites_brodcast_all(&cs)) {
          for (size_t i = 0; i < cs.count; ++i) {
            pthread_mutex_lock(&mutex);
            tkbc_server_shutdown_client(cs.elements[i]);
          }
          free(cs.elements);
        }
      }
    }
    pthread_mutex_unlock(&mutex);
    sleep(1);
  }
  pthread_exit(NULL);
}

void signalhandler(int signal) {
  (void)signal;
  for (size_t i = 0; i < clients->count; ++i) {
    if (pthread_mutex_lock(&mutex) != 0) {
      assert(0 && "ERROR:mutex lock");
    }
    tkbc_server_shutdown_client(clients->elements[i]);
    if (pthread_mutex_unlock(&mutex) != 0) {
      assert(0 && "ERROR:mutex unlock");
    }
  }
  pthread_cancel(execution_thread);

  tkbc_logger(stderr, "INFO: Closing...\n");

  shutdown(server_socket, SHUT_RDWR);
  if (close(server_socket) == -1) {
    tkbc_logger(stderr, "ERROR: Main Server Socket: %s\n", strerror(errno));
  }
  if (clients->elements) {
    free(clients->elements);
  }
  exit(EXIT_SUCCESS);
}
