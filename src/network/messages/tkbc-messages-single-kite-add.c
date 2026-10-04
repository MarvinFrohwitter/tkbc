#include "../../../external/space/space.h"
#include "../../choreographer/tkbc-asset-handler.h"
#include "../../choreographer/tkbc-script-handler.h"
#include "../../global/tkbc-types.h"
#include "../../network/tkbc-network-common.h"
#include "../tkbc-client.h"
#include "../tkbc-servers-common.h"

#include "tkbc-messages.h"

#include <stdbool.h>

/**
 * @brief Handles a SINGLE_KITE_ADD message by registering a new kite from
 * parsed values, associating it with the client if it is the first kite.
 *
 * @param env The global state of the application.
 * @param reader The Message that is scoped to the payload of one received
 * message.
 * @param client The client that sent the message and is possibly requested
 * for texture data.
 * @param client_kite Output parameter set to the client's kite on first add.
 * @return True if the kite was registered successfully, otherwise false.
 */
bool tkbc_messages_single_kite_add(Env *env, Message *reader, Client *client, Kite *client_kite) {
    size_t kite_id;
    float x, y, angle;
    Color color;
    bool is_reversed, is_active, is_script_kite;
    UUID texture_id;
    UUID inline_texture_id;
    int texture_width, texture_height, texture_format;
    Space *data_space = space_get_tspace();
    unsigned char *texture_data = NULL;

    if (!tkbc_parse_message_kite_value(reader, &kite_id, &x, &y, &angle, &color, &texture_id, &texture_width,
                                       &texture_height, &texture_format, data_space, &texture_data, &inline_texture_id,
                                       &is_reversed, &is_active, &is_script_kite)) {
        space_reset_tspace();
        return false;
    }

    Asset *asset = tkbc_find_asset_from_id(texture_id);
    if (!asset && !tkbc_uuid_is_nil(texture_id)) {
        {
            size_t offset =
                tkbc_message_write_begin(&client->send_msg_buffer, &client->send_msg_buffer_space, MESSAGE_GET_TEXTURE);
            tkbc_message_write_uuid(&client->send_msg_buffer, &client->send_msg_buffer_space, texture_id);
            tkbc_message_write_end(&client->send_msg_buffer, offset);
        }

        {
            // requested texture id
            size_t offset = tkbc_message_write_begin(&client->send_msg_buffer, &client->send_msg_buffer_space,
                                                     MESSAGE_GET_TEXTURE_ID);
            tkbc_message_write_u64(&client->send_msg_buffer, &client->send_msg_buffer_space, kite_id);
            tkbc_message_write_end(&client->send_msg_buffer, offset);
        }

        texture_id = _tkbc_get_asset_kite_design(KITE_COLORIZER).id;
    }

    if (tkbc_uuid_is_nil(texture_id)) {
        texture_id = tkbc_append_kite_image_and_kite_texture_with_id(texture_data, texture_width, texture_height,
                                                                     texture_format, inline_texture_id);
    }
    space_reset_tspace();

    // This is just for compilation the function is not used in
    // the server at all. Just the files in this dir are all
    // passed to the server compilations as well.
#ifndef TKBC_SERVER
    tkbc_register_kite_from_values(kite_id, x, y, angle, color, texture_id, is_reversed, is_active, is_script_kite);
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
