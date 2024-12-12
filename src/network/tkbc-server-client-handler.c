#include "tkbc-server-client-handler.h"
#include "tkbc-network-common.h"
#include "tkbc-server.h"

#include "../../external/lexer/tkbc-lexer.h"
#include "../choreographer/tkbc-script-handler.h"
#include "../choreographer/tkbc.h"
#include "../global/tkbc-utils.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

extern Env *env;
extern Clients *clients;
extern pthread_t threads[SERVER_CONNETCTIONS];
extern pthread_mutex_t mutex;

void tkbc_server_shutdown_client(Client client) {
  shutdown(client.socket_id, SHUT_WR);
  char buf[1024] = {0};
  int n;
  do {
    n = read(client.socket_id, buf, sizeof(buf));
  } while (n > 0);

  if (n == 0) {
    tkbc_logger(stderr,
                "INFO: Could not read any more data from the client:" CLIENT_FMT
                ".\n",
                CLIENT_ARG(client));
  }
  if (n < 0) {
    tkbc_logger(stderr, "ERROR: reading failed: %s\n", strerror(errno));
  }

  if (close(client.socket_id) == -1) {
    tkbc_logger(stderr, "ERROR: Close socket: %s\n", strerror(errno));
  }

  size_t thread_id = client.kite_id;
  if (!tkbc_server_remove_client_from_list(client)) {
    tkbc_logger(stderr,
                "INFO: Client:" CLIENT_FMT
                ": could not be removed after broken pipe\n",
                CLIENT_ARG(client));
  }

  pthread_cancel(threads[thread_id]);
}

bool tkbc_server_brodcast_client(Client client, const char *message) {

  ssize_t send_check =
      send(client.socket_id, message, strlen(message), MSG_NOSIGNAL);
  if (send_check == 0) {
    tkbc_logger(stderr,
                "ERROR no bytes where send to the client:" CLIENT_FMT "\n",
                CLIENT_ARG(client));
  }
  if (send_check == -1) {
    tkbc_logger(stderr,
                "ERROR: Client:" CLIENT_FMT
                ":Could not broadcast message: %s\n",
                CLIENT_ARG(client), message);
    tkbc_logger(stderr, "ERROR: %s\n", strerror(errno));

    return false;
  } else {
    tkbc_logger(stderr, "INFO: The amount %ld send to:" CLIENT_FMT "\n",
                send_check, CLIENT_ARG(client));
  }
  return true;
}

bool tkbc_server_brodcast_all_exept(size_t client_id, const char *message) {
  bool ok = true;
  for (size_t i = 0; i < clients->count; ++i) {
    if ((size_t)clients->elements[i].kite_id != client_id) {
      if (!tkbc_server_brodcast_client(clients->elements[i], message)) {
        ok = false;
      }
    }
  }
  return ok;
}

bool tkbc_server_brodcast_all(const char *message) {
  bool ok = true;
  for (size_t i = 0; i < clients->count; ++i) {
    if (!tkbc_server_brodcast_client(clients->elements[i], message)) {
      ok = false;
    }
  }
  return ok;
}

bool tkbc_message_hello(Client client) {
  Message message = {0};
  bool ok = true;
  char buf[64] = {0};

  snprintf(buf, sizeof(buf), "%d", MESSAGE_HELLO);
  tkbc_dapc(&message, buf, strlen(buf));
  tkbc_dap(&message, ':');

  tkbc_dapc(&message, "\"", 1);
  const char *m = "Hello client from server!";
  tkbc_dapc(&message, m, strlen(m));

  tkbc_dapc(&message, PROTOCOL_VERSION, strlen(PROTOCOL_VERSION));
  tkbc_dapc(&message, "\"", 1);
  tkbc_dap(&message, ':');
  tkbc_dapc(&message, "\r\n", 2);

  tkbc_dap(&message, 0);
  if (!tkbc_server_brodcast_client(client, message.elements)) {
    check_return(false);
  }
check:
  if (message.elements) {
    free(message.elements);
  }
  return ok ? true : false;
}

bool tkbc_message_kiteadd(size_t client_index) {
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
  if (!tkbc_server_brodcast_all(message.elements)) {
    check_return(false);
  }
check:
  free(message.elements);
  return ok ? true : false;
}

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
  if (!tkbc_server_brodcast_all_exept(client_id, message.elements)) {
    check_return(false);
  }
check:
  free(message.elements);
  return ok ? true : false;
}

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
  return ok ? true : false;
}

bool tkbc_server_remove_client_from_list(Client client) {
  if (pthread_mutex_lock(&mutex) != 0) {
    assert(0 && "ERROR:mutex lock");
  }

  for (size_t i = 0; i < clients->count; ++i) {
    if (client.kite_id == clients->elements[i].kite_id) {

      // The next index does exist, even if there is just one client, because
      // the default array allocation does allocate 64 slots.
      memmove(&clients->elements[i], &clients->elements[i + 1],
              sizeof(client) * clients->count - i - 1);
      clients->count -= 1;

      if (pthread_mutex_unlock(&mutex) != 0) {
        assert(0 && "ERROR:mutex unlock");
      }
      return true;
    }
  }
  if (pthread_mutex_unlock(&mutex) != 0) {
    assert(0 && "ERROR:mutex unlock");
  }
  return false;
}

bool tkbc_server_received_message_handler(Message receive_message_queue) {
  Token token;
  bool ok = true;
  Lexer *lexer = lexer_new(__FILE__, receive_message_queue.elements,
                           receive_message_queue.count, 0);
  if (receive_message_queue.count == 0) {
    check_return(true);
  }
  tkbc_dap(&receive_message_queue, 0);

  do {
    token = lexer_next(lexer);
    if (token.kind == EOF_TOKEN) {
      break;
    }
    if (token.kind == INVALID) {
      goto err;
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

    assert(MESSAGE_COUNT == 9);
    switch (kind) {
    case MESSAGE_HELLO: {
      token = lexer_next(lexer);
      if (token.kind != STRINGLITERAL) {
        check_return(false);
      }

      const char *greeting = "\"Hello server from client!1.0\"";
      const char *compare = lexer_token_to_cstr(lexer, &token);
      if (strncmp(compare, greeting, strlen(greeting)) != 0) {
        tkbc_logger(stderr, "ERROR: Hello Message failed!\n");
        tkbc_logger(stderr, "ERROR: Wrong protocol version!");
        check_return(false);
      }
      token = lexer_next(lexer);
      if (token.kind != PUNCT_COLON) {
        check_return(false);
      }

      tkbc_logger(stderr, "[[MESSAGEHANDLER]] message = HELLO\n");
    } break;
    case MESSAGE_KITEVALUE: {
      size_t kite_id;
      float x, y, angle;
      Color color;
      if (!tkbc_parse_message_kite_value(lexer, &kite_id, &x, &y, &angle,
                                         &color)) {
        goto err;
      }

      if (pthread_mutex_lock(&mutex) != 0) {
        assert(0 && "ERROR:mutex lock");
      }

      for (size_t i = 0; i < env->kite_array->count; ++i) {
        if (kite_id == env->kite_array->elements[i].kite_id) {
          Kite *kite = env->kite_array->elements[i].kite;
          kite->center.x = x;
          kite->center.y = y;
          kite->angle = angle;
          kite->body_color = color;
          tkbc_center_rotation(kite, NULL, kite->angle);
        }
      }
      if (!tkbc_message_kite_value(kite_id)) {
        check_return(false);
      }

      if (pthread_mutex_unlock(&mutex) != 0) {
        assert(0 && "ERROR:mutex unlock");
      }

      tkbc_logger(stderr, "[[MESSAGEHANDLER]] message = KITEVALUE\n");
    } break;
    case MESSAGE_SCRIPT: {
      if (pthread_mutex_lock(&mutex) != 0) {
        assert(0 && "ERROR:mutex lock");
      }

      Content tmp_buffer = {0};
      bool script_parse_fail = false;
      Block_Frame *scb = env->scratch_buf_block_frame;
      Frames *frames = env->scratch_buf_frames;

      token = lexer_next(lexer);
      if (token.kind != NUMBER) {
        script_parse_fail = true;
        goto script_err;
      }
      scb->script_id = atoi(lexer_token_to_cstr(lexer, &token));
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
        size_t frames_count = atoi(lexer_token_to_cstr(lexer, &token));
        token = lexer_next(lexer);
        if (token.kind != PUNCT_COLON) {
          script_parse_fail = true;
          goto script_err;
        }

        Frame frame = {0};
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
          void *action = NULL;
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
            ((Wait_Action *)action)->starttime = atof(tmp_buffer.elements);
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
            ((Move_Action *)action)->position.x = atof(tmp_buffer.elements);
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
            ((Move_Action *)action)->position.y = atof(tmp_buffer.elements);
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
            ((Rotation_Action *)action)->angle = atof(tmp_buffer.elements);
            tmp_buffer.count = 0;
          } break;

          case KITE_TIP_ROTATION:
          case KITE_TIP_ROTATION_ADD: {
            token = lexer_next(lexer);
            if (token.kind != NUMBER) {
              script_parse_fail = true;
              goto script_err;
            }
            ((Tip_Rotation_Action *)action)->tip =
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
            ((Tip_Rotation_Action *)action)->angle = atof(tmp_buffer.elements);
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
            Kite_Ids kis = {0};
            for (size_t k = 1; k <= kite_ids_count; ++k) {
              token = lexer_next(lexer);
              if (token.kind != NUMBER) {
                script_parse_fail = true;
                goto script_err;
              }
              tkbc_dap(&kis, atoi(lexer_token_to_cstr(lexer, &token)));

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
            frame.kite_id_array = &kis;
          }

          token = lexer_next(lexer);
          if (token.kind != PUNCT_COLON) {
            script_parse_fail = true;
            goto script_err;
          }
        }
        tkbc_dap(frames, frame);
      }
      tkbc_dap(scb, *tkbc_deep_copy_frames(frames));
      frames->count = 0;

      tkbc_dap(env->block_frames, *tkbc_deep_copy_block_frame(scb));
      scb->count = 0;

      if (tmp_buffer.elements) {
        free(tmp_buffer.elements);
      }
      if (pthread_mutex_unlock(&mutex) != 0) {
        assert(0 && "ERROR:mutex unlock");
      }
    script_err:
      if (script_parse_fail) {
        goto err;
      }

      tkbc_logger(stderr, "[[MESSAGEHANDLER]] message = SCRIPT\n");
    } break;
    case MESSAGE_SCRIPT_TOGGLE: {
      if (pthread_mutex_lock(&mutex) != 0) {
        assert(0 && "ERROR:mutex lock");
      }
      env->script_finished = !env->script_finished;
      if (pthread_mutex_unlock(&mutex) != 0) {
        assert(0 && "ERROR:mutex unlock");
      }

      tkbc_logger(stderr, "[[MESSAGEHANDLER]] message = SCRIPT_TOGGLE\n");
    } break;
    case MESSAGE_SCRIPT_NEXT: {
      if (pthread_mutex_lock(&mutex) != 0) {
        assert(0 && "ERROR:mutex lock");
      }
      if (env->script_counter <= env->block_frames->count) {
        if (pthread_mutex_unlock(&mutex) != 0) {
          assert(0 && "ERROR:mutex unlock");
        }
        goto err;
      }

      if (env->script_counter > 0) {
        // Switch to next script.
        size_t count = env->block_frames->count;

        // NOTE: For this to work for the first iteration it relies on the
        // calloc functionality to zero out the rest of the struct.
        size_t id = env->block_frame->script_id;
        size_t script_index = id % count;
        env->block_frame = &env->block_frames->elements[script_index];
        env->frames = &env->block_frame->elements[0];

        tkbc_set_kite_positions_from_kite_frames_positions(env);
        env->script_finished = false;
      }

      if (pthread_mutex_unlock(&mutex) != 0) {
        assert(0 && "ERROR:mutex unlock");
      }

      tkbc_logger(stderr, "[[MESSAGEHANDLER]] message = SCRIPT_NEXT\n");
    } break;
    case MESSAGE_SCRIPT_SCRUB: {

      token = lexer_next(lexer);
      if (token.kind != NUMBER) {
        goto err;
      }
      bool drag_left = atoi(lexer_token_to_cstr(lexer, &token));

      if (pthread_mutex_lock(&mutex) != 0) {
        assert(0 && "ERROR:mutex lock");
      }
      {
        if (env->block_frame->count <= 0) {
          if (pthread_mutex_unlock(&mutex) != 0) {
            assert(0 && "ERROR:mutex unlock");
          }
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
      if (pthread_mutex_unlock(&mutex) != 0) {
        assert(0 && "ERROR:mutex unlock");
      }

      token = lexer_next(lexer);
      if (token.kind != PUNCT_COLON) {
        goto err;
      }
      tkbc_logger(stderr, "[[MESSAGEHANDLER]] message = SCRIPT_SCRUB\n");
    } break;
    default:
      tkbc_logger(stderr, "ERROR: Unknown KIND: %d\n", kind);
      exit(1);
    }
    continue;

  err: {
    char *rn = strstr(receive_message_queue.elements, "\r\n");
    if (rn != NULL) {
      int jump_length = rn + 2 - &lexer->content[lexer->position];
      lexer_chop_char(lexer, jump_length);
      continue;
    }
    tkbc_logger(stderr, "message.elements = %s\n",
                receive_message_queue.elements);
    break;
  }
  } while (token.kind != EOF_TOKEN);

check:
  lexer_del(lexer);
  return ok ? true : false;
}

void *tkbc_client_handler(void *client) {
  // TODO: Check that clients can't have the same socket id as someone before.
  // In this case the messages that are broadcasted to all clients will only be
  // partially be distributed.
  Client c = *(Client *)client;

  tkbc_message_hello(c);

  Color color_array[] = {BLUE, PURPLE, GREEN, RED, TEAL};
  Kite_State *kite_state = tkbc_init_kite();
  kite_state->kite_id = c.kite_id;
  kite_state->kite->body_color =
      color_array[c.kite_id % ARRAY_LENGTH(color_array)];
  Vector2 shift_pos = {.y = kite_state->kite->center.y,
                       .x = kite_state->kite->center.x + 200 + 200 * c.kite_id};
  tkbc_center_rotation(kite_state->kite, &shift_pos, kite_state->kite->angle);

  if (pthread_mutex_lock(&mutex) != 0) {
    assert(0 && "ERROR:mutex lock");
  }
  tkbc_dap(env->kite_array, *kite_state);

  if (!tkbc_message_kiteadd(c.kite_id)) {
    goto check;
  }
  if (!tkbc_message_clientkites(c)) {
    goto check;
  }
  if (pthread_mutex_unlock(&mutex) != 0) {
    assert(0 && "ERROR:mutex unlock");
  }

  tkbc_logger(stderr, "INFO: Connection from host %s, port %hd\n",
              inet_ntoa(c.client_address.sin_addr),
              ntohs(c.client_address.sin_port));

  for (;;) {
    Message receive_message_queue = {0};
    char message[8*1024];
    memset(message, 0, sizeof(message));
    ssize_t message_ckeck =
        recv(c.socket_id, message, sizeof(message) - 1, MSG_NOSIGNAL);
    if (message_ckeck == -1) {
      tkbc_logger(stderr, "ERROR: RECV: %s\n", strerror(errno));
      break;
    }

    assert((unsigned int)message_ckeck < sizeof(message) &&
           "Message buffer is to big.");
    message[message_ckeck] = '\0';

    if (message_ckeck == 0) {
      tkbc_logger(stderr,
                  "No data was received from the client:" CLIENT_FMT "\n",
                  CLIENT_ARG(c));
      break;
    }

    tkbc_dapc(&receive_message_queue, message, strlen(message));

    if (strcmp(message, "quit\n") == 0) {
      break;
    }

    // TODO: Improve the messages queue and maybe do lexer_del manual in split
    // the buffer for the string conversions in the function and the content
    // after the call in her.
    if (!tkbc_server_received_message_handler(receive_message_queue)) {
      if (message_ckeck > 0) {
        tkbc_logger(stderr, "Message: %s\n", message);
      }
      break;
    }
  }

check:
  tkbc_server_shutdown_client(c);
  return NULL;
}
