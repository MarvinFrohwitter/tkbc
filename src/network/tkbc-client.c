#include "tkbc-client.h"
#include "tkbc-network-common.h"

#include <raylib.h>
#include <raymath.h>

#define SPACE_IMPLEMENTATION
#include "../../external/space/space.h"
#undef SPACE_IMPLEMENTATION

#define TKBC_UTILS_IMPLEMENTATION
#include "../global/tkbc-utils.h"
#undef TKBC_UTILS_IMPLEMENTATION

#include "../choreographer/tkbc-asset-handler.h"
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
#include <ws2tcpip.h>
#else

#include <netdb.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#endif

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
#define LOADIMAGE

Env *env = {0};
Client client = {0};
Kite client_kite;
#define RECEIVE_QUEUE_SIZE 1024
// static bool first_message_kite_add = true;
Popup loading = {0};

Space kite_images_space = {0};
Kite_Images kite_images = {0};
Space kite_textures_space = {0};
Kite_Textures kite_textures = {0};

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
 * @param argc The command line argument count.
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
 * @brief This function can be used to create a new client socket and connect it
 * to the server.
 *
 * @param host The address of the server the client should connect to.
 * @param port The port where the server is available.
 * @return The client socket if the creation and connection has succeeded,
 * otherwise the program crashes.
 */
int tkbc_client_socket_creation(const char *host, const char *port) {
#ifdef _WIN32
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    assert(0 && "ERROR: WSAStartup()");
  } else {
    tkbc_fprintf(stderr, "INFO", "Initialization of WSAStartup() succeed.\n");
  }
#endif

  struct addrinfo hints, *servinfo, *rp;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;     // Use IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; // TCP

  // Get address info
  int gai = getaddrinfo(host, port, &hints, &servinfo);
  if (gai != 0) {
    tkbc_fprintf(stderr, "ERROR", "getaddrinfo: %s\n", gai_strerror(gai));
#ifdef _WIN32
    WSACleanup();
#endif
    exit(1);
  }

  //
  // Loop through all results and connect to the first we can
  int client_socket;
  for (rp = servinfo; rp != NULL; rp = rp->ai_next) {
    client_socket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (client_socket == -1) {
#ifdef _WIN32
      tkbc_fprintf(stderr, "ERROR", "%ld\n", WSAGetLastError());
#else
      tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
#endif
      continue;
    }

    //
    // Set SO_REUSEADDR
    int option = 1;
    int sso = setsockopt(client_socket, SOL_SOCKET, SO_REUSEADDR,
                         (char *)&option, sizeof(option));
    if (sso == -1) {
#ifdef _WIN32
      tkbc_fprintf(stderr, "ERROR", "%ld\n", WSAGetLastError());
#else
      tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
#endif
    }

    //
    // Connecting to the possible address.
    int connection_status = connect(client_socket, rp->ai_addr, rp->ai_addrlen);
    if (connection_status == -1) {
#ifdef _WIN32
      tkbc_fprintf(stderr, "ERROR", "%ld\n", WSAGetLastError());
      if (closesocket(client.socket_id) == -1) {
        tkbc_fprintf(stderr, "ERROR", "Could not close socket: %d\n",
                     WSAGetLastError());
      }
#else
      tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
      if (close(client.socket_id) == -1) {
        tkbc_fprintf(stderr, "ERROR", "Could not close socket: %s\n",
                     strerror(errno));
      }
#endif
      continue;
    }

    break; // Successfully connected
  }

  // ==============================================================
  if (rp == NULL) {
    tkbc_fprintf(stderr, "ERROR", "Failed to connect to %s:%s\n", host, port);
#ifdef _WIN32
    WSACleanup();
#endif
    exit(1);
  }
  // ==============================================================

  freeaddrinfo(servinfo); // Free the linked list

  //
  // Set the socket to non-blocking
#ifdef _WIN32
  u_long mode = 1; // 1 to enable non-blocking socket
  if (ioctlsocket(client_socket, FIONBIO, &mode) != 0) {
    tkbc_fprintf(stderr, "ERROR", "ioctlsocket(): %d\n", WSAGetLastError());
    closesocket(client_socket);
    WSACleanup();
    exit(1);
  }
#else
  int flags = fcntl(client_socket, F_GETFL, 0);
  fcntl(client_socket, F_SETFL, flags | O_NONBLOCK);
#endif

  tkbc_fprintf(stderr, "INFO", "Connected to server: %s:%s\n", host, port);
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
                                    float angle, Color color, size_t texture_id,
                                    bool is_reversed) {
  Kite_State kite_state = tkbc_init_kite();
  kite_state.kite_id = kite_id;
  kite_state.kite->center.x = x;
  kite_state.kite->center.y = y;
  kite_state.kite->angle = angle;
  kite_state.kite->body_color = color;
  kite_state.is_kite_reversed = is_reversed;
  kite_state.kite->texture_id = texture_id;

  assert(kite_textures.count > texture_id);
  tkbc_set_kite_texture(kite_state.kite, &kite_textures.elements[texture_id]);

  tkbc_kite_update_internal(kite_state.kite);
  tkbc_dap(env->kite_array, kite_state);
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
  if (client.send_msg_buffer.count) {
    // NOTE: this assumes the whole message buffer could be send in one go.
    ssize_t n = send(client.socket_id, client.send_msg_buffer.elements,
                     client.send_msg_buffer.count, 0);

    if (n == 0) {
      tkbc_fprintf(stderr, "ERROR", "No bytes where send to the server!\n");
      check_return(false);
    }
    if (n == -1) {
      space_dap(&client.msg_space, &client.send_msg_buffer, 0);
      tkbc_fprintf(stderr, "ERROR", "Could not broadcast message: %s\n",
                   client.send_msg_buffer.elements);
      tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
      check_return(false);
    }
  }
check:
  client.send_msg_buffer.count = 0;
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
  bool reset = true;
  Token token;
  bool ok = true;
  if (message->count == 0) {
    return ok;
  }

  Lexer *lexer =
      lexer_new(__FILE__, message->elements, message->count, message->i);
  do {
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
        const char quote = '\"';
        space_dapf(&client.msg_space, &client.send_msg_buffer,
                   "%d:%c%s" PROTOCOL_VERSION "%c:\r\n", MESSAGE_HELLO, quote,
                   "Hello server from client!", quote);
      }

      tkbc_fprintf(stderr, "MESSAGEHANDLER", "HELLO\n");
    } break;
    case MESSAGE_KITEADD: {
      size_t kite_id, texture_id;
      float x, y, angle;
      Color color;
      bool is_reversed;
      if (!tkbc_parse_message_kite_value(lexer, &kite_id, &x, &y, &angle,
                                         &color, &texture_id, &is_reversed)) {
        goto err;
      }

      tkbc_register_kite_from_values(kite_id, x, y, angle, color, texture_id,
                                     is_reversed);

      static bool first_message_kite_add = true;
      if (first_message_kite_add) {
        // This assumes the server sends the first KITEADD to the client, that
        // contains his own kite;
        if (client.kite_id == -1) {
          client.kite_id = kite_id;
        }
        Kite_State *kite_state = tkbc_get_kite_state_by_id(env, kite_id);
        if (kite_state) {
          kite_state->is_kite_input_handler_active = true;
          client_kite = *kite_state->kite;
        }
        first_message_kite_add = false;
      }

      tkbc_fprintf(stderr, "MESSAGEHANDLER", "KITEADD\n");
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
        if (!tkbc_parse_single_kite_value(lexer, -1)) {
          goto err;
        }
      }

      size_t server_width = 1920;
      size_t server_height = 1080;
      size_t width = env->window_width;
      size_t height = env->window_height;

      // Disables the input controlling if a start position reset happen.
      for (size_t i = 0; i < env->kite_array->count; ++i) {
        env->kite_array->elements[i].is_kite_input_handler_active = false;

        // Readjust for the possible different window sizes. The server
        // calculates everything with 1920x1080.
        Kite *kite = env->kite_array->elements[i].kite;
        kite->center.x = (kite->center.x / (float)server_width) * (float)width;
        kite->center.y =
            (kite->center.y / (float)server_height) * (float)height;
        kite->old_center.x =
            (kite->old_center.x / (float)server_width) * (float)width;
        kite->old_center.y =
            (kite->old_center.y / (float)server_height) * (float)height;
        tkbc_kite_update_internal(kite);
      }

      tkbc_fprintf(stderr, "MESSAGEHANDLER", "KITES\n");
    } break;
    case MESSAGE_KITEVALUE: {

      assert(client.kite_id != -1);
      if (!tkbc_parse_single_kite_value(lexer, client.kite_id)) {
        goto err;
      }

      tkbc_fprintf(stderr, "MESSAGEHANDLER", "KITEVALUE\n");
    } break;
    case MESSAGE_SCRIPT_BLOCK_FRAME_VALUE: {
      token = lexer_next(lexer);
      if (token.kind != NUMBER) {
        goto err;
      }
      env->server_script_id = atoi(lexer_token_to_cstr(lexer, &token));

      token = lexer_next(lexer);
      if (token.kind != PUNCT_COLON) {
        goto err;
      }
      token = lexer_next(lexer);
      if (token.kind != NUMBER) {
        goto err;
      }

      env->server_script_frames_in_script_count =
          atoi(lexer_token_to_cstr(lexer, &token));

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
      env->script_finished = true;

      tkbc_fprintf(stderr, "MESSAGEHANDLER", "SCRIPT_BLOCK_FRAME_VALUE\n");
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

      for (size_t i = 0; i < env->kite_array->count; ++i) {
        env->kite_array->elements[i].is_active = false;
      }

      for (size_t i = 0; i < amount; ++i) {
        size_t kite_id, texture_id;
        float x, y, angle;
        Color color;
        bool is_reversed;
        if (!tkbc_parse_message_kite_value(lexer, &kite_id, &x, &y, &angle,
                                           &color, &texture_id, &is_reversed)) {
          goto err;
        }

        Kite_State *kite_state = tkbc_get_kite_state_by_id(env, kite_id);
        if (kite_state == NULL) {
          // If the kite_id is not registered.
          tkbc_register_kite_from_values(kite_id, x, y, angle, color,
                                         texture_id, is_reversed);
          // NOTE: The kite_state defaults ensure. is_active = true;
        } else {
          kite_state->kite->center.x = x;
          kite_state->kite->center.y = y;
          kite_state->kite->angle = angle;
          kite_state->kite->body_color = color;
          kite_state->kite->texture_id = texture_id;
          kite_state->is_kite_reversed = is_reversed;
          assert(kite_textures.count > texture_id);
          tkbc_set_kite_texture(kite_state->kite,
                                &kite_textures.elements[texture_id]);
          kite_state->is_active = true;
        }
      }

      tkbc_fprintf(stderr, "MESSAGEHANDLER", "CLIENTKITES\n");
    } break;
    case MESSAGE_SCRIPT_PARSED: {
      loading.active = false;

      tkbc_fprintf(stderr, "MESSAGEHANDLER", "SCRIPT_PARSED\n");
    } break;
    case MESSAGE_SCRIPT_FINISHED: {
      env->script_finished = true;
      env->server_script_id = 0;
      env->server_script_kite_max_count = 0;

      tkbc_fprintf(stderr, "MESSAGEHANDLER", "SCRIPT_FINISHED\n");
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

      tkbc_fprintf(stderr, "MESSAGEHANDLER", "CLIENT_DISCONNET\n");
    } break;
    default:
      tkbc_fprintf(stderr, "ERROR", "Unknown KIND: %d\n", kind);
      goto err;
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
 * @brief The function handles the incoming messages from the server.
 *
 * @param message The message buffer to which the read message should be
 * appended.
 * @return True if the reading and parsing of the received messages from the
 * server was successful, otherwise false.
 */
bool message_queue_handler(Message *message) {

  if (message->count == 0 && message->capacity > 32 * 1024) {
    tkbc_fprintf(stderr, "INFO", "realloced message: old capacity: %zu",
                 message->capacity);
    free(message->elements);
    message->elements = NULL;
    message->capacity = 0;
  }

  size_t length = 1024;
  if (message->capacity < message->count + length) {
    if (message->capacity == 0) {
      message->capacity = length;
    }

    while (message->capacity < message->count + length) {
      message->capacity += length;
    }

    message->elements = realloc(message->elements,
                                sizeof(*message->elements) * message->capacity);

    if (message->elements == NULL) {
      fprintf(stderr,
              "The allocation for the dynamic array has failed in: %s: %d\n",
              __FILE__, __LINE__);
      abort();
    }
  }

  int n = recv(client.socket_id, message->elements + message->count, length, 0);

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
    return false;
  }

  message->count += n;

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
  Kite_State *kite_state = tkbc_get_kite_state_by_id(env, client.kite_id);
  if (!kite_state) {
    return;
  }

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

  tkbc_input_handler(env->keymaps, kite_state);

  if (Vector2Equals(pos, kite_state->kite->center)) {
    if (FloatEquals(angle, kite_state->kite->angle)) {
      return;
    }
  }

  if (tkbc_float_equals_epsilon(client_kite.angle, kite_state->kite->angle,
                                0.01) &&
      tkbc_vector2_equals_epsilon(client_kite.center, kite_state->kite->center,
                                  0.01)) {
    return;
  }
  client_kite = *kite_state->kite;

  space_dapf(&client.msg_space, &client.send_msg_buffer,
             "%d:", MESSAGE_KITEVALUE);
  tkbc_message_append_clientkite(client.kite_id, &client.send_msg_buffer,
                                 &client.msg_space);
  space_dapf(&client.msg_space, &client.send_msg_buffer, "\r\n");
}

/**
 * @brief The function appends the script found from the given script_id in the
 * block_frames to the given message structure.
 *
 * @param script_id The script number the should be appended.
 * @return True if the script was found and is correctly appended, otherwise
 * false.
 */
bool tkbc_message_append_script(size_t script_id) {

  for (size_t i = 0; i < env->block_frames->count; ++i) {
    if (env->block_frames->elements[i].script_id != script_id) {
      continue;
    }
    space_dapf(&client.msg_space, &client.send_msg_buffer, "%zu:", script_id);

    Block_Frame *block_frame = &env->block_frames->elements[i];
    space_dapf(&client.msg_space, &client.send_msg_buffer,
               "%zu:", block_frame->count);
    for (size_t j = 0; j < block_frame->count; ++j) {
      Frames *frames = &block_frame->elements[j];
      space_dapf(&client.msg_space, &client.send_msg_buffer,
                 "%zu:", frames->block_index);
      space_dapf(&client.msg_space, &client.send_msg_buffer,
                 "%zu:", frames->count);

      for (size_t k = 0; k < frames->count; ++k) {
        space_dapf(&client.msg_space, &client.send_msg_buffer,
                   "%zu:", frames->elements[k].index);
        space_dapf(&client.msg_space, &client.send_msg_buffer,
                   "%d:", frames->elements[k].finished);
        space_dapf(&client.msg_space, &client.send_msg_buffer,
                   "%d:", frames->elements[k].kind);

        assert(ACTION_KIND_COUNT == 9 &&
               "NOT ALL THE Action_Kinds ARE IMPLEMENTED");
        switch (frames->elements[k].kind) {
        case KITE_QUIT:
        case KITE_WAIT: {
          Wait_Action action = frames->elements[k].action.as_wait;
          space_dapf(&client.msg_space, &client.send_msg_buffer, "%lf",
                     action.starttime);
        } break;
        case KITE_MOVE:
        case KITE_MOVE_ADD: {
          Move_Action action = frames->elements[k].action.as_move;
          space_dapf(&client.msg_space, &client.send_msg_buffer, "%f:%f",
                     action.position.x, action.position.y);
        } break;
        case KITE_ROTATION:
        case KITE_ROTATION_ADD: {
          Rotation_Action action = frames->elements[k].action.as_rotation;
          space_dapf(&client.msg_space, &client.send_msg_buffer, "%f",
                     action.angle);
        } break;
        case KITE_TIP_ROTATION:
        case KITE_TIP_ROTATION_ADD: {
          Tip_Rotation_Action action =
              frames->elements[k].action.as_tip_rotation;
          space_dapf(&client.msg_space, &client.send_msg_buffer, "%d:%f",
                     action.tip, action.angle);
        } break;
        default:
          assert(0 && "UNREACHABLE tkbc_message_append_script()");
        }

        space_dapf(&client.msg_space, &client.send_msg_buffer, ":%f",
                   frames->elements[k].duration);

        Kite_Ids *kite_ids = &frames->elements[k].kite_id_array;
        if (kite_ids->count) {
          space_dapf(&client.msg_space, &client.send_msg_buffer, ":%zu:(",
                     kite_ids->count);
          for (size_t id = 0; id < kite_ids->count; ++id) {
            space_dapf(&client.msg_space, &client.send_msg_buffer, "%zu,",
                       kite_ids->elements[id]);
          }
          client.send_msg_buffer.count--;
          space_dapf(&client.msg_space, &client.send_msg_buffer, ")");
        }

        space_dapf(&client.msg_space, &client.send_msg_buffer, ":");
      }
    }
    return true;
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
  bool ok = true;
  space_dapf(&client.msg_space, &client.send_msg_buffer, "%d:%zu:\r\n",
             MESSAGE_SCRIPT_AMOUNT, env->block_frames->count);

  size_t counter = 0;
  for (size_t i = env->send_scripts; i < env->block_frames->count; ++i) {
    char buf[8];
    int size = snprintf(buf, sizeof(buf), "%d:", MESSAGE_SCRIPT);
    space_dapf(&client.msg_space, &client.send_msg_buffer, "%s", buf);

    if (!tkbc_message_append_script(env->block_frames->elements[i].script_id)) {
      tkbc_fprintf(stderr, "ERROR",
                   "The script could not be appended to the message.\n");
      client.send_msg_buffer.count -= size;
      check_return(false);
    }

    space_dapf(&client.msg_space, &client.send_msg_buffer, "\r\n");
    counter++;
  }
check:
  env->send_scripts += counter;
  return ok;
}

/**
 * @brief The function handles the files, that can be registered via drag and
 * drop, those can contain music and the scripts files that have a '.kite'
 * extension other files are ignored.
 */
void tkbc_client_file_handler() {
  size_t prev_kite_array_count = env->kite_array->count;

  tkbc_file_handler(env);
  if (env->block_frames->count > 0 &&
      env->block_frames->count - env->send_scripts > 0) {

    tkbc_message_script();
  }

  if (prev_kite_array_count != env->kite_array->count) {
    // Remove kites that are just generated for sending a script.
    assert(env->kite_array->count > prev_kite_array_count);
    for (size_t i = prev_kite_array_count; i < env->kite_array->count; ++i) {
      free(env->kite_array->elements[i].kite);
      env->kite_array->elements[i].kite = NULL;
    }

    env->kite_array->count = prev_kite_array_count;
  }
}

/**
 * @brief The function constructs a message KITES_POSITIONS and append it to the
 * send_message_queue.
 */
void tkbc_message_kites_positions() {
  space_dapf(&client.msg_space, &client.send_msg_buffer,
             "%d:%zu:", MESSAGE_KITES_POSITIONS, env->kite_array->count);

  for (size_t i = 0; i < env->kite_array->count; ++i) {
    Kite_State *kite_state = &env->kite_array->elements[i];
    tkbc_message_append_kite(kite_state, &client.send_msg_buffer,
                             &client.msg_space);
  }
  space_dapf(&client.msg_space, &client.send_msg_buffer, "\r\n");
}

/**
 * @brief The function warps the user key inputs for script control into
 * messages that are send to the server.
 */
void tkbc_client_input_handler_script() {
  if (env->block_frames->count <= 0) {
    return;
  }

  // Hard reset to startposition angel 0
  // KEY_ENTER
  if (IsKeyPressed(
          tkbc_hash_to_key(env->keymaps, KMH_SET_KITES_TO_START_POSITION))) {
    space_dapf(&client.msg_space, &client.send_msg_buffer, "%d:\r\n",
               MESSAGE_KITES_POSITIONS_RESET);
  }

  // KEY_SPACE
  if (IsKeyPressed(
          tkbc_hash_to_key(env->keymaps, KMH_TOGGLE_SCRIPT_EXECUTION))) {
    space_dapf(&client.msg_space, &client.send_msg_buffer, "%d:\r\n",
               MESSAGE_SCRIPT_TOGGLE);
  }

  // KEY_TAB
  if (env->new_script_selected) {
    // Script ids start from 1 so +1 is needed.
    space_dapf(&client.msg_space, &client.send_msg_buffer, "%d:%zu:\r\n",
               MESSAGE_SCRIPT_NEXT, env->script_menu_mouse_interaction_box + 1);
    env->new_script_selected = false;
  }

  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && env->timeline_interaction) {
    int mouse_x = GetMouseX();
    float slider = env->timeline_front.x + env->timeline_front.width;
    float c = mouse_x - slider;
    bool drag_left = c <= 0;

    space_dapf(&client.msg_space, &client.send_msg_buffer, "%d:%d:\r\n",
               MESSAGE_SCRIPT_SCRUB, drag_left);
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
  const char *port = "8080";
  const char *host = "127.0.0.1";
  if (tkbc_client_commandline_check(argc, program_name)) {
    host = tkbc_shift_args(&argc, &argv);
    port = tkbc_shift_args(&argc, &argv);
  }

  client.socket_id = tkbc_client_socket_creation(host, port);

  const char *title = "TEAM KITE BALLETT CHOREOGRAPHER CLIENT";
  SetTraceLogLevel(LOG_NONE);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT |
                 FLAG_WINDOW_MINIMIZED | FLAG_WINDOW_MAXIMIZED);
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, title);
  SetWindowMaxSize(SCREEN_WIDTH, SCREEN_HEIGHT);
  SetTargetFPS(TARGET_FPS);

  srand(time(NULL));
  env = tkbc_init_env();
  if (tkbc_load_keymaps_from_file(&env->keymaps, ".tkbc-keymaps")) {
    tkbc_fprintf(stderr, "INFO", "No keympas are load from file.\n");
  }
  SetExitKey(tkbc_hash_to_key(env->keymaps, KMH_QUIT_PROGRAM));
  tkbc_init_sound(40);

#ifdef LOADIMAGE
  tkbc_load_kite_images_and_textures();
#endif /* ifdef LOADIMAGE */

  size_t prev_kite_array_count = env->kite_array->count;
  sending_script_handler();
  if (prev_kite_array_count != env->kite_array->count) {
    // Remove kites that are just generated for sending a script.
    assert(env->kite_array->count > prev_kite_array_count);
    for (size_t i = prev_kite_array_count; i < env->kite_array->count; ++i) {
      free(env->kite_array->elements[i].kite);
      env->kite_array->elements[i].kite = NULL;
    }
    env->kite_array->count = prev_kite_array_count;
  }

  Popup disconnect = tkbc_popup_message("The server has disconnected!");
  loading = tkbc_popup_message("Waiting for server.");
  loading.active = true;

  bool sending_receiving = true;
  while (!WindowShouldClose()) {

    if (sending_receiving) {
      if (!message_queue_handler(&client.recv_msg_buffer)) {
        disconnect.active = true;
      }
      sending_receiving = send_message_handler();
    }

    BeginDrawing();
    ClearBackground(TKBC_UI_SKYBLUE);

    if (tkbc_check_popup_interaction(&loading)) {
      break;
    }

    int interaction = tkbc_check_popup_interaction(&disconnect);
    if (interaction == 1) {
      break;
    } else if (interaction == -1) {
      disconnect.active = false;
      loading.active = false;
    }

    if (loading.active) {
      tkbc_popup_resize(&loading);
      tkbc_draw_popup(&loading);
    } else {
      tkbc_update_kites_for_resize_window(env);
      tkbc_draw_kite_array(env->kite_array);
      tkbc_draw_ui(env);
    }

    if (disconnect.active) {
      sending_receiving = false;
      // Clearing for offline continuation.
      client.send_msg_buffer.count = 0;
      tkbc_popup_resize(&disconnect);
      tkbc_draw_popup(&disconnect);
    }

    EndDrawing();
    if (disconnect.active || loading.active || client.kite_id == -1) {
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
  };
  CloseWindow();

  client.recv_msg_buffer.elements = NULL;
  client.send_msg_buffer.elements = NULL;
  space_free_space(&client.msg_space);

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

  tkbc_assets_destroy();
  space_free_space(&kite_images_space);
  space_free_space(&kite_textures_space);
  tkbc_fprintf(stderr, "INFO", "EXITED SUCCESSFULLY.\n");
  return 0;
}
