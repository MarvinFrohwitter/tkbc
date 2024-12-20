#ifndef TKBC_SERVER_CLIENT_HANDLER_H
#define TKBC_SERVER_CLIENT_HANDLER_H

#include "../global/tkbc-types.h"
#include "tkbc-network-common.h"

void *tkbc_client_handler(void *client);
bool tkbc_server_received_message_handler(Message receive_message_queue);

bool tkbc_server_brodcast_client(Client client, const char *message);
bool tkbc_server_brodcast_all_exept(Clients *cs, size_t client_id,
                                    const char *message);
bool tkbc_server_brodcast_all(Clients *cs, const char *message);

bool tkbc_message_hello(Client client);
bool tkbc_message_kiteadd(Clients *cs, size_t client_index);
bool tkbc_message_srcipt_block_frames_value();
bool tkbc_message_kite_value(size_t client_id);
bool tkbc_message_clientkites(Client client);
bool tkbc_message_clientkites_brodcast_all(Clients *cs);
bool tkbc_message_kites_brodcast_all(Clients *cs);
void tkbc_message_append_kite(Kite_State *kite_state, Message *message);
bool tkbc_server_remove_client_from_list(Client client);
void tkbc_server_shutdown_client(Client client);
bool tkbc_single_kitevalue(Lexer *lexer, size_t *kite_id);

#endif // !TKBC_SERVER_CLIENT_HANDLER_H
