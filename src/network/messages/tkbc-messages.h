#ifndef TKBC_MESSAGES_H
#define TKBC_MESSAGES_H

#include "../../../external/lexer/tkbc-lexer.h"
#include "../tkbc-servers-common.h"

#include <stdbool.h>

bool tkbc_messages_hello_verification(Lexer *lexer, const char *greeting);
bool tkbc_messages_get_texture(Lexer *lexer, Client *client);
bool tkbc_messages_send_texture(Lexer *lexer);
bool tkbc_messages_send_texture_id(Env *env, Lexer *lexer, Client *client);
bool tkbc_messages_get_texture_id(Lexer *lexer, Client *client);
bool tkbc_messages_script_meta_data(Lexer *lexer);

bool tkbc_messages_single_kite_add(Env *env, Lexer *lexer, Client *client,
                                   Kite *client_kite);

bool tkbc_messages_script(Env *env, Lexer *lexer, Client *client,
                          bool *script_alleady_there_parsing_skip);
bool tkbc_messages_script_next(Lexer *lexer);
bool tkbc_messages_script_scrub(Lexer *lexer);

#endif // TKBC_MESSAGES_H
