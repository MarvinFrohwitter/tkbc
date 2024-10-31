#ifndef TKBC_NETWORK_COMMON_H
#define TKBC_NETWORK_COMMON_H

#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
  int index;

  int socket_id;
  struct sockaddr_in client_address;
  socklen_t client_address_length;

  bool connected;
} Client;

typedef struct {
  Client *elements;
  size_t count;
  size_t capacity;
} Clients;

uint16_t tkbc_port_parsing(const char *port_check);

#endif // TKBC_NETWORK_COMMON_H
