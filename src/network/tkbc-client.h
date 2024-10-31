#ifndef TKBC_CLIENT_H
#define TKBC_CLIENT_H

#include <stdint.h>

void tkbc_client_usage(const char *program_name);
void tkbc_client_commandline_check(int argc, const char *program_name);
const char *tkbc_host_parsing(const char *host_check);

int tkbc_client_socket_creation(const char *addr, uint16_t port);

#endif // TKBC_CLIENT_H
