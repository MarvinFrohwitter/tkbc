#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "tkbc-servers-common.h"

#define TKBC_UTILS_IMPLEMENTATION
#include "../global/tkbc-utils.h"

#include "../choreographer/tkbc-script-api.h"
#include "../choreographer/tkbc.h"

typedef struct {
  struct pollfd *elements;
  size_t count;
  size_t capacity;
} FDs;

static int server_socket;
static int clients_visited = 0;
Env *env = {0};
Clients clients = {0};
FDs fds = {0};

bool tkbc_remove_client_by_fd(int fd) {
  for (size_t i = 0; i < clients.count; ++i) {
    if (clients.elements[i].socket_id == fd) {
      Client client_tmp = clients.elements[i];
      free(client_tmp.send_msg_buffer.elements);
      tkbc_fprintf(stderr, "INFO", "Removed client:" CLIENT_FMT "\n",
                   CLIENT_ARG(client_tmp));

      clients.elements[i] = clients.elements[clients.count - 1];
      clients.elements[clients.count - 1] = client_tmp;
      clients.count -= 1;
      return true;
    }
  }
  return false;
}

int tkbc_remove_connection(Client client, bool retry) {
  if (!retry) {
    if (!tkbc_remove_kite_from_list(env->kite_array, client.kite_id)) {
      tkbc_fprintf(stderr, "INFO",
                   "Client:" CLIENT_FMT
                   ":Kite could not be removed: not found.\n",
                   CLIENT_ARG(client));
      return 1;
    }
  }

  if (!tkbc_remove_client_by_fd(client.socket_id)) {
    tkbc_fprintf(stderr, "INFO",
                 "Client:" CLIENT_FMT ": Fd could not be removed.\n",
                 CLIENT_ARG(client));
    return -1;
  }

  for (size_t i = 0; i < fds.count; ++i) {
    if (fds.elements[i].fd == client.socket_id) {
      typeof(fds.elements[i]) pollfd_tmp = fds.elements[i];
      fds.elements[i] = fds.elements[fds.count - 1];
      fds.elements[fds.count - 1] = pollfd_tmp;
      fds.count -= 1;
      break;
    }
  }

  return 0;
}

void tkbc_remove_connection_retry(Client client) {
  if (1 == tkbc_remove_connection(client, false)) {
    if (0 != tkbc_remove_connection(client, true)) {
    }
  }
}

void tkbc_server_accept() {

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
    tkbc_dap(&fds, client_fd);

    Client client = {
        .kite_id = env->kite_id_counter++,
        .socket_id = client_socket_id,
        .client_address = client_address,
        .client_address_length = address_length,
    };
    tkbc_fprintf(stderr, "INFO", "CLIENT: " CLIENT_FMT " has connected.\n",
                 CLIENT_ARG(client));
    tkbc_dap(&clients, client);
  }
}

struct pollfd *tkbc_get_pollfd_by_fd(int fd) {
  for (size_t i = 0; i < fds.count; ++i) {
    if (fds.elements[i].fd == fd) {
      return &fds.elements[i];
    }
  }
  return NULL;
}

Client *tkbc_get_client_by_fd(int fd) {
  for (size_t i = 0; i < clients.count; ++i) {
    if (clients.elements[i].socket_id == fd) {
      return &clients.elements[i];
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

void tkbc_write_to_all_send_msg_buffers(Message message) {
  for (size_t i = 0; i < clients.count; ++i) {
    tkbc_dapc(&clients.elements[i].send_msg_buffer, message.elements,
              message.count);
    tkbc_get_pollfd_by_fd(clients.elements[i].socket_id)->events = POLLWRNORM;
  }
}

void tkbc_write_to_all_send_msg_buffers_except(Message message, int fd) {
  for (size_t i = 0; i < clients.count; ++i) {
    if (clients.elements[i].socket_id != fd) {
      tkbc_dapc(&clients.elements[i].send_msg_buffer, message.elements,
                message.count);
      tkbc_get_pollfd_by_fd(clients.elements[i].socket_id)->events = POLLWRNORM;
    }
  }
}

bool tkbc_server_handle_clients(Client *client) {
  struct pollfd *pollfd = tkbc_get_pollfd_by_fd(client->socket_id);
  switch (pollfd->events) {
  // switch (fds.elements[idx].events) {
  case POLLRDNORM: {
    bool result = tkbc_sockets_read(client);
    // Switch to the next write state.
    pollfd->events = POLLWRNORM;
    return result;
  } break;
  case POLLWRNORM: {
    if (client->send_msg_buffer.count <= 0) {
      pollfd->events = POLLRDNORM;
      return true;
    }

    int result = tkbc_socket_write(client);
    if (result == 0) {
      pollfd->events = POLLRDNORM;
    } else if (result == -1) {
      return false;
    }
    return true;
  } break;
  default:
    assert(0 && "UNKNOWN EVENT");
  }
}

void tkbc_socket_handling() {
  for (ssize_t idx = 0; idx < (ssize_t)fds.count; ++idx) {
    if (fds.elements[idx].revents == 0) {
      continue;
    }

    if (fds.elements[idx].fd == server_socket) {
      // This can cause realloc so it is important to iterate the fds by
      // index.
      tkbc_server_accept();
    } else {
      Client *client = tkbc_get_client_by_fd(fds.elements[idx].fd);
      bool result = tkbc_server_handle_clients(client);
      if (!result) {
        tkbc_remove_connection_retry(*client);
        idx--;
        continue;
      }
    }
  }
}

/**
 * @brief The function shutdown the client connection that is given by the
 * client argument.
 *
 * @param client The client connection that should be closed.
 * @param force Represents that every action that might not shutdown the client
 * immediately is omitted.
 */
void tkbc_server_shutdown_client(Client client, bool force) {
  shutdown(client.socket_id, SHUT_WR);
  for (char b[1024]; recv(client.socket_id, b, sizeof(b), MSG_DONTWAIT) > 0;)
    ;
  if (close(client.socket_id) == -1) {
    tkbc_fprintf(stderr, "ERROR", "Close socket: %s\n", strerror(errno));
  }

  if (!force) {
    Message message = {0};
    char buf[64] = {0};
    snprintf(buf, sizeof(buf), "%d", MESSAGE_CLIENT_DISCONNECT);
    tkbc_dapc(&message, buf, strlen(buf));
    tkbc_dap(&message, ':');
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "%zu", client.kite_id);
    tkbc_dapc(&message, buf, strlen(buf));
    tkbc_dap(&message, ':');
    tkbc_dapc(&message, "\r\n", 2);
    tkbc_dap(&message, 0);

    tkbc_write_to_all_send_msg_buffers_except(message, client.socket_id);
    if (message.elements) {
      free(message.elements);
      message.elements = NULL;
    }
  }

  tkbc_remove_connection_retry(client);
}

/**
 * @brief The function constructs and sends the message CLIENTKITES to all
 * registered kites.
 *
 * @return True if the message was send successfully, otherwise false.
 */
bool tkbc_message_clientkites_write_to_all_send_msg_buffers(Clients *cs) {
  Message message = {0};
  char buf[64] = {0};
  bool ok = true;

  snprintf(buf, sizeof(buf), "%d", MESSAGE_CLIENTKITES);
  tkbc_dapc(&message, buf, strlen(buf));
  tkbc_dap(&message, ':');

  memset(buf, 0, sizeof(buf));
  snprintf(buf, sizeof(buf), "%zu", clients.count);
  tkbc_dapc(&message, buf, strlen(buf));

  tkbc_dap(&message, ':');
  for (size_t i = 0; i < clients.count; ++i) {
    if (!tkbc_message_append_clientkite(clients.elements[i].kite_id,
                                        &message)) {
      tkbc_dap(cs, clients.elements[i]);
      check_return(false);
    }
  }
  tkbc_dapc(&message, "\r\n", 2);
  tkbc_dap(&message, 0);

  tkbc_write_to_all_send_msg_buffers(message);

check:
  free(message.elements);
  message.elements = NULL;
  return ok;
}

/**
 * @brief The function is an error handler that updates all the client states
 * and handles all error that occur while broadcasting. It also checks for
 * client disconnects and shuts down the broken connection.
 */
void tkbc_unwrap_handler_message_clientkites_write_all() {
  Clients cs = {0};
  if (!tkbc_message_clientkites_write_to_all_send_msg_buffers(&cs)) {
    for (size_t i = 0; i < cs.count; ++i) {
      tkbc_server_shutdown_client(cs.elements[i], false);
    }
    free(cs.elements);
    cs.elements = NULL;
  }
}

/**
 * @brief The function constructs all the scripts that specified in a block
 * frame.
 *
 * @param script_id The id that is the current load script.
 * @param block_frame_count The amount of scripts that are available.
 * @param block_index The script index.
 */
void tkbc_message_srcipt_block_frames_value_write_to_all_send_msg_buffers(
    size_t script_id, size_t block_frame_count, size_t block_index) {
  Message message = {0};
  char buf[64] = {0};

  snprintf(buf, sizeof(buf), "%d", MESSAGE_SCRIPT_BLOCK_FRAME_VALUE);
  tkbc_dapc(&message, buf, strlen(buf));
  tkbc_dap(&message, ':');

  memset(&buf, 0, sizeof(buf));
  snprintf(buf, sizeof(buf), "%zu", script_id);
  tkbc_dapc(&message, buf, strlen(buf));
  tkbc_dap(&message, ':');
  memset(&buf, 0, sizeof(buf));
  snprintf(buf, sizeof(buf), "%zu", block_frame_count);
  tkbc_dapc(&message, buf, strlen(buf));
  tkbc_dap(&message, ':');
  memset(&buf, 0, sizeof(buf));
  snprintf(buf, sizeof(buf), "%zu", block_index);
  tkbc_dapc(&message, buf, strlen(buf));
  tkbc_dap(&message, ':');
  tkbc_dapc(&message, "\r\n", 2);
  tkbc_write_to_all_send_msg_buffers(message);

  free(message.elements);
  message.elements = NULL;
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

  srand(time(NULL));
  // Get the first 0 out of the way.
  tkbc_get_frame_time();
  env = tkbc_init_env();
  env->window_width = 1920;
  env->window_height = 1080;

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
      tkbc_socket_handling();
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
        tkbc_write_to_all_send_msg_buffers_except(client->recv_msg_buffer,
                                                  client->socket_id);
        client->recv_msg_buffer.count = 0;
      }
    }

    //
    // Base execution
    if (env->script_counter > 0) {
      if (!tkbc_script_finished(env) && env->block_frame != NULL) {
        size_t bindex = env->frames->block_index;
        tkbc_script_update_frames(env);

        if (env->frames->block_index != bindex) {
          bindex = env->frames->block_index;
          tkbc_message_srcipt_block_frames_value_write_to_all_send_msg_buffers(
              env->block_frame->script_id, env->block_frame->count, bindex);
        }
        tkbc_unwrap_handler_message_clientkites_write_all();
      }
    }
  }

  tkbc_fprintf(stderr, "INFO", "Closing...\n");

  for (size_t i = 0; i < clients.count; ++i) {
    tkbc_server_shutdown_client(clients.elements[i], true);
  }

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
