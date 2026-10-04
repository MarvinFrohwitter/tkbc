#include "../../../external/space/space.h"
#include "../../choreographer/tkbc-script-api.h"
#include "../../choreographer/tkbc-script-handler.h"
#include "../../global/tkbc-types.h"
#include "../poll-server.h"
#include "../tkbc-servers-common.h"
#include "tkbc-messages.h"

#include <stdbool.h>

#ifdef TKBC_SERVER
#include "../tkbc-network-common.h"

/**
 * @brief The function combines the MESSAGE_SCRIPT_AMOUNT and a single
 * MESSAGE_SCRIPT into one message that is send to all clients except the
 * originator.
 *
 * @param space The arena style allocator.
 * @param message The message structure where the data should be appended in.
 * @param script_id The id of the script that should be send.
 * @return True if the message could be constructed, otherwise false.
 */
static bool tkbc_combine_message_script_amount_and_message_script_for_one_id(Space *space, Message *message,
                                                                             UUID script_id) {
    size_t total_amount_to_send = 1;
    size_t amount_offset = tkbc_message_write_begin(message, space, MESSAGE_SCRIPT_AMOUNT);
    tkbc_message_write_u64(message, space, (uint64_t) total_amount_to_send);
    tkbc_message_write_end(message, amount_offset);

    // The script has to be known at this point, otherwise it cannot be written
    // out below. Asserting the search for the id instead of only a non empty
    // scripts array keeps the invariant that used to be checked here intact
    // without relying on the scripts array count.
    assert(tkbc_scripts_contains_id(env->scripts, script_id));

    if (!tkbc_message_append_script(space, message, script_id)) {
        tkbc_fprintf(stderr, "ERROR", "The script could not be appended to the message.\n");
        return false;
    }

    return true;
}
#endif

/**
 * @brief Handles a SCRIPT message by parsing and registering a script from the
 * client.
 *
 * @param env The global state of the application.
 * @param reader The Message that is scoped to the payload of one received
 * message.
 * @param client The client that sent the script.
 * @param script_alleady_there_parsing_skip Set to true if the script was
 * already known and parsing was skipped.
 * @return true If the script was parsed and registered successfully.
 * @return false If parsing failed or the script was already known.
 */
bool tkbc_messages_script(Env *env, Message *reader, Client *client, bool *script_alleady_there_parsing_skip) {
    bool ok = true;
    bool check_first_run = true;
    Space *scb_space = &env->scratch_buf_script.space;
    Script *scb_script = &env->scratch_buf_script;
    Frames *scb_frames = &env->scratch_buf_frames;
    Frame frame = {0};
    Kite_Ids collected_kids = {0};
    Kite_Ids generated_kite_ids = {0};

    space_reset_space(&env->scratch_buf_script.space);
    // Reset the whole struct but keep the space so that its planets remain
    // valid for reuse and can still be freed on shutdown.
    Space saved_space = env->scratch_buf_script.space;
    memset(&env->scratch_buf_script, 0, sizeof(env->scratch_buf_script));
    env->scratch_buf_script.space = saved_space;
    memset(&env->scratch_buf_frames, 0, sizeof(env->scratch_buf_frames));

    if (!tkbc_message_read_uuid(reader, &scb_script->id)) {
        check_return(false);
    }

    //
    // This just fast forward a script that is already known and it reduces
    // the parsing afford.
    if (tkbc_scripts_contains_id(env->scripts, scb_script->id)) {
        *script_alleady_there_parsing_skip = true;
        check_return(false);
    }

    char *script_name = NULL;
    size_t script_name_len = 0;
    if (!tkbc_message_read_c_string(reader, scb_space, &script_name, &script_name_len)) {
        check_return(false);
    }

    if (script_name != NULL && script_name_len > 0) {
        tkbc_text_input_set_text(&scb_script->name_input, scb_space, script_name);
    }

    uint64_t script_count;
    if (!tkbc_message_read_u64(reader, &script_count)) {
        check_return(false);
    }

    for (size_t i = 0; i < script_count; ++i) {
        // No reset because the kite_id_array is by pointer in the elements and
        // they should not change. This dose not reuse the memory of the
        // scb_frames, but that is internal the frames data has to be stored
        // some were and can not be overwritten till the script is added to the
        // env.scripts_space.
        //
        // tkbc_reset_frames_internal_data(scb_frames);
        //
        memset(scb_frames, 0, sizeof(*scb_frames));

        if (!tkbc_message_read_u64(reader, &scb_frames->frames_index)) {
            check_return(false);
        }

        uint64_t frames_count;
        if (!tkbc_message_read_u64(reader, &frames_count)) {
            check_return(false);
        }

        for (size_t j = 0; j < frames_count; ++j) {
            memset(&frame, 0, sizeof(frame));

            if (!tkbc_message_read_u64(reader, &frame.index)) {
                check_return(false);
            }
            if (!tkbc_message_read_bool(reader, &frame.finished)) {
                check_return(false);
            }
            if (!tkbc_message_read_u8(reader, (uint8_t *) &frame.kind)) {
                check_return(false);
            }

            Action action = {0};
            static_assert(ACTION_KIND_COUNT == 9, "NOT ALL THE Action_Kinds ARE IMPLEMENTED");
            switch (frame.kind) {
            case ACTION_KITE_QUIT:
            case ACTION_KITE_WAIT: {
            } break;
            case ACTION_KITE_MOVE:
            case ACTION_KITE_MOVE_ADD: {
                if (!tkbc_message_read_f32(reader, &action.as_move.position.x)) {
                    check_return(false);
                }
                if (!tkbc_message_read_f32(reader, &action.as_move.position.y)) {
                    check_return(false);
                }
            } break;
            case ACTION_KITE_ROTATION:
            case ACTION_KITE_ROTATION_ADD: {
                if (!tkbc_message_read_f32(reader, &action.as_rotation.angle)) {
                    check_return(false);
                }
            } break;
            case ACTION_KITE_TIP_ROTATION:
            case ACTION_KITE_TIP_ROTATION_ADD: {
                if (!tkbc_message_read_u8(reader, (uint8_t *) &action.as_tip_rotation.tip)) {
                    check_return(false);
                }
                if (!tkbc_message_read_f32(reader, &action.as_tip_rotation.angle)) {
                    check_return(false);
                }
            } break;
            default: assert(0 && "UNREACHABLE SCRIPT received_message_handler"); check_return(false);
            }
            frame.action = action;

            if (!tkbc_message_read_f32(reader, &frame.duration)) {
                check_return(false);
            }
            frame.original_duration = frame.duration;

            // The kite ids count is always written for every kind, even when
            // it is 0, so the list never has to be guessed. KITE_WAIT and
            // KITE_QUIT carry no kites, so their count is 0.
            uint64_t kite_ids_count;
            if (!tkbc_message_read_u64(reader, &kite_ids_count)) {
                check_return(false);
            }
            for (uint64_t k = 0; k < kite_ids_count; ++k) {
                uint64_t kite_id;
                if (!tkbc_message_read_u64(reader, &kite_id)) {
                    check_return(false);
                }

                bool contains = false;
                space_dap(scb_space, &frame.kite_id_array, kite_id);
                for (size_t id = 0; id < collected_kids.count; ++id) {
                    if (collected_kids.elements[id] == kite_id) {
                        contains = true;
                        break;
                    }
                }
                if (!contains) {
                    tkbc_dap(&collected_kids, kite_id);
                }
            }

            space_dap(scb_space, scb_frames, frame);
        }

        // No deep_copy because the space allocator holds the memory anyways
        // till the script is appended.
        // Frames frames = tkbc_deep_copy_frames(scb_space, scb_frames);
        //
        Frames frames = *scb_frames;
        space_dap(scb_space, scb_script, frames);
    }

    // Post parsing
    size_t kite_count = collected_kids.count;
    size_t prev_count = env->kite_array.count;
    generated_kite_ids = tkbc_kite_array_generate(env, kite_count);

    // Generated kites stay hidden until the script is loaded.
    for (size_t i = prev_count; i < env->kite_array.count; ++i) {
        env->kite_array.elements[i].is_script_kite = true;
    }

    if (generated_kite_ids.count) {
        // Ensure that new kite ids are available
        // collected_kids is already in parse order, so reuse it directly as
        // the current mapping instead of re-collecting (order-stable, no
        // positions/originals needed pre-patch).
        tkbc_remap_script_kite_id_arrays_to_kite_ids(scb_script, collected_kids, generated_kite_ids);
    }
    // NOTE: generated_kite_ids is intentionally not freed here because it holds
    // the ids of the kites that were just created and they are needed for the
    // CLIENTKITES message further down. It is freed in the cleanup section,
    // which also covers the error paths.

    // Set the first kite positions
    tkbc_patch_script_kite_positions(env, scb_script, scb_space);

#ifdef TKBC_SERVER
    // Remember the id because tkbc_add_script() resets the scratch buffer that
    // scb_script points to.
    UUID script_id = scb_script->id;
#endif

    //
    //
    // TODO: @Cleanup @Memory Holding all the scripts in memory is to much
    // even an DOS attac could happen, by providing a large amount of
    // scripts that doesn't fit into memory.
    //
    // Think about storing them on disk and loading them on demand or
    // reducing the memory storage size of a script.
    //
    // Marvin Frohwitter 22.06.2025
    //
    // NOTE: Update:
    // Currently a memory threshold of 10 scripts are implemented in the
    // tkbc_add_script() function.
    //
    // Now that evict_when_full is false the server saves unlimited scripts again.
    // So need to fix
    //
    // Marvin Frohwitter 19.09.2026
    scb_script->was_send = true;

    tkbc_set_script_name_if_not_exists(scb_script);
    tkbc_add_script(env, *scb_script, false);

    // This is just to be explicit is already happen in the script adding.
    //
    // For continues parsing this does not happen in an error case.
    scb_script->count = 0;

    // This parsing function is just used in the server but liked in the client as
    // well so just a simple guard for compilation.
#ifdef TKBC_SERVER
    {
        Message t_message = {0};
        {
            if (!tkbc_combine_message_script_amount_and_message_script_for_one_id(space_get_tspace(), &t_message,
                                                                                  script_id)) {
                space_reset_tspace();
                check_return(false);
            }
            // The receiving client is excluded so it does not get its own script back.
            tkbc_write_to_all_send_msg_buffers_except(t_message, client->socket_id);
        }
        t_message.count = 0;
        {
            // The kite ids that were parsed from the message are the ids of the
            // sending side and have nothing to do with the ids in the kite array
            // of this side. Only the generated ones are known here and in the
            // kite arrays of the receiving clients.
            //
            // The amount is counted up front so it always matches the number of
            // kite values that follow it. A kite that cannot be resolved is
            // skipped instead, which would otherwise leave the receiver reading
            // past the end of the message.
            size_t amount = 0;
            for (size_t i = 0; i < generated_kite_ids.count; ++i) {
                if (tkbc_get_kite_state_by_id(env, generated_kite_ids.elements[i]) != NULL) {
                    amount++;
                }
            }

            {
                size_t offset = tkbc_message_write_begin(&t_message, space_get_tspace(), MESSAGE_CLIENTKITES);
                tkbc_message_write_u64(&t_message, space_get_tspace(), (uint64_t) amount);
                for (size_t i = 0; i < generated_kite_ids.count; ++i) {
                    Kite_State *kite_state = tkbc_get_kite_state_by_id(env, generated_kite_ids.elements[i]);
                    if (!kite_state) {
                        tkbc_fprintf(stderr, "ERROR", "The generated kite id %zu was not found in the kite array.\n",
                                     generated_kite_ids.elements[i]);
                        continue;
                    }
                    tkbc_message_append_clientkite(kite_state->kite_id, &t_message, space_get_tspace());
                }
                tkbc_message_write_end(&t_message, offset);
                tkbc_write_to_all_send_msg_buffers(t_message);
            }
        }
        space_reset_tspace();
    }
#endif

parsing_skip:
    bool was_expecting_script = client->script_amount > 0;
    if (was_expecting_script) {
        client->script_amount--;
    }
    if (was_expecting_script && client->script_amount == 0) {
        size_t offset =
            tkbc_message_write_begin(&client->send_msg_buffer, &client->send_msg_buffer_space, MESSAGE_SCRIPT_PARSED);
        tkbc_message_write_end(&client->send_msg_buffer, offset);
        // The sending is done automatically in the next section or in the client when the send call is performed.
    }
    tkbc_fprintf(stderr, "MESSAGEHANDLER", "SCRIPT\n");

check:
    if (*script_alleady_there_parsing_skip && check_first_run) {
        check_first_run = false;
        goto parsing_skip;
    }

    if (collected_kids.elements) {
        free(collected_kids.elements);
        collected_kids.elements = NULL;
    }
    if (generated_kite_ids.elements) {
        free(generated_kite_ids.elements);
        generated_kite_ids.elements = NULL;
    }

    return ok;
}
