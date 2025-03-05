#ifndef TKBC_NETWORK_COMMON_H
#define TKBC_NETWORK_COMMON_H

#include "tkbc-servers-common.h"
#include "../../external/lexer/tkbc-lexer.h"

#include <raylib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WINUSER_
#define _WINGDI_
#define _IMM_
#define _WINCON_
#include <windows.h>
#include <winsock2.h>

#define SHUT_WR SD_SEND
#define SHUT_RDWR SD_BOTH
typedef int SOCKLEN;

#else
#include <netinet/in.h>
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef socklen_t SOCKLEN;
#endif //_WIN32

typedef struct {
  ssize_t kite_id;

  size_t thread_id;
  int socket_id;
  SOCKADDR_IN client_address;
  SOCKLEN client_address_length;
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


uint16_t tkbc_port_parsing(const char *port_check);
bool tkbc_message_append_clientkite(size_t client_id, Message *message);
bool tkbc_parse_message_kite_value(Lexer *lexer, size_t *kite_id, float *x,
                                   float *y, float *angle, Color *color);

#endif // TKBC_NETWORK_COMMON_H
