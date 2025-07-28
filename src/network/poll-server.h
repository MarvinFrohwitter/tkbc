#ifndef TKBC_POLL_SERVER_H
#define TKBC_POLL_SERVER_H
#include "tkbc-servers-common.h"
#include <stddef.h>

typedef struct {
  struct pollfd *elements;
  size_t count;
  size_t capacity;
} FDs;

struct pollfd *tkbc_get_pollfd_by_fd(int fd);
Client *tkbc_get_client_by_fd(int fd);
void tkbc_write_to_send_msg_buffer(Client *client, Message message);
void tkbc_write_to_all_send_msg_buffers(Message message);
void tkbc_write_to_all_send_msg_buffers_except(Message message, int fd);
bool tkbc_remove_client_by_fd(int fd);
int tkbc_remove_connection(Client client, bool retry);
void tkbc_remove_connection_retry(Client client);
void tkbc_server_shutdown_client(Client client, bool force);
void tkbc_message_hello_write_to_send_msg_buffer(Client *client);
bool tkbc_message_kiteadd_write_to_all_send_msg_buffers(size_t client_index);
void tkbc_message_clientkites_write_to_send_msg_buffer(Client *client);
void tkbc_client_prelog(Client *client);
void tkbc_server_accept();
bool tkbc_sockets_read(Client *client);
int tkbc_socket_write(Client *client);
bool tkbc_server_handle_client(Client *client);
void tkbc_socket_handling();

void tkbc_message_clientkites(Message *message);
void tkbc_message_clientkites_write_to_all_send_msg_buffers();
void tkbc_message_srcipt_meta_data_write_to_all_send_msg_buffers(
    size_t script_id, size_t script_count, size_t frames_index);
bool tkbc_message_kite_value_write_to_all_send_msg_buffers_except(
    size_t client_id);
void tkbc_message_kites_write_to_all_send_msg_buffers();
bool tkbc_received_message_handler(Client *client);
void signalhandler(int signal);
#endif // TKBC_POLL_SERVER_H
