#ifndef TKBC_NETWORK_COMMON_H
#define TKBC_NETWORK_COMMON_H

#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
  size_t index;

  int socket_id;
  struct sockaddr_in client_address;
  socklen_t client_address_length;
} Client;

typedef struct {
  Client *elements;
  size_t count;
  size_t capacity;
} Clients;

typedef struct {
  char *elements;
  size_t count;
  size_t capacity;
} Message;

typedef struct {
  Message *elements;
  size_t count;
  size_t capacity;
} Messages;

// name : kind : data
typedef enum {
  MESSAGE_ZERO,
  MESSAGE_HELLO,
  MESSAGE_KITEADD,
  MESSAGE_CLIENTKITES,
  MESSAGE_COUNT,
} Message_Kind;

uint16_t tkbc_port_parsing(const char *port_check);

#endif // TKBC_NETWORK_COMMON_H
