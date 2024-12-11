#ifndef TKBC_CLIENT_H
#define TKBC_CLIENT_H

#include "tkbc-network-common.h"
#include <stdint.h>

void tkbc_client_usage(const char *program_name);
void tkbc_client_commandline_check(int argc, const char *program_name);
const char *tkbc_host_parsing(const char *host_check);
int tkbc_client_socket_creation(const char *addr, uint16_t port);

void tkbc_register_kite_from_values(size_t kite_id, float x, float y,
                                    float angle, Color color);
void sending_script_handler();
void send_message_handler();
bool received_message_handler(bool first);
void *message_recieving(void *client);
void tkbc_client_input_handler_kite();

#endif // TKBC_CLIENT_H
