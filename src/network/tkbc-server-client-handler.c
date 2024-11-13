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

void tkbc_server_brodcast_client(Client *client, const char *message) {
  int send_check = send(client->socket_id, message, strlen(message), 0);
  if (send_check == -1) {
    fprintf(stderr, "ERROR: Client: %zu: Could broadcast message: %s.\n",
            client->index, message);
    fprintf(stderr, "ERROR: %s\n", strerror(errno));
  } else {
    fprintf(stderr, "The amount send: %d\n", send_check);
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
  tkbc_dap(&message, ':');
  tkbc_dapc(&message, "\r\n", 2);

  tkbc_dap(&message, 0);
  tkbc_server_brodcast_client(client, message.elements);
  free(message.elements);
}
bool tkbc_server_remove_client(Client *client) {
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

      // tkbc_dapc(&cls, &clients->elements[0], i);
      // tkbc_dapc(&cls, &clients->elements[i + 1], clients->count - i - 1);
      // free(clients->elements);
      // clients = &cls;

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
  Client *c = (Client *)client;
  int ok;
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

  printf("Connection from host %s, port %hd\n",
         inet_ntoa(c->client_address.sin_addr),
         ntohs(c->client_address.sin_port));

  for (;;) {

    {
      char message[1024];
      memset(message, 0, sizeof(message));
      int message_ckeck = recv(c->socket_id, message, sizeof(message) - 1, 0);
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
        close(c->socket_id);
        return NULL;
      }
    }

    // TODO: Handle other messages.
  }

  close(c->socket_id);
  {
    bool ok = tkbc_server_remove_client(c);
    if (!ok) {
      fprintf(stderr, "ERROR: Client %zu could not be removed.\n", c->index);
    }
  }

  return NULL;
}
