#include "../../../external/lexer/tkbc-lexer.h"
#include "tkbc-messages.h"

#include <stdbool.h>

/**
 * @brief Handles a SCRIPT_AMOUNT message.
 *
 * @param client The client where the amount of scripts should be assigned.
 * @param lexer The lexer positioned at the message content.
 * @return True if the amount was parsed successfully, otherwise false.
 */
bool tkbc_messages_script_amount(Client *client, Lexer *lexer) {
    Token token;
    token = lexer_next(lexer);
    if (token.kind != NUMBER) {
        return false;
    }

    client->script_amount = strtoul(lexer_token_to_cstr(lexer, &token), NULL, 10);
    token = lexer_next(lexer);
    if (token.kind != PUNCT_COLON) {
        return false;
    }

    return true;
}
