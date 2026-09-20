#include "../../../external/lexer/tkbc-lexer.h"
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
 * @param lexer The lexer positioned at the message content.
 * @return True if the script was loaded successfully, otherwise false.
 */
bool tkbc_messages_script_next(Lexer *lexer) {
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
    bool ok = tkbc_uuid_from_string(lexer_token_to_cstr(lexer, &token), &script_id);
    if (!ok) {
        return false;
    }
    token = lexer_next(lexer);
    if (token.kind != PUNCT_COLON) {
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
    //
    // NOTE: Partial played scripts should not save load there old state, because
    // more than one player could interact with the same script controlling and it
    // could get very wired.
    //
    // This is not a good behavior for multiple clients.
    // tkbc_load_script_id(env, script_id, false);
    //
    // When adding potantiol buttons to the UI revisit and reevaluate the
    // decision.
    //
    // Marvin Frohwitter 13 April 2026

    if (!tkbc_load_script_id(env, script_id, true)) {
        tkbc_fprintf(stderr, "WARNING", "Could not load script: %d not found!", script_id);
    }

    // This parsing function is just used in the server but liked in the client
    // as well so just a simple guard for compilation.
#ifdef TKBC_SERVER
    tkbc_message_clientkites_write_to_all_send_msg_buffers(true);
#endif

    return true;
}
