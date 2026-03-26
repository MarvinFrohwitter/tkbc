#include "../../../external/lexer/tkbc-lexer.h"
#include "../../../external/space/space.h"
#include "../../choreographer/tkbc-script-handler.h"
#include "../../global/tkbc-types.h"
#include "../tkbc-servers-common.h"
#include "tkbc-messages.h"

#include <stdbool.h>
extern Kite_Images kite_images;

bool tkbc_messages_get_texture_id(Lexer *lexer, Client *client) {
  // The client can request a texture id for a kite;
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

  Kite *kite = tkbc_get_kite_by_id(env, kite_id);
  if (!kite) {
    return false;
  }
  assert(kite->texture_id != -1);
  space_dapf(&client->send_msg_buffer_space, &client->send_msg_buffer,
             "%d:%zu:%zu:\r\n", MESSAGE_SEND_TEXTURE_ID, kite_id,
             kite->texture_id);
  return true;
}
