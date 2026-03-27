#include "tkbc-network-common.h"
#include "../choreographer/tkbc-asset-handler.h"
#include "../choreographer/tkbc-script-handler.h"
#include "../choreographer/tkbc.h"
#include "../global/tkbc-utils.h"
#include "tkbc-servers-common.h"

#include <math.h>
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
 * @param texture_id The id of the texture that should be used to display the
 * kite.
 * @param is_reversed If the kite should fly reverse by default.
 * @param is_active If the kite should be displayed on the screen.
 */
void tkbc_assign_values_to_kitestate(Kite_State *state, float x, float y,
                                     float angle, Color color,
                                     ssize_t texture_id, bool is_reversed,
                                     bool is_active) {
  // There should not be a single missing texture in here.
  // This just enshures that not an implicit cast from (ssize_t) to (size_t)
  // happens when calling this function. For the same reason the type of
  // texture_id is (ssize_t) to catch it here explicitly.
  assert(texture_id >= 0);

  assert(state);
  state->kite->center.x = x;
  state->kite->center.y = y;
  state->kite->angle = angle;
  state->kite->body_color = color;
  state->kite->texture_id = texture_id;
  state->is_kite_reversed = is_reversed;
  state->is_active = is_active;

  // This is needed because in the server this step
  // is meaningless. The server don't have to load assets to
  // administrate them the ids of the assets should be enough.
#ifndef TKBC_SERVER
  // NOTE if the new designed texture was not send to the other client the
  // client has a smaller textures.count,
  Kite_Texture *kite_texture = tkbc_find_asset_in_kite_textures(texture_id);
  assert(kite_texture != NULL);
  tkbc_set_kite_texture(state->kite, kite_texture);
#endif

  tkbc_kite_update_internal(state->kite);
}

/**
 * @brief The function extracts the values that should belong to a kite out of
 * the lexer data.
 *
 * @param lexer The current state and data of the string to parse.
 * @param id -1 if the parsed kite values should be updated, if the values
 * should not be updated pass the kite_id.
 * @param parsed_id The kite_id that is parsed out.
 * @return 1 if the kite values can be parsed out of the data the lexer
 * contains and is updated, 2 every thing like 1 but the assigned texture is
 * KITE_COLORIZER because the parsed texture was not available, if the kite
 * values are parsed not updated -1 is returned and 0 is returned if the
 * parsing has failed and no updates were made.
 */
int tkbc_parse_single_kite_value(Lexer *lexer, ssize_t kite_id,
                                 size_t *parsed_id) {
  int ok = 1;

  float x, y, angle;
  Color color;
  bool is_reversed, is_active;

  ssize_t texture_id;
  size_t texture_width, texture_height, texture_format;
  Space *data_space = space_get_tspace();
  unsigned char *texture_data = NULL;

  if (!tkbc_parse_message_kite_value(
          lexer, parsed_id, &x, &y, &angle, &color, &texture_id, &texture_width,
          &texture_height, &texture_format, data_space, &texture_data,
          &is_reversed, &is_active)) {

    check_return(0);
  }

  if (kite_id >= 0) {
    if ((size_t)kite_id == *parsed_id) {
      check_return(-1);
    }
  }

  // Append it
  if (texture_id == -1) {
    texture_id = tkbc_append_kite_image(texture_data, texture_width,
                                        texture_height, texture_format);
#ifndef TKBC_SERVER
    tkbc_append_kite_texture(kite_images.elements[kite_images.count - 1]);
#endif
  }

  bool found = tkbc_find_asset_id_in_kite_images(texture_id);
  if (!found && texture_id != -1) {
    texture_id = KITE_COLORIZER;
    ok = 2;
  }

  Kite_State *state = tkbc_get_kite_state_by_id(env, *parsed_id);
  // NOTE: This ignores unknown kites and just sets the values for valid ones.
  // Unknown kites are not a parsing error so true is returned.
  // TODO: But for the client not the server the kite missing kite should be
  // handled because the server expects the client to have it so the client
  // lost it or hasn't registered one jet.
  //
  // // TODO: So for the client register the kite like single kite add kite.
  if (state) {
    tkbc_assign_values_to_kitestate(state, x, y, angle, color, texture_id,
                                    is_reversed, is_active);
  }

check:
  space_reset_tspace();
  return ok;
}

/**
 * @brief The function parses image data from the lexer. It extracts the texture
 * id, width, height, format, and pixel data for a kite texture.
 *
 * @param lexer The current lexer containing the message data.
 * @param data_space The space for allocating image data.
 * @param data Pointer to store the parsed image data.
 * @param width Pointer to store the image width.
 * @param height Pointer to store the image height.
 * @param format Pointer to store the pixel format.
 * @param texture_id Pointer to store the texture id.
 * @return True if the image was parsed successfully, otherwise false.
 */
bool tkbc_parse_image(Lexer *lexer, Space *data_space, unsigned char **data,
                      size_t *width, size_t *height, size_t *format,
                      size_t *texture_id) {
  bool ok = true;
  Token token;

  token = lexer_next(lexer);
  if (token.kind != NUMBER) {
    return false;
  }
  *texture_id = atoll(lexer_token_to_cstr(lexer, &token));
  token = lexer_next(lexer);
  if (token.kind != PUNCT_COLON) {
    return false;
  }

  token = lexer_next(lexer);
  if (token.kind != NUMBER) {
    return false;
  }
  *width = atoll(lexer_token_to_cstr(lexer, &token));
  token = lexer_next(lexer);
  if (token.kind != PUNCT_COLON) {
    return false;
  }

  token = lexer_next(lexer);
  if (token.kind != NUMBER) {
    return false;
  }
  *height = atoll(lexer_token_to_cstr(lexer, &token));
  token = lexer_next(lexer);
  if (token.kind != PUNCT_COLON) {
    return false;
  }

  token = lexer_next(lexer);
  if (token.kind != NUMBER) {
    return false;
  }
  *format = atoll(lexer_token_to_cstr(lexer, &token));
  token = lexer_next(lexer);
  if (token.kind != PUNCT_COLON) {
    return false;
  }

  assert(*format == PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
  *data = space_malloc(data_space, *width * *height * 4 * sizeof(**data));
  if (!*data) {
    return false;
  }

  size_t offset = 0;
  for (size_t y = 0; y < *height; y++) {
    for (size_t x = 0; x < *width; x++) {
      token = lexer_next(lexer);
      if (token.kind != NUMBER) {
        check_return(false);
      }

      uint32_t color_number =
          strtoull(lexer_token_to_cstr(lexer, &token), NULL, 10);
      memcpy(*data + offset, &color_number, sizeof(color_number));

      token = lexer_next(lexer);
      if (token.kind != PUNCT_COLON) {
        check_return(false);
      }

      offset += sizeof(color_number);
    }
  }

check: {}
  if (!ok) {
    space_reset_space(data_space);
  }
  return ok;
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
 * kite_textures or -1 if the data is passed.
 * @param texture_width The width of the texture.
 * @param texture_height The height of the texture.
 * @param texture_format The format of the texture.
 * @param data_space The space for allocating texture data.
 * @param texture_data Pointer to store the texture data.
 * @param is_reversed If the kite should fly reverse by default.
 * @param is_active If the kite should be displayed on the screen.
 * @return True if all values have been parsed correctly and are assigned,
 * otherwise false.
 */
bool tkbc_parse_message_kite_value(Lexer *lexer, size_t *kite_id, float *x,
                                   float *y, float *angle, Color *color,
                                   ssize_t *texture_id, size_t *texture_width,
                                   size_t *texture_height,
                                   size_t *texture_format, Space *data_space,
                                   unsigned char **texture_data,
                                   bool *is_reversed, bool *is_active) {
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
  uint32_t color_number = atoll(lexer_token_to_cstr(lexer, &token));
  *color = tkbc_uint32_t_to_color(color_number);

  token = lexer_next(lexer);
  if (token.kind != PUNCT_COLON) {
    check_return(false);
  }

  {

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
    *texture_id = atoll(buffer.elements);
    buffer.count = 0;

    token = lexer_next(lexer);
    if (token.kind != PUNCT_COLON) {
      check_return(false);
    }
  }

  if (*texture_id == -1) {
    Id id; // Throw away. This is the id where the client stores the image.
    if (!tkbc_parse_image(lexer, data_space, texture_data, texture_width,
                          texture_height, texture_format, &id)) {
      check_return(false);
    }
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

  token = lexer_next(lexer);
  if (token.kind != NUMBER) {
    check_return(false);
  }
  *is_active = !!atoi(lexer_token_to_cstr(lexer, &token));

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
 * @brief The function tries to find \r\n in the message starting form the
 * given position without allocation in message the same way strstr does it.
 *
 * @param message The message structure the should hold the data.
 * @param position The position from where the search should start.
 * @return The pointer of the position where the needle starts or NULL.
 */
inline char *tkbc_find_rn_in_message_from_position(Message *message,
                                                   size_t position) {

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

inline bool tkbc_error_handling_of_received_message_handler(
    Message *message, Lexer *lexer, bool *reset, bool display_errors) {

  char *rn = tkbc_find_rn_in_message_from_position(message, lexer->position);
  if (rn != NULL) {
    *reset = true;
    size_t jump_length = rn + 2 - &lexer->content[lexer->position];
    //
    // This assumes no logging is needed it destroys the correctness of a line
    // and character reporting.
    // use lexer_chop_char(lexer, jump_length); instead when logging is
    // needed again..
    lexer->position += jump_length;

    if (display_errors) {
      tkbc_fprintf(stderr, "WARNING", "Message: Parsing error: %.*s\n",
                   jump_length, message->elements + message->i);
    }
    return true;
  }

  *reset = false;
  if (display_errors) {
    tkbc_fprintf(stderr, "WARNING",
                 "Message unfinished: first read bytes: %zu\n",
                 message->count - message->i);
  }

  return false;
}
