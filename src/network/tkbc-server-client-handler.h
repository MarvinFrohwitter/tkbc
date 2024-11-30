#ifndef TKBC_SERVER_CLIENT_HANDLER_H
#define TKBC_SERVER_CLIENT_HANDLER_H

#include "../global/tkbc-types.h"
#include "tkbc-network-common.h"

void signal_pipe(int signal);
void signal_int(int signal);
void *tkbc_client_handler(void *client);
void tkbc_server_brodcast_all(const char *message);
void tkbc_server_brodcast_client(Client *client, const char *message);

void tkbc_message_hello(Client *client);
void tkbc_message_kiteadd(size_t client_index, Kite_State *state);
bool tkbc_server_remove_client(Client *client);

#endif // !TKBC_SERVER_CLIENT_HANDLER_H
