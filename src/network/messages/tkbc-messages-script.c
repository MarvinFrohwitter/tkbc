#include "../../../external/lexer/tkbc-lexer.h"
#include "../../../external/space/space.h"
#include "../../choreographer/tkbc-script-api.h"
#include "../../choreographer/tkbc-script-handler.h"
#include "../../global/tkbc-types.h"
#include "../poll-server.h"
#include "../tkbc-servers-common.h"
#include "tkbc-messages.h"

#include <stdbool.h>

bool tkbc_messages_script(Env *env, Lexer *lexer, Client *client,
                          bool *script_alleady_there_parsing_skip) {
  Token token;
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
    *script_alleady_there_parsing_skip = true;
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
        action.as_tip_rotation.tip = atoi(lexer_token_to_cstr(lexer, &token));

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
  tkbc_add_script(env, *scb_script);

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
    if (*script_alleady_there_parsing_skip) {
      goto parsing_skip;
    }
    return false;
  }

  client->script_amount--;
parsing_skip:
  if (client->script_amount && *script_alleady_there_parsing_skip) {
    script_parse_fail = false;
    client->script_amount = 0;
  }
  if (client->script_amount == 0 && !script_parse_fail) {
    space_dapf(&client->send_msg_buffer_space, &client->send_msg_buffer,
               "%d:\r\n", MESSAGE_SCRIPT_PARSED);
  }

  tkbc_fprintf(stderr, "MESSAGEHANDLER", "SCRIPT\n");
  if (*script_alleady_there_parsing_skip) {
    return false;
  }

  // This parsing function is just used in the server but liked in the client as
  // well so just a simple guard for compilation.
#ifdef TKBC_SERVER
  tkbc_message_clientkites_write_to_send_msg_buffer(client, true);
#endif

  return true;
}
