#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
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
static int server_socket;
static int clients_visited = 0;

typedef struct {
  char *elements;
  size_t count;
  size_t capacity;
  size_t i;
} Message;

typedef struct {
  struct pollfd *elements;
  size_t count;
  size_t capacity;
} FDs;

typedef struct {
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

void tkbc_remove_fd_from_fds_by_index(FDs *fds, int idx) {
  assert(fds != NULL);
  typeof(fds->elements[idx]) pollfd_tmp = fds->elements[idx];
  fds->elements[idx] = fds->elements[fds->count - 1];
  fds->elements[fds->count - 1] = pollfd_tmp;
  fds->count -= 1;
}

bool tkbc_remove_client_by_fd(Clients *clients, int fd) {
  if (!clients) {
    return false;
  }
  for (size_t i = 0; i < clients->count; ++i) {
    if (clients->elements[i].socket_id == fd) {
      Client client_tmp = clients->elements[i];
      free(client_tmp.send_msg_buffer.elements);
      tkbc_fprintf(stderr, "INFO ", "Removed client:" CLIENT_FMT "\n",
                   CLIENT_ARG(client_tmp));

      clients->elements[i] = clients->elements[clients->count - 1];
      clients->elements[clients->count - 1] = client_tmp;
      clients->count -= 1;
      return true;
    }
  }
  return false;
}

void tkbc_server_accept(FDs *fds, Clients *clients) {

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
    int flags = fcntl(server_socket, F_GETFL, 0);
    fcntl(server_socket, F_SETFL, flags | O_NONBLOCK);

    clients_visited++;
    struct pollfd client_fd = {
        .fd = client_socket_id,
        .events = POLLRDNORM,
        .revents = 0,
    };
    // This can cause reallocation so it is important to iterate the fds by
    // index.
    tkbc_dap(fds, client_fd);

    Client client = {
        .socket_id = client_socket_id,
        .client_address = client_address,
        .client_address_length = address_length,
    };
    tkbc_fprintf(stderr, "INFO", "CLIENT: " CLIENT_FMT " has connected.\n",
                 CLIENT_ARG(client));
    tkbc_dap(clients, client);
  }
}

struct pollfd *tkbc_get_pollfd_by_fd(FDs *fds, int fd) {
  for (size_t i = 0; i < fds->count; ++i) {
    if (fds->elements[i].fd == fd) {
      return &fds->elements[i];
    }
  }
  return NULL;
}

Client *tkbc_get_client_by_fd(Clients *clients, int fd) {
  for (size_t i = 0; i < clients->count; ++i) {
    if (clients->elements[i].socket_id == fd) {
      return &clients->elements[i];
    }
  }
  return NULL;
}

bool tkbc_sockets_read(Client *client) {
  char buf[1024] = {0};
  int recv_n = recv(client->socket_id, buf, sizeof(buf) - 1, 0);
  if (recv_n < 0) {
    if (errno != EAGAIN) {
      tkbc_fprintf(stderr, "ERROR", "Read: %s\n", strerror(errno));
      return false;
    }
  }
  if (recv_n == 0) {
    return false;
  }

  tkbc_dapc(&client->recv_msg_buffer, buf, strlen(buf));
  return true;
}

int tkbc_socket_write(Client *client) {
  size_t amount =
      (client->send_msg_buffer.count - client->send_msg_buffer.i) % 1024;
  ssize_t n = send(client->socket_id,
                   client->send_msg_buffer.elements + client->send_msg_buffer.i,
                   amount, MSG_NOSIGNAL);

  if (n < 0) {
    if (errno != EAGAIN) {
      tkbc_fprintf(stderr, "ERROR", "Send: %s\n", strerror(errno));
      return -1;
    }
    if (n == 0) {
      tkbc_fprintf(stderr, "ERROR", "No byte where send to:" CLIENT_FMT "\n",
                   CLIENT_ARG(*client));
    }
  }

  if ((size_t)n == client->send_msg_buffer.count) {
    client->send_msg_buffer.count = 0;
    client->send_msg_buffer.i = 0;
  } else {
    client->send_msg_buffer.i += n;
  }
  return n;
}

void tkbc_write_to_all_send_msg_buffers_except(FDs *fds, Clients *clients,
                                               int fd, Message message) {
  for (size_t i = 0; i < clients->count; ++i) {
    if (clients->elements[i].socket_id != fd) {
      tkbc_dapc(&clients->elements[i].send_msg_buffer, message.elements,
                message.count);
      tkbc_get_pollfd_by_fd(fds, clients->elements[i].socket_id)->events =
          POLLWRNORM;
    }
  }
}

bool tkbc_server_handle_clients(FDs *fds, Clients *clients, size_t idx) {
  Client *client = tkbc_get_client_by_fd(clients, fds->elements[idx].fd);
  switch (fds->elements[idx].events) {
  case POLLRDNORM: {
    bool result = tkbc_sockets_read(client);
    // Switch to the next write state.
    fds->elements[idx].events = POLLWRNORM;
    return result;
  } break;
  case POLLWRNORM: {
    if (client->send_msg_buffer.count <= 0) {
      fds->elements[idx].events = POLLRDNORM;
      return true;
    }

    int result = tkbc_socket_write(client);
    if (result == 0) {
      fds->elements[idx].events = POLLRDNORM;
    } else if (result == -1) {
      return false;
    }
    return true;
  } break;
  default:
    assert(0 && "UNKNOWN EVENT");
  }
}

void tkbc_socket_handling(Clients *clients, FDs *fds) {
  for (ssize_t idx = 0; idx < (ssize_t)fds->count; ++idx) {
    if (fds->elements[idx].revents == 0) {
      continue;
    }

    if (fds->elements[idx].fd == server_socket) {
      // This can cause realloc so it is important to iterate the fds by
      // index.
      tkbc_server_accept(fds, clients);
    } else {
      bool result = tkbc_server_handle_clients(fds, clients, idx);
      if (!result) {
        if (!tkbc_remove_client_by_fd(clients, fds->elements[idx].fd)) {
          assert(0 && "Could not remove client");
        }
        tkbc_remove_fd_from_fds_by_index(fds, idx);
        idx--;
        continue;
      }
    }
  }
}

int main(int argc, char *argv[]) {
#ifndef _WIN32
  struct sigaction sig_action = {0};
  sig_action.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &sig_action, NULL);
#endif // _WIN32
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

  FDs fds = {0};
  struct pollfd server_fd = {
      .fd = server_socket,
      .events = POLLRDNORM,
      .revents = 0,
  };
  tkbc_dap(&fds, server_fd);

  for (;;) {
    if (clients_visited >= SERVER_CONNETCTIONS) {
      break;
    }

    int timeout = -1; // Infinite
    int poll_err = poll(fds.elements, fds.count, timeout);
    if (poll_err == -1) {
      tkbc_fprintf(stderr, "ERROR", "The poll has failed:%s\n",
                   strerror(errno));
      break;
    }

    if (poll_err > 0) {
      tkbc_socket_handling(&clients, &fds);
    }

    // Handle messages
    for (size_t i = 0; i < clients.count; ++i) {
      Client *client = &clients.elements[i];
      if (client->recv_msg_buffer.count) {
        tkbc_fprintf(stderr, "MESSAGE", "%.*s",
                     (int)client->recv_msg_buffer.count,
                     client->recv_msg_buffer.elements);
      }
      if (client->recv_msg_buffer.count) {
        tkbc_write_to_all_send_msg_buffers_except(
            &fds, &clients, client->socket_id, client->recv_msg_buffer);
        client->recv_msg_buffer.count = 0;
      }
    }
  }

  tkbc_fprintf(stderr, "INFO", "Closing...\n");
  shutdown(server_socket, SHUT_RDWR);
  if (close(server_socket) == -1) {
    tkbc_fprintf(stderr, "ERROR", "Main Server Socket: %s\n", strerror(errno));
  }

  for (size_t i = 0; i < clients.count; ++i) {
    free(clients.elements[i].send_msg_buffer.elements);
    free(clients.elements[i].recv_msg_buffer.elements);
  }
  free(clients.elements);
  free(fds.elements);
  exit(EXIT_SUCCESS);
  return 0;
}
