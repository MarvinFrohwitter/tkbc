#include "tkbc-server-client-handler.h"
#include "tkbc-network-common.h"
#include "tkbc-server.h"

#include "../choreographer/tkbc.h"
#include "../global/tkbc-utils.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

extern Env *env;
extern Clients *clients;
extern pthread_t threads[SERVER_CONNETCTIONS];
extern pthread_mutex_t mutex;

void signal_int(int signal) {
  (void)signal;
  fprintf(stderr, "Check signal int handler\n");
  pthread_exit(NULL);
}

void tkbc_server_shutdown_client(Client *client) {
  if (!tkbc_server_remove_client_from_list(client)) {
    fprintf(stderr,
            "INFO: Client: " CLIENT_FMT
            ": could not be removed after broken pipe\n",
            CLIENT_ARG(*client));
  }

  shutdown(client->socket_id, SHUT_WR);
  char buf[1024] = {0};
  int n = read(client->socket_id, buf, sizeof(buf));
  while (n > 0) {
    n = read(client->socket_id, buf, sizeof(buf));
  }

  if (n == 0) {
    fprintf(stderr, "INFO: Could not read any more data from the client.\n");
  }
  if (n < 0) {
    fprintf(stderr, "ERROR: reading failed: %s\n", strerror(errno));
  }

  if (close(client->socket_id) == -1) {
    fprintf(stderr, "ERROR: Close socket: %s\n", strerror(errno));
  }

  pthread_kill(threads[client->index], SIGINT);
}

void tkbc_server_brodcast_client(Client *client, const char *message) {

  int send_check =
      send(client->socket_id, message, strlen(message), MSG_NOSIGNAL);
  if (send_check == 0) {
    fprintf(stderr, "ERROR no bytes where send to the client: " CLIENT_FMT "\n",
            CLIENT_ARG(*client));
  }
  if (send_check == -1) {
    fprintf(stderr,
            "ERROR: Client: " CLIENT_FMT ": Could not broadcast message: %s\n",
            CLIENT_ARG(*client), message);
    fprintf(stderr, "ERROR: %s\n", strerror(errno));

    tkbc_server_shutdown_client(client);
  } else {
    fprintf(stderr, "INFO: The amount %d send to: " CLIENT_FMT "\n", send_check,
            CLIENT_ARG(*client));
  }
}

void tkbc_server_brodcast_all(const char *message) {
  for (size_t i = 0; i < clients->count; ++i) {
    tkbc_server_brodcast_client(&clients->elements[i], message);
  }
}

void tkbc_message_kiteadd(size_t client_index, Kite_State *state) {
  Message message = {0};
  char buf[64] = {0};

  snprintf(buf, sizeof(buf), "%d", MESSAGE_KITEADD);
  tkbc_dapc(&message, buf, strlen(buf));
  tkbc_dap(&message, ':');

  memset(buf, 0, sizeof(buf));
  snprintf(buf, sizeof(buf), "%zu", client_index);
  tkbc_dapc(&message, buf, strlen(buf));

  tkbc_dap(&message, ':');

  memset(buf, 0, sizeof(buf));
  snprintf(buf, sizeof(buf), "%u", *((uint32_t *)&state->kite->body_color));
  tkbc_dapc(&message, buf, strlen(buf));
  tkbc_dap(&message, ':');
  tkbc_dapc(&message, "\r\n", 2);

  tkbc_dap(&message, 0);
  tkbc_server_brodcast_all(message.elements);
  free(message.elements);
}

void tkbc_message_hello(Client *client) {
  Message message = {0};
  char buf[64] = {0};

  snprintf(buf, sizeof(buf), "%d", MESSAGE_HELLO);
  tkbc_dapc(&message, buf, strlen(buf));
  tkbc_dap(&message, ':');

  const char *m = "Hello client from server!";
  tkbc_dapc(&message, m, strlen(m));

  tkbc_dapc(&message, PROTOCOL_VERSION, strlen(PROTOCOL_VERSION));
  tkbc_dap(&message, ':');
  tkbc_dapc(&message, "\r\n", 2);

  tkbc_dap(&message, 0);
  tkbc_server_brodcast_client(client, message.elements);
  free(message.elements);
}

bool tkbc_server_remove_client_from_list(Client *client) {
  int ok = pthread_mutex_lock(&mutex);
  if (ok != 0) {
    assert(0 && "ERROR:mutex lock");
  }

  // Clients cls = {0};
  for (size_t i = 0; i < clients->count; ++i) {
    if (client->index == clients->elements[i].index) {

      memmove(&clients->elements[i], &clients->elements[i + 1],
              sizeof(*client) * clients->count - i - 1);
      clients->count -= 1;

      ok = pthread_mutex_unlock(&mutex);
      if (ok != 0) {
        assert(0 && "ERROR:mutex unlock");
      }
      return true;
    }
  }
  ok = pthread_mutex_unlock(&mutex);
  if (ok != 0) {
    assert(0 && "ERROR:mutex unlock");
  }
  return false;
}

void *tkbc_client_handler(void *client) {
  // TODO: Check that clients can't have the same id as someone before. In this
  // case the messages that are broadcasted to all clients will only be
  // partially be distributed.
  Client *c = (Client *)client;
  int ok;
  signal(SIGABRT, signal_int);
  signal(SIGINT, signal_int);
  signal(SIGTERM, signal_int);

  tkbc_message_hello(c);

  Color color_array[] = {BLUE, PURPLE, GREEN, RED, TEAL};
  Kite_State *kite_state = tkbc_init_kite();

  ok = pthread_mutex_lock(&mutex);
  if (ok != 0) {
    assert(0 && "ERROR:mutex lock");
  }
  tkbc_dap(env->kite_array, *kite_state);
  Index index = env->kite_array->count - 1;
  env->kite_array->elements[index].kite_id = c->index;
  env->kite_array->elements[index].kite->body_color =
      color_array[index % ARRAY_LENGTH(color_array)];
  ok = pthread_mutex_unlock(&mutex);
  if (ok != 0) {
    assert(0 && "ERROR:mutex unlock");
  }

  tkbc_message_kiteadd(c->index, kite_state);

  fprintf(stderr, "INFO: Connection from host %s, port %hd\n",
          inet_ntoa(c->client_address.sin_addr),
          ntohs(c->client_address.sin_port));

  for (;;) {

    {
      char message[1024];
      memset(message, 0, sizeof(message));
      int message_ckeck =
          recv(c->socket_id, message, sizeof(message) - 1, MSG_NOSIGNAL);
      if (message_ckeck == -1) {
        fprintf(stderr, "ERROR: RECV: %s\n", strerror(errno));
        break;
      }

      assert((unsigned int)message_ckeck < sizeof(message) &&
             "Message buffer is to big.");
      message[message_ckeck] = '\0';
      if (message_ckeck >= 0) {
        send(1, message, strlen(message), 0);
      }

      if (strcmp(message, "quit\n") == 0) {
        break;
      }
    }

    // TODO: Handle other messages.
  }

  tkbc_server_shutdown_client(client);
  return NULL;
}
