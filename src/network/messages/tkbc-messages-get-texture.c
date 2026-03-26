#include "../../../external/lexer/tkbc-lexer.h"
#include "../../global/tkbc-types.h"
#include "../../../external/space/space.h"
#include "../../choreographer/tkbc-asset-handler.h"
#include "../tkbc-servers-common.h"
#include "tkbc-interface.h"

#include "tkbc-messages.h"

#include <stdbool.h>

extern Kite_Images kite_images;

/**
 * @brief [TODO:description]
 *
 * @param lexer [TODO:parameter]
 * @param client [TODO:parameter]
 * @return [TODO:return]
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

  Kite_Image *kite_image = tkbc_find_asset_in_kite_images(texture_id);
  if (kite_image == NULL) {
    // Can not provide texture.
    return false;
  }

  space_dapf(&client->send_msg_buffer_space, &client->send_msg_buffer,
             "%d:", MESSAGE_SEND_TEXTURE);

  tkbc_message_append_image_data(&client->send_msg_buffer_space,
                                 &client->send_msg_buffer, kite_image->normal,
                                 kite_image->id);

  space_dapf(&client->send_msg_buffer_space, &client->send_msg_buffer, "\r\n");
  return true;
}
