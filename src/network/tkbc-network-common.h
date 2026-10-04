#ifndef TKBC_NETWORK_COMMON_H
#define TKBC_NETWORK_COMMON_H

#include "../../external/space/space.h"
#include "messages/tkbc-binary-protocol.h"
#include "tkbc-servers-common.h"

#include "raylib.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

void tkbc_reset_space_and_null_message(Space *space, Message *message);

void tkbc_assign_values_to_kitestate(Kite_State *state, float x, float y, float angle, Color color, UUID texture_id,
                                     bool is_reversed, bool is_active, bool is_script_kite);

int tkbc_parse_single_kite_value(Message *reader, ssize_t kite_id, size_t *parsed_id);

bool tkbc_message_script(Client *client, bool overwrite_was_send);
bool tkbc_message_append_script(Space *space, Message *message, UUID script_id);

bool tkbc_parse_image(Message *reader, Space *data_space, unsigned char **data, int *width, int *height, int *format,
                      UUID *texture_id);
bool tkbc_parse_message_kite_value(Message *reader, size_t *kite_id, float *x, float *y, float *angle, Color *color,
                                   UUID *texture_id, int *texture_width, int *texture_height, int *texture_format,
                                   Space *data_space, unsigned char **texture_data, UUID *inline_texture_id,
                                   bool *is_reversed, bool *is_active, bool *is_script_kite);

#endif  // TKBC_NETWORK_COMMON_H
