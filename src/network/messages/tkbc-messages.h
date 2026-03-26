#ifndef TKBC_MESSAGES_H
#define TKBC_MESSAGES_H

#include "../../../external/lexer/tkbc-lexer.h"
#include "../tkbc-servers-common.h"

#include <stdbool.h>

bool tkbc_messages_get_texture(Lexer *lexer, Client *client);

#endif // TKBC_MESSAGES_H
