#include "../../../external/lexer/tkbc-lexer.h"
#include "../../../external/space/space.h"
#include "../../choreographer/tkbc-asset-handler.h"
#include "../../global/tkbc-types.h"
#include "../tkbc-servers-common.h"
#include "tkbc-interface.h"

#include "tkbc-messages.h"

#include <stdbool.h>

/**
 * @brief The function parses a texture id out of the given message, looks up
 * the texture asset and appends the kite image data as a MESSAGE_SEND_TEXTURE
 * to the client send buffer.
 *
 * @param lexer The lexer that is used to read the message tokens.
 * @param client The client that the texture data gets send to.
 * @return Returns true if the texture was found and send, otherwise false.
 */
bool tkbc_messages_get_texture(Lexer *lexer, Client *client) {
  Token token;
  token = lexer_next(lexer);
  if (token.kind != NUMBER) {
    return false;
  }
  ssize_t texture_id = atoll(lexer_token_to_cstr(lexer, &token));
  token = lexer_next(lexer);
  if (token.kind != PUNCT_COLON) {
    return false;
  }

  Asset *asset = tkbc_find_asset_from_id(texture_id);
  if (asset == NULL) {
    // Can not provide texture.
    return false;
  }
  assert(asset->type == ASSETS_KITE_DESIGN);
  Kite_Image *kite_image = &asset->as.kite_image;
  if (kite_image == NULL) {
    // Can not provide texture.
    return false;
  }

  space_dapf(&client->send_msg_buffer_space, &client->send_msg_buffer,
             "%d:", MESSAGE_SEND_TEXTURE);

  tkbc_message_append_image_data(&client->send_msg_buffer_space,
                                 &client->send_msg_buffer, kite_image->normal,
                                 asset->id);

  space_dapf(&client->send_msg_buffer_space, &client->send_msg_buffer, "\r\n");
  return true;
}
