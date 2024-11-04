#ifndef TKBC_SERVER_CLIENT_HANDLER_H
#define TKBC_SERVER_CLIENT_HANDLER_H

#include "../global/tkbc-types.h"
#include "tkbc-network-common.h"

void *tkbc_client_handler(void *client);
void tkbc_server_brodcast_all(const char *message);
void tkbc_server_brodcast_client(Client *client, const char *message);

void tkbc_message_hello(Client *client);
void tkbc_message_kiteadd(size_t client_index, Kite_State *state);

#endif // !TKBC_SERVER_CLIENT_HANDLER_H
