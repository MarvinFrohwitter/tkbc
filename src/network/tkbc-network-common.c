#include "tkbc-network-common.h"
#include "../global/tkbc-utils.h"

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern Env *env;

uint16_t tkbc_port_parsing(const char *port_check) {
  for (size_t i = 0; i < strlen(port_check); ++i) {
    if (!isdigit(port_check[i])) {
      fprintf(stderr, "ERROR: The given port [%s] is not valid.\n", port_check);
      exit(1);
    }
  }
  int port = atoi(port_check);
  if (port >= 65535 || port <= 0) {
    fprintf(stderr, "ERROR: The given port [%s] is not valid.\n", port_check);
    exit(1);
  }

  return (uint16_t)port;
}

bool tkbc_message_append_clientkite(size_t client_id, Message *message) {
  char buf[64];
  for (size_t i = 0; i < env->kite_array->count; ++i) {
    if (client_id == env->kite_array->elements[i].kite_id) {
      memset(buf, 0, sizeof(buf));
      snprintf(buf, sizeof(buf), "%zu", client_id);
      tkbc_dapc(message, buf, strlen(buf));

      tkbc_dap(message, ':');
      memset(buf, 0, sizeof(buf));
      float x = env->kite_array->elements[i].kite->center.x;
      float y = env->kite_array->elements[i].kite->center.y;
      float angle = env->kite_array->elements[i].kite->angle;
      snprintf(buf, sizeof(buf), "(%f,%f):%f", x, y, angle);
      tkbc_dapc(message, buf, strlen(buf));

      tkbc_dap(message, ':');
      memset(buf, 0, sizeof(buf));
      snprintf(buf, sizeof(buf), "%u",
               *(uint32_t *)&env->kite_array->elements[i].kite->body_color);
      tkbc_dapc(message, buf, strlen(buf));
      tkbc_dap(message, ':');
      return true;
    }
  }
  return false;
}

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
  size_t color_number = atoi(lexer_token_to_cstr(lexer, &token));
  *color = *(Color *)&color_number;
  token = lexer_next(lexer);
  if (token.kind != PUNCT_COLON) {
    check_return(false);
  }
check:
  if (buffer.elements) {
    free(buffer.elements);
  }
  return ok ? true : false;
}
