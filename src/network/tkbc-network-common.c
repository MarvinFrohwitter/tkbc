#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint16_t tkbc_port_parsing(const char *port_check) {

  for (size_t i = 0; i < strlen(port_check); ++i) {
    if (!isdigit(port_check[i])) {
      fprintf(stderr, "ERROR: The given port [%s] is not valid.\n", port_check);
      exit(1);
    }
  }
  int port = atoi(port_check);
  if (port >= 65535 || port <= 0) {
    fprintf(stderr, "ERROR: The given port [%s] is not valid.\n", port_check);
    exit(1);
  }

  return (uint16_t)port;
}
