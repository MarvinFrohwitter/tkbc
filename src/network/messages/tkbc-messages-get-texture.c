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
 * @param reader The Message that is scoped to the payload of one received
 * message.
 * @param client The client that the texture data gets send to.
 * @return Returns true if the texture was found and send, otherwise false.
 */
bool tkbc_messages_get_texture(Message *reader, Client *client) {
    UUID texture_id;
    if (!tkbc_message_read_uuid(reader, &texture_id)) {
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

    size_t offset =
        tkbc_message_write_begin(&client->send_msg_buffer, &client->send_msg_buffer_space, MESSAGE_SEND_TEXTURE);
    tkbc_message_append_image_data(&client->send_msg_buffer_space, &client->send_msg_buffer, kite_image->normal,
                                   asset->id);
    tkbc_message_write_end(&client->send_msg_buffer, offset);
    return true;
}
