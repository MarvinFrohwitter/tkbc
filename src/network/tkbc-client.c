#include "tkbc-client.h"
#include "tkbc-network-common.h"

#include <raylib.h>
#include <raymath.h>

#define TKBC_UTILS_IMPLEMENTATION
#include "../global/tkbc-utils.h"

#include "../choreographer/tkbc-ffmpeg.h"
#include "../choreographer/tkbc-input-handler.h"
#include "../choreographer/tkbc-keymaps.h"
#include "../choreographer/tkbc-script-api.h"
#include "../choreographer/tkbc-sound-handler.h"
#include "../choreographer/tkbc.h"
#include "../global/tkbc-popup.h"

#include "../../external/lexer/tkbc-lexer.h"
#include "../../tkbc_scripts/first.c"

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
#include <signal.h>
#include <sys/socket.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define WINDOW_SCALE 120
#define SCREEN_WIDTH 16 * WINDOW_SCALE
#define SCREEN_HEIGHT 9 * WINDOW_SCALE
#define TKBC_CLIENT

Env *env = {0};
Client client = {0};
#define RECEIVE_QUEUE_SIZE 1024
Message tkbc_send_message_queue = {0};
// static bool first_message_kite_add = true;

/**
 * @brief The function prints the way the program should be called.
 *
 * @param program_name The name of the program that is currently executing.
 */
void tkbc_client_usage(const char *program_name) {
  tkbc_fprintf(stderr, "INFO", "Usage:\n");
  tkbc_fprintf(stderr, "INFO", "      %s <HOST> <PORT> \n", program_name);
}

/**
 * @brief The function checks if host and port is given to that program
 * execution.
 *
 * @param argc The commandline argument count.
 * @param program_name The name of the program that is currently executing.
 * @return True if there are enough arguments, otherwise false.
 */
bool tkbc_client_commandline_check(int argc, const char *program_name) {
  if (argc > 2) {
    tkbc_fprintf(stderr, "ERROR", "To may arguments.\n");
    tkbc_client_usage(program_name);
    exit(1);
  }
  if (argc == 0) {
    tkbc_fprintf(stderr, "ERROR", "No arguments were provided.\n");
    tkbc_fprintf(stderr, "INFO",
                 "The default localhost and port 8080 is used.\n");
    return false;
  }
  return true;
}

/**
 * @brief The function checks if the given string is a valid host address.
 *
 * @param host_check The possible string that can contain the host.
 * @return The given host if the parsing was flawless, otherwise the program
 * crashes.
 */
const char *tkbc_host_parsing(const char *host_check) {

  const char *host = "127.0.0.1";
  if (strcmp(host_check, "localhost") == 0) {
    return host;
  }

  for (size_t i = 0; i < strlen(host_check); ++i) {
    if (!isdigit(host_check[i]) && host_check[i] != '.') {
      tkbc_fprintf(stderr, "ERROR", "The given host [%s] is not supported.\n",
                   host_check);
      exit(1);
    }
  }

  // TODO: Check for DNS-resolution.
  host = host_check;
  return host;
}

/**
 * @brief This function can be used to create a new client socket and connect it
 * to the server.
 *
 * @param addr The address of the server the client should connect to.
 * @param port The port where the server is available.
 * @return The client socket if the creation and connection has succeeded,
 * otherwise the program crashes.
 */
int tkbc_client_socket_creation(const char *addr, uint16_t port) {
#ifdef _WIN32
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    assert(0 && "ERROR: WSAStartup()");
  } else {
    tkbc_fprintf(stderr, "INFO", "Initialization of WSAStartup() succeed.\n");
  }
  int client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (client_socket == -1) {
    tkbc_fprintf(stderr, "ERROR", "%ld\n", WSAGetLastError());
    exit(1);
  }
#else
  int client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (client_socket == -1) {
    tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
    exit(1);
  }
#endif

  int option = 1;
  int sso = setsockopt(client_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&option,
                       sizeof(option));
  if (sso == -1) {
#ifdef _WIN32
    tkbc_fprintf(stderr, "ERROR", "%ld\n", WSAGetLastError());
#else
    tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
#endif
  }

  SOCKADDR_IN server_address;
  memset(&server_address, 0, sizeof(server_address));

  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port);
  server_address.sin_addr.s_addr = inet_addr(addr);

  int connection_status = connect(client_socket, (SOCKADDR *)&server_address,
                                  sizeof(server_address));

  if (connection_status == -1) {
#ifdef _WIN32
    tkbc_fprintf(stderr, "ERROR", "%ld\n", WSAGetLastError());
    if (closesocket(client.socket_id) == -1) {
      tkbc_fprintf(stderr, "ERROR", "Could not close socket: %d\n",
                   WSAGetLastError());
    }
    WSACleanup();
#else
    tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
    if (close(client.socket_id) == -1) {
      tkbc_fprintf(stderr, "ERROR", "Could not close socket: %s\n",
                   strerror(errno));
    }
#endif
    exit(1);
  }

#ifdef _WIN32
  // Set the socket to non-blocking
  u_long mode = 1; // 1 to enable non-blocking socket
  if (ioctlsocket(client_socket, FIONBIO, &mode) != 0) {
    tkbc_fprintf(stderr, "ERROR", "ioctlsocket(): %d\n", WSAGetLastError());
    closesocket(client_socket);
    WSACleanup();
    exit(1);
  }
#else
  // Set the socket to non-blocking
  int flags = fcntl(client_socket, F_GETFL, 0);
  fcntl(client_socket, F_SETFL, flags | O_NONBLOCK);
#endif

  tkbc_fprintf(stderr, "INFO", "Connected to Server: %s:%hd\n",
               inet_ntoa(server_address.sin_addr),
               ntohs(server_address.sin_port));

  return client_socket;
}

/**
 * @brief The function registers a new kite out of the given values and sets
 * default for every other part.
 *
 * @param kite_id The new kite id the kite gets.
 * @param x The new positional x value of the center of the kite.
 * @param y The new positional y value of the center of the kite.
 * @param angle The new rotation angle of the kite.
 * @param color The new body color of the kite.
 */
void tkbc_register_kite_from_values(size_t kite_id, float x, float y,
                                    float angle, Color color) {
  Kite_State *kite_state = tkbc_init_kite();
  kite_state->kite_id = kite_id;
  kite_state->kite->center.x = x;
  kite_state->kite->center.y = y;
  kite_state->kite->angle = angle;
  kite_state->kite->body_color = color;
  tkbc_kite_update_internal(kite_state->kite);
  tkbc_dap(env->kite_array, *kite_state);
}

/**
 * @brief The function prepares the sending of the default scripts that are
 * compiled into the client and pushes them into the send_message_queue.
 */
void sending_script_handler() {
  if (env->script_setup) {
    // For detection if the begin and end is called correctly.
    env->script_setup = false;
    tkbc__script_input(env);
    for (size_t i = 0; i < env->block_frames->count; ++i) {
      // tkbc_print_script(stderr, &env->block_frames->elements[i]);
      // char buf[32];
      // sprintf(buf, "Script%zu.kite", i);
      // tkbc_write_script_kite_from_mem(&env->block_frames->elements[i], buf);
    }
    tkbc_message_script();
  }
}

/**
 * @brief The function sends all the messages in the send_message_queue to the
 * server. The send_message_queue is reset in every case after the call.
 *
 * @return True if all the messages in the message buffer are send to the server
 * and the message buffer is cleared, otherwise false if an error has occurred.
 */
bool send_message_handler() {
  bool ok = true;
  if (tkbc_send_message_queue.count) {
    // NOTE: this assumes the whole message buffer could be send in one go.
    ssize_t n = send(client.socket_id, tkbc_send_message_queue.elements,
                     tkbc_send_message_queue.count, 0);

    if (n == 0) {
      tkbc_fprintf(stderr, "ERROR", "No bytes where send to the server!\n");
      check_return(false);
    }
    if (n == -1) {
      tkbc_dap(&tkbc_send_message_queue, 0);
      tkbc_fprintf(stderr, "ERROR", "Could not broadcast message: %s\n",
                   tkbc_send_message_queue.elements);
      tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
      check_return(false);
    }
  }
check:
  tkbc_send_message_queue.count = 0;
  return ok;
}

/**
 * @brief The function parses the incoming messages from the server and handles
 * the resulting behavior.
 *
 * @param message The message to parse and handle.
 * @return True if the parsing was successful an all resulting actions could be
 * handled, otherwise false and a parsing error has occurred.
 */
bool received_message_handler(Message *message) {
  Token token;
  bool ok = true;
  if (message->count == 0) {
    return ok;
  }

  Lexer *lexer = lexer_new(__FILE__, message->elements, message->count, 0);
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

    static_assert(MESSAGE_COUNT == 14, "NEW MESSAGE_COUNT WAS INTRODUCED");
    switch (kind) {
    case MESSAGE_HELLO: {
      token = lexer_next(lexer);
      if (token.kind != STRINGLITERAL) {
        check_return(false);
      }

      const char *greeting =
          "\"Hello client from server!" PROTOCOL_VERSION "\"";
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

      {
        char buf[64] = {0};
        snprintf(buf, sizeof(buf), "%d", MESSAGE_HELLO);
        tkbc_dapc(&tkbc_send_message_queue, buf, strlen(buf));
        tkbc_dap(&tkbc_send_message_queue, ':');

        tkbc_dapc(&tkbc_send_message_queue, "\"", 1);
        const char *m = "Hello server from client!";
        tkbc_dapc(&tkbc_send_message_queue, m, strlen(m));

        tkbc_dapc(&tkbc_send_message_queue, PROTOCOL_VERSION,
                  strlen(PROTOCOL_VERSION));
        tkbc_dapc(&tkbc_send_message_queue, "\"", 1);
        tkbc_dap(&tkbc_send_message_queue, ':');
        tkbc_dapc(&tkbc_send_message_queue, "\r\n", 2);
      }

      tkbc_fprintf(stderr, "INFO", "[MESSAGEHANDLER] %s", "HELLO\n");
    } break;
    case MESSAGE_KITEADD: {
      size_t kite_id;
      float x, y, angle;
      Color color;
      if (!tkbc_parse_message_kite_value(lexer, &kite_id, &x, &y, &angle,
                                         &color)) {
        goto err;
      }

      tkbc_register_kite_from_values(kite_id, x, y, angle, color);
      // This assumes the server sends the first KITEADD to the client, that
      // contains his own kite;
      if (client.kite_id == -1) {
        client.kite_id = kite_id;
      }

      static bool first_message_kite_add = true;
      if (first_message_kite_add) {
        for (size_t k = 0; k < env->kite_array->count; ++k) {
          if (env->kite_array->elements[k].kite_id == kite_id) {
            env->kite_array->elements[k].is_kite_input_handler_active = true;
          }
        }
        first_message_kite_add = false;
      }

      tkbc_fprintf(stderr, "INFO", "[MESSAGEHANDLER] %s", "KITEADD\n");
    } break;
    case MESSAGE_KITES: {
      token = lexer_next(lexer);
      if (token.kind != NUMBER) {
        check_return(false);
      }
      size_t kite_count = atoi(lexer_token_to_cstr(lexer, &token));
      token = lexer_next(lexer);
      if (token.kind != PUNCT_COLON) {
        check_return(false);
      }
      for (size_t i = 0; i < kite_count; ++i) {
        if (!tkbc_parse_single_kite_value(lexer)) {
          goto err;
        }
      }

      // Disables the input controlling if a start position reset happen.
      for (size_t i = 0; i < env->kite_array->count; ++i) {
        env->kite_array->elements[i].is_kite_input_handler_active = false;
      }

      tkbc_fprintf(stderr, "INFO", "[MESSAGEHANDLER] %s", "KITES\n");
    } break;
    case MESSAGE_KITEVALUE: {
      if (!tkbc_parse_single_kite_value(lexer)) {
        goto err;
      }

      tkbc_fprintf(stderr, "INFO", "[MESSAGEHANDLER] %s", "KITEVALUE\n");
    } break;
    case MESSAGE_SCRIPT_BLOCK_FRAME_VALUE: {
      token = lexer_next(lexer);
      if (token.kind != NUMBER) {
        goto err;
      }
      size_t script_id = atoi(lexer_token_to_cstr(lexer, &token));
      bool contains = false;
      for (size_t id = 0; id < env->block_frames->count; ++id) {
        if (env->block_frames->elements[id].script_id == script_id) {
          contains = true;
          break;
        }
      }
      if (!contains) {
        goto err;
      }

      token = lexer_next(lexer);
      if (token.kind != PUNCT_COLON) {
        goto err;
      }
      token = lexer_next(lexer);
      if (token.kind != NUMBER) {
        goto err;
      }
      size_t frames_in_script_count = atoi(lexer_token_to_cstr(lexer, &token));

      token = lexer_next(lexer);
      if (token.kind != PUNCT_COLON) {
        goto err;
      }
      token = lexer_next(lexer);
      if (token.kind != NUMBER) {
        goto err;
      }
      env->server_script_block_index = atoi(lexer_token_to_cstr(lexer, &token));

      token = lexer_next(lexer);
      if (token.kind != PUNCT_COLON) {
        goto err;
      }

      if (!env->block_frame) {
        tkbc_load_next_script(env);
        assert(frames_in_script_count == env->block_frame->count);
      }
      if (env->block_frame->script_id != script_id) {
        tkbc_load_script_id(env, script_id);
      }

      tkbc_fprintf(stderr, "INFO", "[MESSAGEHANDLER] %s",
                   "SCRIPT_BLOCK_FRAME_VALUE\n");
    } break;
    case MESSAGE_CLIENTKITES: {
      token = lexer_next(lexer);
      if (token.kind != NUMBER) {
        goto err;
      }
      size_t amount = atoi(lexer_token_to_cstr(lexer, &token));
      token = lexer_next(lexer);
      if (token.kind != PUNCT_COLON) {
        goto err;
      }
      for (size_t i = 0; i < amount; ++i) {
        size_t kite_id;
        float x, y, angle;
        Color color;
        if (!tkbc_parse_message_kite_value(lexer, &kite_id, &x, &y, &angle,
                                           &color)) {
          goto err;
        }
        bool found = false;
        for (size_t j = 0; j < env->kite_array->count; ++j) {
          if (kite_id == env->kite_array->elements[j].kite_id) {
            env->kite_array->elements[j].kite->center.x = x;
            env->kite_array->elements[j].kite->center.y = y;
            env->kite_array->elements[j].kite->angle = angle;
            env->kite_array->elements[j].kite->body_color = color;
            found = true;
            break;
          }
        }

        if (!found) {
          // If the kite_id is not registered.
          tkbc_register_kite_from_values(kite_id, x, y, angle, color);
        }
      }

      tkbc_fprintf(stderr, "INFO", "[MESSAGEHANDLER] %s", "CLIENTKITES\n");
    } break;
    case MESSAGE_CLIENT_DISCONNECT: {
      token = lexer_next(lexer);
      if (token.kind != NUMBER) {
        check_return(false);
      }
      size_t kite_id = atoi(lexer_token_to_cstr(lexer, &token));
      token = lexer_next(lexer);
      if (token.kind != PUNCT_COLON) {
        check_return(false);
      }

      if (!tkbc_remove_kite_from_list(env->kite_array, kite_id)) {
        // TODO Should this be a client crash? If some other client kite was
        // not registered and is now not found, this should not trigger that
        // every client, that has nothing to do with this miss behavior of the
        // server, crashes.
        // The kite is not known by the client anyway.
        /* check_return(false); */
      }

      tkbc_fprintf(stderr, "INFO", "[MESSAGEHANDLER] %s", "CLIENT_DISCONNET\n");
    } break;
    default:
      tkbc_fprintf(stderr, "ERROR", "Unknown KIND: %d\n", kind);
      exit(1);
    }
    continue;

  err: {
    tkbc_dap(message, 0);
    message->count -= 1;
    char *rn = strstr(message->elements + lexer->position, "\r\n");
    if (rn != NULL) {
      int jump_length = rn + 2 - &lexer->content[lexer->position];
      lexer_chop_char(lexer, jump_length);
      continue;
    }
    tkbc_fprintf(stderr, "WARNING", "Message: %s\n", message->elements);
    check_return(false);
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
  message->count = 0;
  return ok;
}

// TODO: Remove the buffer from the static memory.

static char static_buffer[16 * 1024] = {0};

bool tkbc_check_is_less_than_max_allowed_capacity_and_handle(Message *message) {
  if (message->capacity < 256 * RECEIVE_QUEUE_SIZE) {
    return true;
  }
  tkbc_fprintf(stderr, "ERROR", "The size was bigger than 64KB: %zu\n",
               message->capacity);

  // Think about adding a small extra space (64bytes) in the initial allocation
  // because this thing will trigger reallocation in every case.
  tkbc_dap(message, 0);
  char *rest_ptr = strrchr(message->elements, '\n');

  unsigned long d =
      labs((long)(rest_ptr - (message->elements + message->count)));

  // The equal save one byte for the buffer null terminator that is set later.
  if (d >= sizeof(static_buffer)) {
    // This should not happen, because the max message length is less than
    // 16Kb.
    fprintf(stderr, "The amount of the unhandled read buffer => %lu\n", d);
    assert(0 && "ERROR:The scratch buffer is to small!");
  }

  if (d > 0) {
    memcpy(static_buffer, rest_ptr, d);
    static_buffer[d] = 0;
    message->count -= d;
  }
  return false;
}

/**
 * @brief The function handles the incoming messages from the server.
 *
 * @param message The message buffer to which the read message should be
 * appended.
 * @return True if the reading and parsing of the received messages from the
 * server was successful, otherwise false.
 */
bool message_queue_handler(Message *message) {
  if (message->capacity >= 64 * RECEIVE_QUEUE_SIZE) {
    message->capacity = RECEIVE_QUEUE_SIZE;
    free(message->elements);
    message->elements = NULL;
    message->elements = realloc(message->elements,
                                sizeof(*message->elements) * message->capacity);
  }

  ssize_t n = 0;
  message->count = 0;
  do {
    if (*static_buffer) {
      tkbc_dapc(message, static_buffer, strlen(static_buffer));
      memset(static_buffer, 0, sizeof(static_buffer));
    }

    if (message->count + RECEIVE_QUEUE_SIZE > message->capacity) {
      while (message->capacity < message->count + RECEIVE_QUEUE_SIZE) {
        message->capacity += RECEIVE_QUEUE_SIZE;
      }

      message->elements = realloc(
          message->elements, sizeof(*message->elements) * message->capacity);
      if (message->elements == NULL) {
        tkbc_fprintf(stderr, "ERROR", "Realloc failed!: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
      }
    }
    n = recv(client.socket_id, &message->elements[message->count],
             RECEIVE_QUEUE_SIZE, 0);

    if (n == -1) {
      break;
    }

    if (!tkbc_check_is_less_than_max_allowed_capacity_and_handle(message)) {
      break;
    }

    message->count += n;
  } while (n > 0);

#ifdef _WIN32
  if (n == -1) {
    int err_errno = WSAGetLastError();
    if (err_errno != WSAEWOULDBLOCK) {
      tkbc_fprintf(stderr, "ERROR", "Read: %d\n", err_errno);
      return false;
    }
    if (err_errno == WSAEWOULDBLOCK && message->count == 0) {
      return true;
    }
  }
#else
  if (n == -1) {
    if (errno != EAGAIN) {
      tkbc_fprintf(stderr, "ERROR", "Read: %s\n", strerror(errno));
      return false;
    }
    if (errno == EAGAIN && message->count == 0) {
      return true;
    }
  }
#endif // _WIN32

  if (!received_message_handler(message)) {
    if (n > 0) {
      assert(message->count < INT_MAX);
      tkbc_fprintf(stderr, "WARNING", "---------------------------------\n");
      fprintf(stderr, "%.*s", (int)message->count, message->elements);
      tkbc_fprintf(stderr, "WARNING", "---------------------------------\n");
    }
    return false;
  }
  return true;
}

/**
 * @brief The function constructs a message KITEVALUE out of the kite that is
 * associated with this current client. The result is written to the
 * send_message_queue.
 */
void tkbc_client_input_handler_kite() {
  for (size_t i = 0; i < env->kite_array->count; ++i) {
    if (env->kite_array->elements[i].kite_id == (size_t)client.kite_id) {
      Kite_State *kite_state = &env->kite_array->elements[i];

      if (IsKeyPressed(KEY_ONE)) {
        kite_state->is_kite_input_handler_active =
            !kite_state->is_kite_input_handler_active;
      }
      if (!kite_state->is_kite_input_handler_active) {
        return;
      }

      Vector2 pos = {
          .x = kite_state->kite->center.x,
          .y = kite_state->kite->center.y,
      };
      float angle = kite_state->kite->angle;

      tkbc_input_handler(*env->keymaps, kite_state);

      if (Vector2Equals(pos, kite_state->kite->center)) {
        if (FloatEquals(angle, kite_state->kite->angle)) {
          return;
        }
      }

      char buf[64] = {0};
      snprintf(buf, sizeof(buf), "%d", MESSAGE_KITEVALUE);
      tkbc_dapc(&tkbc_send_message_queue, buf, strlen(buf));
      tkbc_dap(&tkbc_send_message_queue, ':');
      tkbc_message_append_clientkite(client.kite_id, &tkbc_send_message_queue);
      tkbc_dapc(&tkbc_send_message_queue, "\r\n", 2);
      return;
    }
  }
}

/**
 * @brief The function appends the script found from the given script_id in the
 * block_frames to the given message structure.
 *
 * @param script_id The script number the should be appended.
 * @param message The message structure the should hold the data after the
 * computation.
 * @return True if the script was found and is correctly appended, otherwise
 * false.
 */
bool tkbc_message_append_script(size_t script_id, Message *message) {
  char buf[64] = {0};
  for (size_t i = 0; i < env->block_frames->count; ++i) {
    if (env->block_frames->elements[i].script_id == script_id) {
      tkbc_ptoa(buf, sizeof(buf), &script_id, TYPE_SIZE_T);
      tkbc_dapc(message, buf, strlen(buf));
      tkbc_dap(message, ':');

      Block_Frame *block_frame = &env->block_frames->elements[i];
      tkbc_ptoa(buf, sizeof(buf), &block_frame->count, TYPE_SIZE_T);
      tkbc_dapc(message, buf, strlen(buf));
      tkbc_dap(message, ':');
      for (size_t j = 0; j < block_frame->count; ++j) {
        tkbc_ptoa(buf, sizeof(buf), &block_frame->elements[j].block_index,
                  TYPE_SIZE_T);
        tkbc_dapc(message, buf, strlen(buf));
        tkbc_dap(message, ':');

        Frames *frames = &block_frame->elements[j];
        tkbc_ptoa(buf, sizeof(buf), &frames->count, TYPE_SIZE_T);
        tkbc_dapc(message, buf, strlen(buf));
        tkbc_dap(message, ':');
        for (size_t k = 0; k < frames->count; ++k) {
          tkbc_ptoa(buf, sizeof(buf), &frames->elements[k].index, TYPE_SIZE_T);
          tkbc_dapc(message, buf, strlen(buf));
          tkbc_dap(message, ':');
          tkbc_ptoa(buf, sizeof(buf), &frames->elements[k].finished, TYPE_INT);
          tkbc_dapc(message, buf, strlen(buf));
          tkbc_dap(message, ':');
          tkbc_ptoa(buf, sizeof(buf), &frames->elements[k].kind, TYPE_INT);
          tkbc_dapc(message, buf, strlen(buf));
          tkbc_dap(message, ':');

          assert(ACTION_KIND_COUNT == 9 &&
                 "NOT ALL THE Action_Kinds ARE IMPLEMENTED");
          switch (frames->elements[k].kind) {
          case KITE_QUIT:
          case KITE_WAIT: {
            Wait_Action action = frames->elements[k].action.as_wait;
            tkbc_ptoa(buf, sizeof(buf), &action.starttime, TYPE_DOUBLE);
            tkbc_dapc(message, buf, strlen(buf));
          } break;

          case KITE_MOVE:
          case KITE_MOVE_ADD: {
            Move_Action action = frames->elements[k].action.as_move;
            tkbc_ptoa(buf, sizeof(buf), &action.position.x, TYPE_FLOAT);
            tkbc_dapc(message, buf, strlen(buf));
            tkbc_dap(message, ':');
            tkbc_ptoa(buf, sizeof(buf), &action.position.y, TYPE_FLOAT);
            tkbc_dapc(message, buf, strlen(buf));
          } break;

          case KITE_ROTATION:
          case KITE_ROTATION_ADD: {
            Rotation_Action action = frames->elements[k].action.as_rotation;
            tkbc_ptoa(buf, sizeof(buf), &action.angle, TYPE_FLOAT);
            tkbc_dapc(message, buf, strlen(buf));
          } break;

          case KITE_TIP_ROTATION:
          case KITE_TIP_ROTATION_ADD: {
            Tip_Rotation_Action action =
                frames->elements[k].action.as_tip_rotation;
            tkbc_ptoa(buf, sizeof(buf), &action.tip, TYPE_INT);
            tkbc_dapc(message, buf, strlen(buf));
            tkbc_dap(message, ':');
            tkbc_ptoa(buf, sizeof(buf), &action.angle, TYPE_FLOAT);
            tkbc_dapc(message, buf, strlen(buf));
          } break;

          default:
            assert(0 && "UNREACHABLE tkbc_message_append_script()");
          }

          tkbc_dap(message, ':');
          tkbc_ptoa(buf, sizeof(buf), &frames->elements[k].duration,
                    TYPE_FLOAT);
          tkbc_dapc(message, buf, strlen(buf));

          Kite_Ids *kite_ids = &frames->elements[k].kite_id_array;
          if (kite_ids->count) {
            tkbc_dap(message, ':');
            tkbc_ptoa(buf, sizeof(buf), &kite_ids->count, TYPE_SIZE_T);
            tkbc_dapc(message, buf, strlen(buf));
            tkbc_dap(message, ':');
            tkbc_dap(message, '(');
            for (size_t id = 0; id < kite_ids->count; ++id) {
              tkbc_ptoa(buf, sizeof(buf), &kite_ids->elements[id], TYPE_SIZE_T);
              tkbc_dapc(message, buf, strlen(buf));
              tkbc_dap(message, ',');
            }
            message->count--;
            tkbc_dap(message, ')');
          }

          tkbc_dap(message, ':');
        }
      }
      return true;
    }
  }
  return false;
}

/**
 * @brief The function can be used to construct the message script out of the
 * currently registered block frames. The result is directly written to the
 * send_message_queue ready to be send to the server.
 *
 * @return True if the message script could be constructed, otherwise false.
 */
bool tkbc_message_script() {
  Message message = {0};
  bool ok = true;
  size_t counter = 0;
  for (size_t i = env->send_scripts; i < env->block_frames->count; ++i) {
    if (!tkbc_message_append_script(env->block_frames->elements[i].script_id,
                                    &message)) {
      tkbc_fprintf(stderr, "ERROR",
                   "The script could not be appended to the message.\n");
      check_return(false);
    }

    char buf[64] = {0};
    snprintf(buf, sizeof(buf), "%d", MESSAGE_SCRIPT);
    tkbc_dapc(&tkbc_send_message_queue, buf, strlen(buf));
    tkbc_dap(&tkbc_send_message_queue, ':');
    tkbc_dapc(&tkbc_send_message_queue, message.elements, message.count);
    tkbc_dapc(&tkbc_send_message_queue, "\r\n", 2);
    message.count = 0;
    counter++;

    // TODO: @Cleanup
    if (i == 0) {
      FILE *file = fopen("SCIPT_1_PROTOCOL_VERSION_" PROTOCOL_VERSION, "wb");
      for (size_t k = 0; k < tkbc_send_message_queue.count; ++k) {
        fprintf(file, "%c", tkbc_send_message_queue.elements[k]);
      }
      fclose(file);
    }
  }
check:
  env->send_scripts += counter;
  // env->block_frames->count -= counter;
  // if (!ok) {
  //   memmove(env->block_frames->elements,
  //   &env->block_frames->elements[counter],
  //           sizeof(*env->block_frames->elements) *
  //           env->block_frames->count);
  // }
  if (message.elements) {
    free(message.elements);
    message.elements = NULL;
  }
  return ok;
}

/**
 * @brief The function handles the files, that can be registered via drag and
 * drop, those can contain music and the scripts files that have a '.kite'
 * extension other files are ignored.
 */
void tkbc_client_file_handler() {
  tkbc_file_handler(env);
  if (env->block_frames->count > 0) {
    tkbc_message_script();
  }
}

void tkbc_message_kites_positions() {
  char buf[64] = {0};
  snprintf(buf, sizeof(buf), "%d", MESSAGE_KITES_POSITIONS);
  tkbc_dapc(&tkbc_send_message_queue, buf, strlen(buf));
  tkbc_dap(&tkbc_send_message_queue, ':');

  memset(buf, 0, sizeof(buf));
  snprintf(buf, sizeof(buf), "%zu", env->kite_array->count);
  tkbc_dapc(&tkbc_send_message_queue, buf, strlen(buf));

  tkbc_dap(&tkbc_send_message_queue, ':');
  for (size_t i = 0; i < env->kite_array->count; ++i) {
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "%zu", env->kite_array->elements[i].kite_id);
    tkbc_dapc(&tkbc_send_message_queue, buf, strlen(buf));

    tkbc_dap(&tkbc_send_message_queue, ':');
    memset(buf, 0, sizeof(buf));
    float x = env->kite_array->elements[i].kite->center.x;
    float y = env->kite_array->elements[i].kite->center.y;
    float angle = env->kite_array->elements[i].kite->angle;
    snprintf(buf, sizeof(buf), "(%f,%f):%f", x, y, angle);
    tkbc_dapc(&tkbc_send_message_queue, buf, strlen(buf));

    tkbc_dap(&tkbc_send_message_queue, ':');
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "%u",
             *(uint32_t *)&env->kite_array->elements[i].kite->body_color);
    tkbc_dapc(&tkbc_send_message_queue, buf, strlen(buf));
    tkbc_dap(&tkbc_send_message_queue, ':');
  }
  tkbc_dapc(&tkbc_send_message_queue, "\r\n", 2);
}

/**
 * @brief The function warps the user key inputs for script control into
 * messages that are send to the server.
 */
void tkbc_client_input_handler_script() {
  if (env->script_counter <= 0) {
    return;
  }

  // Hard reset to startposition angel 0
  // KEY_ENTER
  if (IsKeyDown(
          tkbc_hash_to_key(*env->keymaps, KMH_SET_KITES_TO_START_POSITION))) {
    char buf[64] = {0};
    snprintf(buf, sizeof(buf), "%d", MESSAGE_KITES_POSITIONS_RESET);
    tkbc_dapc(&tkbc_send_message_queue, buf, strlen(buf));
    tkbc_dap(&tkbc_send_message_queue, ':');
    tkbc_dapc(&tkbc_send_message_queue, "\r\n", 2);
  }

  // KEY_SPACE
  if (IsKeyPressed(
          tkbc_hash_to_key(*env->keymaps, KMH_TOGGLE_SCRIPT_EXECUTION))) {
    char buf[64] = {0};
    snprintf(buf, sizeof(buf), "%d", MESSAGE_SCRIPT_TOGGLE);
    tkbc_dapc(&tkbc_send_message_queue, buf, strlen(buf));
    tkbc_dap(&tkbc_send_message_queue, ':');
    tkbc_dapc(&tkbc_send_message_queue, "\r\n", 2);
  }

  // KEY_TAB
  if (IsKeyPressed(tkbc_hash_to_key(*env->keymaps, KMH_SWITCHES_NEXT_SCRIPT))) {
    char buf[64] = {0};
    snprintf(buf, sizeof(buf), "%d", MESSAGE_SCRIPT_NEXT);
    tkbc_dapc(&tkbc_send_message_queue, buf, strlen(buf));
    tkbc_dap(&tkbc_send_message_queue, ':');
    tkbc_dapc(&tkbc_send_message_queue, "\r\n", 2);
  }

  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && env->timeline_interaction) {
    int mouse_x = GetMouseX();
    float slider = env->timeline_front.x + env->timeline_front.width;
    float c = mouse_x - slider;
    bool drag_left = c <= 0;

    char buf[64] = {0};
    snprintf(buf, sizeof(buf), "%d", MESSAGE_SCRIPT_SCRUB);
    tkbc_dapc(&tkbc_send_message_queue, buf, strlen(buf));
    tkbc_dap(&tkbc_send_message_queue, ':');
    tkbc_dap(&tkbc_send_message_queue, drag_left);
    tkbc_dap(&tkbc_send_message_queue, ':');
    tkbc_dapc(&tkbc_send_message_queue, "\r\n", 2);
  }
}

/**
 * @brief The function is the entry point and sets up the client socket and the
 * connection to the server. The main event loop is created and managed. In
 * there message communication to server is handled as well as all the input
 * from the user.
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
#endif // _WIN32

  client.kite_id = -1;

  char *program_name = tkbc_shift_args(&argc, &argv);
  uint16_t port = 8080;
  const char *host = "127.0.0.1";
  if (tkbc_client_commandline_check(argc, program_name)) {
    const char *host_check = tkbc_shift_args(&argc, &argv);
    host = tkbc_host_parsing(host_check);

    char *port_check = tkbc_shift_args(&argc, &argv);
    port = tkbc_port_parsing(port_check);
  }

  client.socket_id = tkbc_client_socket_creation(host, port);

  const char *title = "TEAM KITE BALLETT CHOREOGRAPHER CLIENT";
  SetTraceLogLevel(LOG_NONE);
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, title);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(TARGET_FPS);
  env = tkbc_init_env();
  if (tkbc_load_keymaps_from_file(env->keymaps, ".tkbc-keymaps")) {
    tkbc_fprintf(stderr, "INFO", "No keympas are load from file.\n");
  }
  SetExitKey(tkbc_hash_to_key(*env->keymaps, KMH_QUIT_PROGRAM));
  tkbc_init_sound(40);

  size_t count = env->kite_array->count;
  sending_script_handler();
  env->kite_array->count = count;

  Message tkbc_receive_queue = {0};

  Popup disconnect = {0};
  bool sending = true;
  while (!WindowShouldClose()) {
    if (!message_queue_handler(&tkbc_receive_queue)) {
      break;
    }

    BeginDrawing();
    ClearBackground(TKBC_UI_SKYBLUE);
    if (sending) {
      sending = send_message_handler();
      if (!sending) {
        disconnect = tkbc_popup_message("The server has disconnected!");
      }
    }

    int interaction_disconnect = tkbc_check_popup_interaction(&disconnect);
    if (interaction_disconnect == 1) {
      break;
    } else if (interaction_disconnect == -1) {
      disconnect.active = false;
    }

    tkbc_update_kites_for_resize_window(env);
    tkbc_draw_kite_array(env->kite_array);
    tkbc_draw_ui(env);
    tkbc_popup_resize_disconnect(&disconnect);
    tkbc_draw_popup(&disconnect);
    EndDrawing();
    if (disconnect.active) {
      continue;
    }

    tkbc_client_file_handler();
    if (!env->keymaps_interaction) {
      tkbc_input_sound_handler(env);
      tkbc_client_input_handler_kite();
      tkbc_client_input_handler_script();
      // The end of the current frame has to be executed so ffmpeg gets the full
      // executed fame.
      tkbc_ffmpeg_handler(env, "output.mp4");
    }
    if (!sending) {
      // Clearing for offline continuation.
      tkbc_send_message_queue.count = 0;
    }
  };
  CloseWindow();

  if (tkbc_receive_queue.elements) {
    free(tkbc_receive_queue.elements);
    tkbc_receive_queue.elements = NULL;
  }
  if (tkbc_send_message_queue.elements) {
    free(tkbc_send_message_queue.elements);
    tkbc_send_message_queue.elements = NULL;
  }

  shutdown(client.socket_id, SHUT_WR);
  char buf[1024] = {0};
  int n;
  do {
    n = read(client.socket_id, buf, sizeof(buf));
  } while (n > 0);

  if (n == 0) {
    tkbc_fprintf(stderr, "INFO", "Could not read any more data.\n");
  }
  if (n < 0) {
    tkbc_fprintf(stderr, "ERROR", "Reading failed: %s\n", strerror(errno));
  }

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
#endif

  tkbc_sound_destroy(env->sound);
  tkbc_destroy_env(env);
  tkbc_fprintf(stderr, "INFO", "EXITED SUCCESSFULLY.\n");
  return 0;
}
