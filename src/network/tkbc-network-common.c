#include "tkbc-network-common.h"
#include "../choreographer/tkbc-asset-handler.h"
#include "../choreographer/tkbc-script-api.h"
#include "../choreographer/tkbc-script-handler.h"
#include "../choreographer/tkbc.h"
#include "../global/tkbc-utils.h"
#include "tkbc-servers-common.h"

#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern Env *env;
extern Assets assets;

/**
 * @brief The function resets the space and sets the elements ptr from the
 * message to NULL.
 *
 * @param space The arena style allocator.
 * @param message The dynamic arena of a message.
 */
void tkbc_reset_space_and_null_message(Space *space, Message *message) {
    memset(message, 0, sizeof(*message));
    space_reset_space(space);
}

/**
 * @brief The function assigns the given values to the passed state.
 *
 * @param state The kite_state that should be updated.
 * @param x The new x value of the kite center.
 * @param y The new y value of the kite center.
 * @param angle The new angle of the kite.
 * @param color The new color of the kite.
 * @param texture_id The uuid of the asset that should be used to display the
 * kite.
 * @param is_reversed If the kite should fly reverse by default.
 * @param is_active If the kite should be displayed on the screen.
 * @param is_script_kite Indicates if the kite is part of a script.
 */
void tkbc_assign_values_to_kitestate(Kite_State *state, float x, float y, float angle, Color color, UUID texture_id,
                                     bool is_reversed, bool is_active, bool is_script_kite) {
    // There should not be a single missing texture in here.
    // The nil uuid is reserved for the state where the pixel data is still
    // transferred inline, so it must never end up stored on a kite.
    assert(!tkbc_uuid_is_nil(texture_id));

    assert(state);
    state->kite->center.x = x;
    state->kite->center.y = y;
    state->kite->angle = angle;
    state->kite->body_color = color;
    state->kite->texture_id = texture_id;
    state->is_kite_reversed = is_reversed;

    state->is_active = is_active;
    state->is_script_kite = is_script_kite;

    if (!is_active) {
        state->is_kite_input_handler_active = false;
    }

    // This is needed because in the server this step
    // is meaningless. The server don't have to load assets to
    // administrate them the ids of the assets should be enough.
#ifndef TKBC_SERVER
    // NOTE if the new designed texture was not send to the other client the
    // client has a smaller textures.count,

    Asset *asset = tkbc_find_asset_from_id(texture_id);
    assert(asset != NULL);
    assert(asset->type == ASSETS_KITE_DESIGN);
    Kite_Texture *kite_texture = &asset->as.kite_texture;
    assert(kite_texture != NULL);
    tkbc_set_kite_texture(state->kite, kite_texture);
#endif

    tkbc_kite_update_internal(state->kite);
}

/**
 * @brief The function extracts the values that should belong to a kite out of
 * a received message payload.
 *
 * @param reader The Message that is scoped to the payload of one received
 * message.
 * @param id -1 if the parsed kite values should be updated, if the values
 * should not be updated pass the kite_id.
 * @param parsed_id The kite_id that is parsed out.
 * @return 1 if the kite values can be parsed out of the payload and is
 * updated, 2 every thing like 1 but the assigned texture is KITE_COLORIZER
 * because the parsed texture was not available, if the kite values are parsed
 * not updated -1 is returned and 0 is returned if the parsing has failed and no
 * updates were made.
 */
int tkbc_parse_single_kite_value(Message *reader, ssize_t kite_id, size_t *parsed_id) {
    int ok = 1;

    float x, y, angle;
    Color color;
    bool is_reversed, is_active, is_script_kite;

    UUID texture_id;
    UUID inline_texture_id;
    int texture_width, texture_height, texture_format;
    Space *data_space = space_get_tspace();
    unsigned char *texture_data = NULL;

    if (!tkbc_parse_message_kite_value(reader, parsed_id, &x, &y, &angle, &color, &texture_id, &texture_width,
                                       &texture_height, &texture_format, data_space, &texture_data, &inline_texture_id,
                                       &is_reversed, &is_active, &is_script_kite)) {

        check_return(0);
    }

    if (kite_id >= 0) {
        if ((size_t) kite_id == *parsed_id) {
            check_return(-1);
        }
    }

    // Append it under the uuid the originator assigned to it. Registering it
    // under a fresh uuid here would make this side believe the design is
    // unknown and add it a second time.
    if (tkbc_uuid_is_nil(texture_id)) {
        texture_id = tkbc_append_kite_image_and_kite_texture_with_id(texture_data, texture_width, texture_height,
                                                                     texture_format, inline_texture_id);
    }

    Asset *found = tkbc_find_asset_from_id(texture_id);
    if (!found) {
        texture_id = _tkbc_get_asset_kite_design(KITE_COLORIZER).id;
        ok = 2;
    }

    Kite_State *state = tkbc_get_kite_state_by_id(env, *parsed_id);
    // NOTE: This ignores unknown kites and just sets the values for valid ones.
    // Unknown kites are not a parsing error so true is returned.
    // TODO: But for the client not the server the kite missing kite should be
    // handled because the server expects the client to have it so the client
    // lost it or hasn't registered one jet.
    //
    // // TODO: So for the client register the kite like single kite add kite.
    if (state) {
        tkbc_assign_values_to_kitestate(state, x, y, angle, color, texture_id, is_reversed, is_active, is_script_kite);
    }

check:
    space_reset_tspace();
    return ok;
}

/**
 * @brief The function can be used to construct the message script out of the
 * currently registered scripts. The result is directly written to the
 * send_message_queue ready to be send to the server.
 *
 * @param client The client where the message should be appended into the send buffer.
 * @param overwrite_was_send When true ignore the was_send flag in the Script type that indicates if the script was
 * already send once. This can be useful for sending all scripts that are currently registered.
 * @return True if the message script could be constructed, otherwise false.
 */
bool tkbc_message_script(Client *client, bool overwrite_was_send) {
    bool ok = true;

    size_t total_amount_to_send = 0;
    size_t saved_count = client->send_msg_buffer.count;

    if (overwrite_was_send) {
        total_amount_to_send = env->scripts.count;
    } else {
        for (size_t i = 0; i < env->scripts.count; ++i) {
            if (env->scripts.elements[i].was_send) continue;
            total_amount_to_send += 1;
        }
    }

    if (total_amount_to_send == 0) {
        check_return(true);
    }

    {
        size_t payload_offset =
            tkbc_message_write_begin(&client->send_msg_buffer, &client->send_msg_buffer_space, MESSAGE_SCRIPT_AMOUNT);
        tkbc_message_write_u64(&client->send_msg_buffer, &client->send_msg_buffer_space,
                               (uint64_t) total_amount_to_send);
        tkbc_message_write_end(&client->send_msg_buffer, payload_offset);
    }

    for (size_t i = 0; i < env->scripts.count; ++i) {
        if (!overwrite_was_send) {
            if (env->scripts.elements[i].was_send) continue;
        }
        if (!tkbc_message_append_script(&client->send_msg_buffer_space, &client->send_msg_buffer,
                                        env->scripts.elements[i].id)) {
            tkbc_fprintf(stderr, "ERROR", "The script could not be appended to the message.\n");
            check_return(false);
        }

        env->scripts.elements[i].was_send = true;
    }
check:
    if (!ok) {
        // Abort the complete sending of all scripts.
        client->send_msg_buffer.count = saved_count;
    }
    return ok;
}

/**
 * @brief The function appends the script found from the given script_id in
 * the scripts to the given message structure.
 *
 * @param space The arena style allocator.
 * @param message The message structure where the data should be appended in.
 * @param script_id The script number the should be appended.
 * @return True if the script was found and is correctly appended, otherwise
 * false.
 */
bool tkbc_message_append_script(Space *space, Message *message, UUID script_id) {
    size_t payload_offset = tkbc_message_write_begin(message, space, MESSAGE_SCRIPT);

    for (size_t i = 0; i < env->scripts.count; ++i) {
        if (!tkbc_uuid_equals(env->scripts.elements[i].id, script_id)) {
            continue;
        }
        Script *script = &env->scripts.elements[i];

        tkbc_message_write_uuid(message, space, script_id);

        const char *script_name = tkbc_script_name(script);
        tkbc_message_write_c_string(message, space, script_name);

        // The original non-upscaled script: the receiver upscales and
        // bakes locally, so it can still be saved in its original form and
        // the message stays small no matter how many per-tick slices the
        // live timeline holds.

        tkbc_redirect_script_elements(script);
        tkbc_message_write_u64(message, space, (uint64_t) script->original_count);
        for (size_t j = 0; j < script->original_count; ++j) {
            Frames *frames = &script->original_elements[j];
            tkbc_message_write_u64(message, space, (uint64_t) frames->frames_index);
            tkbc_message_write_u64(message, space, (uint64_t) frames->count);

            for (size_t k = 0; k < frames->count; ++k) {
                Frame *frame = &frames->elements[k];
                tkbc_message_write_u64(message, space, (uint64_t) frame->index);
                tkbc_message_write_bool(message, space, frame->finished);
                tkbc_message_write_u8(message, space, (uint8_t) frame->kind);

                static_assert(ACTION_KIND_COUNT == 9, "NOT ALL THE Action_Kinds ARE IMPLEMENTED");
                switch (frame->kind) {
                case ACTION_KITE_QUIT:
                case ACTION_KITE_WAIT: {
                } break;
                case ACTION_KITE_MOVE:
                case ACTION_KITE_MOVE_ADD: {
                    Move_Action action = frame->action.as_move;
                    tkbc_message_write_f32(message, space, action.position.x);
                    tkbc_message_write_f32(message, space, action.position.y);
                } break;
                case ACTION_KITE_ROTATION:
                case ACTION_KITE_ROTATION_ADD: {
                    Rotation_Action action = frame->action.as_rotation;
                    tkbc_message_write_f32(message, space, action.angle);
                } break;
                case ACTION_KITE_TIP_ROTATION:
                case ACTION_KITE_TIP_ROTATION_ADD: {
                    Tip_Rotation_Action action = frame->action.as_tip_rotation;
                    tkbc_message_write_u8(message, space, (uint8_t) action.tip);
                    tkbc_message_write_f32(message, space, action.angle);
                } break;
                default: assert(0 && "UNREACHABLE tkbc_message_append_script()");
                }

                tkbc_message_write_f32(message, space, frame->duration);

                // The kite ids count is always written, even when it is 0, so
                // the receiver never has to guess whether the list is there.
                Kite_Ids *kite_ids = &frame->kite_id_array;
                tkbc_message_write_u64(message, space, (uint64_t) kite_ids->count);
                for (size_t id = 0; id < kite_ids->count; ++id) {
                    tkbc_message_write_u64(message, space, (uint64_t) kite_ids->elements[id]);
                }
            }
        }

        tkbc_remove_redirect_script_elements(script);

        tkbc_message_write_end(message, payload_offset);
        return true;
    }

    // Rewind the partially written message so the caller can abort cleanly.
    message->count = payload_offset;
    return false;
}

/**
 * @brief The function parses an image block out of a message payload. It
 * extracts the uuid of the design, width, height, format and the raw pixel
 * data.
 *
 * @param reader The Message that is scoped to the payload of one received
 * message.
 * @param data_space The space for allocating image data.
 * @param data Pointer to store the parsed image data.
 * @param width Pointer to store the image width.
 * @param height Pointer to store the image height.
 * @param format Pointer to store the pixel format.
 * @param texture_id Pointer to store the uuid the originator assigned to the
 * design.
 * @return True if the image was parsed successfully, otherwise false.
 */
bool tkbc_parse_image(Message *reader, Space *data_space, unsigned char **data, int *width, int *height, int *format,
                      UUID *texture_id) {
    if (!tkbc_message_read_uuid(reader, texture_id)) {
        return false;
    }

    if (!tkbc_message_read_s32(reader, width)) {
        return false;
    }
    if (!tkbc_message_read_s32(reader, height)) {
        return false;
    }
    if (!tkbc_message_read_s32(reader, format)) {
        return false;
    }

    if (*width * *height > (300 * 300) * 16) {
        // Prevent to much data.
        // Just for safety.
        return false;
    }
    if (*format != PIXELFORMAT_UNCOMPRESSED_R8G8B8A8) {
        // Other file format are not supported.
        return false;
    }

    size_t pixel_bytes = *width * *height * sizeof(uint32_t);
    *data = space_malloc(data_space, pixel_bytes * sizeof(**data));
    if (!*data) {
        return false;
    }
    return tkbc_message_read_bytes(reader, *data, pixel_bytes);
}

/**
 * @brief The function parses all values of a single kite value block out of a
 * message payload.
 *
 * @param reader The Message that is scoped to the payload of one received
 * message.
 * @param kite_id The id the corresponding parsed value is assigned to.
 * @param x The x position the corresponding parsed value is assigned to.
 * @param y The y position the corresponding parsed value is assigned to.
 * @param angle The angle the corresponding parsed value is assigned to.
 * @param color The color the corresponding parsed value is assigned to.
 * @param texture_id The uuid that identifies the texture asset in the global
 * assets. The nil uuid means the pixel data is transferred inline and still
 * has to be appended by the caller via
 * tkbc_append_kite_image_and_kite_texture_with_id.
 * @param texture_width The width of the texture.
 * @param texture_height The height of the texture.
 * @param texture_format The format of the texture.
 * @param data_space The space for allocating texture data.
 * @param texture_data Pointer to store the texture data.
 * @param inline_texture_id Pointer to store the uuid that the originator
 * assigned to the inlined image. It is only meaningful when texture_id came
 * back as the nil uuid, so the caller can store the image under that very uuid
 * instead of registering it as a new asset.
 * @param is_reversed If the kite should fly reverse by default.
 * @param is_active If the kite should be displayed on the screen.
 * @param is_script_kite If the kite is part of a script.
 * @return True if all values have been parsed correctly and are assigned,
 * otherwise false.
 */
bool tkbc_parse_message_kite_value(Message *reader, size_t *kite_id, float *x, float *y, float *angle, Color *color,
                                   UUID *texture_id, int *texture_width, int *texture_height, int *texture_format,
                                   Space *data_space, unsigned char **texture_data, UUID *inline_texture_id,
                                   bool *is_reversed, bool *is_active, bool *is_script_kite) {
    uint64_t kid;
    if (!tkbc_message_read_u64(reader, &kid)) {
        return false;
    }
    *kite_id = kid;

    if (!tkbc_message_read_f32(reader, x)) {
        return false;
    }
    if (!tkbc_message_read_f32(reader, y)) {
        return false;
    }
    if (!tkbc_message_read_f32(reader, angle)) {
        return false;
    }

    uint32_t color_number;
    if (!tkbc_message_read_u32(reader, &color_number)) {
        return false;
    }
    *color = tkbc_uint32_t_to_color(color_number);

    if (!tkbc_message_read_uuid(reader, texture_id)) {
        return false;
    }

    if (tkbc_uuid_is_nil(*texture_id)) {
        // The inlined image block carries the uuid of the originator. It is
        // handed back separately so the caller can store the image under that
        // very uuid instead of appending it as a brand new asset.
        if (!tkbc_parse_image(reader, data_space, texture_data, texture_width, texture_height, texture_format,
                              inline_texture_id)) {
            return false;
        }
    } else {
        *inline_texture_id = tkbc_uuid_nil();
    }

    if (!tkbc_message_read_bool(reader, is_reversed)) {
        return false;
    }
    if (!tkbc_message_read_bool(reader, is_active)) {
        return false;
    }
    return tkbc_message_read_bool(reader, is_script_kite);
}
