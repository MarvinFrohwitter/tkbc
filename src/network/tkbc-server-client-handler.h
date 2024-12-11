#ifndef TKBC_SERVER_CLIENT_HANDLER_H
#define TKBC_SERVER_CLIENT_HANDLER_H

#include "../global/tkbc-types.h"
#include "tkbc-network-common.h"

void *tkbc_client_handler(void *client);
bool tkbc_server_received_message_handler(Message receive_message_queue);

bool tkbc_server_brodcast_client(Client client, const char *message);
bool tkbc_server_brodcast_all_exept(size_t client_id, const char *message);
bool tkbc_server_brodcast_all(const char *message);

bool tkbc_message_hello(Client client);
bool tkbc_message_kiteadd(size_t client_index);
bool tkbc_message_kite_value(size_t client_id);
bool tkbc_message_clientkites(Client client) ;
bool tkbc_server_remove_client_from_list(Client client);
void tkbc_server_shutdown_client(Client client);

#endif // !TKBC_SERVER_CLIENT_HANDLER_H
