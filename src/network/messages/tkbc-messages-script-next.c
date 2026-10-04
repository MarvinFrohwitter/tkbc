#include "../../../external/space/space.h"
#include "../../choreographer/tkbc-script-api.h"
#include "../../choreographer/tkbc-script-handler.h"
#include "../../global/tkbc-types.h"
#include "../poll-server.h"
#include "../tkbc-servers-common.h"
#include "tkbc-messages.h"

#include <stdbool.h>

/**
 * @brief Handles a SCRIPT_NEXT message by loading and activating the next
 * script, deactivating non-script kites.
 *
 * @param reader The Message that is scoped to the payload of one received
 * message.
 * @return True if the script was loaded successfully, otherwise false.
 */
bool tkbc_messages_script_next(Message *reader) {
    UUID script_id;
    if (!tkbc_message_read_uuid(reader, &script_id)) {
        return false;
    }

    if (tkbc_uuid_is_nil(script_id)) {
        tkbc_unload_script(env);
        // This parsing function is just used in the server but liked in the client
        // as well so just a simple guard for compilation.
#ifdef TKBC_SERVER
        tkbc_message_script_meta_data_write_to_all_send_msg_buffers(tkbc_uuid_nil(), 0, 0);
#endif

        // Enable the normal client kites.
        tkbc_change_visibility_to_non_script_kites(env);

        // This parsing function is just used in the server but liked in the client
        // as well so just a simple guard for compilation.
#ifdef TKBC_SERVER
        tkbc_message_clientkites_write_to_all_send_msg_buffers(false);
#endif
        return true;
    }

    // TODO: Report possible failures of loading back to the client.

    // NOTE: Partial played scripts should not save load there old state, because
    // more than one player could interact with the same script controlling and it
    // could get very wired.
    //
    // This is not a good behavior for multiple clients.
    // tkbc_load_script_id(env, script_id, false);
    if (!tkbc_load_script_id(env, script_id, true)) {
        char script_id_cstr[37];
        tkbc_uuid_to_string(script_id, script_id_cstr);
        tkbc_fprintf(stderr, "WARNING", "Could not load script: %s not found!\n", script_id_cstr);
    }

    // This parsing function is just used in the server but liked in the client
    // as well so just a simple guard for compilation.
#ifdef TKBC_SERVER
    tkbc_message_clientkites_write_to_all_send_msg_buffers(true);
#endif

    return true;
}
