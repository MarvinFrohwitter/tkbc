#include "../../../external/lexer/tkbc-lexer.h"
#include "../../../external/space/space.h"
#include "../../choreographer/tkbc-script-handler.h"
#include "../../global/tkbc-types.h"
#include "../poll-server.h"
#include "../tkbc-servers-common.h"
#include "tkbc-messages.h"

#include <stdbool.h>
#include <stdlib.h>

/**
 * @brief Handles a SCRIPT_SCRUB message by jumping the server script timeline
 * to an absolute frame index.
 *
 * The client maps its timeline mouse position onto the upscaled per-tick
 * timeline and sends the target frames_index, so scrubbing works
 * continuously even inside the animation between script keyframes.
 *
 * @param lexer The lexer positioned at the message content.
 * @return True if the scrub was executed successfully, otherwise false.
 */
bool tkbc_messages_script_scrub(Lexer *lexer) {
    Token token;
    token = lexer_next(lexer);
    if (token.kind != NUMBER) {
        return false;
    }
    size_t target_index = strtoull(lexer_token_to_cstr(lexer, &token), NULL, 10);
    token = lexer_next(lexer);
    if (token.kind != PUNCT_COLON) {
        return false;
    }

    // No script is available nothing to scrub
    if (env->script == NULL) {
        return true;
    }

    if (env->script->count <= 0) {
        return false;
    }

    tkbc_scrub_to_index(env, target_index);

    // This parsing function is just used in the server but liked in the client as
    // well so just a simple guard for compilation.
#ifdef TKBC_SERVER
    tkbc_message_script_meta_data_write_to_all_send_msg_buffers(env->script->id, env->script->count,
                                                                env->frames->frames_index);
    // The scrub pauses execution (script_finished), so no regular per-tick
    // kite broadcast follows. Push the jumped positions explicitly, otherwise
    // clients would move the slider but keep stale kite positions.
    tkbc_message_clientkites_write_to_all_send_msg_buffers(false);
#endif
    return true;
}
