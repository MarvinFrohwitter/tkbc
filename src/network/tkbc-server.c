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

#define TKBC_SERVER
Env *env = {0};
Clients *clients = {0};
int server_socket;

pthread_t threads[SERVER_CONNETCTIONS];
pthread_t execution_thread;

#include "../choreographer/tkbc.h"

#define TKBC_UTILS_IMPLEMENTATION
#include "../global/tkbc-utils.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief The function prints the way the program should be called.
 *
 * @param program_name The name of the program that is currently executing.
 */
void tkbc_server_usage(const char *program_name) {
  tkbc_fprintf(stderr, "INFO", "Usage:\n");
  tkbc_fprintf(stderr, "INFO", "      %s <PORT> \n", program_name);
}

/**
 * @brief The function checks if a port is given to that program.
 *
 * @param argc The commandline argument count.
 * @param program_name The name of the program that is currently executing.
 * @return True if there are enough arguments, otherwise false.
 */
bool tkbc_server_commandline_check(int argc, const char *program_name) {
  if (argc > 1) {
    tkbc_fprintf(stderr, "ERROR", "To may arguments.\n");
    tkbc_server_usage(program_name);
    exit(1);
  }
  if (argc == 0) {
    tkbc_fprintf(stderr, "ERROR", "No arguments were provided.\n");
    tkbc_fprintf(stderr, "ERROR", "The default port 8080 is used.\n");
    return false;
  }
  return true;
}

/**
 * @brief The function creates a new server socket and sets up the bind and
 * listing.
 *
 * @param addr The address space that the socket should be bound to.
 * @param port The port where the server is listing for connections.
 * @return The newly creates socket id.
 */
int tkbc_server_socket_creation(uint32_t addr, uint16_t port) {
  int socket_id = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_id == -1) {
    tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
    exit(1);
  }
  int option = 1;
  int sso = setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, (char *)&option,
                       sizeof(option));
  if (sso == -1) {
    tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = addr;

  int bind_status =
      bind(socket_id, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (bind_status == -1) {
    tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
    exit(1);
  }

  int listen_status = listen(socket_id, SERVER_CONNETCTIONS);
  if (listen_status == -1) {
    tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
    exit(1);
  }
  tkbc_fprintf(stderr, "INFO", "%s: %hu\n", "Listening to port", port);

  return socket_id;
}

/**
 * @brief The function allocates a pointer of type Client on the heap.
 *
 * @return The Clients pointer if successful, otherwise false.
 */
Clients *tkbc_init_clients(void) {
  Clients *clients = calloc(1, sizeof(*clients));
  if (clients == NULL) {
    tkbc_fprintf(stderr, "ERROR", "No more memory can be allocated.\n");
    return NULL;
  }
  return clients;
}

/**
 * @brief The function is responsible for setting up the server socket and
 * handling all incoming client connection.
 *
 * @param argc The commandline argument count.
 * @param argv The arguments form the commandline.
 * @return The function never returns. The program is terminated by an exit
 * call, in any situation it has an unacceptable error.
 */
int main(int argc, char *argv[]) {
  signal(SIGABRT, signalhandler);
  signal(SIGINT, signalhandler);
  signal(SIGTERM, signalhandler);
  tkbc_fprintf(stderr, "INFO", "%s\n", "The server has started.");

  char *program_name = tkbc_shift_args(&argc, &argv);
  uint16_t port = 8080;
  if (tkbc_server_commandline_check(argc, program_name)) {
    char *port_check = tkbc_shift_args(&argc, &argv);
    port = tkbc_port_parsing(port_check);
  }

  server_socket = tkbc_server_socket_creation(INADDR_ANY, port);
  tkbc_fprintf(stderr, "INFO", "%s: %d\n", "Server socket", server_socket);

  srand(time(NULL));
  env = tkbc_init_env();
  env->window_width = 1920;
  env->window_height = 1080;
  clients = tkbc_init_clients();
  size_t clients_visited = 0;

  pthread_create(&execution_thread, NULL, tkbc_script_execution_handler, NULL);
  if (pthread_detach(execution_thread)) {
    tkbc_fprintf(stderr, "ERROR", "%s: %s\n",
                 "Detaching execution thread has gone worng", strerror(errno));
    exit(EXIT_FAILURE);
  }

  for (;;) {
    if (clients_visited > SERVER_CONNETCTIONS) {
      signalhandler(SIGINT);
      // This part is never reached, just for documentation purpose that
      // the state is in a failure situation.
      return 1;
    }
    struct sockaddr_in client_address;
    socklen_t address_length = sizeof(client_address);
    int client_socket_id = accept(
        server_socket, (struct sockaddr *)&client_address, &address_length);
    if (client_socket_id != -1) {
      pthread_mutex_lock(&mutex);
      Client client = {
          .kite_id = env->kite_id_counter++,
          .thread_id = clients_visited,
          .socket_id = client_socket_id,
          .client_address = client_address,
          .client_address_length = address_length,
      };
      tkbc_dap(clients, client);
      tkbc_fprintf(stderr, "INFO", "CLIENT: " CLIENT_FMT " has connected.\n",
                   CLIENT_ARG(client));

      assert(clients->count > 0);
      Client c = clients->elements[clients->count - 1];
      pthread_mutex_unlock(&mutex);
      pthread_create(&threads[clients_visited++], NULL, tkbc_client_handler,
                     &c);
    } else {
      tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
      assert(0 && "accept error");
    }
  }

  // This part is never reached, just for documentation purpose that
  // the state is in a success situation.
  signalhandler(SIGINT);
  return 0;
}

/**
 * @brief The function that represents the independent base execution. It
 * updates the state of the current script execution.
 */
void *tkbc_script_execution_handler() {
  for (;;) {
    bool check = false;
    pthread_mutex_lock(&mutex);
    if (env->script_counter > 0 && env->block_frame->count > 0) {
      if (!tkbc_script_finished(env)) {
        size_t bindex = env->frames->block_index;
        size_t bframe_count = env->block_frame->count;
        tkbc_script_update_frames(env);

        if (env->frames->block_index != bindex) {
          bindex = env->frames->block_index;
          tkbc_message_srcipt_block_frames_value(bindex, bframe_count);
        }
        tkbc_unwrap_handler_message_clientkites_brodcast_all();
        check = true;
      }
    }
    pthread_mutex_unlock(&mutex);
    // TODO: Think about this hack to reduce computation load.
    if (!check) {
      sleep(1);
    }
  }
  pthread_exit(NULL);
}

/**
 * @brief The function represents the closing handler of the server. It can be
 * used as a signal handler or be called if the server should be shutdown-
 *
 * @param signal The signal that might has occurred.
 */
void signalhandler(int signal) {
  (void)signal;
  tkbc_fprintf(stderr, "INFO", "Closing...\n");

  for (size_t i = 0; i < clients->count; ++i) {
    pthread_mutex_lock(&mutex);
    tkbc_server_shutdown_client(clients->elements[i], true);
    pthread_mutex_unlock(&mutex);
  }
  pthread_cancel(execution_thread);

  shutdown(server_socket, SHUT_RDWR);
  if (close(server_socket) == -1) {
    tkbc_fprintf(stderr, "ERROR", "Main Server Socket: %s\n", strerror(errno));
  }
  if (clients->elements) {
    free(clients->elements);
  }
  exit(EXIT_SUCCESS);
}
