#ifndef TKBC_MESSAGES_H
#define TKBC_MESSAGES_H

#include "../tkbc-network-common.h"

#include <stdbool.h>

bool tkbc_messages_hello_verification(Message *reader, const char *greeting);
bool tkbc_messages_get_texture(Message *reader, Client *client);
bool tkbc_messages_send_texture(Message *reader);
bool tkbc_messages_send_texture_id(Env *env, Message *reader, Client *client);
bool tkbc_messages_get_texture_id(Message *reader, Client *client);
bool tkbc_messages_script_meta_data(Message *reader);

bool tkbc_messages_single_kite_add(Env *env, Message *reader, Client *client, Kite *client_kite);

bool tkbc_messages_script(Env *env, Message *reader, Client *client, bool *script_alleady_there_parsing_skip);
bool tkbc_messages_script_next(Message *reader);
bool tkbc_messages_script_amount(Client *client, Message *reader);
bool tkbc_messages_script_scrub(Message *reader);
bool tkbc_messages_script_delete(Env *env, Message *reader, Client *client);

#endif  // TKBC_MESSAGES_H