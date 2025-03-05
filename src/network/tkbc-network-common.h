#ifndef TKBC_NETWORK_COMMON_H
#define TKBC_NETWORK_COMMON_H

#include "../../external/lexer/tkbc-lexer.h"
#include "tkbc-servers-common.h"

#include <raylib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>


bool tkbc_message_append_clientkite(size_t client_id, Message *message);
bool tkbc_message_append_clientkite(size_t client_id, Message *message);
bool tkbc_parse_single_kite_value(Lexer *lexer);
bool tkbc_parse_message_kite_value(Lexer *lexer, size_t *kite_id, float *x,
                                   float *y, float *angle, Color *color);

#endif // TKBC_NETWORK_COMMON_H
