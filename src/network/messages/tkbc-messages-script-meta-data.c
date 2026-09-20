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
    bool ok = tkbc_uuid_from_string(lexer_token_to_cstr(lexer, &token), &env->server_script_id);
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

    env->server_script_frames_count = strtoul(lexer_token_to_cstr(lexer, &token), NULL, 10);

    token = lexer_next(lexer);
    if (token.kind != PUNCT_COLON) {
        return false;
    }
    token = lexer_next(lexer);
    if (token.kind != NUMBER) {
        return false;
    }

    env->server_script_frames_index = strtoul(lexer_token_to_cstr(lexer, &token), NULL, 10);

    token = lexer_next(lexer);
    if (token.kind != PUNCT_COLON) {
        return false;
    }

    if (tkbc_uuid_is_nil(env->server_script_id)) {
        tkbc_unload_script(env);
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
    }

    return true;
}
