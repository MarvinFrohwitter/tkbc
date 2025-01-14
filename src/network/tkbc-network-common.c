#include "tkbc-network-common.h"
#include "../choreographer/tkbc.h"
#include "../global/tkbc-utils.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern Env *env;

/**
 * @brief The function checks if the given port is valid and if so returns the
 * port as a number. If the given string does not contain a valid port the
 * program will crash.
 *
 * @param port_check The character string that is potently a port.
 * @return The parsed port as a uint16_t.
 */
uint16_t tkbc_port_parsing(const char *port_check) {
  for (size_t i = 0; i < strlen(port_check); ++i) {
    if (!isdigit(port_check[i])) {
      tkbc_fprintf(stderr, "ERROR", "The given port [%s] is not valid.\n",
                   port_check);
      exit(1);
    }
  }
  int port = atoi(port_check);
  if (port >= 65535 || port <= 0) {
    tkbc_fprintf(stderr, "ERROR", "The given port [%s] is not valid.\n",
                 port_check);
    exit(1);
  }

  return (uint16_t)port;
}

/**
 * @brief The function constructs a message part that contains the information
 * from the given kite_state.
 *
 * @param kite_state The kite state where the information is extracted from.
 * @param message The Message struct that should contain the serialized data.
 */
void tkbc_message_append_kite(Kite_State *kite_state, Message *message) {
  char buf[64];
  memset(buf, 0, sizeof(buf));
  snprintf(buf, sizeof(buf), "%zu", kite_state->kite_id);
  tkbc_dapc(message, buf, strlen(buf));
  tkbc_dap(message, ':');
  memset(buf, 0, sizeof(buf));
  float x = kite_state->kite->center.x;
  float y = kite_state->kite->center.y;
  float angle = kite_state->kite->angle;
  snprintf(buf, sizeof(buf), "(%f,%f):%f", x, y, angle);
  tkbc_dapc(message, buf, strlen(buf));

  tkbc_dap(message, ':');
  memset(buf, 0, sizeof(buf));
  snprintf(buf, sizeof(buf), "%u", *(uint32_t *)&kite_state->kite->body_color);
  tkbc_dapc(message, buf, strlen(buf));
  tkbc_dap(message, ':');
}

/**
 * @brief The function constructs the message part of a kite
 *
 * @param client_id The id of the kite which data should be appended to the
 * message.
 * @param message The Message struct that should contain the serialized data.
 * @return True if the given kite id was found and the data is appended,
 * otherwise false.
 */
bool tkbc_message_append_clientkite(size_t client_id, Message *message) {
  for (size_t i = 0; i < env->kite_array->count; ++i) {
    if (client_id == env->kite_array->elements[i].kite_id) {
      Kite_State *kite_state = &env->kite_array->elements[i];
      tkbc_message_append_kite(kite_state, message);
      return true;
    }
  }
  return false;
}

/**
 * @brief The function extracts the values that should belong to a kite out of
 * the lexer data.
 *
 * @param lexer The current state and data of the sting to parse.
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
  return true;
}

/**
 * @brief The function parses all values out of a single MESSAGE_KITEVALUE that
 * should be located in the lexer data.
 *
 * @param lexer The current state and data of the sting to parse.
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
  }
  return ok;
}
