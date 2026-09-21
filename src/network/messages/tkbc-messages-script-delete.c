#include "../../../external/lexer/tkbc-lexer.h"
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
 * @param lexer The lexer positioned at the message content.
 * @param client The client that sent the message. On the server it is used
 * to exclude the originator from the broadcast. On the client it is unused.
 * @return True if the message was parsed successfully, otherwise false.
 */
bool tkbc_messages_script_delete(Env *env, Lexer *lexer, Client *client) {
    Token token;
    token = lexer_next(lexer);
    if (token.kind != STRINGLITERAL) {
        return false;
    }

    // To strip the quotes manipulate the token directly
    if (token.size <= 2) {
        return false;
    }
    token.size -= 2;
    token.content += 1;
    UUID script_id;
    if (!tkbc_uuid_from_string(lexer_token_to_cstr(lexer, &token), &script_id)) {
        return false;
    }
    token = lexer_next(lexer);
    if (token.kind != PUNCT_COLON) {
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
        char script_id_cstr[37];
        tkbc_uuid_to_string(script_id, script_id_cstr);
        Message message = {0};
        space_dapf(space_get_tspace(), &message, "%d:\"%s\":\r\n", MESSAGE_SCRIPT_DELETE, script_id_cstr);
        // The receiving (originating) client already deleted locally, so it
        // is excluded from the broadcast.
        tkbc_write_to_all_send_msg_buffers_except(message, client->socket_id);
        space_reset_tspace();
    }
#else
    (void) client;
#endif

    tkbc_fprintf(stderr, "MESSAGEHANDLER", "SCRIPT_DELETE\n");
    return true;
}
