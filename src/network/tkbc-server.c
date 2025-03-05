#include "tkbc-server.h"
#include "../choreographer/tkbc-script-api.h"
#include "tkbc-network-common.h"
#include "tkbc-server-client-handler.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WINUSER_
#define _WINGDI_
#define _IMM_
#define _WINCON_
#include <windows.h>
#include <winsock2.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#include "../global/tkbc-types.h"

#define TKBC_SERVER
Env *env = {0};
Clients *clients = {0};
int server_socket;

pthread_t threads[SERVER_CONNETCTIONS];
pthread_t execution_thread;

unsigned long long out_bytes = 0;
unsigned long long in_bytes = 0;

#include "../choreographer/tkbc.h"

#define TKBC_UTILS_IMPLEMENTATION
#include "../global/tkbc-utils.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


/**
 * @brief The function allocates a pointer of type Client on the heap.
 *
 * @return The Clients pointer if successful, otherwise false.
 */
Clients *tkbc_init_clients(void) {
  Clients *clients = malloc(sizeof(*clients));
  if (clients == NULL) {
    tkbc_fprintf(stderr, "ERROR", "No more memory can be allocated.\n");
    return NULL;
  }
  memset(clients, 0, sizeof(*clients));
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
  // Get the first 0 out of the way.
  tkbc_get_frame_time();
  env = tkbc_init_env();
  env->window_width = 1920;
  env->window_height = 1080;
  clients = tkbc_init_clients();
  size_t clients_visited = 0;

  pthread_create(&execution_thread, NULL, tkbc_script_execution_handler, NULL);
  if (pthread_detach(execution_thread)) {
    tkbc_fprintf(stderr, "ERROR", "%s: %s\n",
                 "Detaching execution thread has gone wrong", strerror(errno));
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
    SOCKLEN address_length = sizeof(client_address);
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
      Client *c = &clients->elements[clients->count - 1];
      pthread_mutex_unlock(&mutex);
      pthread_create(&threads[clients_visited++], NULL, tkbc_client_handler, c);
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
    if (env->script_counter > 0) {
      if (!tkbc_script_finished(env) && env->block_frame != NULL) {
        size_t bindex = env->frames->block_index;
        tkbc_script_update_frames(env);

        if (env->frames->block_index != bindex) {
          bindex = env->frames->block_index;
          tkbc_message_srcipt_block_frames_value(
              env->block_frame->script_id, env->block_frame->count, bindex);
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
    tkbc_server_shutdown_client(clients->elements[i], true);
  }
  pthread_cancel(execution_thread);

  shutdown(server_socket, SHUT_RDWR);
#ifdef _WIN32
  if (closesocket(server_socket) == -1) {

    tkbc_fprintf(stderr, "ERROR", "Could not close socket: %d\n",
                 WSAGetLastError());
  }
  WSACleanup();
#else
  if (close(server_socket) == -1) {
    tkbc_fprintf(stderr, "ERROR", "Main Server Socket: %s\n", strerror(errno));
  }
#endif

  if (clients->elements) {
    free(clients->elements);
    clients->elements = NULL;
  }
  tkbc_destroy_env(env);
  printf("The amount of send bytes:%llu\n", out_bytes);
  printf("The amount of recv bytes:%llu\n", in_bytes);
  exit(EXIT_SUCCESS);
}
