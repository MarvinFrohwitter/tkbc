#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define TKBC_LOGGING
#define TKBC_LOGGING_ERROR
#define TKBC_LOGGING_INFO
#define TKBC_LOGGING_WARNING

#define TKBC_SERVER
#define TKBC_UTILS_IMPLEMENTATION
#include "../global/tkbc-utils.h"
Env *env;

#define CLIENT_FMT "Socket: %d, Address: (%s:%hu)"
#define CLIENT_ARG(c)                                                          \
  ((c).socket_id), (inet_ntoa((c).client_address.sin_addr)),                   \
      (ntohs((c).client_address.sin_port))

#define SERVER_CONNETCTIONS 64
int server_socket;

typedef struct {
  char *elements;
  size_t count;
  size_t capacity;
} Message;

typedef struct {
  Message msg_buffer;
  int socket_id;
  struct sockaddr_in client_address;
  socklen_t client_address_length;
} Client;

typedef struct {
  Client *elements;
  size_t count;
  size_t capacity;
} Clients;

/**
 * @brief The function prints the way the program should be called.
 *
 * @param program_name The name of the program that is currently executing.
 */
void tkbc_server_usage(const char *program_name) {
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
bool tkbc_server_commandline_check(int argc, const char *program_name) {
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
uint16_t tkbc_port_parsing(const char *port_check) {
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
int tkbc_server_socket_creation(uint32_t addr, uint16_t port) {
  int socket_id = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_id == -1) {
    tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
    exit(1);
  }
  int option = 1;
  int sso = setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, (char *)&option,
                       sizeof(option));
  if (sso == -1) {
    tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = addr;

  int bind_status =
      bind(socket_id, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (bind_status == -1) {
    tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
    exit(1);
  }

  int listen_status = listen(socket_id, SERVER_CONNETCTIONS);
  if (listen_status == -1) {
    tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
    exit(1);
  }
  tkbc_fprintf(stderr, "INFO", "%s: %hu\n", "Listening to port", port);

  return socket_id;
}

int main(int argc, char *argv[]) {
  tkbc_fprintf(stderr, "INFO", "%s\n", "The server has started.");

  char *program_name = tkbc_shift_args(&argc, &argv);
  uint16_t port = 8080;
  if (tkbc_server_commandline_check(argc, program_name)) {
    char *port_check = tkbc_shift_args(&argc, &argv);
    port = tkbc_port_parsing(port_check);
  }
  server_socket = tkbc_server_socket_creation(INADDR_ANY, port);
  tkbc_fprintf(stderr, "INFO", "%s: %d\n", "Server socket", server_socket);

  // Set the socket to non-blocking
  int flags = fcntl(server_socket, F_GETFL, 0);
  fcntl(server_socket, F_SETFL, flags | O_NONBLOCK);

  Clients clients = {0};
  int clients_visited = 0;
  for (;;) {
    if (clients_visited >= SERVER_CONNETCTIONS) {
      break;
    }
    struct sockaddr_in client_address;
    socklen_t address_length = sizeof(client_address);
    int client_socket_id = accept(
        server_socket, (struct sockaddr *)&client_address, &address_length);

    if (client_socket_id == -1) {
      if (errno != EAGAIN) {
        tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
        assert(0 && "accept error");
      }
    } else {
      // Set the socket to non-blocking
      flags = fcntl(client_socket_id, F_GETFL, 0);
      fcntl(client_socket_id, F_SETFL, flags | O_NONBLOCK);

      clients_visited++;
      Client client = {
          .socket_id = client_socket_id,
          .client_address = client_address,
          .client_address_length = address_length,
      };
      tkbc_fprintf(stderr, "INFO", "CLIENT: " CLIENT_FMT " has connected.\n",
                   CLIENT_ARG(client));
      tkbc_dap(&clients, client);
    }

    for (size_t i = 0; i < clients.count; ++i) {
      char buf[1024] = {0};
      int recv_n = recv(clients.elements[i].socket_id, buf, sizeof(buf) - 1, 0);
      if (recv_n < 0) {
        if (errno != EAGAIN) {
          tkbc_fprintf(stderr, "ERROR", "Read: %s\n", strerror(errno));
          return false;
        }
      }
      if (recv_n > 0) {
        printf("%s", buf);
        for (size_t id = 0; id < clients.count; ++id) {
          int send_n =
              send(clients.elements[id].socket_id, buf, sizeof(buf), 0);
          if (send_n < 0) {
            if (errno != EAGAIN) {
              tkbc_fprintf(stderr, "ERROR", "Read: %s\n", strerror(errno));
            }
          }
          if (send_n == 0) {
            tkbc_fprintf(stderr, "ERROR",
                         "No byte where send to:" CLIENT_FMT "\n",
                         CLIENT_ARG(clients.elements[id]));
          }
        }
      }
    }
  }

  tkbc_fprintf(stderr, "INFO", "Closing...\n");
  shutdown(server_socket, SHUT_RDWR);
  if (close(server_socket) == -1) {
    tkbc_fprintf(stderr, "ERROR", "Main Server Socket: %s\n", strerror(errno));
  }

  free(clients.elements);
  exit(EXIT_SUCCESS);
  return 0;
}
