#include "tkbc-server-client-handler.h"
#include "tkbc-network-common.h"
#include "tkbc-server.h"

#include "../../external/lexer/tkbc-lexer.h"
#include "../choreographer/tkbc-script-api.h"
#include "../choreographer/tkbc-script-handler.h"
#include "../choreographer/tkbc.h"
#include "../global/tkbc-utils.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

extern Env *env;
extern Clients *clients;
extern pthread_t threads[SERVER_CONNETCTIONS];
extern pthread_mutex_t mutex;

/**
 * @brief The function shutdown the client connection that is given by the
 * client argument.
 *
 * @param client The client connection that should be closed.
 * @param force Represents that every action that might not shutdown the client
 * immediately is omitted.
 */
void tkbc_server_shutdown_client(Client client, bool force) {
  pthread_t thread_id = client.thread_id;
  shutdown(client.socket_id, SHUT_WR);
  for (char b[1024];
       recv(client.socket_id, b, sizeof(b), MSG_NOSIGNAL | MSG_DONTWAIT) > 0;)
    ;
  if (close(client.socket_id) == -1) {
    tkbc_fprintf(stderr, "ERROR", "Close socket: %s\n", strerror(errno));
  }
  if (force) {
    goto force;
  }

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

  Clients cs = {0};
  int err = pthread_mutex_lock(&mutex);
  if (err != 0) {
    if (err == EDEADLK) {
      tkbc_fprintf(stderr, "WARNING", "The mutex lock is already set.\n");
    } else {
      assert(0 && "ERROR:mutex lock");
    }
  }

  if (!tkbc_server_brodcast_all_exept(&cs, client.kite_id, message.elements)) {
    for (size_t i = 0; i < cs.count; ++i) {
      // This is not a reinvocation of any client shutdown because the message
      // is just broadcasted to all except this one and if the other clients are
      // in the middle of a shutdown the thread is closed correctly by this call
      // the execution of the other shutdown is killed.
      pthread_mutex_lock(&mutex);
      tkbc_server_shutdown_client(cs.elements[i], false);
    }
  }
  pthread_mutex_unlock(&mutex);
  if (cs.elements) {
    free(cs.elements);
    cs.elements = NULL;
  }
  if (message.elements) {
    free(message.elements);
    message.elements = NULL;
  }

force: {}
  int err_check = tkbc_server_remove_client(client, false);
  if (1 == err_check) {
    tkbc_fprintf(stderr, "INFO",
                 "Client:" CLIENT_FMT
                 ":Kite could not be removed: not found.\n",
                 CLIENT_ARG(client));
    if (0 != tkbc_server_remove_client(client, true)) {
      tkbc_fprintf(stderr, "INFO",
                   "Client:" CLIENT_FMT
                   ": could not be removed after broken pipe: not found.\n",
                   CLIENT_ARG(client));
    }
  }
  if (-1 == err_check) {
    tkbc_fprintf(stderr, "INFO",
                 "Client:" CLIENT_FMT
                 ": could not be removed after broken pipe: not found.\n",
                 CLIENT_ARG(client));
  }

  if (pthread_cancel(threads[thread_id]) != 0) {
    tkbc_fprintf(stderr, "INFO", "Client: Thread is not valid\n");
  }
}

/**
 * @brief The function sends the given message to the specified client.
 *
 * @param client The client where the message should be send to.
 * @param message The data that should be send to the client.
 * @return False if an error has occurred while sending the message data to the
 * client.
 */
bool tkbc_server_brodcast_client(Client client, const char *message) {

  ssize_t send_check =
      send(client.socket_id, message, strlen(message), MSG_NOSIGNAL);
  if (send_check == 0) {
    tkbc_fprintf(stderr, "ERROR",
                 "No bytes where send to the client:" CLIENT_FMT "\n",
                 CLIENT_ARG(client));
  }
  if (send_check == -1) {
    tkbc_fprintf(stderr, "ERROR",
                 "Client:" CLIENT_FMT ":Could not broadcast message: %s\n",
                 CLIENT_ARG(client), message);
    tkbc_fprintf(stderr, "ERROR", "%s\n", strerror(errno));

    return false;
  } else {
    tkbc_fprintf(stderr, "INFO", "The amount %ld send to:" CLIENT_FMT "\n",
                 send_check, CLIENT_ARG(client));
  }
  return true;
}

/**
 * @brief The function sends the given message to all registered clients except
 * the specified client_id.
 *
 * @param cs The struct that contains all the clients where the message could
 * not be send to.
 * @param client_id The id that specifies the one client that should not get the
 * message.
 * @param message The data that should be send to the clients.
 * @return True if no errors occurred and the cs is empty, otherwise false and
 * cs is filled with the client connections that had errors.
 */
bool tkbc_server_brodcast_all_exept(Clients *cs, size_t client_id,
                                    const char *message) {
  bool ok = true;
  for (size_t i = 0; i < clients->count; ++i) {
    if ((size_t)clients->elements[i].kite_id != client_id) {
      if (!tkbc_server_brodcast_client(clients->elements[i], message)) {
        tkbc_dap(cs, clients->elements[i]);
        ok = false;
      }
    }
  }
  return ok;
}

/**
 * @brief The function sends the given message to all registered clients.
 *
 * @param cs The struct that contains all the clients where the message could
 * not be send to.
 * @param message The data that should be send to the clients.
 * @return True if no errors occurred and the cs is empty, otherwise false and
 * cs is filled with the client connections that had errors.
 */
bool tkbc_server_brodcast_all(Clients *cs, const char *message) {
  bool ok = true;
  for (size_t i = 0; i < clients->count; ++i) {
    if (!tkbc_server_brodcast_client(clients->elements[i], message)) {
      tkbc_dap(cs, clients->elements[i]);
      ok = false;
    }
  }
  return ok;
}

/**
 * @brief The function constructs the hello message. That is used to establish
 * the handshake with the server respecting the current PROTOCOL_VERSION.
 *
 * @param client The client id where the message should be send to.
 * @return True if no error occurred while sending the message to the client.
 */
bool tkbc_message_hello(Client client) {
  Message message = {0};
  bool ok = true;
  char buf[64] = {0};
  char *quote = "\"";

  snprintf(buf, sizeof(buf), "%d", MESSAGE_HELLO);
  tkbc_dapc(&message, buf, strlen(buf));
  tkbc_dap(&message, ':');

  tkbc_dapc(&message, quote, 1);
  const char *m = "Hello client from server!";
  tkbc_dapc(&message, m, strlen(m));

  tkbc_dapc(&message, PROTOCOL_VERSION, strlen(PROTOCOL_VERSION));
  tkbc_dapc(&message, quote, 1);
  tkbc_dap(&message, ':');
  tkbc_dapc(&message, "\r\n", 2);

  tkbc_dap(&message, 0);
  if (!tkbc_server_brodcast_client(client, message.elements)) {
    check_return(false);
  }
check:
  if (message.elements) {
    free(message.elements);
    message.elements = NULL;
  }
  return ok;
}

/**
 * @brief The function constructs all the scripts that specified in a block
 * frame.
 *
 * @param script_id The id that is the current load script.
 * @param block_frame_count The amount of scripts that are available.
 * @param block_index The script index.
 * @return True if the construction and sending the message to all registered
 * clients was successful, otherwise false.
 */
bool tkbc_message_srcipt_block_frames_value(size_t script_id,
                                            size_t block_frame_count,
                                            size_t block_index) {
  Message message = {0};
  bool ok = true;
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

  tkbc_dap(&message, 0);
  Clients cs = {0};
  if (!tkbc_server_brodcast_all(&cs, message.elements)) {
    for (size_t i = 0; i < cs.count; ++i) {
      pthread_mutex_lock(&mutex);
      tkbc_server_shutdown_client(cs.elements[i], false);
      pthread_mutex_unlock(&mutex);
    }
    free(cs.elements);
    cs.elements = NULL;
    check_return(false);
  }
check:
  free(message.elements);
  message.elements = NULL;
  return ok;
}

/**
 * @brief The function constructs the message KITEADD that is send to all
 * clients, whenever a new client has connected to the server.
 *
 * @param cs The struct that contains all the clients where the message could be
 * send to.
 * @param client_index The id of the client that has connected.
 * @return True if id was found and the sending has nor raised any errors.
 */
bool tkbc_message_kiteadd(Clients *cs, size_t client_index) {
  Message message = {0};
  bool ok = true;
  char buf[64] = {0};

  snprintf(buf, sizeof(buf), "%d", MESSAGE_KITEADD);
  tkbc_dapc(&message, buf, strlen(buf));
  tkbc_dap(&message, ':');
  if (!tkbc_message_append_clientkite(client_index, &message)) {
    check_return(false);
  }
  tkbc_dapc(&message, "\r\n", 2);

  tkbc_dap(&message, 0);
  if (!tkbc_server_brodcast_all(cs, message.elements)) {
    check_return(false);
  }
check:
  free(message.elements);
  message.elements = NULL;
  return ok;
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
bool tkbc_message_kite_value(size_t client_id) {
  Message message = {0};
  bool ok = true;
  char buf[64] = {0};

  snprintf(buf, sizeof(buf), "%d", MESSAGE_KITEVALUE);
  tkbc_dapc(&message, buf, strlen(buf));
  tkbc_dap(&message, ':');
  if (!tkbc_message_append_clientkite(client_id, &message)) {
    check_return(false);
  }
  tkbc_dapc(&message, "\r\n", 2);
  tkbc_dap(&message, 0);

  Clients cs = {0};
  if (!tkbc_server_brodcast_all_exept(&cs, client_id, message.elements)) {
    for (size_t i = 0; i < cs.count; ++i) {
      pthread_mutex_lock(&mutex);
      tkbc_server_shutdown_client(cs.elements[i], false);
      pthread_mutex_unlock(&mutex);
    }
    free(cs.elements);
    cs.elements = NULL;
    check_return(false);
  }
check:
  free(message.elements);
  message.elements = NULL;
  return ok;
}

/**
 * @brief The function constructs and send the message CLIENTKITES that contain
 * all the data from the current registered kites.
 *
 * @param client The client that should get the message.
 * @return True if the message was send successfully, otherwise false.
 */
bool tkbc_message_clientkites(Client client) {
  Message message = {0};
  char buf[64] = {0};
  bool ok = true;

  snprintf(buf, sizeof(buf), "%d", MESSAGE_CLIENTKITES);
  tkbc_dapc(&message, buf, strlen(buf));
  tkbc_dap(&message, ':');

  memset(buf, 0, sizeof(buf));
  snprintf(buf, sizeof(buf), "%zu", clients->count);
  tkbc_dapc(&message, buf, strlen(buf));

  tkbc_dap(&message, ':');
  for (size_t i = 0; i < clients->count; ++i) {
    if (!tkbc_message_append_clientkite(clients->elements[i].kite_id,
                                        &message)) {
      check_return(false);
    }
  }
  tkbc_dapc(&message, "\r\n", 2);

  tkbc_dap(&message, 0);
  if (!tkbc_server_brodcast_client(client, message.elements)) {
    check_return(false);
  }
check:
  free(message.elements);
  message.elements = NULL;
  return ok;
}

/**
 * @brief The function constructs and sends the message KITES to all registered
 * kites.
 *
 * @param cs The struct that contains all the clients where the message could be
 * send too.
 * @return True if the message was send successfully, otherwise false.
 */
bool tkbc_message_kites_brodcast_all(Clients *cs) {
  Message message = {0};
  char buf[64] = {0};
  bool ok = true;

  snprintf(buf, sizeof(buf), "%d", MESSAGE_KITES);
  tkbc_dapc(&message, buf, strlen(buf));
  tkbc_dap(&message, ':');
  memset(buf, 0, sizeof(buf));
  snprintf(buf, sizeof(buf), "%zu", env->kite_array->count);
  tkbc_dapc(&message, buf, strlen(buf));
  tkbc_dap(&message, ':');

  pthread_mutex_lock(&mutex);
  for (size_t i = 0; i < env->kite_array->count; ++i) {
    Kite_State *kite_state = &env->kite_array->elements[i];
    tkbc_message_append_kite(kite_state, &message);
  }
  pthread_mutex_unlock(&mutex);
  tkbc_dapc(&message, "\r\n", 2);
  tkbc_dap(&message, 0);
  if (!tkbc_server_brodcast_all(cs, message.elements)) {
    check_return(false);
  }
check:
  message.elements = NULL;
  return ok;
}

/**
 * @brief The function constructs and sends the message CLIENTKITES to all
 * registered kites.
 *
 * @param cs The struct that contains all the clients where the message could
 * not be send too and could not be constructed.
 * @return True if the message was send successfully, otherwise false.
 */
bool tkbc_message_clientkites_brodcast_all(Clients *cs) {
  Message message = {0};
  char buf[64] = {0};
  bool ok = true;

  snprintf(buf, sizeof(buf), "%d", MESSAGE_CLIENTKITES);
  tkbc_dapc(&message, buf, strlen(buf));
  tkbc_dap(&message, ':');

  memset(buf, 0, sizeof(buf));
  snprintf(buf, sizeof(buf), "%zu", clients->count);
  tkbc_dapc(&message, buf, strlen(buf));

  tkbc_dap(&message, ':');
  for (size_t i = 0; i < clients->count; ++i) {
    if (!tkbc_message_append_clientkite(clients->elements[i].kite_id,
                                        &message)) {
      tkbc_dap(cs, clients->elements[i]);
      ok = false;
    }
  }
  tkbc_dapc(&message, "\r\n", 2);
  tkbc_dap(&message, 0);
  Clients csi = {0};
  if (!tkbc_server_brodcast_all(&csi, message.elements)) {
    ok = false;
  }
  if (ok) {
    check_return(true);
  }
  if (csi.count > cs->count) {
    void *tmp = &csi;
    csi = *cs;
    cs = tmp;
  }
  for (size_t i = 0; i < csi.count; ++i) {
    bool contains = false;
    for (size_t j = 0; j < cs->count; ++j) {
      if (cs->elements[j].kite_id == csi.elements[i].kite_id) {
        contains = true;
        break;
      }
    }
    if (!contains) {
      tkbc_dap(cs, csi.elements[i]);
    }
  }
check:
  free(message.elements);
  message.elements = NULL;
  if (csi.elements) {
    free(csi.elements);
    csi.elements = NULL;
  }
  return ok;
}

/**
 * @brief The function removes the specified client from the registered list.
 *
 * @param client The client who should be removed from the registration list.
 * @return 1 if the kite could not be removed, -1 if the client could not be
 * removed, 0 if no errors occurred.
 */
int tkbc_server_remove_client(Client client, bool retry) {
  bool ok = 0;
  pthread_mutex_lock(&mutex);
  if (retry) {
    goto retry;
  }
  if (!tkbc_remove_kite_from_list(env->kite_array, client.kite_id)) {
    check_return(1);
  }

retry:
  for (size_t i = 0; i < clients->count; ++i) {
    if (client.kite_id == clients->elements[i].kite_id) {
      Client c_temp = clients->elements[i];
      clients->elements[i] = clients->elements[clients->count - 1];
      clients->elements[clients->count - 1] = c_temp;
      clients->count -= 1;
      check_return(0);
    }
  }
  check_return(-1);

check:
  pthread_mutex_unlock(&mutex);
  return ok;
}

/**
 * @brief The function parses the messages out of the given
 * receive_message_queue data. If an invalid message is found the rest of the
 * receive_message_queue till the '\r\n' is dorpped. The parser continues from
 * there as recovery.
 *
 * @param receive_message_queue The messages that have been received by the
 * server.
 * @return True if every message is parsed correctly from the data, false if an
 * parsing error has occurred.
 */
bool tkbc_server_received_message_handler(Message receive_message_queue) {
  Token token;
  bool ok = true;
  Lexer *lexer = lexer_new(__FILE__, receive_message_queue.elements,
                           receive_message_queue.count, 0);
  if (receive_message_queue.count == 0) {
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

    assert(MESSAGE_COUNT == 13);
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

      tkbc_fprintf(stderr, "INFO", "[MESSAGEHANDLER] %s", "HELLO\n");
    } break;
    case MESSAGE_KITEVALUE: {
      size_t kite_id;
      if (!tkbc_single_kitevalue(lexer, &kite_id)) {
        goto err;
      }
      if (!tkbc_message_kite_value(kite_id)) {
        check_return(false);
      }

      tkbc_fprintf(stderr, "INFO", "[MESSAGEHANDLER] %s", "KITEVALUE\n");
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
        size_t kite_id;
        if (!tkbc_single_kitevalue(lexer, &kite_id)) {
          goto err;
        }
        Clients cs = {0};
        if (!tkbc_message_kites_brodcast_all(&cs)) {
          if (cs.count == 0) {
            check_return(false);
          }
          for (size_t i = 0; i < cs.count; ++i) {
            pthread_mutex_lock(&mutex);
            tkbc_server_shutdown_client(cs.elements[i], false);
            pthread_mutex_unlock(&mutex);
          }
          free(cs.elements);
          cs.elements = NULL;
          check_return(false);
        }
      }

      tkbc_fprintf(stderr, "INFO", "[MESSAGEHANDLER] %s", "KITES_POSITIONS\n");
    } break;
    case MESSAGE_SCRIPT: {
      pthread_mutex_lock(&mutex);

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
          assert(ACTION_KIND_COUNT == 9 &&
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
            assert(0 &&
                   "UNREACHABLE SCRIPT tkbc_server_received_message_handler()");
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
        for (size_t id = 0; id < possible_new_kis.count; ++id) {
          for (size_t i = 0; i < env->kite_array->count; ++i) {
            Kite_State *kite_state = &env->kite_array->elements[i];
            if (possible_new_kis.elements[id] == kite_state->kite_id) {
              kite_count--;
              break;
            }
          }
        }
        tkbc_kite_array_generate(env, kite_count);
        if (possible_new_kis.elements) {
          free(possible_new_kis.elements);
          possible_new_kis.elements = NULL;
        }

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

      if (tmp_buffer.elements) {
        free(tmp_buffer.elements);
        tmp_buffer.elements = NULL;
      }
      pthread_mutex_unlock(&mutex);
    script_err:
      if (script_parse_fail) {
        goto err;
      }

      tkbc_fprintf(stderr, "INFO", "[MESSAGEHANDLER] %s", "SCRIPT\n");
    } break;
    case MESSAGE_SCRIPT_TOGGLE: {
      pthread_mutex_lock(&mutex);
      env->script_finished = !env->script_finished;
      pthread_mutex_unlock(&mutex);

      tkbc_fprintf(stderr, "INFO", "[MESSAGEHANDLER] %s", "SCRIPT_TOGGLE\n");
    } break;
    case MESSAGE_SCRIPT_NEXT: {
      pthread_mutex_lock(&mutex);

      if (env->script_counter > env->block_frames->count) {
        pthread_mutex_unlock(&mutex);
        goto err;
      }
      tkbc_load_next_script(env);
      pthread_mutex_unlock(&mutex);

      tkbc_fprintf(stderr, "INFO", "[MESSAGEHANDLER] %s", "SCRIPT_NEXT\n");
    } break;
    case MESSAGE_SCRIPT_SCRUB: {

      token = lexer_next(lexer);
      if (token.kind != NUMBER) {
        goto err;
      }
      bool drag_left = atoi(lexer_token_to_cstr(lexer, &token));

      pthread_mutex_lock(&mutex);
      {
        if (env->block_frame->count <= 0) {
          pthread_mutex_unlock(&mutex);
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
        tkbc_set_kite_positions_from_kite_frames_positions(env);
      }
      pthread_mutex_unlock(&mutex);

      token = lexer_next(lexer);
      if (token.kind != PUNCT_COLON) {
        goto err;
      }
      tkbc_message_srcipt_block_frames_value(env->block_frame->script_id,
                                             env->block_frame->count,
                                             env->frames->block_index);

      tkbc_fprintf(stderr, "INFO", "[MESSAGEHANDLER] %s", "SCRIPT_SCRUB\n");
    } break;
    default:
      tkbc_fprintf(stderr, "ERROR", "Unknown KIND: %d\n", kind);
      exit(1);
    }
    continue;

  err: {
    tkbc_dap(&receive_message_queue, 0);
    receive_message_queue.count -= 1;
    char *rn = strstr(receive_message_queue.elements + lexer->position, "\r\n");
    if (rn != NULL) {
      int jump_length = rn + 2 - &lexer->content[lexer->position];
      lexer_chop_char(lexer, jump_length);
      continue;
    }
    tkbc_fprintf(stderr, "WARNING", "receive_message_queue: %s\n",
                 receive_message_queue.elements);
    break;
  }
  } while (token.kind != EOF_TOKEN);

check:
  // No lexer_del() for performant reuse of the receive_message_queue.
  if (lexer->buffer.elements) {
    free(lexer->buffer.elements);
    lexer->buffer.elements = NULL;
  }
  free(lexer);
  lexer = NULL;
  return ok;
}

/**
 * @brief The function parses the KITEVALUE message out of the current lexer
 * data.
 *
 * @param lexer The current state of the data to parse.
 * @param kite_id The id that is assigned from the parsed value.
 * @return True if the parsing had not no errors, otherwise false.
 */
bool tkbc_single_kitevalue(Lexer *lexer, size_t *kite_id) {
  float x, y, angle;
  Color color;
  if (!tkbc_parse_message_kite_value(lexer, kite_id, &x, &y, &angle, &color)) {
    return false;
  }

  pthread_mutex_lock(&mutex);
  for (size_t i = 0; i < env->kite_array->count; ++i) {
    if (*kite_id == env->kite_array->elements[i].kite_id) {
      Kite *kite = env->kite_array->elements[i].kite;
      kite->center.x = x;
      kite->center.y = y;
      kite->angle = angle;
      kite->body_color = color;
      tkbc_kite_update_internal(kite);
    }
  }
  pthread_mutex_unlock(&mutex);
  return true;
}

/**
 * @brief The function manages initial handshake and incoming messages from the
 * client.
 *
 * @param client The client of which the connection should be handled.
 */
void *tkbc_client_handler(void *client) {
  Client c = *(Client *)client;

  tkbc_message_hello(c);

  Kite_State *kite_state = tkbc_init_kite();
  kite_state->kite_id = c.kite_id;
  float r = (float)rand() / RAND_MAX;
  kite_state->kite->body_color = ColorFromHSV(r * 360, 0.6, (r + 3) / 4);

  pthread_mutex_lock(&mutex);
  tkbc_dap(env->kite_array, *kite_state);
  // Just free the state and not the kite inside, because the kite is a pointer
  // that lives on and is valid in the copy to the env->kite_array.
  free(kite_state);
  kite_state = NULL;

  Clients cs = {0};
  if (!tkbc_message_kiteadd(&cs, c.kite_id)) {
    if (cs.count == 0) {
      goto check;
    }
    for (size_t i = 0; i < cs.count; ++i) {
      pthread_mutex_lock(&mutex);
      tkbc_server_shutdown_client(cs.elements[i], false);
    }
    free(cs.elements);
    cs.elements = NULL;
  }
  if (!tkbc_message_clientkites(c)) {
    goto check;
  }
  pthread_mutex_unlock(&mutex);

  // Note: If the server is closed forcefully the memory has to be deallocated
  // by to OS, because there is no way to access it from the main thread for all
  // the clients, but that is fine. If the client is closed by it's own handling
  // the memory is deallocated.
  Message receive_queue = {0};
  size_t size = 1024;
  receive_queue.elements = malloc(size);
  receive_queue.capacity = size;
  ssize_t n = 0;
  for (;;) {
    receive_queue.count = 0;
    do {
      n = recv(c.socket_id, &receive_queue.elements[receive_queue.count], size,
               MSG_NOSIGNAL | MSG_DONTWAIT);
      if (n == -1) {
        break;
      }
      receive_queue.count += n;
      if (receive_queue.count >= receive_queue.capacity) {
        receive_queue.capacity += size;
        receive_queue.elements =
            realloc(receive_queue.elements,
                    sizeof(*receive_queue.elements) * receive_queue.capacity);
      }
    } while (n > 0);
    if (n == -1) {
      if (errno != EAGAIN) {
        tkbc_fprintf(stderr, "ERROR", "Read: %d\n", errno);
        tkbc_fprintf(stderr, "ERROR", "Read: %s\n", strerror(errno));
        break;
      }
    }
    if (receive_queue.count == 0) {
      n = recv(c.socket_id, &receive_queue.elements[receive_queue.count], size,
               MSG_NOSIGNAL | MSG_PEEK);
      if (n == -1) {
        if (errno != EAGAIN) {
          tkbc_fprintf(stderr, "ERROR", "MSG_PEEK: %d\n", errno);
          tkbc_fprintf(stderr, "ERROR", "MSG_PEEK: %s\n", strerror(errno));
          break;
        }
      }
      // If the client has disconnected n == 0  and we should close the
      // connection.
      if (n == 0) {
        break;
      }
      continue;
    }

    if (!tkbc_server_received_message_handler(receive_queue)) {
      if (n > 0) {
        printf("---------------------------------\n");
        for (size_t i = 0; i < receive_queue.count; ++i) {
          printf("%c", receive_queue.elements[i]);
        }
        printf("---------------------------------\n");
      }
      break;
    }
  }

  if (receive_queue.elements) {
    free(receive_queue.elements);
    receive_queue.elements = NULL;
  }
check:
  tkbc_server_shutdown_client(c, false);
  return NULL;
}

/**
 * @brief The function is an error handler that updates all the client states
 * and handles all error that occur while broadcasting. It also checks for
 * client disconnects and shuts down the broken connection.
 */
void tkbc_unwrap_handler_message_clientkites_brodcast_all() {
  Clients cs = {0};
  if (!tkbc_message_clientkites_brodcast_all(&cs)) {
    for (size_t i = 0; i < cs.count; ++i) {
      tkbc_server_shutdown_client(cs.elements[i], false);
    }
    free(cs.elements);
    cs.elements = NULL;
  }
}
