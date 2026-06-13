#include "../../../external/lexer/tkbc-lexer.h"
#include "../../../external/space/space.h"
#include "../../choreographer/tkbc-asset-handler.h"
#include "../../choreographer/tkbc-script-handler.h"
#include "../../global/tkbc-types.h"
#include "../../network/tkbc-network-common.h"
#include "../tkbc-client.h"
#include "../tkbc-servers-common.h"

#include "tkbc-messages.h"

#include <stdbool.h>

bool tkbc_messages_single_kite_add(Env *env, Lexer *lexer, Client *client,
                                   Kite *client_kite) {
  size_t kite_id;
  float x, y, angle;
  Color color;
  bool is_reversed, is_active, is_script_kite;
  ssize_t texture_id;
  size_t texture_width, texture_height, texture_format;
  Space *data_space = space_get_tspace();
  unsigned char *texture_data = NULL;

  if (!tkbc_parse_message_kite_value(
          lexer, &kite_id, &x, &y, &angle, &color, &texture_id, &texture_width,
          &texture_height, &texture_format, data_space, &texture_data,
          &is_reversed, &is_active, &is_script_kite)) {
    space_reset_tspace();
    return false;
  }

  Asset *asset = tkbc_find_asset_from_id(texture_id);
  if (!asset && texture_id != -1) {
    space_dapf(&client->send_msg_buffer_space, &client->send_msg_buffer,
               "%d:%zu:\r\n", MESSAGE_GET_TEXTURE, texture_id);

    // requested texture id
    space_dapf(&client->send_msg_buffer_space, &client->send_msg_buffer,
               "%d:%zu:\r\n", MESSAGE_GET_TEXTURE_ID, kite_id);

    texture_id = KITE_COLORIZER;
  }

  if (texture_id == -1) {
    texture_id = tkbc_append_kite_image_and_kite_texture(
        texture_data, texture_width, texture_height, texture_format);
  }
  space_reset_tspace();

  // This is just for compilation the function is not used in
  // the server at all. Just the files in this dir are all
  // passed to the server compilations as well.
#ifndef TKBC_SERVER
  tkbc_register_kite_from_values(kite_id, x, y, angle, color, texture_id,
                                 is_reversed, is_active, is_script_kite);
#endif

  static _Atomic bool first_message_kite_add = true;
  if (first_message_kite_add) {
    // This assumes the server sends the first SINGLE_KITE_ADD to the
    // client, that contains his own kite;
    if (client->kite_id == -1) {
      client->kite_id = kite_id;
    }

    Kite_State *kite_state = tkbc_get_kite_state_by_id(env, kite_id);
    if (kite_state) {
      kite_state->is_kite_input_handler_active = true;
      *client_kite = *kite_state->kite;
    }
    first_message_kite_add = false;
  }
  return true;
}
