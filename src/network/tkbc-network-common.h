#ifndef TKBC_NETWORK_COMMON_H
#define TKBC_NETWORK_COMMON_H

#include "../../external/lexer/tkbc-lexer.h"

#include <netinet/in.h>
#include <raylib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define PROTOCOL_VERSION "0.2"

#define TKBC_LOGGING
#define TKBC_LOGGING_ERROR
#define TKBC_LOGGING_INFO
#define TKBC_LOGGING_WARNING

typedef struct {
  ssize_t kite_id;

  size_t thread_id;
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

// name : kind : data
typedef enum {
  MESSAGE_ZERO = 0,
  MESSAGE_HELLO,
  MESSAGE_KITEADD,
  MESSAGE_CLIENT_DISCONNECT,
  MESSAGE_CLIENTKITES,
  MESSAGE_KITES,
  MESSAGE_KITES_POSITIONS,
  MESSAGE_KITEVALUE,
  MESSAGE_SCRIPT,
  MESSAGE_SCRIPT_BLOCK_FRAME_VALUE,
  MESSAGE_SCRIPT_TOGGLE,
  MESSAGE_SCRIPT_NEXT,
  MESSAGE_SCRIPT_SCRUB,
  MESSAGE_COUNT,
} Message_Kind; // Messages that are supported in the current PROTOCOL_VERSION.

uint16_t tkbc_port_parsing(const char *port_check);
bool tkbc_message_append_clientkite(size_t client_id, Message *message);
bool tkbc_parse_message_kite_value(Lexer *lexer, size_t *kite_id, float *x,
                                   float *y, float *angle, Color *color);

#endif // TKBC_NETWORK_COMMON_H
