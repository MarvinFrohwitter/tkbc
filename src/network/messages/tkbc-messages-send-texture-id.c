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
 * @param reader The Message that is scoped to the payload of one received
 * message.
 * @param client The client that sent the message.
 * @return True if the texture id was processed successfully, otherwise false.
 */
bool tkbc_messages_send_texture_id(Env *env, Message *reader, Client *client) {
    uint64_t kite_id;
    if (!tkbc_message_read_u64(reader, &kite_id)) {
        return false;
    }

    // The nil uuid should not be send by the server. The server should always
    // send a valid texture_id.
    UUID texture_id;
    if (!tkbc_message_read_uuid(reader, &texture_id)) {
        return false;
    }
    assert(!tkbc_uuid_is_nil(texture_id));

    Asset *asset = tkbc_find_asset_from_id(texture_id);
    if (asset == NULL) {
        // The message is split to allow getting a texture by its own at some
        // point. Maybe this is never needed, but it can be useful when a client
        // want to get all the available textures in the server.
        {
            size_t offset =
                tkbc_message_write_begin(&client->send_msg_buffer, &client->send_msg_buffer_space, MESSAGE_GET_TEXTURE);
            tkbc_message_write_uuid(&client->send_msg_buffer, &client->send_msg_buffer_space, texture_id);
            tkbc_message_write_end(&client->send_msg_buffer, offset);
        }

        {
            size_t offset = tkbc_message_write_begin(&client->send_msg_buffer, &client->send_msg_buffer_space,
                                                     MESSAGE_GET_TEXTURE_ID);
            tkbc_message_write_u64(&client->send_msg_buffer, &client->send_msg_buffer_space, kite_id);
            tkbc_message_write_end(&client->send_msg_buffer, offset);
        }

    } else {
        // The kite_id should be present in the client, because it requested the
        // texture_id with that kite_id before.
        Kite *kite = tkbc_get_kite_by_id(env, kite_id);
        assert(kite != NULL);
        kite->texture_id = texture_id;
    }
    return true;
}
