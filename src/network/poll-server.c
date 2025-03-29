#define TKBC_SERVER
#include "poll-server.h"

#include <assert.h>
#include <string.h>
#include <unistd.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WINUSER_
#define _WINGDI_
#define _IMM_
#define _WINCON_
#include <windows.h>
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <sys/socket.h>
#endif

#include "tkbc-network-common.h"
#include "tkbc-servers-common.h"

#define TKBC_UTILS_IMPLEMENTATION
#include "../global/tkbc-utils.h"

#include "../choreographer/tkbc-script-api.h"
#include "../choreographer/tkbc-script-handler.h"
#include "../choreographer/tkbc.h"

#define MAX_BUFFER_CAPACITY 128 * 1024
static int server_socket;
static int clients_visited = 0;
Env *env = {0};
Clients clients = {0};
FDs fds = {0};

/**
 * @brief  The function can be used to get the pollfd structure that corresponds
 * to the given file descriptor.
 *
 * @param fd The file descriptor for that the corresponding pollfd structure is
 * returned.
 * @return The found pollfd structure or NULL if the fd is not a valid
 * registered one.
 */
struct pollfd *tkbc_get_pollfd_by_fd(int fd) {
  for (size_t i = 0; i < fds.count; ++i) {
    if (fds.elements[i].fd == fd) {
      return &fds.elements[i];
    }
  }
  return NULL;
}

/**
 * @brief The function can be used to get the Client structure that corresponds
 * to the given kite_id.
 *
 * @param kite_id The id that represents a kite.
 * @return The found Client if it is available, otherwise NULL.
 */
Client *tkbc_get_client_by_kite_id(int kite_id) {
  for (size_t i = 0; i < clients.count; ++i) {
    if (clients.elements[i].kite_id == kite_id) {
      return &clients.elements[i];
    }
  }
  return NULL;
}

/**
 * @brief The function can be used to get the Client structure that corresponds
 * to the given file descriptor.
 *
 * @param fd The file descriptor that is stored for the client.
 * @return The found Client if it is available, otherwise NULL.
 */
Client *tkbc_get_client_by_fd(int fd) {
  for (size_t i = 0; i < clients.count; ++i) {
    if (clients.elements[i].socket_id == fd) {
      return &clients.elements[i];
    }
  }
  return NULL;
}

/**
 * @brief The function appends the given message to the client send messages
 * buffer that later is send in a batch to the client.
 *
 * @param client The client where the message should be send to.
 * @param message The message that should be send to the given client.
 */
void tkbc_write_to_send_msg_buffer(Client *client, Message message) {
  tkbc_dapc(&client->send_msg_buffer, message.elements, message.count);
  tkbc_get_pollfd_by_fd(client->socket_id)->events = POLLWRNORM;
}

/**
 * @brief The function appends the given message to all the registered clients
 * send messages buffers that later are send in a batch to the clients.
 *
 * @param message The message that should be send to the given clients.
 */
void tkbc_write_to_all_send_msg_buffers(Message message) {
  for (size_t i = 0; i < clients.count; ++i) {
    tkbc_write_to_send_msg_buffer(&clients.elements[i], message);
  }
}

/**
 * @brief The function appends the given message to all the registered clients
 * send message buffers except the one given by the fd.
 *
 * @param message The message that should be send to the clients except the one
 * specified with the fd..
 * @param fd The file descriptor where the message should not be send to.
 */
void tkbc_write_to_all_send_msg_buffers_except(Message message, int fd) {
  for (size_t i = 0; i < clients.count; ++i) {
    if (clients.elements[i].socket_id != fd) {
      tkbc_write_to_send_msg_buffer(&clients.elements[i], message);
    }
  }
}

/**
 * @brief The function can be used to remove a client specified by the fd from
 * the server clients list.
 *
 * @param fd The file descriptor of the client that should be removed from the
 * server.
 * @return True if the client was found and removed, otherwise the client was
 * not found and false is returned
 */
bool tkbc_remove_client_by_fd(int fd) {
  for (size_t i = 0; i < clients.count; ++i) {
    if (clients.elements[i].socket_id == fd) {
      Client client_tmp = clients.elements[i];
      free(client_tmp.send_msg_buffer.elements);
      free(client_tmp.recv_msg_buffer.elements);
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

/**
 * @brief The function can be used to remove a client specified by the fd from
 * the server clients list and it also tries to remove the corresponding kite.
 *
 * @param client The client that should be disconnected from the server.
 * @param retry The boolean indicator if the client should retry to disconnect
 * the client from the server.
 * @return 0 If the client kite has been removed from the list and the server
 * hat disconnected the client socket connection. 1 if the client kite count not
 * be removed, in this case the function can be recalled with the retry option
 * on that will then try to remove the network connection. -1 is returned if tue
 * removal of the network connection hat failed in that case the kite was
 * removed if no retry is on, if retry is on the kite list will be changed.
 */
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

/**
 * @brief The function is a wrapper around tkbc_remove_connection() that
 * automatically retries the disconnection for the client if first attempted
 * failed. For more information check the documentation for
 * tkbc_remove_connection().
 *
 * @param client The client that should be removed from the server.
 */
void tkbc_remove_connection_retry(Client client) {
  if (1 == tkbc_remove_connection(client, false)) {
    if (0 != tkbc_remove_connection(client, true)) {
      tkbc_fprintf(stderr, "ERROR",
                   "Could not remove Client from list:" CLIENT_FMT "\n",
                   CLIENT_ARG(client));
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
  for (char b[1024]; recv(client.socket_id, b, sizeof(b), 0) > 0;)
    ;

#ifdef _WIN32
  if (closesocket(client.socket_id) == -1) {

    tkbc_fprintf(stderr, "ERROR", "Could not close socket: %d\n",
                 WSAGetLastError());
  }
  WSACleanup();
#else
  if (close(client.socket_id) == -1) {
    tkbc_fprintf(stderr, "ERROR", "Could not close socket: %s\n",
                 strerror(errno));
  }
#endif // _WIN32

  if (!force) {
    Message message = {0};
    tkbc_dapf(&message, "%d:%zu:\r\n", MESSAGE_CLIENT_DISCONNECT,
              client.kite_id);
    tkbc_write_to_all_send_msg_buffers_except(message, client.socket_id);

    if (message.elements) {
      free(message.elements);
      message.elements = NULL;
    }
  }

  tkbc_remove_connection_retry(client);
}

/**
 * @brief The function constructs the hello message. That is used to establish
 * the handshake with the server respecting the current PROTOCOL_VERSION.
 *
 * @param client The client id where the message should be send to.
 */
void tkbc_message_hello_write_to_send_msg_buffer(Client *client) {
  Message message = {0};
  const char quote = '\"';
  tkbc_dapf(&message, "%d:%c%s" PROTOCOL_VERSION "%c:\r\n", MESSAGE_HELLO,
            quote, "Hello client from server!", quote);
  tkbc_write_to_send_msg_buffer(client, message);

  free(message.elements);
  message.elements = NULL;
}

/**
 * @brief The function constructs the message KITEADD that is send to all
 * clients, whenever a new client has connected to the server.
 *
 * @param client_index The id of the client that has connected.
 * @return True if id was found and the sending has nor raised any errors.
 */
bool tkbc_message_kiteadd_write_to_all_send_msg_buffers(size_t client_index) {
  Message message = {0};
  bool ok = true;
  tkbc_dapf(&message, "%d:", MESSAGE_KITEADD);
  if (!tkbc_message_append_clientkite(client_index, &message)) {
    check_return(false);
  }
  tkbc_dapf(&message, "\r\n");
  tkbc_write_to_all_send_msg_buffers(message);

check:
  free(message.elements);
  message.elements = NULL;
  return ok;
}

/**
 * @brief The function constructs the message CLIENTKITES.
 *
 * @param message The message buffer where the constructed message should be
 * appended to.
 */
void tkbc_message_clientkites(Message *message) {
  size_t active_count = 0;
  for (size_t i = 0; i < env->kite_array->count; ++i) {
    if (env->kite_array->elements[i].is_active) {
      active_count++;
    }
  }

  tkbc_dapf(message, "%d:%zu:", MESSAGE_CLIENTKITES, active_count);
  for (size_t i = 0; i < env->kite_array->count; ++i) {
    Kite_State *kite_state = &env->kite_array->elements[i];
    if (!kite_state->is_active) {
      continue;
    }

    if (!tkbc_message_append_clientkite(kite_state->kite_id, message)) {
      Client *client = tkbc_get_client_by_kite_id(kite_state->kite_id);
      tkbc_server_shutdown_client(*client, false);
    }
  }
  tkbc_dapf(message, "\r\n");
}

/**
 * @brief The function constructs and send the message CLIENTKITES that contain
 * all the data from the current registered kites.
 *
 * @param client The client that should get the message.
 */
void tkbc_message_clientkites_write_to_send_msg_buffer(Client *client) {
  Message message = {0};
  tkbc_message_clientkites(&message);
  tkbc_write_to_send_msg_buffer(client, message);

  free(message.elements);
  message.elements = NULL;
}

/**
 * @brief The function combines the fist initial construction of the kite for a
 * new client and sends the fist hello message and sends the positions for all
 * the rest of the registered kites.
 *
 * @param client The new client that should get a kite associated with.
 */
void tkbc_client_prelog(Client *client) {
  tkbc_message_hello_write_to_send_msg_buffer(client);

  Kite_State *kite_state = tkbc_init_kite();
  kite_state->kite_id = client->kite_id;
  float r = (float)rand() / RAND_MAX;
  kite_state->kite->body_color = ColorFromHSV(r * 360, 0.6, (r + 3) / 4);
  if (!tkbc_script_finished(env)) {
    kite_state->is_active = false;
  }
  tkbc_dap(env->kite_array, *kite_state);
  // Just free the state and not the kite inside, because the kite is a
  // pointer that lives on and is valid in the copy to the env->kite_array.
  free(kite_state);
  kite_state = NULL;

  tkbc_message_kiteadd_write_to_all_send_msg_buffers(client->kite_id);

  tkbc_message_clientkites_write_to_send_msg_buffer(client);
}

/**
 * @brief The function handles the accept call of new clients.
 */
void tkbc_server_accept() {

  SOCKADDR_IN client_address;
  SOCKLEN address_length = sizeof(client_address);
  int client_socket_id =
      accept(server_socket, (SOCKADDR *)&client_address, &address_length);

  if (client_socket_id == -1) {
    if (errno != EAGAIN) {
      tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
      assert(0 && "accept error");
    }
  } else {

#ifdef _WIN32
    // Set the socket to non-blocking
    u_long mode = 1; // 1 to enable non-blocking socket
    if (ioctlsocket(client_socket_id, FIONBIO, &mode) != 0) {
      tkbc_fprintf(stderr, "ERROR", "ioctlsocket(): %d\n", WSAGetLastError());
      closesocket(client_socket_id);
      WSACleanup();
      exit(1);
    }
#else
    // Set the socket to non-blocking
    int flags = fcntl(client_socket_id, F_GETFL, 0);
    fcntl(client_socket_id, F_SETFL, flags | O_NONBLOCK);
#endif // _WIN32

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

    tkbc_client_prelog(&clients.elements[clients.count - 1]);
  }
}

/**
 * @brief The function tries to read messages from the socket connection.
 *
 * @param client The client where the data should be read from.
 * @return True if data was read, otherwise false.
 */
bool tkbc_sockets_read(Client *client) {
  Message *recv_buffer = &client->recv_msg_buffer;
  size_t length = 1024;

  if (recv_buffer->count == 0 && recv_buffer->capacity > MAX_BUFFER_CAPACITY) {
    tkbc_fprintf(stderr, "INFO", "realloced recv_buffer: old capacity: %zu\n",
                 recv_buffer->capacity);
    free(recv_buffer->elements);
    recv_buffer->elements = NULL;
    recv_buffer->capacity = 0;
  }

  if (recv_buffer->capacity < recv_buffer->count + length) {
    if (recv_buffer->capacity == 0) {
      recv_buffer->capacity = length;
    }

    while (recv_buffer->capacity < recv_buffer->count + length) {
      recv_buffer->capacity += length;
    }

    recv_buffer->elements =
        realloc(recv_buffer->elements,
                sizeof(*recv_buffer->elements) * recv_buffer->capacity);

    if (recv_buffer->elements == NULL) {
      fprintf(stderr,
              "The allocation for the dynamic array has failed in: %s: %d\n",
              __FILE__, __LINE__);
      abort();
    }
  }

  int n = recv(client->socket_id,
               client->recv_msg_buffer.elements + client->recv_msg_buffer.count,
               length, 0);

  if (n < 0) {
#ifdef _WIN32
    int err_errno = WSAGetLastError();
    if (err_errno != WSAEWOULDBLOCK) {
      tkbc_fprintf(stderr, "ERROR", "Read: %d\n", err_errno);
      return false;
    } else {
      return true;
    }
#else
    if (errno != EAGAIN) {
      tkbc_fprintf(stderr, "ERROR", "Read: %s\n", strerror(errno));
      return false;
    } else {
      return true;
    }
#endif // _WIN32
  }

  if (n == 0) {
    tkbc_fprintf(stderr, "ERROR", "No bytes could be read:" CLIENT_FMT "\n",
                 CLIENT_ARG(*client));
    return false;
  }

  assert(n != -1);
  recv_buffer->count += n;
  return true;
}

/**
 * @brief The function manages sending the internal message stored in the
 * send_msg_buffer from the client to the client.
 *
 * @param client The client where the message stored in the send_msg_buffer in
 * the client structure should be send to.
 * @return The amount send to the client socket, 0 if not data was send, -1 if
 * an error occurred or -11 if the error was EAGAIN.
 */
int tkbc_socket_write(Client *client) {
  size_t amount =
      (client->send_msg_buffer.count - client->send_msg_buffer.i) % 1024;
  ssize_t n = send(client->socket_id,
                   client->send_msg_buffer.elements + client->send_msg_buffer.i,
                   amount, 0);

  if (n < 0) {
#ifdef _WIN32
    int err_errno = WSAGetLastError();
    if (err_errno != WSAEWOULDBLOCK) {
      tkbc_fprintf(stderr, "ERROR", "Write: %d\n", err_errno);
      return -1;
    } else {
      return -11;
    }
#else
    if (errno != EAGAIN) {
      tkbc_fprintf(stderr, "ERROR", "Write: %s\n", strerror(errno));
      return -1;
    } else {
      return -11;
    }
#endif // _WIN32
  }

  if (n == 0) {
    tkbc_fprintf(stderr, "ERROR", "No bytes where written to:" CLIENT_FMT "\n",
                 CLIENT_ARG(*client));
  }

  assert(n != -1);
  if ((size_t)n == client->send_msg_buffer.count - client->send_msg_buffer.i) {
    client->send_msg_buffer.count = 0;
    client->send_msg_buffer.i = 0;
  } else {
    client->send_msg_buffer.i += n;
  }

  if (client->send_msg_buffer.count == 0 &&
      client->send_msg_buffer.capacity > MAX_BUFFER_CAPACITY) {
    tkbc_fprintf(stderr, "INFO", "realloced send_msg_buffer: old capacity: %zu",
                 client->send_msg_buffer.capacity);
    free(client->send_msg_buffer.elements);
    client->send_msg_buffer.elements = NULL;
    client->send_msg_buffer.capacity = 0;
  }
  return n;
}

/**
 * @brief The function encapsulates the reading and writing communication of the
 * given client.
 *
 * @param client The client where the message stored in the send_msg_buffer in
 * @return True if the handling (reading or writing data) of the clients has
 * succeeded, otherwise false.
 */
bool tkbc_server_handle_client(Client *client) {
  struct pollfd *pollfd = tkbc_get_pollfd_by_fd(client->socket_id);
  int result;
  switch (pollfd->events) {
  case POLLRDNORM:
    result = tkbc_sockets_read(client);
    // Switch to the next write state.
    pollfd->events = POLLWRNORM;
    return result;
  case POLLWRNORM:
    if (client->send_msg_buffer.count <= 0) {
      pollfd->events = POLLRDNORM;
      return true;
    }

    result = tkbc_socket_write(client);
    if (result <= 0) {
      pollfd->events = POLLRDNORM;
    }
    if (result == -1) {
      return false;
    }
    return true;
  default:
    assert(0 && "UNKNOWN EVENT");
  }
}

/**
 * @brief The function handles new incoming connections and checks for the data
 * on the sockets after an updates was detected.
 */
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
      bool result = tkbc_server_handle_client(client);
      if (!result) {
        tkbc_server_shutdown_client(*client, false);
        idx--;
        continue;
      }
    }
  }
}

/**
 * @brief The function constructs and sends the message CLIENTKITES to all
 * registered kites.
 */
void tkbc_message_clientkites_write_to_all_send_msg_buffers() {
  Message message = {0};
  tkbc_message_clientkites(&message);
  tkbc_write_to_all_send_msg_buffers(message);

  free(message.elements);
  message.elements = NULL;
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
  tkbc_dapf(&message, "%d:%zu:%zu:%zu:\r\n", MESSAGE_SCRIPT_BLOCK_FRAME_VALUE,
            script_id, block_frame_count, block_index);

  tkbc_write_to_all_send_msg_buffers(message);

  free(message.elements);
  message.elements = NULL;
}

/**
 * @brief The function constructs the message KITEVALUE that contains a data
 * serialization for one specified kite.
 *
 * @param client_id The client id that corresponds to the data that should be
 * appended.
 * @return True if the serialization has finished without errors, otherwise
 * false.
 */
bool tkbc_message_kite_value_write_to_all_send_msg_buffers_except(
    size_t client_id) {
  Message message = {0};
  bool ok = true;
  tkbc_dapf(&message, "%d:", MESSAGE_KITEVALUE);
  if (!tkbc_message_append_clientkite(client_id, &message)) {
    check_return(false);
  }
  tkbc_dapf(&message, "\r\n");
  tkbc_write_to_all_send_msg_buffers_except(message, client_id);

check:
  free(message.elements);
  message.elements = NULL;
  return ok;
}

/**
 * @brief The function constructs and sends the message KITES to all registered
 * kites.
 *
 * @return True if the message was send successfully, otherwise false.
 */
void tkbc_message_kites_write_to_all_send_msg_buffers() {
  Message message = {0};

  tkbc_dapf(&message, "%d:%zu:", MESSAGE_KITES, env->kite_array->count);
  for (size_t i = 0; i < env->kite_array->count; ++i) {
    Kite_State *kite_state = &env->kite_array->elements[i];
    if (kite_state->is_active) {
      tkbc_message_append_kite(kite_state, &message);
    }
  }
  tkbc_dapf(&message, "\r\n");
  tkbc_write_to_all_send_msg_buffers(message);

  free(message.elements);
  message.elements = NULL;
}

/**
 * @brief The function parses the messages out of the given
 * receive_message_queue data. If an invalid message is found the rest of the
 * receive_message_queue till the '\r\n' is dorpped. The parser continues from
 * there as recovery.
 *
 * @param client The client, that holds the received message to parse.
 * @return True if every message is parsed correctly from the data, false if an
 * parsing error has occurred.
 */
bool tkbc_received_message_handler(Client *client) {
  bool reset = true;
  Token token;
  bool ok = true;
  Message *message = &client->recv_msg_buffer;
  Lexer *lexer =
      lexer_new(__FILE__, message->elements, message->count, message->i);
  if (message->count == 0) {
    check_return(true);
  }
  do {
    token = lexer_next(lexer);
    if (token.kind == EOF_TOKEN) {
      break;
    }
    if (token.kind == INVALID) {
      // This is '\0' same as EOF in this case.
      break;
    }
    if (token.kind == ERROR) {
      goto err;
    }

    if (token.kind != NUMBER) {
      goto err;
    }

    int kind = atoi(lexer_token_to_cstr(lexer, &token));
    token = lexer_next(lexer);
    if (token.kind != PUNCT_COLON) {
      goto err;
    }

    message->i = lexer->position - 2;
    static_assert(MESSAGE_COUNT == 17, "NEW MESSAGE_COUNT WAS INTRODUCED");
    switch (kind) {
    case MESSAGE_HELLO: {
      token = lexer_next(lexer);
      if (token.kind != STRINGLITERAL) {
        check_return(false);
      }

      const char *greeting =
          "\"Hello server from client!" PROTOCOL_VERSION "\"";
      const char *compare = lexer_token_to_cstr(lexer, &token);
      if (strncmp(compare, greeting, strlen(greeting)) != 0) {
        tkbc_fprintf(stderr, "ERROR", "Hello message failed!\n");
        tkbc_fprintf(stderr, "ERROR", "Wrong protocol version!\n");
        check_return(false);
      }
      token = lexer_next(lexer);
      if (token.kind != PUNCT_COLON) {
        check_return(false);
      }

      tkbc_fprintf(stderr, "MESSAGEHANDLER", "HELLO\n");
    } break;
    case MESSAGE_KITEVALUE: {
      size_t kite_id;
      float x, y, angle;
      Color color;
      if (!tkbc_parse_message_kite_value(lexer, &kite_id, &x, &y, &angle,
                                         &color)) {
        goto err;
      }
      Kite *kite = tkbc_get_kite_by_id(env, kite_id);
      kite->center.x = x;
      kite->center.y = y;
      kite->angle = angle;
      kite->body_color = color;
      tkbc_kite_update_internal(kite);

      if (!tkbc_message_kite_value_write_to_all_send_msg_buffers_except(
              kite_id)) {
        check_return(false);
      }

      tkbc_fprintf(stderr, "MESSAGEHANDLER", "KITEVALUE\n");
    } break;
    case MESSAGE_KITES_POSITIONS: {
      token = lexer_next(lexer);
      if (token.kind != NUMBER) {
        check_return(false);
      }
      size_t kite_count = atoi(lexer_token_to_cstr(lexer, &token));
      token = lexer_next(lexer);
      if (token.kind != PUNCT_COLON) {
        check_return(false);
      }
      for (size_t id = 0; id < kite_count; ++id) {
        if (!tkbc_parse_single_kite_value(lexer, -1)) {
          goto err;
        }

        tkbc_message_kites_write_to_all_send_msg_buffers();
      }

      tkbc_fprintf(stderr, "MESSAGEHANDLER", "KITES_POSITIONS\n");
    } break;
    case MESSAGE_KITES_POSITIONS_RESET: {
      // All parsing is already done above.
      tkbc_kite_array_start_position(env->kite_array, env->window_width,
                                     env->window_height);

      tkbc_message_kites_write_to_all_send_msg_buffers();

      tkbc_fprintf(stderr, "MESSAGEHANDLER", "KITES_POSITIONS_RESET\n");
    } break;
    case MESSAGE_SCRIPT: {

      Content tmp_buffer = {0};
      bool script_parse_fail = false;
      Block_Frame *scb_block_frame = &env->scratch_buf_block_frame;
      Frames *scb_frames = &env->scratch_buf_frames;
      Kite_Ids possible_new_kis = {0};

      token = lexer_next(lexer);
      if (token.kind != NUMBER) {
        script_parse_fail = true;
        goto script_err;
      }
      scb_block_frame->script_id = atoi(lexer_token_to_cstr(lexer, &token));
      token = lexer_next(lexer);
      if (token.kind != PUNCT_COLON) {
        script_parse_fail = true;
        goto script_err;
      }
      token = lexer_next(lexer);
      if (token.kind != NUMBER) {
        script_parse_fail = true;
        goto script_err;
      }
      size_t block_frame_count = atoi(lexer_token_to_cstr(lexer, &token));
      token = lexer_next(lexer);
      if (token.kind != PUNCT_COLON) {
        script_parse_fail = true;
        goto script_err;
      }

      for (size_t i = 0; i < block_frame_count; ++i) {
        token = lexer_next(lexer);
        if (token.kind != NUMBER) {
          script_parse_fail = true;
          goto script_err;
        }
        scb_frames->block_index = atoi(lexer_token_to_cstr(lexer, &token));
        token = lexer_next(lexer);
        if (token.kind != PUNCT_COLON) {
          script_parse_fail = true;
          goto script_err;
        }

        token = lexer_next(lexer);
        if (token.kind != NUMBER) {
          script_parse_fail = true;
          goto script_err;
        }
        size_t frames_count = atoi(lexer_token_to_cstr(lexer, &token));
        token = lexer_next(lexer);
        if (token.kind != PUNCT_COLON) {
          script_parse_fail = true;
          goto script_err;
        }

        for (size_t j = 0; j < frames_count; ++j) {
          Frame frame = {0};
          token = lexer_next(lexer);
          if (token.kind != NUMBER) {
            script_parse_fail = true;
            goto script_err;
          }
          frame.index = atoi(lexer_token_to_cstr(lexer, &token));
          token = lexer_next(lexer);
          if (token.kind != PUNCT_COLON) {
            script_parse_fail = true;
            goto script_err;
          }
          token = lexer_next(lexer);
          if (token.kind != NUMBER) {
            script_parse_fail = true;
            goto script_err;
          }
          frame.finished = atoi(lexer_token_to_cstr(lexer, &token));
          token = lexer_next(lexer);
          if (token.kind != PUNCT_COLON) {
            script_parse_fail = true;
            goto script_err;
          }
          token = lexer_next(lexer);
          if (token.kind != NUMBER) {
            script_parse_fail = true;
            goto script_err;
          }
          frame.kind = atoi(lexer_token_to_cstr(lexer, &token));
          token = lexer_next(lexer);
          if (token.kind != PUNCT_COLON) {
            script_parse_fail = true;
            goto script_err;
          }

          char sign;
          Action action;
          static_assert(ACTION_KIND_COUNT == 9,
                        "NOT ALL THE Action_Kinds ARE IMPLEMENTED");
          switch (frame.kind) {
          case KITE_QUIT:
          case KITE_WAIT: {
            token = lexer_next(lexer);
            if (token.kind != NUMBER && token.kind != PUNCT_SUB) {
              script_parse_fail = true;
              goto script_err;
            }
            if (token.kind == PUNCT_SUB) {
              sign = *(char *)token.content;
              tkbc_dap(&tmp_buffer, sign);
              token = lexer_next(lexer);
            }
            tkbc_dapc(&tmp_buffer, token.content, token.size);
            tkbc_dap(&tmp_buffer, 0);
            action.as_wait.starttime = atof(tmp_buffer.elements);
            tmp_buffer.count = 0;
          } break;

          case KITE_MOVE:
          case KITE_MOVE_ADD: {
            token = lexer_next(lexer);
            if (token.kind != NUMBER && token.kind != PUNCT_SUB) {
              script_parse_fail = true;
              goto script_err;
            }
            if (token.kind == PUNCT_SUB) {
              sign = *(char *)token.content;
              tkbc_dap(&tmp_buffer, sign);
              token = lexer_next(lexer);
            }
            tkbc_dapc(&tmp_buffer, token.content, token.size);
            tkbc_dap(&tmp_buffer, 0);
            action.as_move.position.x = atof(tmp_buffer.elements);
            tmp_buffer.count = 0;

            token = lexer_next(lexer);
            if (token.kind != PUNCT_COLON) {
              script_parse_fail = true;
              goto script_err;
            }

            token = lexer_next(lexer);
            if (token.kind != NUMBER && token.kind != PUNCT_SUB) {
              script_parse_fail = true;
              goto script_err;
            }
            if (token.kind == PUNCT_SUB) {
              sign = *(char *)token.content;
              tkbc_dap(&tmp_buffer, sign);
              token = lexer_next(lexer);
            }
            tkbc_dapc(&tmp_buffer, token.content, token.size);
            tkbc_dap(&tmp_buffer, 0);
            action.as_move.position.y = atof(tmp_buffer.elements);
            tmp_buffer.count = 0;
          } break;

          case KITE_ROTATION:
          case KITE_ROTATION_ADD: {
            token = lexer_next(lexer);
            if (token.kind != NUMBER && token.kind != PUNCT_SUB) {
              script_parse_fail = true;
              goto script_err;
            }
            if (token.kind == PUNCT_SUB) {
              sign = *(char *)token.content;
              tkbc_dap(&tmp_buffer, sign);
              token = lexer_next(lexer);
            }
            tkbc_dapc(&tmp_buffer, token.content, token.size);
            tkbc_dap(&tmp_buffer, 0);
            action.as_rotation.angle = atof(tmp_buffer.elements);
            tmp_buffer.count = 0;
          } break;

          case KITE_TIP_ROTATION:
          case KITE_TIP_ROTATION_ADD: {
            token = lexer_next(lexer);
            if (token.kind != NUMBER) {
              script_parse_fail = true;
              goto script_err;
            }
            action.as_tip_rotation.tip =
                atoi(lexer_token_to_cstr(lexer, &token));

            token = lexer_next(lexer);
            if (token.kind != PUNCT_COLON) {
              script_parse_fail = true;
              goto script_err;
            }

            token = lexer_next(lexer);
            if (token.kind != NUMBER && token.kind != PUNCT_SUB) {
              script_parse_fail = true;
              goto script_err;
            }
            if (token.kind == PUNCT_SUB) {
              sign = *(char *)token.content;
              tkbc_dap(&tmp_buffer, sign);
              token = lexer_next(lexer);
            }
            tkbc_dapc(&tmp_buffer, token.content, token.size);
            tkbc_dap(&tmp_buffer, 0);
            action.as_tip_rotation.angle = atof(tmp_buffer.elements);
            tmp_buffer.count = 0;
          } break;

          default:
            assert(0 && "UNREACHABLE SCRIPT received_message_handler");
          }

          frame.action = action;
          token = lexer_next(lexer);
          if (token.kind != PUNCT_COLON) {
            script_parse_fail = true;
            goto script_err;
          }

          token = lexer_next(lexer);
          if (token.kind != NUMBER) {
            script_parse_fail = true;
            goto script_err;
          }
          frame.duration = atof(lexer_token_to_cstr(lexer, &token));

          // These tow have no kites attached.
          if (frame.kind != KITE_WAIT && frame.kind != KITE_QUIT) {
            token = lexer_next(lexer);
            if (token.kind != PUNCT_COLON) {
              script_parse_fail = true;
              goto script_err;
            }
            token = lexer_next(lexer);
            if (token.kind != NUMBER) {
              script_parse_fail = true;
              goto script_err;
            }
            size_t kite_ids_count = atoi(lexer_token_to_cstr(lexer, &token));
            token = lexer_next(lexer);
            if (token.kind != PUNCT_COLON) {
              script_parse_fail = true;
              goto script_err;
            }
            token = lexer_next(lexer);
            if (token.kind != PUNCT_LPAREN) {
              script_parse_fail = true;
              goto script_err;
            }
            for (size_t k = 1; k <= kite_ids_count; ++k) {
              token = lexer_next(lexer);
              if (token.kind != NUMBER) {
                script_parse_fail = true;
                goto script_err;
              }

              size_t kite_id = atoi(lexer_token_to_cstr(lexer, &token));
              bool contains = false;
              tkbc_dap(&frame.kite_id_array, kite_id);
              for (size_t id = 0; id < possible_new_kis.count; ++id) {
                if (possible_new_kis.elements[id] == kite_id) {
                  contains = true;
                  break;
                }
              }
              if (!contains) {
                tkbc_dap(&possible_new_kis, kite_id);
              }

              token = lexer_next(lexer);
              if (token.kind != PUNCT_COMMA && token.kind != PUNCT_RPAREN) {
                script_parse_fail = true;
                goto script_err;
              }
              if (token.kind == PUNCT_RPAREN && k != kite_ids_count) {
                script_parse_fail = true;
                goto script_err;
              }
            }
          }

          token = lexer_next(lexer);
          if (token.kind != PUNCT_COLON) {
            script_parse_fail = true;
            goto script_err;
          }
          tkbc_dap(scb_frames, frame);
        }

        tkbc_dap(scb_block_frame, tkbc_deep_copy_frames(scb_frames));
        tkbc_reset_frames_internal_data(scb_frames);
      }

      bool found = false;
      for (size_t i = 0; i < env->block_frames->count; ++i) {
        if (env->block_frames->elements[i].script_id ==
            scb_block_frame->script_id) {
          found = true;
        }
      }
      if (!found) {
        size_t kite_count = possible_new_kis.count;
        size_t prev_count = env->kite_array->count;
        Kite_Ids kite_ids = tkbc_kite_array_generate(env, kite_count);
        for (size_t i = prev_count; i < env->kite_array->count; ++i) {
          env->kite_array->elements[i].is_active = false;
        }

        tkbc_remap_script_kite_id_arrays_to_kite_ids(env, scb_block_frame,
                                                     kite_ids);
        free(kite_ids.elements);
        kite_ids.elements = NULL;

        // Set the first kite frame positions
        for (size_t i = 0; i < scb_block_frame->count; ++i) {
          tkbc_patch_block_frame_kite_positions(env,
                                                &scb_block_frame->elements[i]);
        }

        tkbc_dap(env->block_frames,
                 tkbc_deep_copy_block_frame(scb_block_frame));
        for (size_t i = 0; i < scb_block_frame->count; ++i) {
          tkbc_destroy_frames_internal_data(&scb_block_frame->elements[i]);
        }
        env->script_counter = env->block_frames->count;
      }
      scb_block_frame->count = 0;

    script_err:
      if (possible_new_kis.elements) {
        free(possible_new_kis.elements);
        possible_new_kis.elements = NULL;
      }
      if (tmp_buffer.elements) {
        free(tmp_buffer.elements);
        tmp_buffer.elements = NULL;
      }
      if (script_parse_fail) {
        goto err;
      }

      client->script_amount--;
      if (client->script_amount == 0) {
        tkbc_dapf(&client->send_msg_buffer, "%d:\r\n", MESSAGE_SCRIPT_PARSED);
      }

      tkbc_fprintf(stderr, "MESSAGEHANDLER", "SCRIPT\n");
    } break;
    case MESSAGE_SCRIPT_AMOUNT: {
      token = lexer_next(lexer);
      if (token.kind != NUMBER) {
        goto err;
      }

      client->script_amount = atoi(lexer_token_to_cstr(lexer, &token));

      token = lexer_next(lexer);
      if (token.kind != PUNCT_COLON) {
        goto err;
      }

      tkbc_fprintf(stderr, "MESSAGEHANDLER", "SCRIPT_AMOUNT\n");
    } break;
    case MESSAGE_SCRIPT_TOGGLE: {
      // TODO:Think about toggling the  script kites and normal client kites
      // back and forth.
      env->script_finished = !env->script_finished;

      tkbc_fprintf(stderr, "MESSAGEHANDLER", "SCRIPT_TOGGLE\n");
    } break;
    case MESSAGE_SCRIPT_NEXT: {

      if (env->script_counter > env->block_frames->count) {
        goto err;
      }

      tkbc_load_next_script(env);
      env->server_script_kite_max_count = 0;

      // TODO: Find a better way to do it reliable.
      // Generate kites if needed, if a script needs more kites than there are
      // currently registered.

      //
      // Activate the kites that belong to the script.
      Kite_Ids ids = {0};
      for (size_t i = 0; i < env->block_frame->count; ++i) {
        for (size_t j = 0; j < env->block_frame->elements[i].count; ++j) {
          Kite_Ids *kite_id_array =
              &env->block_frame->elements[i].elements[j].kite_id_array;

          size_t frame_max_kites = kite_id_array->count;
          env->server_script_kite_max_count =
              tkbc_max(frame_max_kites, env->server_script_kite_max_count);

          for (size_t k = 0; k < kite_id_array->count; ++k) {
            Id id = kite_id_array->elements[k];
            if (!tkbc_contains_id(ids, id)) {
              tkbc_dap(&ids, id);
            }
          }
        }
      }

      for (size_t i = 0; i < env->kite_array->count; ++i) {
        env->kite_array->elements[i].is_active = false;
        for (size_t j = 0; j < ids.count; ++j) {
          if (ids.elements[j] == env->kite_array->elements[i].kite_id) {
            env->kite_array->elements[i].is_active = true;
            break;
          }
        }
      }
      free(ids.elements);

      // TODO: Find out if the assert actual triggers in any situation.
      // Otherwise delete the code below if also dies not handle the id
      // remapping.
      assert(env->server_script_kite_max_count <= env->kite_array->count);
      if (env->server_script_kite_max_count > env->kite_array->count) {
        size_t needed_kites =
            env->server_script_kite_max_count - env->kite_array->count;
        Kite_Ids kite_ids = tkbc_kite_array_generate(env, needed_kites);
        free(kite_ids.elements);
      }

      tkbc_fprintf(stderr, "MESSAGEHANDLER", "SCRIPT_NEXT\n");
    } break;
    case MESSAGE_SCRIPT_SCRUB: {

      token = lexer_next(lexer);
      if (token.kind != NUMBER) {
        goto err;
      }
      bool drag_left = atoi(lexer_token_to_cstr(lexer, &token));

      // TODO: Ensure a script is loaded. And the kite_ids are correctly mapped.
      {
        if (env->block_frame->count <= 0) {
          goto err;
        }

        env->script_finished = true;
        // The block indexes are assumed in order and at the corresponding
        // index.
        int index = drag_left ? env->frames->block_index - 1
                              : env->frames->block_index + 1;

        if (index >= 0 && index < (int)env->block_frame->count) {
          env->frames = &env->block_frame->elements[index];
        }
        // TODO: map the kite_ids before setting this.
        tkbc_set_kite_positions_from_kite_frames_positions(env);
      }

      token = lexer_next(lexer);
      if (token.kind != PUNCT_COLON) {
        goto err;
      }

      tkbc_message_srcipt_block_frames_value_write_to_all_send_msg_buffers(
          env->block_frame->script_id, env->block_frame->count,
          env->frames->block_index);

      tkbc_fprintf(stderr, "MESSAGEHANDLER", "SCRIPT_SCRUB\n");
    } break;
    default:
      tkbc_fprintf(stderr, "ERROR", "Unknown KIND: %d\n", kind);
      exit(1);
    }
    continue;

  err: {
    char *rn = tkbc_find_rn_in_message_from_position(message, lexer->position);
    if (rn == NULL) {
      reset = false;
    } else {
      int jump_length = rn + 2 - &lexer->content[lexer->position];
      lexer_chop_char(lexer, jump_length);
      tkbc_fprintf(stderr, "WARNING", "Message: Parsing error: %.*s\n",
                   jump_length, message->elements + message->i);
      continue;
    }
    tkbc_fprintf(stderr, "WARNING",
                 "Message unfinished: first read bytes: %zu\n",
                 message->count - message->i);
    break;
  }
  } while (token.kind != EOF_TOKEN);

check:
  // No lexer_del() for performant reuse of the message.
  if (lexer->buffer.elements) {
    free(lexer->buffer.elements);
    lexer->buffer.elements = NULL;
  }
  free(lexer);
  lexer = NULL;

  if (reset) {
    message->count = 0;
    message->i = 0;
  }

  return ok;
}

/**
 * @brief The entry point sets up the event loop checks for socket connections
 * and computes different positions up on the messages.
 *
 * @param argc The commandline argument count.
 * @param argv The arguments form the commandline.
 * @return The exit code of program that is always 0 or the execution is kill
 * before with code 1.
 */
int main(int argc, char *argv[]) {
#ifndef _WIN32
  struct sigaction sig_action = {0};
  sig_action.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &sig_action, NULL);

  signal(SIGABRT, signalhandler);
  signal(SIGINT, signalhandler);
  signal(SIGTERM, signalhandler);
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

#ifdef _WIN32
  // Set the socket to non-blocking
  u_long mode = 1; // 1 to enable non-blocking socket
  if (ioctlsocket(server_socket, FIONBIO, &mode) != 0) {
    tkbc_fprintf(stderr, "ERROR", "ioctlsocket(): %d\n", WSAGetLastError());
    closesocket(server_socket);
    WSACleanup();
    exit(1);
  }
#else
  // Set the socket to non-blocking
  int flags = fcntl(server_socket, F_GETFL, 0);
  fcntl(server_socket, F_SETFL, flags | O_NONBLOCK);
#endif // _WIN32

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

#ifdef _WIN32
    int poll_err = WSAPoll(fds.elements, fds.count, timeout);
    if (poll_err == -1) {
      tkbc_fprintf(stderr, "ERROR", "The poll has failed:%d\n",
                   WSAGetLastError());
      break;
    }
#else
    int poll_err = poll(fds.elements, fds.count, timeout);
    if (poll_err == -1) {
      tkbc_fprintf(stderr, "ERROR", "The poll has failed:%s\n",
                   strerror(errno));
      break;
    }
#endif // _WIN32

    if (poll_err > 0) {
      tkbc_socket_handling();
    }

    // Handle messages
    for (size_t i = 0; i < clients.count; ++i) {
      Client *client = &clients.elements[i];
      //
      // Messages
      if (!tkbc_received_message_handler(client)) {
        if (client->recv_msg_buffer.count) {
          tkbc_fprintf(stderr, "MESSAGE", "%.*s",
                       (int)client->recv_msg_buffer.count,
                       client->recv_msg_buffer.elements);
        }
        tkbc_server_shutdown_client(*client, false);
      }
    }

    //
    // Base execution
    if (env->script_counter > 0) {
      if (!tkbc_script_finished(env) && env->block_frame != NULL) {
        size_t bindex = env->frames->block_index;
        // TODO: Map script ids to current registered ids.
        // TODO: Make the default client kite.ids higher numbers starting around
        // 1000 or so.
        tkbc_script_update_frames(env);

        if (env->frames->block_index != bindex) {
          bindex = env->frames->block_index;
          tkbc_message_srcipt_block_frames_value_write_to_all_send_msg_buffers(
              env->block_frame->script_id, env->block_frame->count, bindex);
        }

        tkbc_message_clientkites_write_to_all_send_msg_buffers();

        if (tkbc_script_finished(env)) {
          Message message = {0};
          tkbc_dapf(&message, "%d:\r\n", MESSAGE_SCRIPT_FINISHED);
          tkbc_write_to_all_send_msg_buffers(message);
          free(message.elements);
          message.elements = NULL;
          for (size_t i = 0; i < env->kite_array->count; ++i) {
            Kite_State *kite_state = &env->kite_array->elements[i];
            kite_state->is_active = !kite_state->is_active;
          }
        }
      }
    }
  }

  signalhandler(0);
  return 0;
}

/**
 * @brief The function represents the closing handler of the server. It can be
 * used as a signal handler or be called if the server should be shutdown-
 *
 * @param signal The signal that might has occurred.
 */
void signalhandler(int signal) {
  (void)signal;
  tkbc_fprintf(stderr, "INFO", "Closing...\n");

  for (size_t i = 0; i < clients.count; ++i) {
    tkbc_server_shutdown_client(clients.elements[i], true);
  }

  shutdown(server_socket, SHUT_RDWR);
#ifdef _WIN32
  if (closesocket(server_socket) == -1) {

    tkbc_fprintf(stderr, "ERROR", "Main Server Socket: %d\n",
                 WSAGetLastError());
  }
  WSACleanup();
#else
  if (close(server_socket) == -1) {
    tkbc_fprintf(stderr, "ERROR", "Main Server Socket: %s\n", strerror(errno));
  }
#endif // _WIN32

  free(clients.elements);
  free(fds.elements);
  exit(EXIT_SUCCESS);
}
