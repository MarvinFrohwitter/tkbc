#include "../../../external/lexer/tkbc-lexer.h"
#include "../../../external/space/space.h"
#include "../../global/tkbc-types.h"
#include "../tkbc-servers-common.h"
#include "tkbc-messages.h"

#include "../../choreographer/tkbc-script-handler.h"

#include <stdbool.h>
extern Client client;

/**
 * @brief The function parses the script meta data message that contains the
 * script id, the frames count and the frames index and updates the server
 * script values in the environment.
 *
 * @param lexer The lexer that is used to read the message tokens.
 * @return Returns true if the message was parsed successfully, otherwise false.
 */
bool tkbc_messages_script_meta_data(Lexer *lexer) {
    Token token;
    UUID previous_id = env->server_script_id;
    UUID parsed_id;
    size_t parsed_count = 0;
    size_t parsed_index = 0;
    token = lexer_next(lexer);
    if (token.kind != STRINGLITERAL) {
        return false;
    }

    // To strip the quotes manipulate the token directly
    if (token.size <= 2) {
        return false;
    }
    token.content += 1;
    token.size -= 2;
    bool ok = tkbc_uuid_from_string(lexer_token_to_cstr(lexer, &token), &parsed_id);
    if (!ok) {
        return false;
    }

    token = lexer_next(lexer);
    if (token.kind != PUNCT_COLON) {
        return false;
    }
    token = lexer_next(lexer);
    if (token.kind != NUMBER) {
        return false;
    }

    parsed_count = strtoul(lexer_token_to_cstr(lexer, &token), NULL, 10);

    token = lexer_next(lexer);
    if (token.kind != PUNCT_COLON) {
        return false;
    }
    token = lexer_next(lexer);
    if (token.kind != NUMBER) {
        return false;
    }

    parsed_index = strtoul(lexer_token_to_cstr(lexer, &token), NULL, 10);

    token = lexer_next(lexer);
    if (token.kind != PUNCT_COLON) {
        return false;
    }

    // Reset through the shared helper so the local view, the frame cursor
    // and the finished flag are cleared in one place. Afterwards the parsed
    // server tracking is applied on the clean state.
    tkbc_unload_script(env);
    env->server_script_id = parsed_id;
    env->server_script_frames_count = parsed_count;
    env->server_script_frames_index = parsed_index;

    if (tkbc_uuid_is_nil(parsed_id)) {
        for (size_t i = 0; i < env->kite_array.count; ++i) {
            Kite_State *kite_state = &env->kite_array.elements[i];
            if (kite_state->is_script_kite) {
                kite_state->is_active = false;
                kite_state->is_kite_input_handler_active = false;
            } else {
                kite_state->is_active = true;
                kite_state->is_kite_input_handler_active = true;
            }
        }
    } else if (tkbc_uuid_is_nil(previous_id) || !tkbc_uuid_equals(previous_id, parsed_id)) {
        // The server started executing a (new) script: from here visibility
        // is server-driven, the per-tick snapshots show exactly the executing
        // kites. Start blank so stale local script kites (which use different
        // per-side ids) cannot linger next to them.
        for (size_t i = 0; i < env->kite_array.count; ++i) {
            env->kite_array.elements[i].is_active = false;
            env->kite_array.elements[i].is_kite_input_handler_active = false;
        }
    }

    return true;
}
