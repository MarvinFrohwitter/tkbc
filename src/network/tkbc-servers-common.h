#ifndef TKBC_SERVERS_COMMON_H
#define TKBC_SERVERS_COMMON_H

#include "../global/tkbc-utils.h"
#include <ctype.h>

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

//////////////////////////////////////////////////////////////////////////////
#define PROTOCOL_VERSION "0.2.002"
#define SERVER_CONNETCTIONS 64

#define TKBC_LOGGING
#define TKBC_LOGGING_ERROR
#define TKBC_LOGGING_INFO
#define TKBC_LOGGING_WARNING

#define TKBC_SERVER

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

typedef struct {
  char *elements;
  size_t count;
  size_t capacity;
  size_t i;
} Message;

typedef struct {
  size_t thread_id; // Not used for poll the polling implementation.

  ssize_t kite_id;
  Message send_msg_buffer;
  Message recv_msg_buffer;

  int socket_id;
  struct sockaddr_in client_address;
  socklen_t client_address_length;
} Client;

typedef struct {
  Client *elements;
  size_t count;
  size_t capacity;
} Clients;

#define CLIENT_FMT "Index: %zu, Socket: %d, Address: (%s:%hu)"
#define CLIENT_ARG(c)                                                          \
  ((c).kite_id), ((c).socket_id), (inet_ntoa((c).client_address.sin_addr)),    \
      (ntohs((c).client_address.sin_port))

/**
 * @brief The function prints the way the program should be called.
 *
 * @param program_name The name of the program that is currently executing.
 */
static inline void tkbc_server_usage(const char *program_name) {
  tkbc_fprintf(stderr, "INFO", "Usage:\n");
  tkbc_fprintf(stderr, "INFO", "      %s <PORT> \n", program_name);
}

/**
 * @brief The function checks if a port is given to that program.
 *
 * @param argc The commandline argument count.
 * @param program_name The name of the program that is currently executing.
 * @return True if there are enough arguments, otherwise false.
 */
static inline bool tkbc_server_commandline_check(int argc,
                                                 const char *program_name) {
  if (argc > 1) {
    tkbc_fprintf(stderr, "ERROR", "To may arguments.\n");
    tkbc_server_usage(program_name);
    exit(1);
  }
  if (argc == 0) {
    tkbc_fprintf(stderr, "ERROR", "No arguments were provided.\n");
    tkbc_fprintf(stderr, "INFO", "The default port 8080 is used.\n");
    return false;
  }
  return true;
}

/**
 * @brief The function checks if the given port is valid and if so returns the
 * port as a number. If the given string does not contain a valid port the
 * program will crash.
 *
 * @param port_check The character string that is potently a port.
 * @return The parsed port as a uint16_t.
 */
static inline uint16_t tkbc_port_parsing(const char *port_check) {
  for (size_t i = 0; i < strlen(port_check); ++i) {
    if (!isdigit(port_check[i])) {
      tkbc_fprintf(stderr, "ERROR", "The given port [%s] is not valid.\n",
                   port_check);
      exit(1);
    }
  }
  int port = atoi(port_check);
  if (port >= 65535 || port <= 0) {
    tkbc_fprintf(stderr, "ERROR", "The given port [%s] is not valid.\n",
                 port_check);
    exit(1);
  }

  return (uint16_t)port;
}

/**
 * @brief The function creates a new server socket and sets up the bind and
 * listing.
 *
 * @param addr The address space that the socket should be bound to.
 * @param port The port where the server is listing for connections.
 * @return The newly creates socket id.
 */
static inline int tkbc_server_socket_creation(uint32_t addr, uint16_t port) {
#ifdef _WIN32
  // MAKEWORD(2, 2) is a version, and wsaData will be filled with initialized
  // library information.

  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    assert(0 && "ERROR: WSAStartup()");
  } else {
    tkbc_fprintf(stderr, "INFO", "Initialization of WSAStartup() succeed.\n");
  }
#endif

  int socket_id = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_id == -1) {
#ifdef _WIN32
    tkbc_fprintf(stderr, "ERROR", "%ld\n", WSAGetLastError());
#else
    tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
#endif
    exit(1);
  }
  int option = 1;
  int sso = setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, (char *)&option,
                       sizeof(option));
  if (sso == -1) {
#ifdef _WIN32
    tkbc_fprintf(stderr, "ERROR", "%ld\n", WSAGetLastError());
#else
    tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
#endif
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = addr;

  int bind_status =
      bind(socket_id, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (bind_status == -1) {
#ifdef _WIN32
    tkbc_fprintf(stderr, "ERROR", "%ld\n", WSAGetLastError());
    WSACleanup();
#else
    tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
#endif
    exit(1);
  }

  int listen_status = listen(socket_id, SERVER_CONNETCTIONS);
  if (listen_status == -1) {
#ifdef _WIN32
    tkbc_fprintf(stderr, "ERROR", "%ld\n", WSAGetLastError());
    WSACleanup();
#else
    tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
#endif
    exit(1);
  }
  tkbc_fprintf(stderr, "INFO", "%s: %hu\n", "Listening to port", port);

  return socket_id;
}

#endif // TKBC_SERVERS_COMMON_H
