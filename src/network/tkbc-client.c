#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "tkbc-client.h"
#include "tkbc-network-common.h"

#define TKBC_UTILS_IMPLEMENTATION
#include "../tkbc-utils.h"

void tkbc_client_usage(const char *program_name) {
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "       %s <HOST> <PORT> \n", program_name);
}

void tkbc_client_commandline_check(int argc, const char *program_name) {
  if (argc > 2) {
    fprintf(stderr, "ERROR: To may arguments.\n");
    tkbc_client_usage(program_name);
    exit(1);
  }
  if (argc == 0) {
    fprintf(stderr, "ERROR: No arguments were provided.\n");
    tkbc_client_usage(program_name);
    exit(1);
  }
}

const char *tkbc_host_parsing(const char *host_check) {

  const char *host = "127.0.0.1";
  if (strcmp(host_check, "localhost") == 0) {
    return host;
  }

  for (size_t i = 0; i < strlen(host_check); ++i) {
    if (!isdigit(host_check[i]) && host_check[i] != '.') {
      fprintf(stderr, "ERROR: The given host [%s] is not supported.\n",
              host_check);
      exit(1);
    }
  }

  // TODO: Check for DNS-resolution.
  host = host_check;
  return host;
}

int tkbc_client_socket_creation(const char *addr, uint16_t port) {
  int client_socket = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port);
  server_address.sin_addr.s_addr = inet_addr(addr);

  int connection_status =
      connect(client_socket, (struct sockaddr *)&server_address,
              sizeof(server_address));

  if (connection_status == -1) {
    fprintf(stderr, "Error: %s\n", strerror(errno));
    exit(1);
  }

  // TODO: Think about the decoding when ip and port are available through
  // arguments.
  printf("Connected to Server: %s:%hd\n", inet_ntoa(server_address.sin_addr),
         ntohs(server_address.sin_port));

  return client_socket;
}

void *message_recieving(void *client) {
  int client_socket = *((int *)client);

  for (;;) {
    char message[1024];
    memset(message, 0, sizeof(message));
    // The recv call blocks.
    int message_ckeck = recv(client_socket, message, sizeof(message) - 1, 0);

    if (message_ckeck == -1) {
      fprintf(stderr, "Error: %s\n", strerror(errno));
      break;
    }

    assert((unsigned int)message_ckeck < sizeof(message));
    message[message_ckeck] = '\0';
    if (message_ckeck >= 0) {
      printf("%s", message);
    }
  }

  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {

  char *program_name = tkbc_shift_args(&argc, &argv);
  tkbc_client_commandline_check(argc, program_name);

  const char *host_check = tkbc_shift_args(&argc, &argv);
  const char *host = tkbc_host_parsing(host_check);

  char *port_check = tkbc_shift_args(&argc, &argv);
  uint16_t port = tkbc_port_parsing(port_check);

  int client_socket = tkbc_client_socket_creation(host, port);

  pthread_t thread;

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&thread, &attr, message_recieving, (void *)&client_socket);
  pthread_attr_destroy(&attr);

  char input[64];
  for (int i = 0;; ++i) {
    int in = scanf("%s\n", input);
    int send_out = send(client_socket, input, sizeof(input), 0);
    if (send_out == -1) {
      printf("ERROR: Nothing is send!\n");
    } else {
      printf("send_out = %d\n", send_out);
    }
  }

  close(client_socket);
  return 0;
}
