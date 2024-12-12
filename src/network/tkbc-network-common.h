#ifndef TKBC_NETWORK_COMMON_H
#define TKBC_NETWORK_COMMON_H

#include "../../external/lexer/tkbc-lexer.h"

#include <netinet/in.h>
#include <raylib.h>
#include <stdbool.h>
#include <stddef.h>

#define PROTOCOL_VERSION "1.0"

typedef struct {
  ssize_t kite_id;

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
  MESSAGE_KITEVALUE,
  MESSAGE_COUNT,
} Message_Kind;

uint16_t tkbc_port_parsing(const char *port_check);
bool tkbc_message_append_clientkite(size_t client_id, Message *message);
bool tkbc_parse_message_kite_value(Lexer *lexer, size_t *kite_id, float *x,
                                   float *y, float *angle, Color *color);

#endif // TKBC_NETWORK_COMMON_H
