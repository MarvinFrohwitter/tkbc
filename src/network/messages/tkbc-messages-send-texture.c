#include "../../../external/lexer/tkbc-lexer.h"
#include "../../../external/space/space.h"
#include "../../choreographer/tkbc-asset-handler.h"
#include "../../global/tkbc-types.h"
#include "../tkbc-network-common.h"
#include "../tkbc-servers-common.h"
#include "tkbc-messages.h"

#include <stdbool.h>
extern Assets assets;

/**
 * @brief [TODO:description]
 *
 * @param lexer [TODO:parameter]
 * @return [TODO:return]
 */
bool tkbc_messages_send_texture(Lexer *lexer) {
  size_t width, height, format;
  Space *data_space = space_get_tspace();
  unsigned char *data = NULL;
  size_t texture_id;

  if (!tkbc_parse_image(lexer, data_space, &data, &width, &height, &format,
                        &texture_id)) {
    return false;
  }

  Asset *asset = tkbc_find_asset_from_id(texture_id);
  if (!asset) {
    tkbc_append_kite_image_and_kite_texture(data, width, height, format);
  }
  space_reset_tspace();
  return true;
}
