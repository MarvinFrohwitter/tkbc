#include "tkbc-network-common.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void *client_handler(void *client) {
  Client *c = (Client *)client;

  printf("Connection from host %s, port %hd\n",
         inet_ntoa(c->client_address.sin_addr),
         ntohs(c->client_address.sin_port));

  for (;;) {
    if (!c->connected) {
      c->connected = true;
      char *hello_msg = "Hello client from server!\n";
      int send_check = send(c->socket_id, hello_msg, strlen(hello_msg), 0);
      if (send_check == -1) {
        fprintf(stderr,
                "ERROR: Hello Message could not be send to client %d.\n",
                c->socket_id);
      }
    }

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

      if (strcmp(message, "quit") == 0) {
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
