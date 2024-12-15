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
  char read_buffer[1024] = {0};
  int n;
  do {
    n = read(client.socket_id, read_buffer, sizeof(read_buffer));
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
      fprintf(stderr, "The mutex lock is already set.\n");
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
      tkbc_server_shutdown_client(cs.elements[i]);
    }
  }
  if (pthread_mutex_unlock(&mutex) != 0) {
    assert(0 && "ERROR:mutex unlock");
  }
  if (cs.elements) {
    free(cs.elements);
  }
  if (message.elements) {
    free(message.elements);
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

  Clients cs = {0};
  if (!tkbc_server_brodcast_all_exept(&cs, client_id, message.elements)) {
    for (size_t i = 0; i < cs.count; ++i) {
      pthread_mutex_lock(&mutex);
      tkbc_server_shutdown_client(cs.elements[i]);
    }
    free(cs.elements);
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

bool tkbc_message_clientkites_brodcast_all(Clients *cs) {
  Message message = {0};
  char buf[64] = {0};
  bool ok = true;

  snprintf(buf, sizeof(buf), "%d", MESSAGE_CLIENTKITES);
  tkbc_dapc(&message, buf, strlen(buf));
  tkbc_dap(&message, ':');

  memset(buf, 0, sizeof(buf));
  snprintf(buf, sizeof(buf), "%zu", cs->count);
  tkbc_dapc(&message, buf, strlen(buf));

  tkbc_dap(&message, ':');
  for (size_t i = 0; i < cs->count; ++i) {
    if (!tkbc_message_append_clientkite(cs->elements[i].kite_id, &message)) {
      tkbc_dap(cs, cs->elements[i]);
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
  if (csi.elements) {
    free(csi.elements);
  }
  return ok ? true : false;
}

bool tkbc_server_remove_client_from_list(Client client) {
  if (pthread_mutex_lock(&mutex) != 0) {
    assert(0 && "ERROR:mutex lock");
  }

  for (size_t i = 0; i < clients->count; ++i) {
    if (client.kite_id == clients->elements[i].kite_id) {
      if (i + 1 < clients->count) {
        memmove(&clients->elements[i], &clients->elements[i + 1],
                sizeof(client) * clients->count - i - 1);
      }
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

    assert(MESSAGE_COUNT == 10);
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
        frames->block_index = atoi(lexer_token_to_cstr(lexer, &token));
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
            action_alloc(Wait_Action);
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
            action_alloc(Move_Action);
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
            action_alloc(Rotation_Action);
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
            action_alloc(Tip_Rotation_Action);
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
            // TODO: The kis maybe have to be heap allocated.
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
          tkbc_dap(frames, frame);
        }
        tkbc_dap(scb, *tkbc_deep_copy_frames(frames));
        frames->count = 0;
      }

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
    tkbc_dap(&receive_message_queue, 0);
    receive_message_queue.count -= 1;
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
  // No lexer_del() for performant reuse of the receive_message_queue.
  if (lexer->buffer.elements) {
    free(lexer->buffer.elements);
  }
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

  Clients cs = {0};
  if (!tkbc_message_kiteadd(&cs, c.kite_id)) {
    if (cs.count == 0) {
      goto check;
    }
    for (size_t i = 0; i < cs.count; ++i) {
      pthread_mutex_lock(&mutex);
      tkbc_server_shutdown_client(cs.elements[i]);
    }
    free(cs.elements);
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

  Message receive_queue = {0};
  size_t size = 1024;
  receive_queue.elements = calloc(1, size);
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
        tkbc_logger(stderr, "ERROR: read: %d\n", errno);
        tkbc_logger(stderr, "ERROR: read: %s\n", strerror(errno));
        break;
      }
    }
    if (receive_queue.count == 0) {
      tkbc_logger(stderr, "ERROR: read: Busy loop\n");
      n = recv(c.socket_id, &receive_queue.elements[receive_queue.count], size,
               MSG_NOSIGNAL | MSG_PEEK);
      if (n == -1) {
        if (errno != EAGAIN) {
          tkbc_logger(stderr, "ERROR: MSG_PEEK: %d\n", errno);
          tkbc_logger(stderr, "ERROR: MSG_PEEK: %s\n", strerror(errno));
          break;
        }
      }
      // If the client has disconnected n ==0  and we should close the
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

check:
  tkbc_server_shutdown_client(c);
  return NULL;
}
