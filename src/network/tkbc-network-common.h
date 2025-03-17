#ifndef TKBC_NETWORK_COMMON_H
#define TKBC_NETWORK_COMMON_H

#include "../../external/lexer/tkbc-lexer.h"
#include "tkbc-servers-common.h"

#include <raylib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>


int tkbc_parse_single_kite_value(Lexer *lexer, ssize_t kite_id);
bool tkbc_parse_message_kite_value(Lexer *lexer, size_t *kite_id, float *x,
                                   float *y, float *angle, Color *color);
char *tkbc_find_rn_in_message_from_position(Message *message,
                                            unsigned long long position);

#endif // TKBC_NETWORK_COMMON_H
