#include "tkbc-parser.h"
#include "tkbc-script-api.h"
#include "tkbc-team-figures-api.h"
#include <stdlib.h>
#include <string.h>

#define LEXER_IMPLEMENTAION
#define EXLEX_IMPLEMENTATION
#define LEX_LOGERROR
#include "../../external/lexer/tkbc-lexer.h"

#include "../global/tkbc-types.h"
#include "../global/tkbc-utils.h"

/**
 * @brief The function parses the script that is currently represented by the
 * filename in env->script_file_name.
 *
 * @param env The env that represents the global state of the application.
 */
void tkbc_script_parser(Env *env) {
  Content tmp_buffer = {0};
  Content content = {0};
  int err = tkbc_read_file(env->script_file_name, &content);
  if (err) {
    return;
  }
  Lexer *lexer =
      lexer_new(env->script_file_name, content.elements, content.count, 0);

  bool script_begin = false;
  bool brace = false;
  Kite_Ids ki = {0};
  Frames *frames = &env->scratch_buf_frames;
  Frame *frame = NULL;

  Token t = lexer_next(lexer);
  while (t.kind != EOF_TOKEN) {
    switch (t.kind) {
    case PREPROCESSING:
    case COMMENT:
      break;
    case IDENTIFIER: {
      if (strncmp("EXTERN", t.content, t.size) == 0) {
        bool ok = true;
        t = lexer_next(lexer);
        if (t.kind != IDENTIFIER) {
          break;
        }

        char *function_name = strdup(lexer_token_to_cstr(lexer, &t));
        assert(function_name != NULL);
        Kite_Ids kis = {0};
        if (!tkbc_parse_kis_after_generation(env, lexer, &kis, ki)) {
          check_return(false);
        }
        if (!tkbc_parse_team_figures(env, kis, lexer, function_name,
                                     tmp_buffer)) {
          check_return(false);
        }

      check:
        free(function_name);
        function_name = NULL;
        if (kis.elements) {
          free(kis.elements);
          kis.elements = NULL;
        }
        if (!ok) {
          goto err;
        }
        break;
      } else if (strncmp("BEGIN", t.content, t.size) == 0) {
        script_begin = true;
        tkbc__script_begin(env);
        break;
      } else if (strncmp("END", t.content, t.size) == 0) {
        tkbc__script_end(env);
        script_begin = false;
        break;
      } else if (strncmp("KITES", t.content, t.size) == 0) {
        if (ki.count > 0) {
          break;
        }

        t = lexer_next(lexer);
        if (t.kind == NUMBER) {
          size_t kite_number = atoi(lexer_token_to_cstr(lexer, &t));
          if (env->kite_array->count >= kite_number) {
            ki = tkbc_indexs_generate(kite_number);
            for (size_t i = 0; i < ki.count; ++i) {
              ki.elements[i] = env->kite_array->elements[i].kite_id;
            }
            break;
          }
          kite_number -= env->kite_array->count;
          ki = tkbc_kite_array_generate(env, kite_number);
        }
        break;
      } else if (strncmp("MOVE", t.content, t.size) == 0) {
        if (!tkbc_parse_move(env, lexer, KITE_MOVE, ki, brace, tmp_buffer)) {
          goto err;
        }
        break;
      } else if (strncmp("MOVE_ADD", t.content, t.size) == 0) {
        if (!tkbc_parse_move(env, lexer, KITE_MOVE_ADD, ki, brace,
                             tmp_buffer)) {
          goto err;
        }
        break;
      } else if (strncmp("ROTATION", t.content, t.size) == 0) {
        if (!tkbc_parse_rotation(env, lexer, KITE_ROTATION, ki, brace,
                                 tmp_buffer)) {
          goto err;
        }
        break;
      } else if (strncmp("ROTATION_ADD", t.content, t.size) == 0) {
        if (!tkbc_parse_rotation(env, lexer, KITE_ROTATION_ADD, ki, brace,
                                 tmp_buffer)) {
          goto err;
        }
        break;
      } else if (strncmp("TIP_ROTATION", t.content, t.size) == 0) {
        if (!tkbc_parse_tip_rotation(env, lexer, KITE_TIP_ROTATION, ki, brace,
                                     tmp_buffer)) {
          break;
        }
        break;
      } else if (strncmp("TIP_ROTATION_ADD", t.content, t.size) == 0) {
        if (!tkbc_parse_tip_rotation(env, lexer, KITE_TIP_ROTATION_ADD, ki,
                                     brace, tmp_buffer)) {
          goto err;
        }
        break;
      } else if (strncmp("WAIT", t.content, t.size) == 0) {
        t = lexer_next(lexer);
        float duration = atof(lexer_token_to_cstr(lexer, &t));

        if (brace) {
          frame = tkbc_script_wait(duration);
          tkbc_sript_team_scratch_buf_frames_append_and_free(env, frame);
        } else {
          tkbc_register_frames(env, tkbc_script_wait(duration));
        }
        break;
      } else if (strncmp("QUIT", t.content, t.size) == 0) {
        t = lexer_next(lexer);
        float duration = atof(lexer_token_to_cstr(lexer, &t));
        if (brace) {
          frame = tkbc_script_frames_quit(duration);
          tkbc_sript_team_scratch_buf_frames_append_and_free(env, frame);
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
      tkbc_register_frames_array(env, frames);
    } break;

    err:
    case ERROR:
    default:
      tkbc_fprintf(stderr, "ERROR", "Invalid token: %s\n",
                   lexer_token_to_cstr(lexer, &t));
    }

    t = lexer_next(lexer);
  }

  if (script_begin) {
    tkbc_fprintf(stderr, "ERROR", "Script END is not defined.");
    tkbc__script_end(env);
  }

  lexer_del(lexer);
  if (tmp_buffer.elements)
    free(tmp_buffer.elements);
  tmp_buffer.elements = NULL;
  if (ki.elements)
    free(ki.elements);
  ki.elements = NULL;
}

/**
 * @brief The function can be used to check if every element in the given
 * kite indies is part of the current registered kites.
 *
 * @param env The global state of the application.
 * @param kis The kite indies that should be check against the existing kites.
 * @return True if the given kite indies are valid, otherwise false.
 */
bool tkbc_parsed_kis_is_in_env(Env *env, Index index) {
  for (size_t i = 0; i < env->kite_array->count; ++i) {
    if (env->kite_array->elements[i].kite_id == index) {
      return true;
    }
  }
  return false;
}

/**
 * @brief The function parses the kite ids and unions them with the given
 * orig_kis that are then placed into dest_kis.
 *
 * @param env The global state of the application.
 * @param lexer The data to parse should be located in her.
 * @param dest_kis The out parameter contains the union of the parsed and
 * already existing kis.
 * @param orig_kis The already generated and existing kis.
 * @return True if the merging has worked, false if there are no kites
 * generated yet but the KITES keyword is used or if a to high id was
 * specified in the script that is not generated, or if an parser error
 * occurred.
 */
bool tkbc_parse_kis_after_generation(Env *env, Lexer *lexer, Kite_Ids *dest_kis,
                                     Kite_Ids orig_kis) {

  Token t = lexer_next(lexer);
  if (strncmp("KITES", t.content, t.size) == 0) {
    if (orig_kis.count == 0) {
      return false;
    }
    for (size_t i = 0; i < orig_kis.count; ++i) {
      tkbc_dap(dest_kis, orig_kis.elements[i]);
    }

  } else if (t.kind == PUNCT_LPAREN) {
    t = lexer_next(lexer);
    while (t.kind == NUMBER) {
      int number = atoi(lexer_token_to_cstr(lexer, &t));
      tkbc_dap(dest_kis, number);
      if (!tkbc_parsed_kis_is_in_env(env, number)) {
        tkbc_fprintf(stderr, "ERROR",
                     "The given kites in the listing are invalid: "
                     "Position:%llu:%ld\n",
                     lexer->line_count, (t.content - lexer->content));
        return false;
      }
      t = lexer_next(lexer);
    }

    if (t.kind != PUNCT_RPAREN) {
      return false;
    }

  } else {
    return false;
  }

  return true;
}

/**
 * @brief The function parses a possible move action out of the current
 * lexer content.
 *
 * @param env The global state of the application.
 * @param lexer The data to parse should be located in her.
 * @param kind The kind specifies if the adding or non adding version is
 * expected.
 * @param ki The already generated kite ids to compare to the possible new
 * parsed kite ids.
 * @param brace Represents if the parsing has happened inside a frame block.
 * These for these blocks the frame has to be generated for parallel
 * visualisation.
 * @param tmp_buffer A scratch buffer for number sign constructing after
 * parsing.
 * @return True if the parsing and frame construction has worked, otherwise
 * false.
 */
bool tkbc_parse_move(Env *env, Lexer *lexer, Action_Kind kind, Kite_Ids ki,
                     bool brace, Content tmp_buffer) {
  bool ok = true;
  Kite_Ids kis = {0};
  float x, y, duration;
  Frame *frame = NULL;

  if (!tkbc_parse_kis_after_generation(env, lexer, &kis, ki)) {
    check_return(false);
  }

  if (!tkbc_parse_float(&x, lexer, tmp_buffer)) {
    return false;
  }
  if (!tkbc_parse_float(&y, lexer, tmp_buffer)) {
    return false;
  }
  if (!tkbc_parse_float(&duration, lexer, tmp_buffer)) {
    return false;
  }

  if (kind == KITE_MOVE_ADD) {
    if (brace) {
      frame = tkbc_frame_generate(KITE_MOVE_ADD, kis,
                                  &(CLITERAL(Move_Add_Action){
                                      .position.x = x,
                                      .position.y = y,
                                  }),
                                  duration);
      tkbc_sript_team_scratch_buf_frames_append_and_free(env, frame);
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
      frame = tkbc_frame_generate(KITE_MOVE, kis,
                                  &(CLITERAL(Move_Action){
                                      .position.x = x,
                                      .position.y = y,
                                  }),
                                  duration);
      tkbc_sript_team_scratch_buf_frames_append_and_free(env, frame);
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
  if (kis.elements) {
    free(kis.elements);
    kis.elements = NULL;
  }
  return ok;
}

/**
 * @brief The function parses a possible rotation action out of the current
 * lexer content.
 *
 * @param env The global state of the application.
 * @param lexer The data to parse should be located in her.
 * @param kind The kind specifies if the adding or non adding version is
 * expected.
 * @param ki The already generated kite ids to compare to the possible new
 * parsed kite ids.
 * @param brace Represents if the parsing has happened inside a frame block.
 * These for these blocks the frame has to be generated for parallel
 * visualisation.
 * @param tmp_buffer A scratch buffer for number sign constructing after
 * parsing.
 * @return True if the parsing and frame construction has worked, otherwise
 * false.
 */
bool tkbc_parse_rotation(Env *env, Lexer *lexer, Action_Kind kind, Kite_Ids ki,
                         bool brace, Content tmp_buffer) {
  bool ok = true;
  Kite_Ids kis = {0};
  float angle, duration;
  Frame *frame = NULL;

  if (!tkbc_parse_kis_after_generation(env, lexer, &kis, ki)) {
    check_return(false);
  }

  if (!tkbc_parse_float(&angle, lexer, tmp_buffer)) {
    return false;
  }
  if (!tkbc_parse_float(&duration, lexer, tmp_buffer)) {
    return false;
  }

  if (kind == KITE_ROTATION_ADD) {
    if (brace) {
      frame = tkbc_frame_generate(KITE_ROTATION_ADD, kis,
                                  &(CLITERAL(Rotation_Add_Action){
                                      .angle = angle,
                                  }),
                                  duration);
      tkbc_sript_team_scratch_buf_frames_append_and_free(env, frame);
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
      frame = tkbc_frame_generate(KITE_ROTATION, kis,
                                  &(CLITERAL(Rotation_Action){
                                      .angle = angle,
                                  }),
                                  duration);
      tkbc_sript_team_scratch_buf_frames_append_and_free(env, frame);
    } else {
      tkbc_register_frames(env, tkbc_frame_generate(KITE_ROTATION, kis,
                                                    &(CLITERAL(Rotation_Action){
                                                        .angle = angle,
                                                    }),
                                                    duration));
    }
  }

check:
  if (kis.elements) {
    free(kis.elements);
    kis.elements = NULL;
  }
  return ok;
}

/**
 * @brief The function parses a possible tip rotation action out of the
 * current lexer content.
 *
 * @param env The global state of the application.
 * @param lexer The
 * @param kind The kind specifies if the adding or non adding version is
 * expected.
 * @param frames The out parameter is filled with the generated frame if the
 * parsing succeeds.
 * @param ki The already generated kite ids to compare to the possible new
 * parsed kite ids.
 * @param brace Represents if the parsing has happened inside a frame block.
 * These for these blocks the frame has to be generated for parallel
 * visualisation.
 * @param tmp_buffer A scratch buffer for number sign constructing after
 * parsing.
 * @return True if the parsing and frame construction has worked, otherwise
 * false.
 */
bool tkbc_parse_tip_rotation(Env *env, Lexer *lexer, Action_Kind kind,
                             Kite_Ids ki, bool brace, Content tmp_buffer) {
  bool ok = true;
  Kite_Ids kis = {0};
  TIP tip;
  float angle, duration;
  Frame *frame = NULL;

  if (!tkbc_parse_kis_after_generation(env, lexer, &kis, ki)) {
    check_return(false);
  }

  if (!tkbc_parse_float(&angle, lexer, tmp_buffer)) {
    return false;
  }

  Token t = lexer_next(lexer);
  if (strncmp("RIGHT", t.content, t.size) == 0) {
    tip = RIGHT_TIP;
  } else if (strncmp("LEFT", t.content, t.size) == 0) {
    tip = LEFT_TIP;
  } else {
    check_return(false);
  }

  if (!tkbc_parse_float(&duration, lexer, tmp_buffer)) {
    return false;
  }

  if (kind == KITE_TIP_ROTATION_ADD) {
    if (brace) {
      frame = tkbc_frame_generate(KITE_TIP_ROTATION_ADD, kis,
                                  &(CLITERAL(Tip_Rotation_Add_Action){
                                      .angle = angle,
                                      .tip = tip,
                                  }),
                                  duration);
      tkbc_sript_team_scratch_buf_frames_append_and_free(env, frame);
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
      frame = tkbc_frame_generate(KITE_TIP_ROTATION, kis,
                                  &(CLITERAL(Tip_Rotation_Action){
                                      .angle = angle,
                                      .tip = tip,
                                  }),
                                  duration);
      tkbc_sript_team_scratch_buf_frames_append_and_free(env, frame);
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
  if (kis.elements) {
    free(kis.elements);
    kis.elements = NULL;
  }
  return ok;
}

#define tkbc_parse_number_prolog(lexer, tmp_buffer)                            \
  do {                                                                         \
    Token token = lexer_next(lexer);                                           \
    char sign;                                                                 \
    bool issign = false;                                                       \
                                                                               \
    if (strncmp("-", token.content, token.size) == 0 ||                        \
        strncmp("+", token.content, token.size) == 0) {                        \
      issign = true;                                                           \
      sign = *(char *)token.content;                                           \
    }                                                                          \
                                                                               \
    if (token.kind != NUMBER && !issign) {                                     \
      return false;                                                            \
    }                                                                          \
    if (issign) {                                                              \
      token = lexer_next(lexer);                                               \
    }                                                                          \
    if (token.kind != NUMBER) {                                                \
      return false;                                                            \
    }                                                                          \
                                                                               \
    tmp_buffer.count = 0;                                                      \
    if (issign) {                                                              \
      tkbc_dap(&tmp_buffer, sign);                                             \
    }                                                                          \
    tkbc_dapc(&tmp_buffer, token.content, token.size);                         \
    tkbc_dap(&tmp_buffer, 0);                                                  \
  } while (0)

bool tkbc_parse_float(float *number, Lexer *lexer, Content tmp_buffer) {
  tkbc_parse_number_prolog(lexer, tmp_buffer);
  *number = atof(tmp_buffer.elements);
  return true;
}

bool tkbc_parse_size_t(size_t *number, Lexer *lexer, Content tmp_buffer) {
  tkbc_parse_number_prolog(lexer, tmp_buffer);
  *number = (size_t)atol(tmp_buffer.elements);
  return true;
}

bool tkbc_parse_team_figures(Env *env, Kite_Ids kis, Lexer *lexer,
                             const char *function_name, Content tmp_buffer) {

  if (strcmp("TEAM_LINE", function_name) == 0) {
    Vector2 position, offset;
    float h_padding, move_duration;

    if (!tkbc_parse_float(&position.x, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&position.y, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&offset.x, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&offset.y, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&h_padding, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&move_duration, lexer, tmp_buffer)) {
      return false;
    }

    tkbc_script_team_line(env, kis, position, offset, h_padding, move_duration);
  } else if (strcmp("TEAM_GRID", function_name) == 0) {
    Kite_Ids kis = {0};
    Vector2 position, offset;
    float v_padding, h_padding;
    size_t rows, columns;
    float move_duration;

    if (!tkbc_parse_float(&position.x, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&position.y, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&offset.x, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&offset.y, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&v_padding, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&h_padding, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_size_t(&rows, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_size_t(&columns, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&move_duration, lexer, tmp_buffer)) {
      return false;
    }

    tkbc_script_team_grid(env, kis, position, offset, v_padding, h_padding,
                          rows, columns, move_duration);
  } else if (strcmp("TEAM_MOUNTAIN", function_name) == 0) {
    Vector2 position, offset;
    float v_padding, h_padding;
    float move_duration, rotation_duration;

    if (!tkbc_parse_float(&position.x, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&position.y, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&offset.x, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&offset.y, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&v_padding, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&h_padding, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&move_duration, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&rotation_duration, lexer, tmp_buffer)) {
      return false;
    }

    tkbc_script_team_mountain(env, kis, position, offset, v_padding, h_padding,
                              move_duration, rotation_duration);
  } else if (strcmp("TEAM_VALLEY", function_name) == 0) {
    Vector2 position, offset;
    float v_padding, h_padding;
    float move_duration, rotation_duration;

    if (!tkbc_parse_float(&position.x, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&position.y, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&offset.x, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&offset.y, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&v_padding, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&h_padding, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&move_duration, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&rotation_duration, lexer, tmp_buffer)) {
      return false;
    }

    tkbc_script_team_valley(env, kis, position, offset, v_padding, h_padding,
                            move_duration, rotation_duration);
  } else if (strcmp("TEAM_ARC", function_name) == 0) {
    Vector2 position, offset;
    float v_padding, h_padding;
    float angle;
    float move_duration, rotation_duration;

    if (!tkbc_parse_float(&position.x, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&position.y, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&offset.x, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&offset.y, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&v_padding, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&h_padding, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&angle, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&move_duration, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&rotation_duration, lexer, tmp_buffer)) {
      return false;
    }

    tkbc_script_team_arc(env, kis, position, offset, v_padding, h_padding,
                         angle, move_duration, rotation_duration);
  } else if (strcmp("TEAM_MOUTH", function_name) == 0) {
    Vector2 position, offset;
    float v_padding, h_padding;
    float angle;
    float move_duration, rotation_duration;

    if (!tkbc_parse_float(&position.x, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&position.y, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&offset.x, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&offset.y, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&v_padding, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&h_padding, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&angle, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&move_duration, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&rotation_duration, lexer, tmp_buffer)) {
      return false;
    }

    tkbc_script_team_mouth(env, kis, position, offset, v_padding, h_padding,
                           angle, move_duration, rotation_duration);
  } else if (strcmp("TEAM_BOX", function_name) == 0) {
    DIRECTION direction;
    float angle, box_size;
    float move_duration, rotation_duration;

    {
      Token token = lexer_next(lexer);
      if (token.kind != IDENTIFIER) {
        return false;
      }
      if (strcmp("LEFT", lexer_token_to_cstr(lexer, &token)) == 0) {
        direction = LEFT;
      } else if (strcmp("RIGHT", lexer_token_to_cstr(lexer, &token)) == 0) {
        direction = RIGHT;
      } else {
        return false;
      }
    }

    if (!tkbc_parse_float(&angle, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&box_size, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&move_duration, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&rotation_duration, lexer, tmp_buffer)) {
      return false;
    }

    tkbc_script_team_box(env, kis, direction, angle, box_size, move_duration,
                         rotation_duration);
  } else if (strcmp("TEAM_BOX_LEFT", function_name) == 0) {
    float box_size;
    float move_duration, rotation_duration;

    if (!tkbc_parse_float(&box_size, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&move_duration, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&rotation_duration, lexer, tmp_buffer)) {
      return false;
    }

    tkbc_script_team_box_left(env, kis, box_size, move_duration,
                              rotation_duration);
  } else if (strcmp("TEAM_BOX_RIGHT", function_name) == 0) {
    float box_size;
    float move_duration, rotation_duration;

    if (!tkbc_parse_float(&box_size, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&move_duration, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&rotation_duration, lexer, tmp_buffer)) {
      return false;
    }

    tkbc_script_team_box_right(env, kis, box_size, move_duration,
                               rotation_duration);
  } else if (strcmp("TEAM_SPLIT_BOX_UP", function_name) == 0) {
    ODD_EVEN odd_even;
    float box_size;
    float move_duration, rotation_duration;

    {
      Token token = lexer_next(lexer);
      if (token.kind != IDENTIFIER) {
        return false;
      }
      if (strcmp("ODD", lexer_token_to_cstr(lexer, &token)) == 0) {
        odd_even = ODD;
      } else if (strcmp("EVEN", lexer_token_to_cstr(lexer, &token)) == 0) {
        odd_even = EVEN;
      } else {
        return false;
      }
    }

    if (!tkbc_parse_float(&box_size, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&move_duration, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&rotation_duration, lexer, tmp_buffer)) {
      return false;
    }

    tkbc_script_team_split_box_up(env, kis, odd_even, box_size, move_duration,
                                  rotation_duration);

  } else if (strcmp("TEAM_DIAMOND", function_name) == 0) {
    DIRECTION direction;
    float angle, box_size;
    float move_duration, rotation_duration;

    {
      Token token = lexer_next(lexer);
      if (token.kind != IDENTIFIER) {
        return false;
      }
      if (strcmp("LEFT", lexer_token_to_cstr(lexer, &token)) == 0) {
        direction = LEFT;
      } else if (strcmp("RIGHT", lexer_token_to_cstr(lexer, &token)) == 0) {
        direction = RIGHT;
      } else {
        return false;
      }
    }

    if (!tkbc_parse_float(&angle, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&box_size, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&move_duration, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&rotation_duration, lexer, tmp_buffer)) {
      return false;
    }

    tkbc_script_team_diamond(env, kis, direction, angle, box_size,
                             move_duration, rotation_duration);
  } else if (strcmp("TEAM_DIAMOND_LEFT", function_name) == 0) {
    float box_size;
    float move_duration, rotation_duration;

    if (!tkbc_parse_float(&box_size, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&move_duration, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&rotation_duration, lexer, tmp_buffer)) {
      return false;
    }

    tkbc_script_team_diamond_left(env, kis, box_size, move_duration,
                                  rotation_duration);
  } else if (strcmp("TEAM_DIAMOND_RIGHT", function_name) == 0) {
    float box_size;
    float move_duration, rotation_duration;

    if (!tkbc_parse_float(&box_size, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&move_duration, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&rotation_duration, lexer, tmp_buffer)) {
      return false;
    }

    tkbc_script_team_diamond_right(env, kis, box_size, move_duration,
                                   rotation_duration);
  } else if (strcmp("TEAM_ROLL_SPLIT_UP", function_name) == 0) {
    ODD_EVEN odd_even;
    float radius;
    size_t begin_angle;
    size_t end_angle;
    float move_duration;

    {
      Token token = lexer_next(lexer);
      if (token.kind != IDENTIFIER) {
        return false;
      }
      if (strcmp("ODD", lexer_token_to_cstr(lexer, &token)) == 0) {
        odd_even = ODD;
      } else if (strcmp("EVEN", lexer_token_to_cstr(lexer, &token)) == 0) {
        odd_even = EVEN;
      } else {
        return false;
      }
    }

    if (!tkbc_parse_float(&radius, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_size_t(&begin_angle, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_size_t(&end_angle, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&move_duration, lexer, tmp_buffer)) {
      return false;
    }

    tkbc_script_team_roll_split_up(env, kis, odd_even, radius, begin_angle,
                                   end_angle, move_duration);
  } else if (strcmp("TEAM_ROLL_SPLIT_DOWN", function_name) == 0) {
    ODD_EVEN odd_even;
    float radius;
    size_t begin_angle;
    size_t end_angle;
    float move_duration;

    {
      Token token = lexer_next(lexer);
      if (token.kind != IDENTIFIER) {
        return false;
      }
      if (strcmp("ODD", lexer_token_to_cstr(lexer, &token)) == 0) {
        odd_even = ODD;
      } else if (strcmp("EVEN", lexer_token_to_cstr(lexer, &token)) == 0) {
        odd_even = EVEN;
      } else {
        return false;
      }
    }

    if (!tkbc_parse_float(&radius, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_size_t(&begin_angle, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_size_t(&end_angle, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&move_duration, lexer, tmp_buffer)) {
      return false;
    }

    tkbc_script_team_roll_split_down(env, kis, odd_even, radius, begin_angle,
                                     end_angle, move_duration);
  } else if (strcmp("TEAM_ROLL_UP_ANTI_CLOCKWISE", function_name) == 0) {
    float radius;
    size_t begin_angle;
    size_t end_angle;
    float move_duration;

    if (!tkbc_parse_float(&radius, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_size_t(&begin_angle, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_size_t(&end_angle, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&move_duration, lexer, tmp_buffer)) {
      return false;
    }

    tkbc_script_team_roll_up_anti_clockwise(env, kis, radius, begin_angle,
                                            end_angle, move_duration);
  } else if (strcmp("TEAM_ROLL_UP_CLOCKWISE", function_name) == 0) {
    float radius;
    size_t begin_angle;
    size_t end_angle;
    float move_duration;

    if (!tkbc_parse_float(&radius, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_size_t(&begin_angle, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_size_t(&end_angle, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&move_duration, lexer, tmp_buffer)) {
      return false;
    }

    tkbc_script_team_roll_up_clockwise(env, kis, radius, begin_angle, end_angle,
                                       move_duration);
  } else if (strcmp("TEAM_ROLL_DOWN_ANTI_CLOCKWISE", function_name) == 0) {
    float radius;
    size_t begin_angle;
    size_t end_angle;
    float move_duration;

    if (!tkbc_parse_float(&radius, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_size_t(&begin_angle, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_size_t(&end_angle, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&move_duration, lexer, tmp_buffer)) {
      return false;
    }

    tkbc_script_team_roll_down_anti_clockwise(env, kis, radius, begin_angle,
                                              end_angle, move_duration);
  } else if (strcmp("TEAM_ROLL_DOWN_CLOCKWISE", function_name) == 0) {
    float radius;
    size_t begin_angle;
    size_t end_angle;
    float move_duration;

    if (!tkbc_parse_float(&radius, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_size_t(&begin_angle, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_size_t(&end_angle, lexer, tmp_buffer)) {
      return false;
    }
    if (!tkbc_parse_float(&move_duration, lexer, tmp_buffer)) {
      return false;
    }

    tkbc_script_team_roll_down_clockwise(env, kis, radius, begin_angle,
                                         end_angle, move_duration);
  }

  return true;
}
