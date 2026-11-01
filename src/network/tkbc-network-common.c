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
    tkbc_assign_values_to_kitestate_from_source(state, x, y, angle, color, texture_id, is_reversed, is_active,
                                                is_script_kite, -1, 0);
}

/**
 * @brief The function assigns the given values to the passed state and routes
 * direct remote edits of script kites into ghost kites.
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
 * @param source_kite_id The kite id of the remote client that produced the
 * update, or -1 if the update has no remote source.
 * @param controller_count The amount of clients that control the script kite
 * at the time the update was produced. A value of 2 or more turns a direct
 * remote edit into a ghost, because the original kite then belongs to another
 * local controller.
 */
void tkbc_assign_values_to_kitestate_from_source(Kite_State *state, float x, float y, float angle, Color color,
                                                 UUID texture_id, bool is_reversed, bool is_active, bool is_script_kite,
                                                 ssize_t source_kite_id, size_t controller_count) {
    // There should not be a single missing texture in here.
    // The nil uuid is reserved for the state where the pixel data is still
    // transferred inline, so it must never end up stored on a kite.
    assert(!tkbc_uuid_is_nil(texture_id));

    assert(state);

    // A direct remote edit of a script kite with more than one controller (a
    // SINGLE_KITE_UPDATE with a source client) never moves the local original
    // kite. The original is the kite this client displays and controls itself,
    // while the remote edit is shown as a ghost on every client except the one
    // that produced it (the server never echoes an update back to its sender).
    // This way every controlling client keeps its own original and sees all
    // other controlling clients as ghosts.
    //
    // With only one controller no ghost exists: every client applies the
    // update to the original kite as before. Free-fly kites are not shared
    // between clients either, so those updates also keep moving the original.
    //
    // A per-tick CLIENTKITES snapshot has no source client and keeps driving
    // the original kite, but it must never steal the local authority of a kite
    // this client controls. It does not create or clear ghosts, so a ghost
    // stays at the remote client's last position until it is explicitly
    // cleared (remote deselect, remote disconnect or script change/unload).
    //
    // A remote deactivation is still applied, which also releases the local
    // control below and removes the stale ghosts of that kite.
    //
    // Marvin Frohwitter 30.10.2026
    //
    bool has_local_authority = is_script_kite && state->is_script_kite && state->is_kite_input_handler_active;
    if (is_script_kite && source_kite_id >= 0 && controller_count >= 2) {
        if (is_active) {
            Kite_State *ghost = tkbc_update_or_create_ghost_kite(env, state, (int64_t) source_kite_id, x, y, angle);
            if (ghost) {
                // Mirror the appearance of the remote edit as well.
                ghost->kite->body_color = color;
                ghost->kite->texture_id = texture_id;
                ghost->is_kite_reversed = is_reversed;
            }
        }
        // The original stays at the position this client knows. The remote
        // edit is represented by the ghost above.
        x = state->kite->center.x;
        y = state->kite->center.y;
        angle = state->kite->angle;
    } else if (has_local_authority) {
        // A snapshot or a stale direct update must not overwrite a kite this
        // client currently controls locally.
        x = state->kite->center.x;
        y = state->kite->center.y;
        angle = state->kite->angle;
    }

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
        tkbc_clear_ghost_kites_for_target(env, state->kite_id);
    }

    tkbc_kite_update_internal(state->kite);
}

/**
 * @brief The function extracts the values that should belong to a kite out of
 * a received message payload.
 *
 * @param reader The Message that is scoped to the payload of one received
 * message.
 * @param kite_id -1 if the parsed kite values should be updated, if the values
 * should not be updated pass the kite_id.
 * @param parsed_id The kite_id that is parsed out.
 * @param source_kite_id The kite id of the remote client that produced the
 * update, or -1 if the payload did not carry one.
 * @param controller_count The amount of clients that controlled the kite when
 * the update was produced.
 * @return 1 if the kite values can be parsed out of the payload and is
 * updated, 2 every thing like 1 but the assigned texture is KITE_COLORIZER
 * because the parsed texture was not available, if the kite values are parsed
 * not updated -3 is returned,-2 is returned if a new kite is created with it's texture set to KITE_COLORIZER, -1 is
 * returned when a new kite is created out of the parsed_id with it's texture set to KITE_COLORIZER and 0 is returned if
 * the parsing has failed and no updates were made.
 */
int tkbc_parse_single_kite_value(Message *reader, ssize_t kite_id, size_t *parsed_id, ssize_t *source_kite_id,
                                 size_t *controller_count) {
    int ok = 1;

    float x, y, angle;
    Color color;
    bool is_reversed, is_active, is_script_kite;

    UUID texture_id;
    UUID inline_texture_id;
    int texture_width, texture_height, texture_format;
    Space *data_space = space_get_tspace();
    unsigned char *texture_data = NULL;
    uint64_t parsed_source_kite_id = 0;
    uint64_t parsed_controller_count = 0;

    if (!tkbc_parse_message_kite_value(reader, parsed_id, &x, &y, &angle, &color, &texture_id, &texture_width,
                                       &texture_height, &texture_format, data_space, &texture_data, &inline_texture_id,
                                       &is_reversed, &is_active, &is_script_kite)) {

        check_return(0);
    }

    if (!tkbc_message_read_u64(reader, &parsed_source_kite_id)) {
        check_return(0);
    }

    if (!tkbc_message_read_u64(reader, &parsed_controller_count)) {
        check_return(0);
    }

    if (source_kite_id) {
        *source_kite_id = (ssize_t) parsed_source_kite_id;
    }
    if (controller_count) {
        *controller_count = (size_t) parsed_controller_count;
    }

    if (kite_id >= 0) {
        if ((size_t) kite_id == *parsed_id) {
            check_return(-3);
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
    if (state) {
        tkbc_assign_values_to_kitestate_from_source(state, x, y, angle, color, texture_id, is_reversed, is_active,
                                                    is_script_kite, (ssize_t) parsed_source_kite_id,
                                                    (size_t) parsed_controller_count);
    } else {
        // In the client the missing kite should be handled because the server expects the client to have it so the
        // client lost it or hasn't registered one jet.
        // So for the client a new kite is registered.

        // This should never happen and is just for safety outer calling code relays on that handling.
        Kite_State state = tkbc_init_kite();
        if (kite_id == -1) {
            state.kite_id = env->kite_id_counter++;
            ok = -2;
        } else {
            state.kite_id = *parsed_id;
            ok = -1;
        }
        tkbc_assign_values_to_kitestate(&state, x, y, angle, color, texture_id, is_reversed, is_active, is_script_kite);
        tkbc_dap(&env->kite_array, state);
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

    // Scripts are never part of the handshake protocol. Defer them until the
    // handshake has passed (server: client answered HELLO and HELLO_PASSED
    // was queued; client: HELLO_PASSED was received), without consuming the
    // was_send flags so the send can still happen afterwards.
    if (!client->handshake_passed) {
        return true;
    }

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
