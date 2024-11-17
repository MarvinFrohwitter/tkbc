#include "tkbc-parser.h"
#include "tkbc-script-api.h"
#include <stdlib.h>
#include <string.h>

#define TKBC_LEXER_IMPLEMENTAION
#define EXLEX_IMPLEMENTATION
#define LEX_LOGERROR
#include "../../external/lexer/tkbc-lexer.h"

#include "../global/tkbc-types.h"
#include "../global/tkbc-utils.h"

Content tmp_buffer = {0};

const char *token_to_cstr(Token *token) {
  tmp_buffer.count = 0;
  tkbc_dapc(&tmp_buffer, token->content, token->size);
  tkbc_dap(&tmp_buffer, 0);

  return tmp_buffer.elements;
}

void tkbc_script_parser(Env *env) {
  Content content = {0};
  int err = tkbc_read_file(env->script_file_name, &content);
  if (err) {
    return;
  }
  Lexer *lexer =
      lexer_new(env->script_file_name, content.elements, content.count, 0);

  bool script_begin = false;
  bool brace = false;
  Kite_Indexs ki = {0};
  ki.count = env->kite_array->count;
  Frames frames = {0};

  Token t = lexer_next(lexer);
  while (t.kind != EOF_TOKEN) {
    switch (t.kind) {
    case COMMENT:
      break;
    case IDENTIFIER: {
      if (strncmp("BEGIN", t.content, t.size) == 0) {
        script_begin = true;
        tkbc_script_begin(env);
        break;
      } else if (strncmp("END", t.content, t.size) == 0) {
        tkbc_script_end(env);
        script_begin = false;
        break;
      } else if (strncmp("KITES", t.content, t.size) == 0) {
        if (ki.count > 0) {
          break;
        }

        t = lexer_next(lexer);
        if (t.kind == NUMBER) {
          ki = tkbc_kite_array_generate(env, atoi(token_to_cstr(&t)));
        }
        break;
      } else if (strncmp("MOVE_ADD", t.content, t.size) == 0) {
        if (!tkbc_parse_move(env, lexer, KITE_MOVE_ADD, &frames, &ki, brace)) {
          goto err;
        }
        break;
      } else if (strncmp("MOVE", t.content, t.size) == 0) {
        if (!tkbc_parse_move(env, lexer, KITE_MOVE, &frames, &ki, brace)) {
          goto err;
        }
        break;
      } else if (strncmp("ROTATION", t.content, t.size) == 0) {
        if (!tkbc_parse_rotation(env, lexer, KITE_ROTATION, &frames, &ki,
                                 brace)) {
          goto err;
        }
        break;
      } else if (strncmp("ROTATION_ADD", t.content, t.size) == 0) {
        if (!tkbc_parse_rotation(env, lexer, KITE_ROTATION_ADD, &frames, &ki,
                                 brace)) {
          goto err;
        }
        break;
      } else if (strncmp("TIP_ROTATION", t.content, t.size) == 0) {
        if (!tkbc_parse_tip_rotation(env, lexer, KITE_TIP_ROTATION, &frames,
                                     &ki, brace)) {
          break;
        }
        break;
      } else if (strncmp("TIP_ROTATION_ADD", t.content, t.size) == 0) {
        if (!tkbc_parse_tip_rotation(env, lexer, KITE_TIP_ROTATION_ADD, &frames,
                                     &ki, brace)) {
          goto err;
        }
        break;
      } else if (strncmp("WAIT", t.content, t.size) == 0) {
        t = lexer_next(lexer);
        float duration = atof(token_to_cstr(&t));

        if (brace) {
          Frame *frame = tkbc_script_wait(duration);
          tkbc_dap(&frames, *frame);
        } else {
          tkbc_register_frames(env, tkbc_script_wait(duration));
        }
        break;
      } else if (strncmp("QUIT", t.content, t.size) == 0) {
        t = lexer_next(lexer);
        float duration = atof(token_to_cstr(&t));
        if (brace) {
          Frame *frame = tkbc_script_frames_quit(duration);
          tkbc_dap(&frames, *frame);
        } else {
          tkbc_register_frames(env, tkbc_script_frames_quit(duration));
        }
        break;
      }

      goto err;
    } break;

    case PUNCT_LBRACE: {
      if (brace) {
        goto err;
      }
      brace = true;
    } break;

    case PUNCT_RBRACE: {
      if (!brace) {
        goto err;
      }
      brace = false;
      tkbc_register_frames_array(env, &frames);
    } break;

    err:
    case ERROR:
    default:
      fprintf(stderr, "ERROR: Invalid token: %s\n", token_to_cstr(&t));
    }

    t = lexer_next(lexer);
  }

  if (script_begin) {
    fprintf(stderr, "ERROR: script END is not defined.");
    tkbc_script_end(env);
  }

  lexer_del(lexer);
  free(tmp_buffer.elements);
  free(frames.elements);
  free(ki.elements);
}

/**
 * @brief The function can be used to check if every element in the given
 * kite indies is part of the current registered kites.
 *
 * @param env The global state of the application.
 * @param kis The kite indies that should be check against the existing kites.
 * @return True if the given kite indies are valid, otherwise false.
 */
bool tkbc_parse_ki_check(Env *env, Kite_Indexs *kis) {
  bool ok = false;
  for (size_t i = 0; i < env->kite_array->count; ++i) {
    for (size_t j = 0; j < kis->count; ++j) {
      if (env->kite_array->elements[i].kite_id == kis->elements[j]) {
        ok = true;
        break;
      }
      ok = false;
    }

    if (!ok)
      return false;
  }

  return true;
}

bool tkbc_parse_move(Env *env, Lexer *lexer, Action_Kind kind, Frames *frames,
                     Kite_Indexs *ki, bool brace) {
  bool ok = true;
  Kite_Indexs kis = {0};
  Token t = {0};
  char sign;

  t = lexer_next(lexer);
  if (strncmp("KITES", t.content, t.size) == 0) {
    t = lexer_next(lexer);
    if (ki->count == 0) {
      check_return(false);
    }
    kis = *ki;

  } else if (t.kind == PUNCT_LPAREN) {
    t = lexer_next(lexer);
    while (t.kind == NUMBER) {
      tkbc_dap(&kis, atoi(token_to_cstr(&t)));
      if (!tkbc_parse_ki_check(env, &kis)) {
        fprintf(stderr,
                "ERROR: The given kites in the listing are invalid: "
                "Position:%llu:%ld\n",
                lexer->line_count, (t.content - lexer->content));
      }
      t = lexer_next(lexer);
    }
  }

  bool issign = false;
  if (strncmp("-", t.content, t.size) == 0 ||
      strncmp("+", t.content, t.size) == 0) {
    issign = true;
    sign = *(char *)t.content;
  }

  if (t.kind != PUNCT_RPAREN && t.kind != NUMBER && !issign) {
    check_return(false);
  }

  if (t.kind != NUMBER) {
    t = lexer_next(lexer);
  }

  if (strncmp("-", t.content, t.size) == 0 ||
      strncmp("+", t.content, t.size) == 0) {
    issign = true;
    sign = *(char *)t.content;
    t = lexer_next(lexer);
  }

  tmp_buffer.count = 0;
  if (issign) {
    tkbc_dap(&tmp_buffer, sign);
  }
  tkbc_dapc(&tmp_buffer, t.content, t.size);
  tkbc_dap(&tmp_buffer, 0);

  float x = atof(tmp_buffer.elements);
  t = lexer_next(lexer);

  issign = false;
  if (strncmp("-", t.content, t.size) == 0 ||
      strncmp("+", t.content, t.size) == 0) {
    issign = true;
    sign = *(char *)t.content;
  }

  if (t.kind != NUMBER && !issign) {
    check_return(false);
  }

  if (t.kind != NUMBER) {
    t = lexer_next(lexer);
  }

  tmp_buffer.count = 0;
  if (issign) {
    tkbc_dap(&tmp_buffer, sign);
  }
  tkbc_dapc(&tmp_buffer, t.content, t.size);
  tkbc_dap(&tmp_buffer, 0);

  float y = atof(tmp_buffer.elements);
  t = lexer_next(lexer);

  float duration = atof(token_to_cstr(&t));

  if (kind == KITE_MOVE_ADD) {
    if (brace) {
      Frame *frame = tkbc_frame_generate(KITE_MOVE_ADD, kis,
                                         &(CLITERAL(Move_Add_Action){
                                             .position.x = x,
                                             .position.y = y,
                                         }),
                                         duration);
      tkbc_dap(frames, *frame);
    } else {
      tkbc_register_frames(env, tkbc_frame_generate(KITE_MOVE_ADD, kis,
                                                    &(CLITERAL(Move_Add_Action){
                                                        .position.x = x,
                                                        .position.y = y,
                                                    }),
                                                    duration));
    }
  } else if (kind == KITE_MOVE) {
    if (brace) {
      Frame *frame = tkbc_frame_generate(KITE_MOVE, kis,
                                         &(CLITERAL(Move_Action){
                                             .position.x = x,
                                             .position.y = y,
                                         }),
                                         duration);
      tkbc_dap(frames, *frame);
    } else {
      tkbc_register_frames(env, tkbc_frame_generate(KITE_MOVE, kis,
                                                    &(CLITERAL(Move_Action){
                                                        .position.x = x,
                                                        .position.y = y,
                                                    }),
                                                    duration));
    }
  }
check:
  free(kis.elements);
  return ok ? true : false;
}

bool tkbc_parse_rotation(Env *env, Lexer *lexer, Action_Kind kind,
                         Frames *frames, Kite_Indexs *ki, bool brace) {
  bool ok = true;
  Kite_Indexs kis = {0};
  Token t = {0};
  char sign;

  t = lexer_next(lexer);
  if (strncmp("KITES", t.content, t.size) == 0) {
    t = lexer_next(lexer);
    if (ki->count == 0) {
      check_return(false);
    }
    kis = *ki;

  } else if (t.kind == PUNCT_LPAREN) {
    t = lexer_next(lexer);
    while (t.kind == NUMBER) {
      tkbc_dap(&kis, atoi(token_to_cstr(&t)));
      if (!tkbc_parse_ki_check(env, &kis)) {
        fprintf(stderr,
                "ERROR: The given kites in the listing are invalid: "
                "Position:%llu:%ld\n",
                lexer->line_count, (t.content - lexer->content));
      }
      t = lexer_next(lexer);
    }
  }
  bool issign = false;
  if (strncmp("-", t.content, t.size) == 0 ||
      strncmp("+", t.content, t.size) == 0) {
    issign = true;
    sign = *(char *)t.content;
  }

  if (t.kind != PUNCT_RPAREN && t.kind != NUMBER && !issign) {
    check_return(false);
  }

  if (t.kind != NUMBER) {
    t = lexer_next(lexer);
  }

  if (strncmp("-", t.content, t.size) == 0 ||
      strncmp("+", t.content, t.size) == 0) {
    issign = true;
    sign = *(char *)t.content;
    t = lexer_next(lexer);
  }

  tmp_buffer.count = 0;
  if (issign) {
    tkbc_dap(&tmp_buffer, sign);
  }
  tkbc_dapc(&tmp_buffer, t.content, t.size);
  tkbc_dap(&tmp_buffer, 0);

  float angle = atof(tmp_buffer.elements);
  t = lexer_next(lexer);

  float duration = atof(token_to_cstr(&t));

  if (kind == KITE_ROTATION_ADD) {
    if (brace) {
      Frame *frame = tkbc_frame_generate(KITE_ROTATION_ADD, kis,
                                         &(CLITERAL(Rotation_Add_Action){
                                             .angle = angle,
                                         }),
                                         duration);
      tkbc_dap(frames, *frame);
    } else {
      tkbc_register_frames(env,
                           tkbc_frame_generate(KITE_ROTATION_ADD, kis,
                                               &(CLITERAL(Rotation_Add_Action){
                                                   .angle = angle,
                                               }),
                                               duration));
    }
  } else if (kind == KITE_ROTATION) {
    if (brace) {
      Frame *frame = tkbc_frame_generate(KITE_ROTATION, kis,
                                         &(CLITERAL(Rotation_Action){
                                             .angle = angle,
                                         }),
                                         duration);
      tkbc_dap(frames, *frame);
    } else {
      tkbc_register_frames(env, tkbc_frame_generate(KITE_ROTATION, kis,
                                                    &(CLITERAL(Rotation_Action){
                                                        .angle = angle,
                                                    }),
                                                    duration));
    }
  }

check:
  free(kis.elements);
  return ok ? true : false;
}

bool tkbc_parse_tip_rotation(Env *env, Lexer *lexer, Action_Kind kind,
                             Frames *frames, Kite_Indexs *ki, bool brace) {
  bool ok = true;
  Kite_Indexs kis = {0};
  Token t = {0};
  TIP tip;
  char sign;

  t = lexer_next(lexer);
  if (strncmp("KITES", t.content, t.size) == 0) {
    t = lexer_next(lexer);
    if (ki->count == 0) {
      check_return(false);
      goto check;
    }
    kis = *ki;

  } else if (t.kind == PUNCT_LPAREN) {
    t = lexer_next(lexer);
    while (t.kind == NUMBER) {
      tkbc_dap(&kis, atoi(token_to_cstr(&t)));
      if (!tkbc_parse_ki_check(env, &kis)) {
        fprintf(stderr,
                "ERROR: The given kites in the listing are invalid: "
                "Position:%llu:%ld\n",
                lexer->line_count, (t.content - lexer->content));
      }
      t = lexer_next(lexer);
    }
  }
  bool issign = false;
  if (strncmp("-", t.content, t.size) == 0 ||
      strncmp("+", t.content, t.size) == 0) {
    issign = true;
    sign = *(char *)t.content;
  }

  if (t.kind != PUNCT_RPAREN && t.kind != NUMBER && !issign) {
    check_return(false);
  }

  if (t.kind != NUMBER) {
    t = lexer_next(lexer);
  }

  if (strncmp("-", t.content, t.size) == 0 ||
      strncmp("+", t.content, t.size) == 0) {
    issign = true;
    sign = *(char *)t.content;
    t = lexer_next(lexer);
  }

  tmp_buffer.count = 0;
  if (issign) {
    tkbc_dap(&tmp_buffer, sign);
  }
  tkbc_dapc(&tmp_buffer, t.content, t.size);
  tkbc_dap(&tmp_buffer, 0);

  float angle = atof(tmp_buffer.elements);
  t = lexer_next(lexer);

  if (strncmp("RIGHT", t.content, t.size) == 0) {
    tip = RIGHT_TIP;
  } else if (strncmp("LEFT", t.content, t.size) == 0) {
    tip = LEFT_TIP;
  } else {
    check_return(false);
  }

  t = lexer_next(lexer);
  float duration = atof(token_to_cstr(&t));

  if (kind == KITE_TIP_ROTATION_ADD) {
    if (brace) {
      Frame *frame = tkbc_frame_generate(KITE_TIP_ROTATION_ADD, kis,
                                         &(CLITERAL(Tip_Rotation_Add_Action){
                                             .angle = angle,
                                             .tip = tip,
                                         }),
                                         duration);
      tkbc_dap(frames, *frame);
    } else {
      tkbc_register_frames(
          env, tkbc_frame_generate(KITE_TIP_ROTATION_ADD, kis,
                                   &(CLITERAL(Tip_Rotation_Add_Action){
                                       .angle = angle,
                                       .tip = tip,
                                   }),
                                   duration));
    }
  } else if (kind == KITE_TIP_ROTATION) {
    if (brace) {
      Frame *frame = tkbc_frame_generate(KITE_TIP_ROTATION, kis,
                                         &(CLITERAL(Tip_Rotation_Action){
                                             .angle = angle,
                                             .tip = tip,
                                         }),
                                         duration);
      tkbc_dap(frames, *frame);
    } else {
      tkbc_register_frames(env,
                           tkbc_frame_generate(KITE_TIP_ROTATION, kis,
                                               &(CLITERAL(Tip_Rotation_Action){
                                                   .angle = angle,
                                                   .tip = tip,
                                               }),
                                               duration));
    }
  }

check:
  free(kis.elements);
  return ok ? true : false;
}
