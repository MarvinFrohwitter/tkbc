#include "../../../external/lexer/tkbc-lexer.h"
#include "../../../external/space/space.h"
#include "../../choreographer/tkbc-asset-handler.h"
#include "../../choreographer/tkbc-script-handler.h"
#include "../../global/tkbc-types.h"
#include "../tkbc-servers-common.h"
#include "tkbc-messages.h"

#include <stdbool.h>
extern Kite_Images kite_images;

bool tkbc_messages_send_texture_id(Env *env, Lexer *lexer, Client *client) {
  Token token;
  token = lexer_next(lexer);
  if (token.kind != NUMBER) {
    return false;
  }
  size_t kite_id = atoi(lexer_token_to_cstr(lexer, &token));
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

  Kite_Image *kite_image = tkbc_find_asset_in_kite_images(texture_id);
  if (kite_image == NULL) {
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
