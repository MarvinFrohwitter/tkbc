#ifndef TKBC_MESSAGES_H
#define TKBC_MESSAGES_H

#include "../../../external/lexer/tkbc-lexer.h"
#include "../tkbc-servers-common.h"

#include <stdbool.h>

bool tkbc_messages_hello_verification(Lexer *lexer, const char *greeting);
bool tkbc_messages_get_texture(Lexer *lexer, Client *client);
bool tkbc_messages_send_texture(Lexer *lexer);

#endif // TKBC_MESSAGES_H
