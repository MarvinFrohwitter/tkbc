#ifndef TKBC_CLIENT_H
#define TKBC_CLIENT_H

#include "tkbc-network-common.h"
#include <stdint.h>

void tkbc_client_usage(const char *program_name);
bool tkbc_client_commandline_check(int argc, const char *program_name);
int tkbc_client_socket_creation(const char *host, const char *port);

void tkbc_register_kite_from_values(size_t kite_id, float x, float y,
                                    float angle, Color color, size_t texture_id,
                                    bool is_reversed, bool is_active);
void sending_script_handler(void);
bool send_message_handler(void);
bool received_message_handler(Message *message);
bool message_queue_handler(Message *message);
void tkbc_client_input_handler_kite(void);
bool tkbc_message_append_script(size_t script_id);
bool tkbc_message_script(void);
void tkbc_client_file_handler(void);
void tkbc_client_input_handler_script(void);

#endif // TKBC_CLIENT_H
