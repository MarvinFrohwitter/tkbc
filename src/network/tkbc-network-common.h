#ifndef TKBC_NETWORK_COMMON_H
#define TKBC_NETWORK_COMMON_H

#include "../../external/lexer/tkbc-lexer.h"
#include "../../external/space/space.h"
#include "tkbc-servers-common.h"

#include <raylib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

void tkbc_reset_space_and_null_message(Space *space, Message *message);

void tkbc_assign_values_to_kitestate(Kite_State *state, float x, float y,
                                     float angle, Color color,
                                     size_t texture_id, bool is_reversed,
                                     bool is_active);
int tkbc_parse_single_kite_value(Lexer *lexer, ssize_t kite_id);
bool tkbc_parse_message_kite_value(Lexer *lexer, size_t *kite_id, float *x,
                                   float *y, float *angle, Color *color,
                                   size_t *texture_id, bool *is_reversed,
                                   bool *is_active);
char *tkbc_find_rn_in_message_from_position(Message *message, size_t position);

bool tkbc_error_handling_of_received_message_handler(Message *message,
                                                     Lexer *lexer, bool *reset,
                                                     bool display_errors);

#endif // TKBC_NETWORK_COMMON_H
