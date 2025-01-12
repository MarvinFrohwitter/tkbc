#include "tkbc-client.h"
#include "tkbc-network-common.h"

#define TKBC_UTILS_IMPLEMENTATION
#include "../global/tkbc-utils.h"

#include "../choreographer/tkbc-ffmpeg.h"
#include "../choreographer/tkbc-input-handler.h"
#include "../choreographer/tkbc-script-api.h"
#include "../choreographer/tkbc-sound-handler.h"
#include "../choreographer/tkbc.h"
#include "../global/tkbc-popup.h"

#include "../../external/lexer/tkbc-lexer.h"
#include "../../tkbc_scripts/first.c"

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define WINDOW_SCALE 120
#define SCREEN_WIDTH 16 * WINDOW_SCALE
#define SCREEN_HEIGHT 9 * WINDOW_SCALE
#define TKBC_CLIENT

Env *env = {0};
Client client = {0};
#define RECEIVE_QUEUE_SIZE 1024
Message receive_queue = {0};
Message send_message_queue = {0};

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
    tkbc_fprintf(stderr, "ERROR",
                 "The default localhost and port 8080 is used.\n");
    return false;
  }
  return true;
}

/**
 * @brief The function checks if the given sting is a valid host address.
 *
 * @param host_check The possible sting that can contain the host.
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
  int client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (client_socket == -1) {
    tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
    exit(1);
  }

  int option = 1;
  int sso = setsockopt(client_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&option,
                       sizeof(option));
  if (sso == -1) {
    tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
  }

  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port);
  server_address.sin_addr.s_addr = inet_addr(addr);

  int connection_status =
      connect(client_socket, (struct sockaddr *)&server_address,
              sizeof(server_address));

  if (connection_status == -1) {
    tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
    exit(1);
  }

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
  tkbc_center_rotation(kite_state->kite, NULL, kite_state->kite->angle);
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
  if (send_message_queue.count) {
    // NOTE: this assumes the whole message buffer could be send in one go.
    ssize_t n = send(client.socket_id, send_message_queue.elements,
                     send_message_queue.count, MSG_NOSIGNAL);
    if (n == 0) {
      tkbc_fprintf(stderr, "ERROR", "No bytes where send to the server!\n");
      check_return(false);
    }
    if (n == -1) {
      tkbc_dap(&send_message_queue, 0);
      tkbc_fprintf(stderr, "ERROR", "Could not broadcast message: %s\n",
                   send_message_queue.elements);
      tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));
      check_return(false);
    }
  }
check:
  send_message_queue.count = 0;
  return ok;
}

/**
 * @brief The function parses the incoming messages from the server and handles
 * the resulting behavior.
 *
 * @return True if the parsing was successful an all resulting actions could be
 * handled, otherwise false and a parsing error has occurred.
 */
bool received_message_handler() {
  Message message = {0};
  Token token;
  bool ok = true;
  if (receive_queue.count == 0) {
    check_return(true);
  }
  tkbc_dapc(&message, receive_queue.elements, receive_queue.count);
  tkbc_dap(&message, 0);
  receive_queue.count = 0;

  Lexer *lexer = lexer_new(__FILE__, message.elements, message.count, 0);
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

    assert(MESSAGE_COUNT == 13);
    switch (kind) {
    case MESSAGE_HELLO: {
      token = lexer_next(lexer);
      if (token.kind != STRINGLITERAL) {
        check_return(false);
      }

      const char *greeting = "\"Hello client from server!1.0\"";
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
        tkbc_dapc(&send_message_queue, buf, strlen(buf));
        tkbc_dap(&send_message_queue, ':');

        tkbc_dapc(&send_message_queue, "\"", 1);
        const char *m = "Hello server from client!";
        tkbc_dapc(&send_message_queue, m, strlen(m));

        tkbc_dapc(&send_message_queue, PROTOCOL_VERSION,
                  strlen(PROTOCOL_VERSION));
        tkbc_dapc(&send_message_queue, "\"", 1);
        tkbc_dap(&send_message_queue, ':');
        tkbc_dapc(&send_message_queue, "\r\n", 2);
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
      env->server_script_block_index = atoi(lexer_token_to_cstr(lexer, &token));
      token = lexer_next(lexer);
      if (token.kind != PUNCT_COLON) {
        goto err;
      }
      token = lexer_next(lexer);
      if (token.kind != NUMBER) {
        goto err;
      }
      env->server_script_block_index_count =
          atoi(lexer_token_to_cstr(lexer, &token));
      token = lexer_next(lexer);
      if (token.kind != PUNCT_COLON) {
        goto err;
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

      for (size_t i = 0; i < env->kite_array->count; ++i) {
        if (env->kite_array->elements[i].kite_id == kite_id) {
          if (i + 1 < env->kite_array->count) {
            memmove(&env->kite_array->elements[i],
                    &env->kite_array->elements[i + 1],
                    sizeof(*env->kite_array->elements) *
                        (env->kite_array->count - 1 - i));
          }
          env->kite_array->count -= 1;
          break;
        }
      }

      tkbc_fprintf(stderr, "INFO", "[MESSAGEHANDLER] %s", "CLIENT_DISCONNET\n");
    } break;
    default:
      tkbc_fprintf(stderr, "ERROR", "Unknown KIND: %d\n", kind);
      exit(1);
    }
    continue;

  err: {
    tkbc_dap(&message, 0);
    message.count -= 1;
    char *rn = strstr(message.elements + lexer->position, "\r\n");
    if (rn != NULL) {
      int jump_length = rn + 2 - &lexer->content[lexer->position];
      lexer_chop_char(lexer, jump_length);
      continue;
    }
    tkbc_fprintf(stderr, "WARNING", "Message: %s\n", message.elements);
    check_return(false);
  }
  } while (token.kind != EOF_TOKEN);

  lexer_del(lexer);
check:
  return ok;
}

/**
 * @brief The function handles the incoming messages from the server.
 *
 * @return True if the reading and parsing of the received messages from the
 * server was successful, otherwise false.
 */
bool message_queue_handler() {
  if (receive_queue.capacity >= 32 * RECEIVE_QUEUE_SIZE) {
    receive_queue.elements =
        realloc(receive_queue.elements,
                sizeof(*receive_queue.elements) * RECEIVE_QUEUE_SIZE);
  }

  ssize_t n = 0;
  receive_queue.count = 0;
  do {
    n = recv(client.socket_id, &receive_queue.elements[receive_queue.count],
             RECEIVE_QUEUE_SIZE, MSG_NOSIGNAL | MSG_DONTWAIT);
    if (receive_queue.capacity >= 32 * RECEIVE_QUEUE_SIZE) {
      tkbc_fprintf(stderr, "ERROR", "32 : %zu\n", receive_queue.capacity);
      assert(0 && "The receive_queue is massive");
    }
    if (receive_queue.capacity >= 16 * RECEIVE_QUEUE_SIZE) {
      tkbc_fprintf(stderr, "ERROR", "16 : %zu\n", receive_queue.capacity);
      assert(0 && "The receive_queue is massive");
    }

    if (n == -1) {
      break;
    }
    receive_queue.count += n;
    if (receive_queue.count >= receive_queue.capacity) {
      receive_queue.capacity += RECEIVE_QUEUE_SIZE;
      receive_queue.elements =
          realloc(receive_queue.elements,
                  sizeof(*receive_queue.elements) * receive_queue.capacity);
      if (receive_queue.elements == NULL) {
        tkbc_fprintf(stderr, "ERROR", "realloc() failed!\n");
      }
    }
  } while (n > 0);

  if (n == -1) {
    if (errno != EAGAIN) {
      tkbc_fprintf(stderr, "ERROR", "Read: %d\n", errno);
      tkbc_fprintf(stderr, "ERROR", "Read: %s\n", strerror(errno));
      return false;
    }
  }

  if (!received_message_handler()) {
    if (n > 0) {
      tkbc_fprintf(stderr, "WARNING", "---------------------------------\n");
      for (size_t i = 0; i < receive_queue.count; ++i) {
        fprintf(stderr, "%c", receive_queue.elements[i]);
      }
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
      Vector2 pos = {
          .x = kite_state->kite->center.x,
          .y = kite_state->kite->center.y,
      };
      float angle = kite_state->kite->angle;

      kite_state->kite_input_handler_active = true;
      tkbc_input_handler(kite_state);

      if (Vector2Equals(pos, kite_state->kite->center)) {
        if (FloatEquals(angle, kite_state->kite->angle)) {
          return;
        }
      }

      char buf[64] = {0};
      snprintf(buf, sizeof(buf), "%d", MESSAGE_KITEVALUE);
      tkbc_dapc(&send_message_queue, buf, strlen(buf));
      tkbc_dap(&send_message_queue, ':');
      tkbc_message_append_clientkite(client.kite_id, &send_message_queue);
      tkbc_dapc(&send_message_queue, "\r\n", 2);
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
      tkbc_ptoa(buf, sizeof(buf), &script_id, SIZE_T);
      tkbc_dapc(message, buf, strlen(buf));
      tkbc_dap(message, ':');

      Block_Frame *block_frame = &env->block_frames->elements[i];
      tkbc_ptoa(buf, sizeof(buf), &block_frame->count, SIZE_T);
      tkbc_dapc(message, buf, strlen(buf));
      tkbc_dap(message, ':');
      for (size_t j = 0; j < block_frame->count; ++j) {
        tkbc_ptoa(buf, sizeof(buf), &block_frame->elements[j].block_index,
                  SIZE_T);
        tkbc_dapc(message, buf, strlen(buf));
        tkbc_dap(message, ':');

        Frames *frames = &block_frame->elements[j];
        tkbc_ptoa(buf, sizeof(buf), &frames->count, SIZE_T);
        tkbc_dapc(message, buf, strlen(buf));
        tkbc_dap(message, ':');
        for (size_t k = 0; k < frames->count; ++k) {
          tkbc_ptoa(buf, sizeof(buf), &frames->elements[k].index, SIZE_T);
          tkbc_dapc(message, buf, strlen(buf));
          tkbc_dap(message, ':');
          tkbc_ptoa(buf, sizeof(buf), &frames->elements[k].finished, INT);
          tkbc_dapc(message, buf, strlen(buf));
          tkbc_dap(message, ':');
          tkbc_ptoa(buf, sizeof(buf), &frames->elements[k].kind, INT);
          tkbc_dapc(message, buf, strlen(buf));
          tkbc_dap(message, ':');

          assert(ACTION_KIND_COUNT == 9 &&
                 "NOT ALL THE Action_Kinds ARE IMPLEMENTED");
          switch (frames->elements[k].kind) {
          case KITE_QUIT:
          case KITE_WAIT: {
            Wait_Action *action = frames->elements[k].action;
            tkbc_ptoa(buf, sizeof(buf), &action->starttime, DOUBLE);
            tkbc_dapc(message, buf, strlen(buf));
          } break;

          case KITE_MOVE:
          case KITE_MOVE_ADD: {
            Move_Action *action = frames->elements[k].action;
            tkbc_ptoa(buf, sizeof(buf), &action->position.x, FLOAT);
            tkbc_dapc(message, buf, strlen(buf));
            tkbc_dap(message, ':');
            tkbc_ptoa(buf, sizeof(buf), &action->position.y, FLOAT);
            tkbc_dapc(message, buf, strlen(buf));
          } break;

          case KITE_ROTATION:
          case KITE_ROTATION_ADD: {
            Rotation_Action *action = frames->elements[k].action;
            tkbc_ptoa(buf, sizeof(buf), &action->angle, FLOAT);
            tkbc_dapc(message, buf, strlen(buf));
          } break;

          case KITE_TIP_ROTATION:
          case KITE_TIP_ROTATION_ADD: {
            Tip_Rotation_Action *action = frames->elements[k].action;
            tkbc_ptoa(buf, sizeof(buf), &action->tip, INT);
            tkbc_dapc(message, buf, strlen(buf));
            tkbc_dap(message, ':');
            tkbc_ptoa(buf, sizeof(buf), &action->angle, FLOAT);
            tkbc_dapc(message, buf, strlen(buf));
          } break;

          default:
            assert(0 && "UNREACHABLE tkbc_message_append_script()");
          }

          tkbc_dap(message, ':');
          tkbc_ptoa(buf, sizeof(buf), &frames->elements[k].duration, FLOAT);
          tkbc_dapc(message, buf, strlen(buf));

          Kite_Ids *kite_ids = frames->elements[k].kite_id_array;
          if (kite_ids) {
            tkbc_dap(message, ':');
            tkbc_ptoa(buf, sizeof(buf), &kite_ids->count, SIZE_T);
            tkbc_dapc(message, buf, strlen(buf));
            tkbc_dap(message, ':');
            tkbc_dap(message, '(');
            for (size_t id = 0; id < kite_ids->count; ++id) {
              tkbc_ptoa(buf, sizeof(buf), &kite_ids->elements[id], SIZE_T);
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
    tkbc_dapc(&send_message_queue, buf, strlen(buf));
    tkbc_dap(&send_message_queue, ':');
    tkbc_dapc(&send_message_queue, message.elements, message.count);
    tkbc_dapc(&send_message_queue, "\r\n", 2);
    message.count = 0;
    counter++;

    // TODO: @Cleanup
    if (i == 0) {
      FILE *file = fopen("SCIPT_1_PROTOCOL_VERSION_" PROTOCOL_VERSION, "wb");
      for (size_t k = 0; k < send_message_queue.count; ++k) {
        fprintf(file, "%c", send_message_queue.elements[k]);
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

/**
 * @brief The function warps the user key inputs for script control into
 * messages that are send to the server.
 */
void tkbc_client_input_handler_script() {
  if (env->script_counter <= 0) {
    return;
  }

  // Hard reset to startposition angel 0
  if (IsKeyDown(KEY_ENTER)) {
    tkbc_kite_array_start_position(env->kite_array, env->window_width,
                                   env->window_height);
    char buf[64] = {0};
    snprintf(buf, sizeof(buf), "%d", MESSAGE_KITES_POSITIONS);
    tkbc_dapc(&send_message_queue, buf, strlen(buf));
    tkbc_dap(&send_message_queue, ':');

    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "%zu", env->kite_array->count);
    tkbc_dapc(&send_message_queue, buf, strlen(buf));

    tkbc_dap(&send_message_queue, ':');
    for (size_t i = 0; i < env->kite_array->count; ++i) {
      memset(buf, 0, sizeof(buf));
      snprintf(buf, sizeof(buf), "%zu", env->kite_array->elements[i].kite_id);
      tkbc_dapc(&send_message_queue, buf, strlen(buf));

      tkbc_dap(&send_message_queue, ':');
      memset(buf, 0, sizeof(buf));
      float x = env->kite_array->elements[i].kite->center.x;
      float y = env->kite_array->elements[i].kite->center.y;
      float angle = env->kite_array->elements[i].kite->angle;
      snprintf(buf, sizeof(buf), "(%f,%f):%f", x, y, angle);
      tkbc_dapc(&send_message_queue, buf, strlen(buf));

      tkbc_dap(&send_message_queue, ':');
      memset(buf, 0, sizeof(buf));
      snprintf(buf, sizeof(buf), "%u",
               *(uint32_t *)&env->kite_array->elements[i].kite->body_color);
      tkbc_dapc(&send_message_queue, buf, strlen(buf));
      tkbc_dap(&send_message_queue, ':');
    }
    tkbc_dapc(&send_message_queue, "\r\n", 2);
  }

  if (IsKeyPressed(KEY_SPACE)) {
    char buf[64] = {0};
    snprintf(buf, sizeof(buf), "%d", MESSAGE_SCRIPT_TOGGLE);
    tkbc_dapc(&send_message_queue, buf, strlen(buf));
    tkbc_dap(&send_message_queue, ':');
    tkbc_dapc(&send_message_queue, "\r\n", 2);
  }

  if (IsKeyPressed(KEY_TAB)) {
    char buf[64] = {0};
    snprintf(buf, sizeof(buf), "%d", MESSAGE_SCRIPT_NEXT);
    tkbc_dapc(&send_message_queue, buf, strlen(buf));
    tkbc_dap(&send_message_queue, ':');
    tkbc_dapc(&send_message_queue, "\r\n", 2);
  }

  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && env->timeline_interaction) {
    int mouse_x = GetMouseX();
    float slider = env->timeline_front.x + env->timeline_front.width;
    float c = mouse_x - slider;
    bool drag_left = c <= 0;

    char buf[64] = {0};
    snprintf(buf, sizeof(buf), "%d", MESSAGE_SCRIPT_SCRUB);
    tkbc_dapc(&send_message_queue, buf, strlen(buf));
    tkbc_dap(&send_message_queue, ':');
    tkbc_dap(&send_message_queue, drag_left);
    tkbc_dap(&send_message_queue, ':');
    tkbc_dapc(&send_message_queue, "\r\n", 2);
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
  SetExitKey(KEY_ESCAPE);
  tkbc_init_sound(40);
  env = tkbc_init_env();

  size_t count = env->kite_array->count;
  sending_script_handler();
  env->kite_array->count = count;

  receive_queue.elements = malloc(RECEIVE_QUEUE_SIZE);
  receive_queue.capacity += RECEIVE_QUEUE_SIZE;

  Popup disconnect = {0};
  bool sending = true;
  while (!WindowShouldClose()) {
    if (!message_queue_handler()) {
      break;
    }

    BeginDrawing();
    ClearBackground(SKYBLUE);
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

    tkbc_draw_kite_array(env->kite_array);
    tkbc_draw_ui(env);
    tkbc_popup_resize_disconnect(&disconnect);
    tkbc_draw_popup(&disconnect);
    EndDrawing();
    if (disconnect.active) {
      continue;
    }

    tkbc_client_file_handler();
    tkbc_input_sound_handler(env);
    tkbc_client_input_handler_kite();
    tkbc_client_input_handler_script();
    // The end of the current frame has to be executed so ffmpeg gets the full
    // executed fame.
    tkbc_ffmpeg_handler(env, "output.mp4");
    if (!sending) {
      // Clearing for offline continuation.
      send_message_queue.count = 0;
    }
  };
  CloseWindow();

  if (receive_queue.elements) {
    free(receive_queue.elements);
  }
  if (send_message_queue.elements) {
    free(send_message_queue.elements);
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
  if (close(client.socket_id) == -1) {
    tkbc_fprintf(stderr, "ERROR", "Could not close socket: %s\n",
                 strerror(errno));
  }

  tkbc_sound_destroy(env->sound);
  tkbc_destroy_env(env);
  tkbc_fprintf(stderr, "INFO", "EXITED SUCCESSFULLY.\n");
  return 0;
}
