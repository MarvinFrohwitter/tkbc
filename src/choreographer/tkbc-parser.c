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
  Kite_Indexs ki;
  Frames frames = {0};

  Token t = lexer_next(lexer);
  while (t.kind != EOF_TOKEN) {
    t = lexer_next(lexer);
    switch (t.kind) {
    case COMMENT: {
      continue;
    } break;
    case IDENTIFIER: {
      if (strncmp("BEGIN", t.content, t.size) == 0) {
        script_begin = true;
        tkbc_script_begin(env);
        continue;
      } else if (strncmp("END", t.content, t.size) == 0) {
        tkbc_script_end(env);
        script_begin = false;
        continue;
      } else if (strncmp("KITES", t.content, t.size) == 0) {
        if (ki.count > 0) {
          continue;
        }

        t = lexer_next(lexer);
        if (t.kind == NUMBER) {
          ki = tkbc_indexs_generate(atoi(token_to_cstr(&t)));
        }
        continue;
      } else if (strncmp("MOVE_ADD", t.content, t.size) == 0) {
        t = lexer_next(lexer);
        if (!tkbc_parse_move(env, lexer, KITE_MOVE_ADD, &frames, &ki, brace)) {
          goto err;
        }
        continue;
      } else if (strncmp("MOVE", t.content, t.size) == 0) {
        t = lexer_next(lexer);
        if (!tkbc_parse_move(env, lexer, KITE_MOVE, &frames, &ki, brace)) {
          goto err;
        }
        continue;
      } else if (strncmp("ROTATION", t.content, t.size) == 0) {
        t = lexer_next(lexer);
        if (!tkbc_parse_rotation(env, lexer, KITE_ROTATION, &frames, &ki,
                                 brace)) {
          goto err;
        }
        continue;
      } else if (strncmp("ROTATION_ADD", t.content, t.size) == 0) {
        t = lexer_next(lexer);
        if (!tkbc_parse_rotation(env, lexer, KITE_ROTATION_ADD, &frames, &ki,
                                 brace)) {
          goto err;
        }
        continue;
      } else if (strncmp("TIP_ROTATION", t.content, t.size) == 0) {
        t = lexer_next(lexer);
        if (!tkbc_parse_tip_rotation(env, lexer, KITE_TIP_ROTATION, &frames,
                                     &ki, brace)) {
          goto err;
        }
        continue;
      } else if (strncmp("TIP_ROTATION_ADD", t.content, t.size) == 0) {
        t = lexer_next(lexer);
        if (!tkbc_parse_tip_rotation(env, lexer, KITE_TIP_ROTATION_ADD, &frames,
                                     &ki, brace)) {
          goto err;
        }
        continue;
      } else if (strncmp("WAIT", t.content, t.size) == 0) {
        t = lexer_next(lexer);
        float duration = atof(token_to_cstr(&t));
        tkbc_register_frames(env, tkbc_script_wait(duration));
        continue;
      } else if (strncmp("QUIT", t.content, t.size) == 0) {
        t = lexer_next(lexer);
        float duration = atof(token_to_cstr(&t));
        tkbc_register_frames(env, tkbc_script_frames_quit(duration));
        continue;
      }

      fprintf(stderr, "Token: %s\n", token_to_cstr(&t));
    } break;

    case PUNCT_LBRACE: {
      brace = true;
    } break;

    case PUNCT_RBRACE: {
      brace = false;
      tkbc_register_frames_array(env, &frames);
    } break;

    err:
    case ERROR:
    default:
      fprintf(stderr, "ERROR: Invalid token: %s\n", token_to_cstr(&t));
    }
  }

  if (script_begin) {
    fprintf(stderr, "ERROR: script END is not defined.");
    tkbc_script_end(env);
  }

  lexer_del(lexer);
  free(tmp_buffer.elements);
}

bool tkbc_parse_move(Env *env, Lexer *lexer, Action_Kind kind, Frames *frames,
                     Kite_Indexs *ki, bool brace) {

  Kite_Indexs kis = {0};
  Token t = {0};
  char sign;

  if (strncmp("KITES", t.content, t.size) == 0) {
    t = lexer_next(lexer);
    if (ki->count == 0) {
      return false;
    }
    kis = *ki;

  } else if (t.kind != PUNCT_LPAREN) {
    t = lexer_next(lexer);
    while (t.kind == NUMBER) {
      tkbc_dap(&kis, atoi(token_to_cstr(&t)));
      t = lexer_next(lexer);
    }
  }
  bool issign = false;
  if (strncmp("-", t.content, t.size) == 0 ||
      strncmp("+", t.content, t.size) == 0) {
    issign = true;
    sign = *(char *)t.content;
  }

  if (t.kind != PUNCT_RPAREN || t.kind != NUMBER || !issign) {
    return false;
  }

  if (t.kind != NUMBER) {
    t = lexer_next(lexer);
  }

  if (issign) {
    tmp_buffer.count = 0;
    tkbc_dap(&tmp_buffer, sign);
    tkbc_dapc(&tmp_buffer, t.content, t.size);
    tkbc_dap(&tmp_buffer, 0);
  }

  float x = atof(tmp_buffer.elements);
  t = lexer_next(lexer);

  issign = false;
  if (strncmp("-", t.content, t.size) == 0 ||
      strncmp("+", t.content, t.size) == 0) {
    issign = true;
    sign = *(char *)t.content;
  }

  if (t.kind != NUMBER || !issign) {
    return false;
  }

  if (t.kind != NUMBER) {
    t = lexer_next(lexer);
  }

  if (issign) {
    tmp_buffer.count = 0;
    tkbc_dap(&tmp_buffer, sign);
    tkbc_dapc(&tmp_buffer, t.content, t.size);
    tkbc_dap(&tmp_buffer, 0);
  }

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
  return true;
}

bool tkbc_parse_rotation(Env *env, Lexer *lexer, Action_Kind kind,
                         Frames *frames, Kite_Indexs *ki, bool brace) {
  Kite_Indexs kis = {0};
  Token t = {0};
  char sign;

  if (strncmp("KITES", t.content, t.size) == 0) {
    t = lexer_next(lexer);
    if (ki->count == 0) {
      return false;
    }
    kis = *ki;

  } else if (t.kind != PUNCT_LPAREN) {
    t = lexer_next(lexer);
    while (t.kind == NUMBER) {
      tkbc_dap(&kis, atoi(token_to_cstr(&t)));
      t = lexer_next(lexer);
    }
  }
  bool issign = false;
  if (strncmp("-", t.content, t.size) == 0 ||
      strncmp("+", t.content, t.size) == 0) {
    issign = true;
    sign = *(char *)t.content;
  }

  if (t.kind != PUNCT_RPAREN || t.kind != NUMBER || !issign) {
    return false;
  }

  if (t.kind != NUMBER) {
    t = lexer_next(lexer);
  }

  if (issign) {
    tmp_buffer.count = 0;
    tkbc_dap(&tmp_buffer, sign);
    tkbc_dapc(&tmp_buffer, t.content, t.size);
    tkbc_dap(&tmp_buffer, 0);
  }

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
  return true;
}

bool tkbc_parse_tip_rotation(Env *env, Lexer *lexer, Action_Kind kind,
                             Frames *frames, Kite_Indexs *ki, bool brace) {
  Kite_Indexs kis = {0};
  Token t = {0};
  TIP tip;
  char sign;

  if (strncmp("KITES", t.content, t.size) == 0) {
    t = lexer_next(lexer);
    if (ki->count == 0) {
      return false;
    }
    kis = *ki;

  } else if (t.kind != PUNCT_LPAREN) {
    t = lexer_next(lexer);
    while (t.kind == NUMBER) {
      tkbc_dap(&kis, atoi(token_to_cstr(&t)));
      t = lexer_next(lexer);
    }
  }
  bool issign = false;
  if (strncmp("-", t.content, t.size) == 0 ||
      strncmp("+", t.content, t.size) == 0) {
    issign = true;
    sign = *(char *)t.content;
  }

  if (t.kind != PUNCT_RPAREN || t.kind != NUMBER || !issign) {
    return false;
  }

  if (t.kind != NUMBER) {
    t = lexer_next(lexer);
  }

  if (issign) {
    tmp_buffer.count = 0;
    tkbc_dap(&tmp_buffer, sign);
    tkbc_dapc(&tmp_buffer, t.content, t.size);
    tkbc_dap(&tmp_buffer, 0);
  }

  float angle = atof(tmp_buffer.elements);
  t = lexer_next(lexer);

  if (strncmp("RIGHT", t.content, t.size)) {
    tip = RIGHT_TIP;
  } else if (strncmp("LEFT", t.content, t.size)) {
    tip = LEFT_TIP;
  } else {
    return false;
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
  return true;
}
