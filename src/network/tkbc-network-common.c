#include "tkbc-network-common.h"
#include "../choreographer/tkbc-script-handler.h"
#include "../choreographer/tkbc.h"
#include "../global/tkbc-utils.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern Env *env;

/**
 * @brief The function extracts the values that should belong to a kite out of
 * the lexer data.
 *
 * @param lexer The current state and data of the string to parse.
 * @return True if the kite values can be parsed out of the data the lexer
 * contains.
 */
bool tkbc_parse_single_kite_value(Lexer *lexer) {
  size_t kite_id;
  float x, y, angle;
  Color color;
  if (!tkbc_parse_message_kite_value(lexer, &kite_id, &x, &y, &angle, &color)) {
    return false;
  }

  Kite *kite = tkbc_get_kite_by_id(env, kite_id);
  // NOTE: This ignores unknown kites and just sets the values for valid ones.
  // Unknown kites are not a parsing error so true is returned.
  // TODO: But for the client not the server the kite missing kite should be
  // handled because the server expects the client to have it so the client lost
  // it or hasn't registered one jet.
  if (kite) {
    kite->center.x = x;
    kite->center.y = y;
    kite->angle = angle;
    kite->body_color = color;
    tkbc_kite_update_internal(kite);
  }
  return true;
}

/**
 * @brief The function parses all values out of a single MESSAGE_KITEVALUE that
 * should be located in the lexer data.
 *
 * @param lexer The current state and data of the string to parse.
 * @param kite_id The id the corresponding parsed value is assigned to.
 * @param x The x position the corresponding parsed value is assigned to.
 * @param y The y position the corresponding parsed value is assigned to.
 * @param angle The angle the corresponding parsed value is assigned to.
 * @param color The color the corresponding parsed value is assigned to.
 * @return True if all values have been parsed correctly and are assigned,
 * otherwise false.
 */
bool tkbc_parse_message_kite_value(Lexer *lexer, size_t *kite_id, float *x,
                                   float *y, float *angle, Color *color) {
  Content buffer = {0};
  Token token;
  bool ok = true;
  token = lexer_next(lexer);
  if (token.kind != NUMBER) {
    check_return(false);
  }
  *kite_id = atoi(lexer_token_to_cstr(lexer, &token));
  token = lexer_next(lexer);
  if (token.kind != PUNCT_COLON) {
    check_return(false);
  }
  token = lexer_next(lexer);
  if (token.kind != PUNCT_LPAREN) {
    check_return(false);
  }

  token = lexer_next(lexer);
  if (token.kind != NUMBER && token.kind != PUNCT_SUB) {
    check_return(false);
  }
  if (token.kind == PUNCT_SUB) {
    tkbc_dapc(&buffer, token.content, token.size);
    token = lexer_next(lexer);
    if (token.kind != NUMBER) {
      check_return(false);
    }
  }
  tkbc_dapc(&buffer, token.content, token.size);
  tkbc_dap(&buffer, 0);
  *x = atof(buffer.elements);
  buffer.count = 0;

  token = lexer_next(lexer);
  if (token.kind != PUNCT_COMMA) {
    check_return(false);
  }
  token = lexer_next(lexer);
  if (token.kind != NUMBER && token.kind != PUNCT_SUB) {
    check_return(false);
  }
  if (token.kind == PUNCT_SUB) {
    tkbc_dapc(&buffer, token.content, token.size);
    token = lexer_next(lexer);
    if (token.kind != NUMBER) {
      check_return(false);
    }
  }
  tkbc_dapc(&buffer, token.content, token.size);
  tkbc_dap(&buffer, 0);
  *y = atof(buffer.elements);
  buffer.count = 0;

  token = lexer_next(lexer);
  if (token.kind != PUNCT_RPAREN) {
    check_return(false);
  }
  token = lexer_next(lexer);
  if (token.kind != PUNCT_COLON) {
    check_return(false);
  }
  token = lexer_next(lexer);
  if (token.kind != NUMBER && token.kind != PUNCT_SUB) {
    check_return(false);
  }
  if (token.kind == PUNCT_SUB) {
    tkbc_dapc(&buffer, token.content, token.size);
    token = lexer_next(lexer);
    if (token.kind != NUMBER) {
      check_return(false);
    }
  }
  tkbc_dapc(&buffer, token.content, token.size);
  tkbc_dap(&buffer, 0);
  *angle = atof(buffer.elements);
  buffer.count = 0;

  token = lexer_next(lexer);
  if (token.kind != PUNCT_COLON) {
    check_return(false);
  }

  token = lexer_next(lexer);
  if (token.kind != NUMBER) {
    check_return(false);
  }
  uint32_t color_number = atoi(lexer_token_to_cstr(lexer, &token));
  Color *c = (Color *)&color_number;
  *color = *c;
  token = lexer_next(lexer);
  if (token.kind != PUNCT_COLON) {
    check_return(false);
  }
check:
  if (buffer.elements) {
    free(buffer.elements);
    buffer.elements = NULL;
  }
  return ok;
}
