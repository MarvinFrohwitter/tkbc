#include "../../../external/space/space.h"
#include "../../choreographer/tkbc-script-handler.h"
#include "../../global/tkbc-types.h"
#include "../tkbc-servers-common.h"
#include "tkbc-messages.h"

#ifdef TKBC_SERVER
#include "../poll-server.h"
#endif

#include <stdbool.h>

/**
 * @brief Handles a SCRIPT_DELETE message by removing the script with the
 * given id from the known scripts.
 *
 * On the server the deletion is broadcast to all other clients via the same
 * message so that they delete the script from their env.scripts list as
 * well. Scripts are identified via their Script.id (UUID).
 *
 * The deletion is idempotent: an unknown id is not a parsing error, so that
 * the originator (which already deleted locally) and repeated broadcasts do
 * not trigger the error recovery path.
 *
 * @param env The global state of the application.
 * @param reader The Message that is scoped to the payload of one received
 * message.
 * @param client The client that sent the message. On the server it is used
 * to exclude the originator from the broadcast. On the client it is unused.
 * @return True if the message was parsed successfully, otherwise false.
 */
bool tkbc_messages_script_delete(Env *env, Message *reader, Client *client) {
    UUID script_id;
    if (!tkbc_message_read_uuid(reader, &script_id)) {
        return false;
    }

    int ok = tkbc_unload_script_from_memory(env, script_id);
    if (ok == 1) {
        tkbc_fprintf(stderr, "WARNING", "SCRIPT_DELETE: script id not found, nothing to delete.\n");
    }

// This parsing function is used on both sides but the broadcast back to
// the other clients only happens on the server.
#ifdef TKBC_SERVER
    if (client == NULL) {
        return true;
    }
    {
        Message message = {0};
        size_t offset = tkbc_message_write_begin(&message, space_get_tspace(), MESSAGE_SCRIPT_DELETE);
        tkbc_message_write_uuid(&message, space_get_tspace(), script_id);
        tkbc_message_write_end(&message, offset);
        // The receiving (originating) client already deleted locally, so it
        // is excluded from the broadcast.
        tkbc_write_to_all_send_msg_buffers_except(message, client->socket_id);
        space_reset_tspace();

        if (ok == -1) {
            tkbc_message_script_meta_data_write_to_all_send_msg_buffers(tkbc_uuid_nil(), 0, 0);
        }
    }
#else
    (void) client;
#endif

    tkbc_fprintf(stderr, "MESSAGEHANDLER", "SCRIPT_DELETE\n");
    return true;
}
