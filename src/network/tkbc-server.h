#ifndef TKBC_SERVER_H
#define TKBC_SERVER_H

#include <stdint.h>

void tkbc_server_usage(const char *program_name);
void tkbc_server_commandline_check(int argc, const char *program_name);
int tkbc_server_socket_creation(uint32_t addr, uint16_t port);

#endif // TKBC_SERVER_H
