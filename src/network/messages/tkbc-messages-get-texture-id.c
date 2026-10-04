#include "../../../external/space/space.h"
#include "../../choreographer/tkbc-script-handler.h"
#include "../../global/tkbc-types.h"
#include "../tkbc-servers-common.h"
#include "tkbc-messages.h"

#include <stdbool.h>

/**
 * @brief Handles a GET_TEXTURE_ID message from a client by responding with the
 * texture id for a given kite id.
 *
 * @param reader The Message that is scoped to the payload of one received
 * message.
 * @param client The client that sent the request.
 * @return True if the message was parsed and responded to successfully,
 * otherwise false.
 */
bool tkbc_messages_get_texture_id(Message *reader, Client *client) {
    // The client can request a texture id for a kite;
    uint64_t kite_id;
    if (!tkbc_message_read_u64(reader, &kite_id)) {
        return false;
    }

    Kite *kite = tkbc_get_kite_by_id(env, kite_id);
    if (!kite) {
        return false;
    }
    assert(!tkbc_uuid_is_nil(kite->texture_id));

    size_t offset =
        tkbc_message_write_begin(&client->send_msg_buffer, &client->send_msg_buffer_space, MESSAGE_SEND_TEXTURE_ID);
    tkbc_message_write_u64(&client->send_msg_buffer, &client->send_msg_buffer_space, kite_id);
    tkbc_message_write_uuid(&client->send_msg_buffer, &client->send_msg_buffer_space, kite->texture_id);
    tkbc_message_write_end(&client->send_msg_buffer, offset);
    return true;
}
