#include "../../../external/lexer/tkbc-lexer.h"
#include "../../../external/space/space.h"
#include "../../choreographer/tkbc-asset-handler.h"
#include "../../choreographer/tkbc-script-handler.h"
#include "../../global/tkbc-types.h"
#include "../tkbc-servers-common.h"
#include "tkbc-messages.h"

#include <stdbool.h>

/**
 * @brief Handles a SEND_TEXTURE_ID message by associating a texture with a
 * kite, requesting the texture data if not yet available.
 *
 * @param env The global state of the application.
 * @param lexer The lexer positioned at the message content.
 * @param client The client that sent the message.
 * @return True if the texture id was processed successfully, otherwise false.
 */
bool tkbc_messages_send_texture_id(Env *env, Lexer *lexer, Client *client) {
  Token token;
  token = lexer_next(lexer);
  if (token.kind != NUMBER) {
    return false;
  }
  size_t kite_id = strtoul(lexer_token_to_cstr(lexer, &token), NULL, 10);
  token = lexer_next(lexer);
  if (token.kind != PUNCT_COLON) {
    return false;
  }

  token = lexer_next(lexer);
  if (token.kind != NUMBER) {
    return false;
  }
  // Negative values should not be send by the server. The serer should
  // always send a valid texture_id.
  ssize_t texture_id = atoll(lexer_token_to_cstr(lexer, &token));
  assert(texture_id != -1);
  token = lexer_next(lexer);
  if (token.kind != PUNCT_COLON) {
    return false;
  }

  Asset *asset = tkbc_find_asset_from_id(texture_id);
  if (asset == NULL) {
    // The message is split to allow getting a texture by its own at some
    // point. Maybe this is never needed, but it can be useful when a client
    // want to get all the available textures in the server.
    space_dapf(&client->send_msg_buffer_space, &client->send_msg_buffer,
               "%d:%zu:\r\n", MESSAGE_GET_TEXTURE, texture_id);

    space_dapf(&client->send_msg_buffer_space, &client->send_msg_buffer,
               "%d:%zu:\r\n", MESSAGE_GET_TEXTURE_ID, kite_id);

  } else {
    // The kite_id should be present in the client, because it requested the
    // texture_id with that kite_id before.
    Kite *kite = tkbc_get_kite_by_id(env, kite_id);
    assert(kite != NULL);
    kite->texture_id = texture_id;
  }
  return true;
}
