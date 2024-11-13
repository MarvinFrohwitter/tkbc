#ifndef TKBC_CLIENT_H
#define TKBC_CLIENT_H

#include "tkbc-network-common.h"
#include <stdint.h>

void tkbc_client_usage(const char *program_name);
void tkbc_client_commandline_check(int argc, const char *program_name);
const char *tkbc_host_parsing(const char *host_check);
int tkbc_client_socket_creation(const char *addr, uint16_t port);

void message_handler(void);
void *message_recieving(void *client);

#endif // TKBC_CLIENT_H
