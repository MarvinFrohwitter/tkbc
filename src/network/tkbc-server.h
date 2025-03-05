#ifndef TKBC_SERVER_H
#define TKBC_SERVER_H

#include "../global/tkbc-types.h"
#include "tkbc-network-common.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WINUSER_
#define _WINGDI_
#define _IMM_
#define _WINCON_
#include <windows.h>
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

#include <stdint.h>

void signalhandler(int signal);
int tkbc_server_socket_creation(uint32_t addr, uint16_t port);
Clients *tkbc_init_clients(void);
void *tkbc_script_execution_handler();

#endif // TKBC_SERVER_H
