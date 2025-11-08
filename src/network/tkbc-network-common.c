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
extern Kite_Textures kite_textures;

/**
 * @brief The function resets the space and sets the elements ptr from the
 * message to NULL.
 *
 * @param space The arena style allocator.
 * @param message The dynamic arena of a message.
 */
void tkbc_reset_space_and_null_message(Space *space, Message *message) {
  memset(message, 0, sizeof(*message));
  space_reset_space(space);
}

/**
 * @brief The function assigns the given values to the passed state.
 *
 * @param state The kite_state that should be updated.
 * @param x The new x value of the kite center.
 * @param y The new y value of the kite center.
 * @param angle The new angle of the kite.
 * @param color The new color of the kite.
 * @param is_reversed If the kite should fly reverse by default.
 * @param texture_id The id of the texture that should be used to display the
 * kite.
 */
void tkbc_assign_values_to_kitestate(Kite_State *state, float x, float y,
                                     float angle, Color color, bool is_reversed,
                                     size_t texture_id) {
  assert(state);
  state->kite->center.x = x;
  state->kite->center.y = y;
  state->kite->angle = angle;
  state->kite->body_color = color;
  state->is_kite_reversed = is_reversed;
  state->kite->texture_id = texture_id;

  assert(kite_textures.count > texture_id);
  tkbc_set_kite_texture(state->kite, &kite_textures.elements[texture_id]);

  tkbc_kite_update_internal(state->kite);
}

/**
 * @brief The function extracts the values that should belong to a kite out of
 * the lexer data.
 *
 * @param lexer The current state and data of the string to parse.
 * @param id -1 if the parsed kite values should be updated, if the values
 * should not be updated pass the kite_id.
 * @return True if the kite values can be parsed out of the data the lexer
 * contains and is updated, if the kite values are parsed not updated -1 is
 * returned and false is returned if the parsing has failed and no updates were
 * made.
 */
int tkbc_parse_single_kite_value(Lexer *lexer, ssize_t kite_id) {
  size_t id, texture_id;
  float x, y, angle;
  Color color;
  bool is_reversed;
  if (!tkbc_parse_message_kite_value(lexer, &id, &x, &y, &angle, &color,
                                     &texture_id, &is_reversed)) {
    return 0;
  }

  if (kite_id >= 0) {
    if ((size_t)kite_id == id) {
      return -1;
    }
  }

  Kite_State *state = tkbc_get_kite_state_by_id(env, id);
  // NOTE: This ignores unknown kites and just sets the values for valid ones.
  // Unknown kites are not a parsing error so true is returned.
  // TODO: But for the client not the server the kite missing kite should be
  // handled because the server expects the client to have it so the client
  // lost it or hasn't registered one jet.
  if (state) {
    tkbc_assign_values_to_kitestate(state, x, y, angle, color, is_reversed,
                                    texture_id);
  }
  return 1;
}

/**
 * @brief The function parses all values out of a single
 * MESSAGE_SINGLE_KITE_UPDATE that should be located in the lexer data.
 *
 * @param lexer The current state and data of the string to parse.
 * @param kite_id The id the corresponding parsed value is assigned to.
 * @param x The x position the corresponding parsed value is assigned to.
 * @param y The y position the corresponding parsed value is assigned to.
 * @param angle The angle the corresponding parsed value is assigned to.
 * @param color The color the corresponding parsed value is assigned to.
 * @param texture_id The id that represents the texture in the global
 * kite_textures.
 * @return True if all values have been parsed correctly and are assigned,
 * otherwise false.
 */
bool tkbc_parse_message_kite_value(Lexer *lexer, size_t *kite_id, float *x,
                                   float *y, float *angle, Color *color,
                                   size_t *texture_id, bool *is_reversed) {
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
  uint32_t color_number = atol(lexer_token_to_cstr(lexer, &token));
  (*color).a = (color_number >> 0) & 0xFF;
  (*color).b = (color_number >> 8) & 0xFF;
  (*color).g = (color_number >> 16) & 0xFF;
  (*color).r = (color_number >> 24) & 0xFF;
  token = lexer_next(lexer);
  if (token.kind != PUNCT_COLON) {
    check_return(false);
  }

  token = lexer_next(lexer);
  if (token.kind != NUMBER) {
    check_return(false);
  }
  *texture_id = atoi(lexer_token_to_cstr(lexer, &token));

  token = lexer_next(lexer);
  if (token.kind != PUNCT_COLON) {
    check_return(false);
  }

  token = lexer_next(lexer);
  if (token.kind != NUMBER) {
    check_return(false);
  }
  *is_reversed = !!atoi(lexer_token_to_cstr(lexer, &token));

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

/**
 * @brief The function tries to find \r\n in the message starting form the given
 * position without allocation in message the same way strstr does it.
 *
 * @param message The message structure the should hold the data.
 * @param position The position from where the search should start.
 * @return The pointer of the position where the needle starts or NULL.
 */
char *tkbc_find_rn_in_message_from_position(Message *message,
                                            unsigned long long position) {

  if (!message || !message->elements || position >= message->count) {
    return NULL;
  }
  if (message->count < 2) {
    return NULL;
  }

  char message_last = message->elements[message->count - 1];
  message->elements[message->count - 1] = '\0';
  char *ptr = strstr(message->elements + position, "\r\n");
  message->elements[message->count - 1] = message_last;
  if (ptr == NULL) {
    if (message->elements[message->count - 2] == '\r' &&
        message->elements[message->count - 1] == '\n') {
      return &message->elements[message->count - 2];
    }

    return NULL;
  }

  return ptr;
}
