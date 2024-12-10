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

void tkbc_server_shutdown_client(Client *client) {
  shutdown(client->socket_id, SHUT_WR);
  char buf[1024] = {0};
  int n;
  do {
    n = read(client->socket_id, buf, sizeof(buf));
  } while (n > 0);

  if (n == 0) {
    fprintf(stderr,
            "INFO: Could not read any more data from the client:" CLIENT_FMT
            ".\n",
            CLIENT_ARG(*client));
  }
  if (n < 0) {
    fprintf(stderr, "ERROR: reading failed: %s\n", strerror(errno));
  }

  if (close(client->socket_id) == -1) {
    fprintf(stderr, "ERROR: Close socket: %s\n", strerror(errno));
  }

  size_t thread_id = client->index;
  if (!tkbc_server_remove_client_from_list(client)) {
    fprintf(stderr,
            "INFO: Client: " CLIENT_FMT
            ": could not be removed after broken pipe\n",
            CLIENT_ARG(*client));
  }

  pthread_cancel(threads[thread_id]);
}

bool tkbc_server_brodcast_client(Client *client, const char *message) {

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

    return false;
  } else {
    fprintf(stderr, "INFO: The amount %d send to: " CLIENT_FMT "\n", send_check,
            CLIENT_ARG(*client));
  }
  return true;
}

bool tkbc_server_brodcast_all(const char *message) {
  bool ok = true;
  for (size_t i = 0; i < clients->count; ++i) {
    if (!tkbc_server_brodcast_client(&clients->elements[i], message)) {
      ok = false;
    }
  }
  return ok;
}

bool tkbc_message_append_clientkite(size_t client_index, Message *message) {
  char buf[64];
  for (size_t i = 0; i < env->kite_array->count; ++i) {
    if (client_index == env->kite_array->elements[i].kite_id) {
      memset(buf, 0, sizeof(buf));
      snprintf(buf, sizeof(buf), "%zu", client_index);
      tkbc_dapc(message, buf, strlen(buf));

      tkbc_dap(message, ':');
      memset(buf, 0, sizeof(buf));
      float x = env->kite_array->elements[i].kite->center.x;
      float y = env->kite_array->elements[i].kite->center.y;
      float angle = env->kite_array->elements[i].kite->angle;
      snprintf(buf, sizeof(buf), "(%f,%f):%f", x, y, angle);
      tkbc_dapc(message, buf, strlen(buf));

      tkbc_dap(message, ':');
      memset(buf, 0, sizeof(buf));
      snprintf(buf, sizeof(buf), "%u",
               *(uint32_t *)&env->kite_array->elements[i].kite->body_color);
      tkbc_dapc(message, buf, strlen(buf));
      tkbc_dap(message, ':');
      return true;
    }
  }
  return false;
}

bool tkbc_message_clientkites(Client *client) {
  Message message = {0};
  char buf[64] = {0};
  bool ok = true;

  snprintf(buf, sizeof(buf), "%d", MESSAGE_CLIENTKITES);
  tkbc_dapc(&message, buf, strlen(buf));
  tkbc_dap(&message, ':');

  memset(buf, 0, sizeof(buf));
  snprintf(buf, sizeof(buf), "%zu", clients->count);
  tkbc_dapc(&message, buf, strlen(buf));

  tkbc_dap(&message, ':');
  for (size_t i = 0; i < clients->count; ++i) {
    if (!tkbc_message_append_clientkite(clients->elements[i].index, &message)) {
      check_return(false);
    }
  }
  tkbc_dapc(&message, "\r\n", 2);

  tkbc_dap(&message, 0);
  if (!tkbc_server_brodcast_client(client, message.elements)) {
    check_return(false);
  }
check:
  free(message.elements);
  return ok ? true : false;
}

bool tkbc_message_kiteadd(size_t client_index) {
  Message message = {0};
  bool ok = true;
  char buf[64] = {0};

  snprintf(buf, sizeof(buf), "%d", MESSAGE_KITEADD);
  tkbc_dapc(&message, buf, strlen(buf));
  tkbc_dap(&message, ':');
  if (!tkbc_message_append_clientkite(client_index, &message)) {
    check_return(false);
  }
  tkbc_dapc(&message, "\r\n", 2);

  tkbc_dap(&message, 0);
  if (!tkbc_server_brodcast_all(message.elements)) {
    check_return(false);
  }
check:
  free(message.elements);
  return ok ? true : false;
}

bool tkbc_message_hello(Client *client) {
  Message message = {0};
  bool ok = true;
  char buf[64] = {0};

  snprintf(buf, sizeof(buf), "%d", MESSAGE_HELLO);
  tkbc_dapc(&message, buf, strlen(buf));
  tkbc_dap(&message, ':');

  tkbc_dapc(&message, "\"", 1);
  const char *m = "Hello client from server!";
  tkbc_dapc(&message, m, strlen(m));

  tkbc_dapc(&message, PROTOCOL_VERSION, strlen(PROTOCOL_VERSION));
  tkbc_dapc(&message, "\"", 1);
  tkbc_dap(&message, ':');
  tkbc_dapc(&message, "\r\n", 2);

  tkbc_dap(&message, 0);
  if (!tkbc_server_brodcast_client(client, message.elements)) {
    check_return(false);
  }
check:
  free(message.elements);
  return ok ? true : false;
}

bool tkbc_server_remove_client_from_list(Client *client) {
  if (pthread_mutex_lock(&mutex) != 0) {
    assert(0 && "ERROR:mutex lock");
  }

  for (size_t i = 0; i < clients->count; ++i) {
    if (client->index == clients->elements[i].index) {

      // The next index does exist, even if there is just one client, because
      // the default array allocation does allocate 64 slots.
      memmove(&clients->elements[i], &clients->elements[i + 1],
              sizeof(*client) * clients->count - i - 1);
      clients->count -= 1;

      if (pthread_mutex_unlock(&mutex) != 0) {
        assert(0 && "ERROR:mutex unlock");
      }
      return true;
    }
  }
  if (pthread_mutex_unlock(&mutex) != 0) {
    assert(0 && "ERROR:mutex unlock");
  }
  return false;
}

void *tkbc_client_handler(void *client) {
  // TODO: Check that clients can't have the same id as someone before. In this
  // case the messages that are broadcasted to all clients will only be
  // partially be distributed.
  Client *c = (Client *)client;

  tkbc_message_hello(c);

  Color color_array[] = {BLUE, PURPLE, GREEN, RED, TEAL};
  Kite_State *kite_state = tkbc_init_kite();
  kite_state->kite_id = c->index;
  kite_state->kite->body_color =
      color_array[c->index % ARRAY_LENGTH(color_array)];
  Vector2 shift_pos = {.y = kite_state->kite->center.y,
                       .x = kite_state->kite->center.x + 200 * c->index};
  tkbc_center_rotation(kite_state->kite, &shift_pos, kite_state->kite->angle);

  if (pthread_mutex_lock(&mutex) != 0) {
    assert(0 && "ERROR:mutex lock");
  }
  tkbc_dap(env->kite_array, *kite_state);

  if (!tkbc_message_kiteadd(c->index)) {
    goto check;
  }
  if (!tkbc_message_clientkites(c)) {
    goto check;
  }
  if (pthread_mutex_unlock(&mutex) != 0) {
    assert(0 && "ERROR:mutex unlock");
  }

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

check:
  tkbc_server_shutdown_client(client);
  return NULL;
}
