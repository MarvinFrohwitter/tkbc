#include "../../../external/lexer/tkbc-lexer.h"
#include "../../../external/space/space.h"
#include "../../choreographer/tkbc-asset-handler.h"
#include "../../global/tkbc-types.h"
#include "../tkbc-network-common.h"
#include "../tkbc-servers-common.h"
#include "tkbc-messages.h"

#include <stdbool.h>
extern Kite_Images kite_images;

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

  Kite_Image *kite_image = tkbc_find_asset_in_kite_images(texture_id);
  if (!kite_image) {
    tkbc_append_kite_image(data, width, height, format);
#ifndef TKBC_SERVER
    Kite_Image kite_image = kite_images.elements[kite_images.count - 1];
    tkbc_append_kite_texture(kite_image);
#endif
  }
  space_reset_tspace();
  return true;
}
