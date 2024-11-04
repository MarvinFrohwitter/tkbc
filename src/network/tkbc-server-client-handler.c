#include "tkbc-network-common.h"

#include "../choreographer/tkbc.h"
#include "../global/tkbc-utils.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

extern Env *env;
extern Clients *clients;
extern pthread_mutex_t mutex;

void tkbc_server_brodcast_all(const char *message) {
  for (size_t i = 0; i < clients->count; ++i) {
    Client *c = &clients->elements[i];
    int send_check = send(c->socket_id, message, strlen(message), 0);
    if (send_check == -1) {
      fprintf(stderr, "ERROR: Client: %zu: Could broadcast message: %s.\n",
              c->index, message);
    }
  }
}

void tkbc_server_brodcast_client(Client *client, const char *message) {
  int send_check = send(client->socket_id, message, strlen(message), 0);
  if (send_check == -1) {
    fprintf(stderr, "ERROR: Client: %zu: Could broadcast message: %s.\n",
            client->index, message);
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

  tkbc_dap(&message, 0);
  tkbc_server_brodcast_client(client, message.elements);
  free(message.elements);
}

void *tkbc_client_handler(void *client) {
  Client *c = (Client *)client;
  tkbc_message_hello(c);

  Color color_array[] = {BLUE, GREEN, PURPLE, RED, TEAL};
  Kite_State *kite_state = tkbc_init_kite();

  pthread_mutex_lock(&mutex);
  tkbc_dap(env->kite_array, *kite_state);
  Index index = env->kite_array->count - 1;
  env->kite_array->elements[index].kite_id = c->index;
  env->kite_array->elements[index].kite->body_color =
      color_array[index % ARRAY_LENGTH(color_array)];
  pthread_mutex_unlock(&mutex);

  tkbc_message_kiteadd(c->index, kite_state);

  printf("Connection from host %s, port %hd\n",
         inet_ntoa(c->client_address.sin_addr),
         ntohs(c->client_address.sin_port));

  for (;;) {

    {
      char message[1024];
      memset(message, 0, sizeof(message));
      int message_ckeck = recv(c->socket_id, message, sizeof(message) - 1, 0);

      if (message_ckeck == -1) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        break;
      }

      assert((unsigned int)message_ckeck < sizeof(message));
      message[message_ckeck] = '\0';
      if (message_ckeck >= 0) {
        write(1, message, strlen(message));
      }

      if (strcmp(message, "quit\n") == 0) {
        close(c->socket_id);
        return NULL;
      }
    }

    // TODO: Handle other messages.
  }

  // TODO: Call the close in error situations in this function.
  close(c->socket_id);

  return NULL;
}
