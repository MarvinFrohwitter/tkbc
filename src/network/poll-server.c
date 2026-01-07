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
#undef TKBC_UTILS_IMPLEMENTATION

#define SPACE_IMPLEMENTATION
#include "../../external/space/space.h"
#undef SPACE_IMPLEMENTATION

#include "../choreographer/tkbc-script-api.h"
#include "../choreographer/tkbc-script-handler.h"
#include "../choreographer/tkbc.h"

#define CLIENT_BASE_ID 10e6
#define MAX_BUFFER_CAPACITY 256 * 1024
static int server_socket;
static int clients_visited = 0;
Env *env = {0};
Clients clients = {0};
FDs fds = {0};
Space t_space = {0};
Message t_message = {0}; // The elements ptr is allocated inside of the t_space.

Kite_Images kite_images;     // Just for the definition do not use.
Kite_Textures kite_textures; // Just for the definition do not use.

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
  space_dapc(&client->msg_space, &client->send_msg_buffer, message.elements,
             message.count);

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
      space_free_space(&client_tmp.msg_space);

      client_tmp.recv_msg_buffer.elements = NULL;
      client_tmp.send_msg_buffer.elements = NULL;

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
    space_dapf(&t_space, &t_message, "%d:%zu:\r\n", MESSAGE_CLIENT_DISCONNECT,
               client.kite_id);
    tkbc_write_to_all_send_msg_buffers_except(t_message, client.socket_id);
    tkbc_reset_space_and_null_message(&t_space, &t_message);
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
  const char quote = '\"';
  space_dapf(&t_space, &t_message, "%d:%c%s" PROTOCOL_VERSION "%c:\r\n",
             MESSAGE_HELLO, quote, "Hello client from server!", quote);
  tkbc_write_to_send_msg_buffer(client, t_message);
  tkbc_reset_space_and_null_message(&t_space, &t_message);
}

/**
 * @brief The function constructs the message SINGLE_KITE_ADD that is send to
 * all clients, whenever a new client has connected to the server.
 *
 * @param client_index The id of the client that has connected.
 * @return True if id was found and the sending has nor raised any errors.
 */
bool tkbc_message_kiteadd_write_to_all_send_msg_buffers(size_t client_index) {
  bool ok = true;
  space_dapf(&t_space, &t_message, "%d:", MESSAGE_SINGLE_KITE_ADD);
  if (!tkbc_message_append_clientkite(client_index, &t_message, &t_space)) {
    check_return(false);
  }
  space_dapf(&t_space, &t_message, "\r\n");
  tkbc_write_to_all_send_msg_buffers(t_message);

check:
  tkbc_reset_space_and_null_message(&t_space, &t_message);
  return ok;
}

/**
 * @brief The function constructs the message CLIENTKITES.
 *
 * @param message The message buffer where the constructed message should be
 * appended to.
 */
void tkbc_message_clientkites(Message *t_message) {
  size_t active_count = 0;
  for (size_t i = 0; i < env->kite_array->count; ++i) {
    if (env->kite_array->elements[i].is_active) {
      active_count++;
    }
  }

  space_dapf(&t_space, t_message, "%d:%zu:", MESSAGE_CLIENTKITES, active_count);
  for (size_t i = 0; i < env->kite_array->count; ++i) {
    Kite_State *kite_state = &env->kite_array->elements[i];
    if (!kite_state->is_active) {
      continue;
    }

    if (!tkbc_message_append_clientkite(kite_state->kite_id, t_message,
                                        &t_space)) {
      Client *client = tkbc_get_client_by_kite_id(kite_state->kite_id);
      tkbc_server_shutdown_client(*client, false);
    }
  }
  space_dapf(&t_space, t_message, "\r\n");
}

/**
 * @brief The function constructs and send the message CLIENTKITES that contain
 * all the data from the current registered kites.
 *
 * @param client The client that should get the message.
 */
void tkbc_message_clientkites_write_to_send_msg_buffer(Client *client) {
  tkbc_message_clientkites(&t_message);
  tkbc_write_to_send_msg_buffer(client, t_message);

  tkbc_reset_space_and_null_message(&t_space, &t_message);
}

/**
 * @brief The function combines the fist initial construction of the kite for a
 * new client and sends the fist hello message and sends the positions for all
 * the rest of the registered kites.
 *
 * @param client The new client that should get a kite associated with.
 */
void tkbc_client_prolog(Client *client) {
  tkbc_message_hello_write_to_send_msg_buffer(client);

  Kite_State kite_state = tkbc_init_kite();
  kite_state.kite_id = client->kite_id;
  float r = (float)rand() / RAND_MAX;
  kite_state.kite->body_color = ColorFromHSV(r * 360, 0.6, (r + 3) / 4);
  if (!tkbc_script_finished(env)) {
    kite_state.is_active = false;
  }
  tkbc_dap(env->kite_array, kite_state);
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
        .kite_id = CLIENT_BASE_ID + env->kite_id_counter++,
        .socket_id = client_socket_id,
        .client_address = client_address,
        .client_address_length = address_length,
    };

    space_init_capacity(&client.msg_space, MAX_BUFFER_CAPACITY);

    tkbc_fprintf(stderr, "INFO", "CLIENT: " CLIENT_FMT " has connected.\n",
                 CLIENT_ARG(client));
    tkbc_dap(&clients, client);

    tkbc_client_prolog(&clients.elements[clients.count - 1]);
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
  size_t length = 1024 * 32;

  if (recv_buffer->count == 0 && recv_buffer->capacity > MAX_BUFFER_CAPACITY) {
    tkbc_fprintf(stderr, "INFO", "realloced recv_buffer: old capacity: %zu\n",
                 recv_buffer->capacity);

    // free(recv_buffer->elements);

    Planet *planet =
        space__find_planet_from_ptr(&client->msg_space, recv_buffer->elements);
    space_free_planet(&client->msg_space, planet);

    recv_buffer->elements = NULL;
    recv_buffer->capacity = 0;
  }

  if (recv_buffer->capacity < recv_buffer->count + length) {
    size_t old_capacity = recv_buffer->capacity;
    if (recv_buffer->capacity == 0) {
      recv_buffer->capacity = length;
    }

    while (recv_buffer->capacity < recv_buffer->count + length) {
      recv_buffer->capacity += length;
    }

    recv_buffer->elements =
        space_realloc(&client->msg_space, recv_buffer->elements,
                      sizeof(*recv_buffer->elements) * old_capacity,
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
  size_t length = 1024;
  size_t diff = (client->send_msg_buffer.count - client->send_msg_buffer.i);
  size_t amount = diff < length ? diff : length;

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
    tkbc_fprintf(stderr, "INFO",
                 "realloced send_msg_buffer: old capacity: %zu\n",
                 client->send_msg_buffer.capacity);

    // free(client->send_msg_buffer.elements);

    Planet *planet = space__find_planet_from_ptr(
        &client->msg_space, client->send_msg_buffer.elements);
    space_free_planet(&client->msg_space, planet);

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
    pollfd->events = POLLRDNORM;

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
  tkbc_message_clientkites(&t_message);
  tkbc_write_to_all_send_msg_buffers(t_message);

  tkbc_reset_space_and_null_message(&t_space, &t_message);
}

/**
 * @brief The function constructs all the scripts that specified in a block
 * frame.
 *
 * @param script_id The id that is the current load script.
 * @param script_count The amount of scripts that are available.
 * @param frames_index The current index of the collection of individual frames
 * in a script.
 */
void tkbc_message_srcipt_meta_data_write_to_all_send_msg_buffers(
    size_t script_id, size_t script_count, size_t frames_index) {

  space_dapf(&t_space, &t_message, "%d:%zu:%zu:%zu:\r\n",
             MESSAGE_SCRIPT_META_DATA, script_id, script_count, frames_index);

  tkbc_write_to_all_send_msg_buffers(t_message);
  tkbc_reset_space_and_null_message(&t_space, &t_message);
}

/**
 * @brief The function constructs the message SINGLE_KITE_UPDATE that contains a
 * data serialization for one specified kite.
 *
 * @param client_id The client id that corresponds to the data that should be
 * appended.
 * @return True if the serialization has finished without errors, otherwise
 * false.
 */
bool tkbc_message_kite_value_write_to_all_send_msg_buffers_except(
    size_t client_id) {
  bool ok = true;
  space_dapf(&t_space, &t_message, "%d:", MESSAGE_SINGLE_KITE_UPDATE);
  if (!tkbc_message_append_clientkite(client_id, &t_message, &t_space)) {
    check_return(false);
  }
  space_dapf(&t_space, &t_message, "\r\n");
  tkbc_write_to_all_send_msg_buffers_except(t_message, client_id);

check:
  tkbc_reset_space_and_null_message(&t_space, &t_message);
  return ok;
}

/**
 * @brief The function constructs and sends the message KITES to all registered
 * kites.
 *
 * @return True if the message was send successfully, otherwise false.
 */
void tkbc_message_kites_write_to_all_send_msg_buffers() {
  space_dapf(&t_space, &t_message, "%d:%zu:", MESSAGE_KITES,
             env->kite_array->count);
  for (size_t i = 0; i < env->kite_array->count; ++i) {
    Kite_State *kite_state = &env->kite_array->elements[i];
    if (kite_state->is_active) {
      tkbc_message_append_kite(kite_state, &t_message, &t_space);
    }
  }
  space_dapf(&t_space, &t_message, "\r\n");
  tkbc_write_to_all_send_msg_buffers(t_message);

  tkbc_reset_space_and_null_message(&t_space, &t_message);
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
    bool script_alleady_there_parsing_skip = false;
    token = lexer_next(lexer);
    if (token.kind == EOF_TOKEN) {
      break;
    }
    if (token.kind == INVALID) {
      break;
    }
    if (token.kind == NULL_TERMINATOR) {
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
    size_t digits_count_of_kind = token.size;
    token = lexer_next(lexer);
    if (token.kind != PUNCT_COLON) {
      goto err;
    }

    if (kind != MESSAGE_HELLO && !client->handshake_passed) {
      goto err;
    }

    message->i = lexer->position - digits_count_of_kind - 1;
    static_assert(MESSAGE_COUNT == 18, "NEW MESSAGE_COUNT WAS INTRODUCED");
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

      space_dapf(&client->msg_space, &client->send_msg_buffer, "%d:\r\n",
                 MESSAGE_HELLO_PASSED);
      client->handshake_passed = true;

      tkbc_message_kiteadd_write_to_all_send_msg_buffers(client->kite_id);
      tkbc_message_clientkites_write_to_send_msg_buffer(client);

      tkbc_fprintf(stderr, "MESSAGEHANDLER", "HELLO\n");
    } break;
    case MESSAGE_SINGLE_KITE_UPDATE: {
      size_t kite_id, texture_id;
      float x, y, angle;
      Color color;
      bool is_reversed, is_active;
      if (!tkbc_parse_message_kite_value(lexer, &kite_id, &x, &y, &angle,
                                         &color, &texture_id, &is_reversed,
                                         &is_active)) {
        goto err;
      }

      Kite_State *state = tkbc_get_kite_state_by_id(env, kite_id);
      // Consider disconnecting the client if the client is not found by its
      // kite id. Instead of crashing the complete server.
      // With a correct client implementation the state should always be
      // available. No memory corruption on the server side implied.
      tkbc_assign_values_to_kitestate(state, x, y, angle, color, texture_id,
                                      is_reversed, is_active);

      if (!tkbc_message_kite_value_write_to_all_send_msg_buffers_except(
              kite_id)) {
        check_return(false);
      }

      tkbc_fprintf(stderr, "MESSAGEHANDLER", "SINGLE_KITE_UPDATE\n");
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
      Space *scb_space = &env->script_creation_space;
      Script *scb_script = &env->scratch_buf_script;
      Frames *scb_frames = &env->scratch_buf_frames;
      Frame frame = {0};
      Kite_Ids possible_new_kis = {0};

      token = lexer_next(lexer);
      if (token.kind != NUMBER) {
        script_parse_fail = true;
        goto script_err;
      }
      scb_script->script_id = atoi(lexer_token_to_cstr(lexer, &token));
      //
      // This just fast forward a script that is already known and it reduces
      // the parsing afford.
      if (tkbc_scripts_contains_id(env->scripts, scb_script->script_id)) {
        script_alleady_there_parsing_skip = true;
        script_parse_fail = true;
        goto script_err;
      }

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
      size_t script_count = atoi(lexer_token_to_cstr(lexer, &token));
      token = lexer_next(lexer);
      if (token.kind != PUNCT_COLON) {
        script_parse_fail = true;
        goto script_err;
      }

      for (size_t i = 0; i < script_count; ++i) {
        token = lexer_next(lexer);
        if (token.kind != NUMBER) {
          script_parse_fail = true;
          goto script_err;
        }
        scb_frames->frames_index = atoi(lexer_token_to_cstr(lexer, &token));
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
              space_dap(scb_space, &frame.kite_id_array, kite_id);
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
          space_dap(scb_space, scb_frames, frame);
          memset(&frame, 0, sizeof(frame));
        }

        // No deep_copy because the space allocator holds the memory anyways
        // till the script is appended.
        // Frames frames = tkbc_deep_copy_frames(scb_space, scb_frames);
        //
        Frames frames = *scb_frames;
        space_dap(scb_space, scb_script, frames);

        // No reset because the kite_id_array is by pointer in the elements and
        // thy should not change. This dose not reuse the memory of the
        // scb_frames, but that is internal the frames data has to be stored
        // some were and can not be overwritten till the script is added to the
        // env.scripts_space.
        //
        // tkbc_reset_frames_internal_data(scb_frames);
        //
        memset(scb_frames, 0, sizeof(*scb_frames));
      }

      // Post parsing
      size_t kite_count = possible_new_kis.count;
      size_t prev_count = env->kite_array->count;
      Kite_Ids kite_ids = tkbc_kite_array_generate(env, kite_count);

      for (size_t i = prev_count; i < env->kite_array->count; ++i) {
        env->kite_array->elements[i].is_active = false;
      }

      tkbc_remap_script_kite_id_arrays_to_kite_ids(env, scb_script, kite_ids);
      free(kite_ids.elements);
      kite_ids.elements = NULL;

      // Set the first kite positions
      tkbc_patch_script_kite_positions(env, scb_script);

      //
      //
      // TODO: @Cleanup @Memory Holding all the scripts in memory is to much
      // even an DOS attac could happen, by providing a large amount of
      // scripts that doesn't fit into memory.
      //
      // Think about storing them on disk and loading them on demand or
      // reducing the memory storage size of a script.
      //
      // Marvin Frohwitter 22.06.2025
      tkbc_add_script(env,
                      tkbc_deep_copy_script(&env->scripts_space, scb_script));

      // This is just to be explicit is already happen in the script adding.
      //
      // For continues parsing this does not happen in an error case.
      scb_script->count = 0;

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
        if (script_alleady_there_parsing_skip) {
          goto parsing_skip;
        }
        goto err;
      }

      client->script_amount--;
    parsing_skip:
      if (client->script_amount && script_alleady_there_parsing_skip) {
        client->script_amount = 0;
      }
      if (client->script_amount == 0) {
        space_dapf(&client->msg_space, &client->send_msg_buffer, "%d:\r\n",
                   MESSAGE_SCRIPT_PARSED);
      }

      tkbc_fprintf(stderr, "MESSAGEHANDLER", "SCRIPT\n");

      if (script_alleady_there_parsing_skip) {
        goto err;
      }
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
      // TODO:Think about toggling the script kites and normal client kites
      // back and forth.
      env->script_finished = !env->script_finished;

      tkbc_fprintf(stderr, "MESSAGEHANDLER", "SCRIPT_TOGGLE\n");
    } break;
    case MESSAGE_SCRIPT_NEXT: {
      token = lexer_next(lexer);
      if (token.kind != NUMBER) {
        goto err;
      }

      int script_id = atoi(lexer_token_to_cstr(lexer, &token));

      token = lexer_next(lexer);
      if (token.kind != PUNCT_COLON) {
        goto err;
      }

      if (script_id == 0) {
        tkbc_unload_script(env);
        tkbc_message_srcipt_meta_data_write_to_all_send_msg_buffers(0, 0, 0);
        goto no_script;
      }
      // TODO: Report possible failures of loading back to the client.
      tkbc_load_script_id(env, script_id);
      env->server_script_kite_max_count = 0;

      // TODO: Find a better way to do it reliable.
      // Generate kites if needed, if a script needs more kites than there are
      // currently registered.

      //
      // Activate the kites that belong to the script.
      Kite_Ids ids = {0};
      for (size_t i = 0; i < env->script->count; ++i) {

        // TODO: Just filter for the kites that are in the parsed script_id.

        for (size_t j = 0; j < env->script->elements[i].count; ++j) {
          Kite_Ids *kite_id_array =
              &env->script->elements[i].elements[j].kite_id_array;

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

    no_script:
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
        if (env->script->count <= 0) {
          goto err;
        }

        env->script_finished = true;
        // The block indexes are assumed in order and at the corresponding
        // index.
        // This is needed to avoid a down cast of size_t to long or int that can
        // hold ever value of size_t.
        if (drag_left) {
          if (env->frames->frames_index > 0) {
            env->frames = &env->script->elements[env->frames->frames_index - 1];
          }
        } else {
          env->frames = &env->script->elements[env->frames->frames_index + 1];
        }

        // TODO: map the kite_ids before setting this.
        tkbc_set_kite_positions_from_kite_frames_positions(env);
      }

      token = lexer_next(lexer);
      if (token.kind != PUNCT_COLON) {
        goto err;
      }

      tkbc_message_srcipt_meta_data_write_to_all_send_msg_buffers(
          env->script->script_id, env->script->count,
          env->frames->frames_index);

      tkbc_fprintf(stderr, "MESSAGEHANDLER", "SCRIPT_SCRUB\n");
    } break;
    default:
      tkbc_fprintf(stderr, "ERROR", "Unknown KIND: %d\n", kind);
      // exit(1);
    }
    continue;

  err: {
    bool rerun = tkbc_error_handling_of_received_message_handler(
        message, lexer, &reset, !script_alleady_there_parsing_skip);
    if (rerun) {
      continue;
    }
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
    //
    // TODO: Set the TARGET_DT to 0.
    //
    // The client has to collect the positions before displaying them, if the
    // server is to fast.
    //
    tkbc_make_frame_time(TARGET_DT);
    if (clients_visited >= SERVER_CONNETCTIONS) {
      break;
    }

    int timeout = -1; // Infinite
    int poll_err = tkbc_poll(timeout);
    if (poll_err == -1) {
      break;
    }

    if (poll_err > 0) {
      tkbc_socket_handling();

      //
      // The second call of socket handling is related to the fact that the base
      // execution can set a POLLWRNORM for clients so a send call has to be
      // called and then poll can wait for reading events.
      {
        timeout = 0; // immediately return
        int poll_err = tkbc_poll(timeout);
        if (poll_err == -1) {
          break;
        } else if (poll_err == 0) {
          continue;
        }
      }

      if (!tkbc_script_finished(env) && poll_err > 0) {
        tkbc_socket_handling();
      }
    }

    // Handle messages
    for (size_t i = 0; i < clients.count; ++i) {
      Client *client = &clients.elements[i];
      //
      // Messages
      if (!tkbc_received_message_handler(client)) {
        if (client->recv_msg_buffer.count &&
            client->recv_msg_buffer.count < INT_MAX) {
          tkbc_fprintf(stderr, "MESSAGE", "%.*s",
                       (int)client->recv_msg_buffer.count,
                       client->recv_msg_buffer.elements);
        }
        tkbc_server_shutdown_client(*client, false);
      }
    }

    tkbc_base_execution();
  }

  signalhandler(0);
  return 0;
}

int tkbc_poll(int timeout) {

#ifdef _WIN32
  int poll_err = WSAPoll(fds.elements, fds.count, timeout);
  if (poll_err == -1) {
    tkbc_fprintf(stderr, "ERROR", "The poll has failed:%d\n",
                 WSAGetLastError());
  }
#else
  int poll_err = poll(fds.elements, fds.count, timeout);
  if (poll_err == -1) {
    tkbc_fprintf(stderr, "ERROR", "The poll has failed:%s\n", strerror(errno));
  }
#endif // _WIN32

  return poll_err;
}

/**
 * @brief The function encapsulates the script handling and possible other base
 * execution of the server.
 *
 * @return True if the base execution and script handling succeeded, otherwise
 * false.
 */
bool tkbc_base_execution() {
  if (env->scripts.count <= 0) {
    return false;
  }

  if (!tkbc_script_finished(env) && env->script != NULL) {
    size_t bindex = env->frames->frames_index;
    tkbc_script_update_frames(env);

    if (env->frames->frames_index != bindex) {
      bindex = env->frames->frames_index;
      tkbc_message_srcipt_meta_data_write_to_all_send_msg_buffers(
          env->script->script_id, env->script->count, bindex);
    }

    tkbc_message_clientkites_write_to_all_send_msg_buffers();

    if (tkbc_script_finished(env)) {
      space_dapf(&t_space, &t_message, "%d:\r\n", MESSAGE_SCRIPT_FINISHED);
      tkbc_write_to_all_send_msg_buffers(t_message);
      tkbc_reset_space_and_null_message(&t_space, &t_message);

      for (size_t i = 0; i < env->kite_array->count; ++i) {
        Kite_State *kite_state = &env->kite_array->elements[i];
        kite_state->is_active = !kite_state->is_active;
      }
    }
    return true;
  }

  return false;
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
  space_free_space(&t_space);

  tkbc_destroy_env(env);
  exit(EXIT_SUCCESS);
}
