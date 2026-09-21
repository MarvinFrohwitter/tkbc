#include "raylib.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "../../external/space/space.h"
#include "../global/tkbc-types.h"
#include "../global/tkbc-utils.h"
#include "tkbc-keymaps.h"
#include "tkbc-script-handler.h"
#include "tkbc.h"

#include "raymath.h"
#include "tkbc-script-api.h"
#include "tkbc-ui.h"

// ========================== Script Handler =================================

/**
 * @brief The function can be used to get a new allocated zero initialized
 * frame.
 *
 * @param space The space where the allocation should happen.
 * @return A new on the heap allocated frame region is given back.
 */
Frame *tkbc_init_frame(Space *space) {
    Frame *frame = space_malloc(space, sizeof(*frame));
    if (frame == NULL) {
        tkbc_fprintf(stderr, "ERROR", "No more memory can be allocated.\n");
        return NULL;
    }
    memset(frame, 0, sizeof(*frame));
    return frame;
}

/**
 * @brief The function can be used to get a direct view pointer to the kite in
 * the kite_array stored in the env by providing its id.
 *
 * @param env The global state of the application.
 * @param id THe id that identifies the kite.
 * @return A pointer to the requested kite or NULL if the kite doesn't exist.
 */
Kite *tkbc_get_kite_by_id(Env *env, size_t id) {
    for (size_t i = 0; i < env->kite_array.count; ++i) {
        if (env->kite_array.elements[i].kite_id == id) {
            return env->kite_array.elements[i].kite;
        }
    }
    return NULL;
}

/**
 * @brief The function can be used to get a direct view pointer to the kite
 * state in the kite_array stored in the env by providing its id.
 *
 * @param env The global state of the application.
 * @param id THe id that identifies the kite.
 * @return A pointer to the requested kite or NULL if the kite doesn't exist.
 */
Kite_State *tkbc_get_kite_state_by_id(Env *env, size_t id) {
    for (size_t i = 0; i < env->kite_array.count; ++i) {
        if (env->kite_array.elements[i].kite_id == id) {
            return &env->kite_array.elements[i];
        }
    }
    return NULL;
}

/**
 * @brief The function check for the error and crashes if the kite doesn't
 * exist it will report the error.
 *
 * @param env The global state of the application.
 * @param id THe id that identifies the kite.
 * @return A pointer to the requested kite.
 */
Kite *tkbc_get_kite_by_id_unwrap(Env *env, size_t id) {
    Kite *kite = tkbc_get_kite_by_id(env, id);
    if (!kite) {
        tkbc_fprintf(stderr, "ERROR", "The kite index array is invalid.\n");
        tkbc_fprintf(stderr, "ERROR", "The id: %zu was not found.\n", id);
    }
    assert(kite != NULL);
    return kite;
}

/**
 * @brief The function checks if a given script, represented by its id, can be
 * found in the given scripts collection.
 *
 * @param scripts The collection of scripts.
 * @param script_id The id of a script to search for.
 * @return True if the given script was found in the scripts, otherwise false.
 */
bool tkbc_scripts_contains_id(Scripts scripts, UUID script_id) {
    for (size_t i = 0; i < scripts.count; ++i) {
        if (tkbc_uuid_equals(scripts.elements[i].id, script_id)) {
            return true;
        }
    }
    return false;
}

/**
 * @brief The function can be used to check if the given id is located in the
 * kite_ids.
 *
 * @param kite_ids The ids array that potentially contains the id.
 * @param id The id to find.
 * @return True if the id was found, otherwise false.
 */
bool tkbc_contains_id(Kite_Ids kite_ids, size_t id) {
    for (size_t i = 0; i < kite_ids.count; ++i) {
        if (kite_ids.elements[i] == id) {
            return true;
        }
    }
    return false;
}

/**
 * @brief The function searches the kite array for the first active script kite
 * and stores its kite id.
 *
 * @param env The environment that holds the kite array.
 * @param id A pointer where the kite id of the found kite gets stored.
 * @return Returns true if an active script kite was found, otherwise false.
 */
bool tkbc_find_first_active_script_kite(Env *env, Id *id) {
    for (size_t k = 0; k < env->kite_array.count; ++k) {
        if (env->kite_array.elements[k].is_active && env->kite_array.elements[k].is_script_kite) {
            *id = env->kite_array.elements[k].kite_id;
            return true;
        }
    }
    return false;
}

/**
 * @brief Counts the number of active kites in the kite states array.
 *
 * @param kite_states The kite states array.
 * @return size_t The number of active kites.
 */
size_t tkbc_get_active_kite_count(Kite_States *kite_states) {
    size_t result = 0;
    for (size_t k = 0; k < kite_states->count; ++k) {
        if (kite_states->elements[k].is_active) {
            ++result;
        }
    }
    return result;
}

/**
 * @brief The function copies every single value even the values that are just
 * represented by a pointer of the struct Frames to a new instance. Every
 * internal pointer is a new one in the created representation and points to the
 * new copied values. The result is a complete copy of the given frames. It can
 * be used to move a creation of a temporary struct of type Frames to a
 * permanently stored one.
 *
 * @param space The space where the internal allocation should happen.
 * @param frames The pointer that holds the values that should be copied.
 * @return The value ready copy of the frames.
 */
Frames tkbc_deep_copy_frames(Space *space, Frames *frames) {
    Frames new_frames = {0};
    if (frames == NULL) {
        return new_frames;
    }
    new_frames.frames_index = frames->frames_index;

    if (frames->kite_frame_positions.count) {
        space_dapc(space, &new_frames.kite_frame_positions, frames->kite_frame_positions.elements,
                   frames->kite_frame_positions.count);
    }

    if (frames->elements == NULL) {
        return new_frames;
    }

    for (size_t i = 0; i < frames->count; ++i) {
        Frame frame = tkbc_deep_copy_frame(space, &frames->elements[i]);
        space_dap(space, &new_frames, frame);
    }

    return new_frames;
}

/**
 * @brief Returns the NUL terminated string of the input, "" when empty.
 */
const char *tkbc_text_input_cstr(const Text_Input *input) {
    if (!input || !input->text.elements) {
        return "";
    }
    return input->text.elements;
}

/**
 * @brief Ensures the text buffer can hold needed_count chars plus NUL.
 * Grows via space_realloc when space is NULL. The owning Space
 * is passed explicitly so inputs store no allocator pointer.
 */
bool tkbc_text_input_reserve(Space *space, Text_Buffer *buffer, size_t needed_count) {
    if (!buffer) {
        return false;
    }
    size_t need = needed_count + 1;
    if (need <= buffer->capacity) {
        return true;
    }
    size_t new_cap = buffer->capacity ? buffer->capacity : 64;
    while (new_cap < need) {
        new_cap *= 2;
    }
    size_t old_cap = buffer->capacity;
    char *new_elements = NULL;
    if (space) {
        new_elements = space_realloc(space, buffer->elements, old_cap, new_cap);
    } else {
        new_elements = realloc(buffer->elements, new_cap);
    }
    if (!new_elements) {
        return false;
    }
    buffer->elements = new_elements;
    buffer->capacity = new_cap;
    if (buffer->count >= new_cap) {
        buffer->count = new_cap - 1;
    }
    buffer->elements[buffer->count] = '\0';
    return true;
}

/**
 * @brief Copies str into the input buffer, growing it as needed.
 */
void tkbc_text_input_set_text(Text_Input *input, Space *space, const char *str) {
    if (!input) return;
    if (!str) str = "";
    size_t len = strlen(str);
    if (!tkbc_text_input_reserve(space, &input->text, len)) return;
    memcpy(input->text.elements, str, len + 1);
    input->text.count = len;
    if (input->cursor_pos > len) input->cursor_pos = len;
    if (input->selection_start != SIZE_MAX && input->selection_start > len) input->selection_start = len;
}

/**
 * @brief Initializes an input with the given allocator and initial content.
 */
void tkbc_text_input_init(Text_Input *input, Space *space, const char *initial) {
    if (!input) {
        return;
    }
    input->text.elements = NULL;
    input->text.count = 0;
    input->text.capacity = 0;
    tkbc_text_input_set_text(input, space, initial);
}

/**
 * @brief The function can be used to copy a frame struct.
 *
 * @param space The space where the internal allocation should happen.
 * @param frame The frame that should be copied.
 * @return The copy of the original fames provided in the argument.
 */
Frame tkbc_deep_copy_frame(Space *space, Frame *frame) {
    Frame f = {0};
    f.duration = frame->duration;
    f.original_duration = frame->original_duration;
    f.finished = frame->finished;
    f.kind = frame->kind;
    f.index = frame->index;
    f.action = frame->action;
    if (frame->kite_id_array.count) {
        space_dapc(space, &f.kite_id_array, frame->kite_id_array.elements, frame->kite_id_array.count);

        // This is new since 19.11.2025 Marvin Frohwitter
        f.kite_id_array.script_id_append = frame->kite_id_array.script_id_append;
    }

    return f;
}

/**
 * @brief The function copies every single value even the values that are just
 * represented by a pointer of the struct script to a new instance. It can
 * be used to move a creation of a temporary struct of type script to a
 * permanently stored one.
 *
 * @param space The space where the internal allocation should happen.
 * @param script The pointer that holds the values that should be copied.
 * @return The value ready copy of the script.
 */
Script tkbc_deep_copy_script(Space *space, Script *script) {
    Script new_script = {0};
    if (!script) {
        return new_script;
    }
    new_script.id = script->id;
    new_script.was_send = script->was_send;

    // Deep copy the name buffer into the new allocation. The name is stored
    // only in name_input.text.
    const char *src_name = tkbc_script_name(script);
    size_t src_len = strlen(src_name);
    if (src_len > 0 || script->name_input.text.elements) {
        char *dst = space_malloc(space, src_len + 1);
        if (dst) {
            memcpy(dst, src_name, src_len + 1);
            new_script.name_input.text.elements = dst;
            new_script.name_input.text.count = src_len;
            new_script.name_input.text.capacity = src_len + 1;
        }
    }
    new_script.name_input.box = script->name_input.box;
    new_script.name_input.shadow_text = script->name_input.shadow_text;
    new_script.name_input.font = script->name_input.font;
    new_script.name_input.text_color = script->name_input.text_color;
    new_script.name_input.font_size = script->name_input.font_size;
    new_script.name_input.spacing = script->name_input.spacing;
    new_script.name_input.cursor_pos = script->name_input.cursor_pos;
    new_script.name_input.selection_start = script->name_input.selection_start;
    new_script.name_input.max_char = script->name_input.max_char;
    new_script.name_input.scroll_offset = script->name_input.scroll_offset;
    new_script.name_input.key_constrained = script->name_input.key_constrained;
    new_script.name_input.is_active = script->name_input.is_active;

    for (size_t i = 0; i < script->count; ++i) {
        Frames frames = tkbc_deep_copy_frames(space, &script->elements[i]);
        space_dap(space, &new_script, frames);
    }
    return new_script;
}

/**
 * @brief The function can be used to free all the elements and related memory
 * of the given frames. It recursevly handles all the internal saved values.
 *
 * @param frames The frames the memory should be free.
 */
void tkbc_destroy_frames_internal_data(Frames *frames) {
    if (!frames) {
        return;
    }

    for (size_t i = 0; i < frames->count; ++i) {
        if (frames->elements[i].kite_id_array.elements) {
            free(frames->elements[i].kite_id_array.elements);
            frames->elements[i].kite_id_array.elements = NULL;
            frames->elements[i].kite_id_array.count = 0;
            frames->elements[i].kite_id_array.capacity = 0;
        }
    }

    if (frames->kite_frame_positions.elements) {
        free(frames->kite_frame_positions.elements);
        frames->kite_frame_positions.elements = NULL;
        frames->kite_frame_positions.count = 0;
        frames->kite_frame_positions.capacity = 0;
    }

    if (frames->elements) {
        free(frames->elements);
        frames->elements = NULL;
        frames->capacity = 0;
    }
    frames->count = 0;
}

/**
 * @brief The function can be used to free all the related memory inside of the
 * elements given frames but not the elements and the kite_frame_positions. The
 * count is just reset in the frames and kite_frame_positions. It recursevly
 * handles all the internal saved values.
 *
 * @param frames The frames that should be reset.
 */
void tkbc_reset_frames_internal_data(Frames *frames) {
    if (!frames) {
        return;
    }

    for (size_t i = 0; i < frames->count; ++i) {
        if (frames->elements[i].kite_id_array.elements) {
            frames->elements[i].kite_id_array.count = 0;
            frames->elements[i].kite_id_array.capacity = 0;
        }
    }

    frames->kite_frame_positions.count = 0;
    frames->count = 0;
}

/**
 * @brief Checks if the given kite has a move frame (MOVE or MOVE_ADD) in the
 * current block.
 *
 * @param env The global state of the application.
 * @param kite_id The id of the kite to check.
 * @return True if a move frame exists for this kite, otherwise false.
 */
static bool tkbc_kite_has_move_in_block(Env *env, Id kite_id) {
    for (size_t i = 0; i < env->frames->count; ++i) {
        Frame *f = &env->frames->elements[i];
        if (f->kind == ACTION_KITE_MOVE || f->kind == ACTION_KITE_MOVE_ADD) {
            for (size_t j = 0; j < f->kite_id_array.count; ++j) {
                if (f->kite_id_array.elements[j] == kite_id) {
                    return true;
                }
            }
        }
    }
    return false;
}

/**
 * @brief Checks if the given kite has a tip rotation frame (TIP_ROTATION or
 * TIP_ROTATION_ADD) in the current block.
 *
 * @param env The global state of the application.
 * @param kite_id The id of the kite to check.
 * @return The tip rotation frame for the kite or NULL if none exists.
 */
static Frame *tkbc_kite_tip_rotation_in_block(Env *env, Id kite_id) {
    for (size_t i = 0; i < env->frames->count; ++i) {
        Frame *f = &env->frames->elements[i];
        if (f->kind == ACTION_KITE_TIP_ROTATION || f->kind == ACTION_KITE_TIP_ROTATION_ADD) {
            for (size_t j = 0; j < f->kite_id_array.count; ++j) {
                if (f->kite_id_array.elements[j] == kite_id) {
                    return f;
                }
            }
        }
    }
    return NULL;
}

/**
 * @brief Computes the destination of a MOVE_ADD action that is combined with a
 * tip rotation frame in the same block. A tip rotation moves the kite center
 * around the tip, so the move destination has to account for that center
 * displacement. This prevents the infinite loop where the move and the tip
 * rotation fight over kite->center without modifying the frame action.
 *
 * @param kite The kite that is moved.
 * @param tip_frame The tip rotation frame of the kite in the current block.
 * @param offset The original MOVE_ADD offset.
 * @return The combined destination of the move.
 */
static Vector2 tkbc_combined_move_destination(Kite *kite, Frame *tip_frame, Vector2 offset) {
    Tip_Rotation_Action *tip_action = &tip_frame->action.as_tip_rotation;

    Vector2 saved_center = kite->center;
    float saved_angle = kite->angle;

    // Provided the old position, because the kite center moves as a circle
    // around the old fixed position.
    kite->center = kite->old_center;
    kite->angle = kite->old_angle;
    tkbc_kite_update_internal(kite);

    float final_tip_angle =
        tip_frame->kind == ACTION_KITE_TIP_ROTATION_ADD ? kite->old_angle + tip_action->angle : tip_action->angle;
    tkbc_tip_rotation(kite, &kite->old_center, final_tip_angle, tip_action->tip);

    Vector2 destination = Vector2Add(kite->center, offset);

    kite->center = saved_center;
    kite->angle = saved_angle;
    tkbc_kite_update_internal(kite);

    return destination;
}

/**
 * @brief The function supports all the action kinds that are defined. It can be
 * used to calculate the given frame and its action. For kite actions the new
 * state of the kite results and the internal action values related to time and
 * intermediate positioning is saved and ready to use in a nest update call.
 *
 * @param env The global state of the application.
 * @param frame The frame the action should the handled for. It also can hold
 * intermediate values.
 * @return true if a global quit is active fired, otherwise false.
 */
void tkbc_render_frame_with_dt(Env *env, Frame *frame, float dt) {
    Kite *kite = NULL;
    Frame *env_frame = &env->frames->elements[frame->index];

    assert(ACTION_KIND_COUNT == 9 && "NOT ALL THE Action_Kinds ARE IMPLEMENTED");
    switch (frame->kind) {
    case ACTION_KITE_QUIT: {
        if (env->frames->count == 1 && !env->global_quit.is_script_quit) {
            env->global_quit.script_quit_duration = frame->duration;
            env->global_quit.is_script_quit = true;
        }
        if (tkbc_check_finished_frames_count(env) == env->frames->count - 1) {
            frame->finished = true;
            break;
        }
    } /* FALLTHROUGH */
    case ACTION_KITE_WAIT: {
        if (frame->duration <= 0) {
            frame->finished = true;
            frame->duration = 0;
        } else {
            frame->duration -= dt;
        }
    } break;

    case ACTION_KITE_MOVE_ADD: {
        Move_Add_Action *action = &frame->action.as_move_add;

        for (size_t i = 0; i < env_frame->kite_id_array.count; ++i) {
            Id id = env_frame->kite_id_array.elements[i];
            kite = tkbc_get_kite_by_id_unwrap(env, id);

            Vector2 dest_position = Vector2Add(kite->old_center, action->position);
            Frame *tip_frame = tkbc_kite_tip_rotation_in_block(env, id);
            if (tip_frame) {
                dest_position = tkbc_combined_move_destination(kite, tip_frame, action->position);
            }
            Vector2 d = tkbc_script_move_with_dt(kite, dest_position, frame->duration, dt);

            if (Vector2Equals(dest_position, kite->old_center)) {
                frame->duration -= dt;
                if (frame->duration <= 0) {
                    frame->finished = true;
                }
                continue;
            }

            int res = fabsf(dest_position.x - kite->center.x) <= d.x && fabsf(dest_position.y - kite->center.y) <= d.y;

            if (res) {
                frame->finished = true;
                tkbc_script_move_with_dt(kite, dest_position, 0, dt);
            }
        }

    } break;

    case ACTION_KITE_MOVE: {
        Move_Action *action = &frame->action.as_move;

        for (size_t i = 0; i < env_frame->kite_id_array.count; ++i) {
            Id id = env_frame->kite_id_array.elements[i];
            kite = tkbc_get_kite_by_id_unwrap(env, id);

            Vector2 d = tkbc_script_move_with_dt(kite, action->position, frame->duration, dt);

            if (Vector2Equals(action->position, Vector2Zero())) {
                if (frame->duration > 0) {
                    frame->duration -= dt;
                    continue;
                }
                bool result = fabsf(action->position.x - kite->center.x) <= d.x &&
                              fabsf(action->position.y - kite->center.y) <= d.y;
                if (result) {
                    frame->finished = true;
                }
                continue;
            }

            int res =
                fabsf(action->position.x - kite->center.x) <= d.x && fabsf(action->position.y - kite->center.y) <= d.y;

            if (res) {
                frame->finished = true;
                tkbc_script_move_with_dt(kite, action->position, 0, dt);
            }
        }
    } break;

    case ACTION_KITE_ROTATION_ADD: {
        Rotation_Action *action = &frame->action.as_rotation_add;

        for (size_t i = 0; i < env_frame->kite_id_array.count; ++i) {
            Id id = env_frame->kite_id_array.elements[i];
            kite = tkbc_get_kite_by_id_unwrap(env, id);

            float d = tkbc_script_rotate_with_dt(kite, action->angle, frame->duration, true, dt);
            if (action->angle == 0) {
                frame->duration -= dt;
                if (frame->duration <= 0) {
                    frame->finished = true;
                }
                continue;
            }

            int result = fabsf((kite->old_angle + action->angle) - kite->angle) <= d;
            if (result) {
                frame->finished = true;
                // Enable for setting the correct angle precision.
                tkbc_script_rotate_with_dt(kite, action->angle, 0, true, dt);
            }
        }
    } break;

    case ACTION_KITE_ROTATION: {
        Rotation_Action *action = &frame->action.as_rotation;

        for (size_t i = 0; i < env_frame->kite_id_array.count; ++i) {
            Id id = env_frame->kite_id_array.elements[i];
            kite = tkbc_get_kite_by_id_unwrap(env, id);

            float intermediate_angle = tkbc_check_angle_zero(kite, frame->kind, *(Action *) action, frame->duration);
            float d = tkbc_script_rotate_with_dt(kite, intermediate_angle, frame->duration, false, dt);

            if (action->angle == 0) {
                if (frame->duration > 0) {
                    frame->duration -= dt;
                    continue;
                }
                bool result = fabsf(kite->angle) <= d * fmaxf(1.0f, fabsf(kite->angle));
                if (result) {
                    frame->finished = true;
                }
                continue;
            }

            // NOTE: Different from the ADDing version.
            int result = fabsf(fabsf(fmodf(action->angle, 360)) - fabsf(fmodf(kite->angle, 360))) <= d;
            if (result) {
                frame->finished = true;
                // Enable for setting the correct angle precision.
                tkbc_script_rotate_with_dt(kite, intermediate_angle, 0, false, dt);
            }
        }
    } break;

    case ACTION_KITE_TIP_ROTATION_ADD: {
        Tip_Rotation_Action *action = &frame->action.as_tip_rotation_add;

        for (size_t i = 0; i < env_frame->kite_id_array.count; ++i) {
            Id id = env_frame->kite_id_array.elements[i];
            kite = tkbc_get_kite_by_id_unwrap(env, id);

            // When combined with a move frame, use angle-only rotation so the
            // move handles the full center animation without interference.
            bool has_move = tkbc_kite_has_move_in_block(env, id);

            float d;
            if (has_move) {
                d = tkbc_script_rotate_with_dt(kite, action->angle, frame->duration, true, dt);
            } else {
                d = tkbc_script_rotate_tip_with_dt(kite, action->tip, action->angle, frame->duration, true, dt);
            }
            if (action->angle == 0) {
                frame->duration -= dt;
                if (frame->duration <= 0) {
                    frame->finished = true;
                }
                continue;
            }

            int result = fabsf((kite->old_angle + action->angle) - kite->angle) <= d;
            if (result) {
                frame->finished = true;
                if (has_move) {
                    tkbc_script_rotate_with_dt(kite, action->angle, 0, true, dt);
                } else {
                    tkbc_script_rotate_tip_with_dt(kite, action->tip, action->angle, 0, true, dt);
                }
            }
        }
    } break;

    case ACTION_KITE_TIP_ROTATION: {
        Tip_Rotation_Action *action = &frame->action.as_tip_rotation;

        for (size_t i = 0; i < env_frame->kite_id_array.count; ++i) {
            Id id = env_frame->kite_id_array.elements[i];
            kite = tkbc_get_kite_by_id_unwrap(env, id);

            bool has_move = tkbc_kite_has_move_in_block(env, id);
            float intermediate_angle = tkbc_check_angle_zero(kite, frame->kind, *(Action *) action, frame->duration);

            float d;
            if (has_move) {
                d = tkbc_script_rotate_with_dt(kite, intermediate_angle, frame->duration, false, dt);
            } else {
                d = tkbc_script_rotate_tip_with_dt(kite, action->tip, intermediate_angle, frame->duration, false, dt);
            }

            if (action->angle == 0) {
                if (frame->duration > 0) {
                    frame->duration -= dt;
                    continue;
                }
                bool result = fabsf(kite->angle) <= d * fmaxf(1.0f, fabsf(kite->angle));
                if (result) {
                    frame->finished = true;
                }
                continue;
            }

            // NOTE: Different from the ADDing version.
            int result = fabsf(fabsf(fmodf(action->angle, 360)) - fabsf(fmodf(kite->angle, 360))) <= d;
            if (result) {
                frame->finished = true;
                if (has_move) {
                    tkbc_script_rotate_with_dt(kite, intermediate_angle, 0, false, dt);
                } else {
                    tkbc_script_rotate_tip_with_dt(kite, action->tip, intermediate_angle, 0, false, dt);
                }
            }
        }
    } break;

    default: assert(0 && "UNREACHABLE tkbc_render_frame()");
    }
}

void tkbc_render_frame(Env *env, Frame *frame) {
    tkbc_render_frame_with_dt(env, frame, tkbc_get_frame_time());
}

/**
 * @brief The function sets the kite_ids in a given script to new values
 * provided in the kite_ids array passed into the function.
 *
 * @param env The global state of the application.
 * @param script The script where the kite ids should be remapped to new values.
 * @param kite_ids The ids array that contain the new values.
 */
void tkbc_remap_script_kite_id_arrays_to_kite_ids(Script *script, Kite_Ids kite_ids) {
    assert(script);
    assert(script->count > 0);
    assert(kite_ids.count > 0);

    Kite_Ids current_kite_ids = {0};
    for (size_t i = 0; i < script->count; ++i) {

        for (size_t j = 0; j < script->elements[i].count; ++j) {
            Frame *frame = &script->elements[i].elements[j];
            for (size_t k = 0; k < frame->kite_id_array.count; ++k) {
                Kite_Ids ids = frame->kite_id_array;
                Id id = ids.elements[k];
                if (!tkbc_contains_id(current_kite_ids, id)) {
                    tkbc_dap(&current_kite_ids, id);
                }
            }
        }

        for (size_t j = 0; j < script->elements[i].kite_frame_positions.count; ++j) {
            Id id = script->elements[i].kite_frame_positions.elements[j].kite_id;
            if (!tkbc_contains_id(current_kite_ids, id)) {
                tkbc_dap(&current_kite_ids, id);
            }
        }
    }

    assert(current_kite_ids.count == kite_ids.count);

    for (size_t i = 0; i < script->count; ++i) {
        assert(script->elements);
        Frames *frames = &script->elements[i];

        for (size_t new_id = 0; new_id < current_kite_ids.count; ++new_id) {

            assert(frames->elements);
            for (size_t j = 0; j < frames->count; ++j) {
                if (frames->elements[j].kind == ACTION_KITE_WAIT || frames->elements[j].kind == ACTION_KITE_QUIT) {
                    continue;
                }
                Kite_Ids *ids = &frames->elements[j].kite_id_array;
                assert(ids->elements);
                for (size_t k = 0; k < ids->count; ++k) {
                    Id *id = &ids->elements[k];

                    if (current_kite_ids.elements[new_id] == *id) {
                        *id = kite_ids.elements[new_id];
                        break;
                    }
                }
            }

            for (size_t j = 0; j < frames->kite_frame_positions.count; ++j) {
                Id *id = &frames->kite_frame_positions.elements[j].kite_id;

                if (current_kite_ids.elements[new_id] == *id) {
                    *id = kite_ids.elements[new_id];
                    break;
                }
            }
        }
    }

    free(current_kite_ids.elements);
}

/**
 * @brief The function can be used to backpatch current kite positions in
 * the given script to be used later in the redrawing and calculation of a
 * script frame after the script has executed successfully.
 *
 * @param env The global state of the application.
 * @param script The script where the patch should happen.
 * @param space The space where the allocation should happen.
 */
void tkbc_patch_script_kite_positions(Env *env, Script *script, Space *space) {
    for (size_t i = 0; i < script->count; ++i) {
        tkbc_patch_frames_kite_positions(env, &script->elements[i], space);
    }
}

/**
 * @brief The function can be used to backpatch current kite positions in
 * the frames array to be used later in the redrawing and calculation of a
 * script frame after the script has executed successfully.
 *
 * @param env The global state of the application.
 * @param frames The frames where the kite positions should be updated to the
 * current kite values.
 * @param space The space where the allocation should happen.
 */
void tkbc_patch_frames_kite_positions(Env *env, Frames *frames, Space *space) {
    for (size_t i = 0; i < frames->count; ++i) {
        if (!frames->elements[i].kite_id_array.count) {
            continue;
        }

        for (size_t j = 0; j < frames->elements[i].kite_id_array.count; ++j) {
            Index kite_id = frames->elements[i].kite_id_array.elements[j];
            Kite *kite = tkbc_get_kite_by_id(env, kite_id);
            assert(kite != NULL);

            Kite_Position kite_position = {
                .kite_id = kite_id,
                .position = kite->center,
                .angle = kite->angle,
            };

            bool contains = false;
            for (size_t k = 0; k < frames->kite_frame_positions.count; ++k) {
                if (frames->kite_frame_positions.elements[k].kite_id == kite_id) {
                    contains = true;
                    // NOTE: Patching angle in case the kite_position was already added by
                    // just a move action, but later the corresponding angle action is
                    // handled.
                    frames->kite_frame_positions.elements[k].position = kite_position.position;
                    frames->kite_frame_positions.elements[k].angle = kite_position.angle;
                    break;
                }
            }

            if (!contains) {
                space_dap(space, &frames->kite_frame_positions, kite_position);
            }
        }
    }
}

/**
 * @brief The function can be used to get the state of the current executed
 * frame.
 *
 * @param env The global state of the application.
 * @return True if the current frame has finished its execution, otherwise
 * false.
 */
bool tkbc_check_finished_frames(Env *env) {
    for (size_t i = 0; i < env->frames->count; ++i) {
        if (!env->frames->elements[i].finished) {
            return false;
        }
    }
    return true;
}

/**
 * @brief The function can collect the amount of finished frames in the
 * current block frame execution.
 *
 * @param env The global state of the application.
 * @return It returns the amount of finished frames and 0 if no frames has
 * finished yet.
 */
size_t tkbc_check_finished_frames_count(Env *env) {
    int count = 0;
    for (size_t i = 0; i < env->frames->count; ++i) {
        if (env->frames->elements[i].finished) {
            count++;
        }
    }

    return count;
}

/**
 * @brief The function enables all the non script kites visibility and disables
 * the rest of them.
 *
 * @param env The global state of the application.
 */
void tkbc_change_visibility_to_non_script_kites(Env *env) {
    // Enable the normal client kites.
    for (size_t i = 0; i < env->kite_array.count; ++i) {
        Kite_State *kite_state = &env->kite_array.elements[i];
        kite_state->is_kite_input_handler_active = false;
        kite_state->is_active = false;
        if (!kite_state->is_script_kite) {
            kite_state->is_active = true;
        }
    }
}

/**
 * @brief The function enables all the kites visibility that belong to a script
 * and disables all others.
 *
 * @param env The global state of the application.
 * @param script The script where the belonging kites should be toggled on.
 */
void tkbc_change_visibility_to_script_kites(Env *env, Script *script) {

    // TODO: Find a better way to do it reliable. And faster!!!

    Kite_Ids ids = {0};
    for (size_t i = 0; i < script->count; ++i) {
        for (size_t j = 0; j < script->elements[i].count; ++j) {
            Kite_Ids *kite_id_array = &script->elements[i].elements[j].kite_id_array;

            for (size_t k = 0; k < kite_id_array->count; ++k) {
                Id id = kite_id_array->elements[k];
                if (!tkbc_contains_id(ids, id)) {
                    tkbc_dap(&ids, id);
                }
            }
        }
    }

    //
    // Activate the kites that belong to the script.
    for (size_t i = 0; i < env->kite_array.count; ++i) {
        env->kite_array.elements[i].is_active = false;
        env->kite_array.elements[i].is_kite_input_handler_active = false;

        for (size_t j = 0; j < ids.count; ++j) {
            if (ids.elements[j] == env->kite_array.elements[i].kite_id) {
                env->kite_array.elements[i].is_active = true;
                break;
            }
        }
    }
    free(ids.elements);
}

/**
 * @brief The function switches to the next available script. It loads the views
 * script and frames in the env. And sets the kite_frame_positions.

 * @param env The global state of the application.
 */
void tkbc_load_next_script(Env *env) {
    if (env->scripts.count <= 0) {
        return;
    }

    // Switch to next script.
    // NOTE: The first iteration has no loaded value jet so 0 is the first index.
    size_t current_index = env->script == NULL ? 0 : 0;
    for (size_t i = 0; i < env->scripts.count; ++i) {
        if (env->script && tkbc_uuid_equals(env->script->id, env->scripts.elements[i].id)) {
            current_index = i;
            break;
        }
    }
    size_t script_index = (current_index + 1) % env->scripts.count;
    tkbc_load_script_id(env, env->scripts.elements[script_index].id, true);
}

/**
 * @brief The function sets the views of the script in the env and loads the
 * corresponding kite_frame_positions of the first frame.
 *
 * @param env The global state of the application.
 * @param script_id The id of the script that should be loaded into the
 * current execution.
 * @return True if the script could be loaded successfully, otherwise false.
 */
bool tkbc_load_script_id(Env *env, UUID script_id, bool fresh) {
    bool found = false;
    for (size_t i = 0; i < env->scripts.count; ++i) {
        if (tkbc_uuid_equals(env->scripts.elements[i].id, script_id)) {
            env->script = &env->scripts.elements[i];
            found = true;
            break;
        }
    }

    if (!found) {
        return false;
    }

    env->frames = &env->script->elements[0];
    if (!fresh) {
        tkbc_set_kite_positions_from_kite_frames_positions(env);
    } else {
        // Fresh play (offline selection or server NEXT): start from the
        // current kite positions and rebake the timeline eagerly, so smooth
        // scrubbing works immediately without playing the script one time
        // first. Absolute targets stay fixed while relative offsets shift
        // with the new start, exactly like a live run would.
        assert(env->script);
        tkbc_restore_script_frame_states(env);
        tkbc_patch_script_kite_positions(env, env->script, &env->script->space);
        tkbc_bake_script_timeline(env, env->script);
        tkbc_set_kite_positions_from_kite_frames_positions(env);
    }
    env->script_finished = false;
    env->script_loading = true;

    tkbc_change_visibility_to_script_kites(env, env->script);
    return true;
}

/**
 * @brief The function unloads the view of the currently executing script but
 * not from the memory.
 *
 * @param env The global state of the application.
 */
void tkbc_unload_script(Env *env) {
    env->server_script_id = tkbc_uuid_nil();
    env->server_script_frames_count = 0;
    env->server_script_frames_index = 0;
    env->script_finished = true;
    env->frames = NULL;
    env->script = NULL;
}

/**
 * @brief The function removes the script from the known scripts of the array.
 * It does not handle the actual memory deallocation, because is is stored in a
 * space allocator any way.
 *
 * @param env The global state of the application.
 * @param script_id The id of the script that should be unloaded.
 * @return 0 if the unloading was successful, otherwise 1 if the script could not be found or -1 if the currently loaded
 * script was deleted in the middle of unloading the given one.
 */
int tkbc_unload_script_from_memory(Env *env, UUID script_id) {
    Index loaded_frames_index = 0;
    UUID loaded_script_id = tkbc_uuid_nil();
    bool is_frames = false;
    bool is_script = false;
    int ok = 1;

    if (env->frames) {
        is_frames = true;
        loaded_frames_index = env->frames->frames_index;
    }

    if (env->script) {
        is_script = true;
        loaded_script_id = env->script->id;
    }

    for (size_t i = 0; i < env->scripts.count; ++i) {
        if (tkbc_uuid_equals(script_id, env->scripts.elements[i].id)) {
            if (env->script && tkbc_uuid_equals(script_id, env->script->id)) {
                tkbc_unload_script(env);
                tkbc_change_visibility_to_non_script_kites(env);
            }

            space_free_space(&env->scripts.elements[i].space);

            if (i + 1 < env->scripts.count) {
                memmove(&env->scripts.elements[i], &env->scripts.elements[i + 1],
                        sizeof(*env->scripts.elements) * (env->scripts.count - i - 1));
            }

            env->scripts.count -= 1;
            ok = 0;
            break;
        }
    }

    if (is_script) {
        if (!tkbc_load_script_id(env, loaded_script_id, false)) {
            ok = -1;
        }
    }

    if (is_frames && ok != -1) {
        // The env->script pointer is now valid again, so we can use it.
        for (size_t i = 0; i < env->script->count; ++i) {
            if (env->script->elements[i].frames_index == loaded_frames_index) {
                env->frames = &env->script->elements[i];
                break;
            }
        }
    }

    if (tkbc_uuid_equals(env->server_script_id, script_id)) {
        env->server_script_id = tkbc_uuid_nil();
        env->server_script_frames_count = 0;
        env->server_script_frames_index = 0;
    }

    return ok;
}

/**
 * @brief Calculates the size that the given frames currently take.
 *
 * @param frame The frame where you want to get the complete size for.
 * @return The resulting size in bytes that is necessary to allocate the
 * complete thing.
 */
size_t tkbc_calculate_frame_byte_size(Frame frame) {
    size_t result = 0;
    result += sizeof(frame);

    result += frame.kite_id_array.count * sizeof(*frame.kite_id_array.elements);

    return result;
}

/**
 * @brief Calculates the size that the given frames currently take.
 *
 * @param frames The frames where you want to get the complete size for.
 * @return The resulting size in bytes that is necessary to allocate the
 * complete thing.
 */
size_t tkbc_calculate_frames_byte_size(Frames frames) {
    size_t result = 0;
    result += sizeof(frames);

    for (size_t i = 0; i < frames.count; ++i) {
        Frame frame = frames.elements[i];
        result += tkbc_calculate_frame_byte_size(frame);
    }

    result += frames.kite_frame_positions.count * sizeof(*frames.kite_frame_positions.elements);

    return result;
}

/**
 * @brief Calculates the size that the given script currently take.
 *
 * @param script The script where the resulting size has to be calculated.
 * @return The resulting size in bytes that is necessary to allocate the
 * complete thing.
 */
size_t tkbc_calculate_script_byte_size(Script script) {

    size_t result = 0;
    result += sizeof(script);
    const char *script_name = tkbc_script_name(&script);
    if (script_name[0] != '\0') {
        result += strlen(script_name) + 1;
    }

    for (size_t i = 0; i < script.count; ++i) {
        Frames frames = script.elements[i];
        result += tkbc_calculate_frames_byte_size(frames);
    }

    return result;
}

/**
 * @brief Calculates the total allocated byte size of a script including its
 * internal data.
 *
 * @param script The script to calculate the size for.
 * @return size_t The total allocated byte size.
 */
size_t tkbc_calculate_script_byte_size_allocated(Script script) {
    size_t result = 0;

    const char *script_name = tkbc_script_name(&script);
    if (script_name[0] != '\0') {
        result += strlen(script_name) + 1;
    }

    if (script.count > 0) {
        result += script.capacity * sizeof(Frames);
    }
    for (size_t i = 0; i < script.count; ++i) {
        Frames *frames = &script.elements[i];

        // When reusing the same dynamic array the capacity can already be allocated
        // but there is actually nothing in the array.
        if (frames->kite_frame_positions.count > 0) {
            result += frames->kite_frame_positions.capacity * sizeof(Kite_Position);
        }

        if (frames->count > 0) {
            result += frames->capacity * sizeof(Frame);
        }

        for (size_t j = 0; j < frames->count; ++j) {

            if (frames->elements[j].kite_id_array.count > 0) {
                result += frames->elements[j].kite_id_array.capacity * sizeof(Id);
            }
        }
    }

    return result;
}

/**
 * @brief Returns the longest original duration of all frames in the block.
 *
 * The upscaling uses the original durations (not the mutated live ones) so
 * that a replay or a second upscale pass sees the same timing.
 *
 * @param frames The block to inspect.
 * @return The maximum original_duration, or duration as a fallback.
 */
float tkbc_block_original_duration(const Frames *frames) {
    float max = 0;
    if (!frames) {
        return 0;
    }
    for (size_t i = 0; i < frames->count; ++i) {
        float d = frames->elements[i].original_duration;
        if (d <= 0) {
            d = frames->elements[i].duration;
        }
        if (d > max) {
            max = d;
        }
    }
    return max;
}

/**
 * @brief Returns how many timeline steps a duration needs at the given fps.
 *
 * @param duration The duration in seconds.
 * @param fps The frames per second, e.g. TARGET_FPS.
 * @return At least 1. ceil(duration * fps) otherwise.
 */
size_t tkbc_upscale_step_count(float duration, float fps) {
    if (duration <= 0 || fps <= 0) {
        return 1;
    }
    size_t n = (size_t) ceilf(duration * fps - 1e-6f);
    if (n < 1) {
        n = 1;
    }
    // Safety cap so a pathological WAIT (e.g. 1000s) cannot OOM the script.
    // 10000 steps are ~166s at 60fps and still scrub smoothly.
    if (n > 10000) {
        n = 10000;
    }
    return n;
}

typedef struct {
    Id id;
    Kite kite;
} Upscale_Kite;

static Upscale_Kite *upscale_find_kite(Upscale_Kite *map, size_t map_count, Id id) {
    for (size_t i = 0; i < map_count; ++i) {
        if (map[i].id == id) {
            return &map[i];
        }
    }
    return NULL;
}

static bool upscale_block_has_move(const Frames *block, Id id) {
    for (size_t i = 0; i < block->count; ++i) {
        const Frame *f = &block->elements[i];
        if (f->kind == ACTION_KITE_MOVE || f->kind == ACTION_KITE_MOVE_ADD) {
            for (size_t j = 0; j < f->kite_id_array.count; ++j) {
                if (f->kite_id_array.elements[j] == id) {
                    return true;
                }
            }
        }
    }
    return false;
}

static bool upscale_frame_is_motion(Action_Kind kind) {
    return kind == ACTION_KITE_MOVE || kind == ACTION_KITE_MOVE_ADD || kind == ACTION_KITE_ROTATION ||
           kind == ACTION_KITE_ROTATION_ADD || kind == ACTION_KITE_TIP_ROTATION || kind == ACTION_KITE_TIP_ROTATION_ADD;
}

/**
 * @brief Finds the signed travel in direction of the target for absolute rotations.
 *
 * Absolute rotations move in direction sign(target) until the wrapped angle
 * matches. The search samples the same fabs(fmod()) comparison the playback
 * uses, so the baked travel matches what the live execution would do.
 */
static float upscale_absolute_travel(float start_angle, float target_angle) {
    float dir = signbit(target_angle) ? -1.0f : 1.0f;
    float target_mod = fabsf(fmodf(target_angle, 360.0f));
    for (float d = 0; d <= 360.0f; d += 0.05f) {
        float y = start_angle + dir * d;
        float m = fabsf(fmodf(y, 360.0f));
        if (fabsf(m - target_mod) <= 0.06f) {
            return dir * d;
        }
    }
    return dir * 360.0f;
}

static void upscale_ensure_kite(Env *env, Upscale_Kite **map, size_t *map_count, size_t *map_cap, Id id,
                                const Frames *block) {
    if (upscale_find_kite(*map, *map_count, id)) {
        return;
    }
    if (*map_count >= *map_cap) {
        size_t ncap = *map_cap ? *map_cap * 2 : 16;
        Upscale_Kite *n = realloc(*map, ncap * sizeof(**map));
        if (!n) {
            abort();
        }
        *map = n;
        *map_cap = ncap;
    }
    Upscale_Kite *slot = &(*map)[*map_count];
    slot->id = id;
    Kite *src = tkbc_get_kite_by_id(env, id);
    if (src) {
        slot->kite = *src;
    } else if (env->vanilla_kite) {
        slot->kite = *env->vanilla_kite;
    } else {
        memset(&slot->kite, 0, sizeof(slot->kite));
    }
    // Start from the stored block start when available, otherwise keep the
    // current (cloned) position.
    bool found = false;
    for (size_t i = 0; i < block->kite_frame_positions.count; ++i) {
        if (block->kite_frame_positions.elements[i].kite_id == id) {
            Vector2 p = block->kite_frame_positions.elements[i].position;
            float a = block->kite_frame_positions.elements[i].angle;
            tkbc_center_rotation(&slot->kite, &p, a);
            slot->kite.old_center = slot->kite.center;
            slot->kite.old_angle = slot->kite.angle;
            found = true;
            break;
        }
    }
    if (!found) {
        slot->kite.old_center = slot->kite.center;
        slot->kite.old_angle = slot->kite.angle;
    }
    *map_count += 1;
}

/**
 * @brief Instantly applies a whole block to the temp kite map for chaining.
 *
 * Used for blocks that are kept as-is (N <= 1) so the next block still starts
 * from the correct end positions. Rotations/tips run first, moves second, so
 * a combined MOVE_ADD + TIP_ROTATION ends at tip_end + offset like the live
 * combined destination.
 */
static void upscale_apply_block_instant(Upscale_Kite *map, size_t map_count, const Frames *block) {
    for (size_t pass = 0; pass < 2; ++pass) {
        for (size_t i = 0; i < block->count; ++i) {
            const Frame *f = &block->elements[i];
            bool is_move = f->kind == ACTION_KITE_MOVE || f->kind == ACTION_KITE_MOVE_ADD;
            if ((pass == 0 && is_move) || (pass == 1 && !is_move && upscale_frame_is_motion(f->kind))) {
                // pass 0: rotations/tips only, pass 1: moves only.
                if (pass == 0 && is_move) {
                    continue;
                }
                if (pass == 1 && !is_move) {
                    continue;
                }
            }
            if (pass == 0 && is_move) {
                continue;
            }
            if (pass == 1 && !is_move) {
                continue;
            }
            for (size_t j = 0; j < f->kite_id_array.count; ++j) {
                Upscale_Kite *uk = upscale_find_kite(map, map_count, f->kite_id_array.elements[j]);
                if (!uk) {
                    continue;
                }
                switch (f->kind) {
                case ACTION_KITE_MOVE: {
                    Vector2 target = f->action.as_move.position;
                    tkbc_center_rotation(&uk->kite, &target, uk->kite.angle);
                } break;
                case ACTION_KITE_MOVE_ADD: {
                    Vector2 np = Vector2Add(uk->kite.center, f->action.as_move_add.position);
                    tkbc_kite_update_position(&uk->kite, &np);
                } break;
                case ACTION_KITE_ROTATION:
                    tkbc_kite_update_angle(&uk->kite, f->action.as_rotation.angle);
                    break;
                case ACTION_KITE_ROTATION_ADD:
                    tkbc_kite_update_angle(&uk->kite, uk->kite.angle + f->action.as_rotation_add.angle);
                    break;
                case ACTION_KITE_TIP_ROTATION:
                    tkbc_tip_rotation(&uk->kite, NULL, f->action.as_tip_rotation.angle, f->action.as_tip_rotation.tip);
                    break;
                case ACTION_KITE_TIP_ROTATION_ADD:
                    tkbc_tip_rotation(&uk->kite, NULL, uk->kite.angle + f->action.as_tip_rotation_add.angle,
                                      f->action.as_tip_rotation_add.tip);
                    break;
                default: break;
                }
            }
        }
    }
    for (size_t i = 0; i < map_count; ++i) {
        map[i].kite.old_center = map[i].kite.center;
        map[i].kite.old_angle = map[i].kite.angle;
    }
}

static void upscale_push_single_id_frame(Space *space, Frames *slice, Action_Kind kind, Action action, float duration,
                                         Id kite_id) {
    Frame f = {0};
    f.kind = kind;
    f.action = action;
    f.duration = duration;
    f.original_duration = duration;
    f.finished = false;
    f.index = slice->count;
    space_dap(space, &f.kite_id_array, kite_id);
    space_dap(space, slice, f);
}

static void upscale_push_wait_frame(Space *space, Frames *slice, Action_Kind kind, float duration) {
    Frame f = {0};
    f.kind = kind;
    f.duration = duration;
    f.original_duration = duration;
    f.finished = false;
    f.index = slice->count;
    space_dap(space, slice, f);
}

/**
 * @brief Expands every long block into per-tick slices at the given fps.
 *
 * Each block with duration D becomes N = ceil(D * fps) slices of duration
 * D / N (~1/fps). Relative actions (ADD) are split proportionally so they
 * stay start-independent, absolute MOVE targets are linearly interpolated,
 * and absolute rotations are split into ADD deltas plus a final absolute
 * snap. Blocks with N <= 1 are kept as-is (idempotent second pass).
 *
 * The script is rebuilt in its own Space; old arrays stay allocated in the
 * same Space and are simply orphaned (the Space frees everything at once).
 *
 * @param env The global state, used for kite geometry templates.
 * @param script The script to upscale in place.
 * @param fps The timeline resolution, usually TARGET_FPS.
 */
void tkbc_upscale_script(Env *env, Script *script, float fps) {
    if (!env || !script || script->count == 0 || fps <= 0) {
        return;
    }

    Script upscaled = {0};
    Upscale_Kite *map = NULL;
    size_t map_count = 0;
    size_t map_cap = 0;

    for (size_t b = 0; b < script->count; ++b) {
        Frames *block = &script->elements[b];
        float D = tkbc_block_original_duration(block);
        size_t N = tkbc_upscale_step_count(D, fps);

        // Collect motion kite ids and wait/quit presence.
        Id *involved = NULL;
        size_t involved_count = 0;
        size_t involved_cap = 0;
        bool has_motion = false;
        for (size_t i = 0; i < block->count; ++i) {
            Frame *f = &block->elements[i];
            if (f->kind == ACTION_KITE_WAIT || f->kind == ACTION_KITE_QUIT) {
                continue;
            } else if (upscale_frame_is_motion(f->kind)) {
                has_motion = true;
                for (size_t j = 0; j < f->kite_id_array.count; ++j) {
                    Id id = f->kite_id_array.elements[j];
                    bool seen = false;
                    for (size_t k = 0; k < involved_count; ++k) {
                        if (involved[k] == id) {
                            seen = true;
                            break;
                        }
                    }
                    if (!seen) {
                        if (involved_count >= involved_cap) {
                            size_t ncap = involved_cap ? involved_cap * 2 : 8;
                            Id *n = realloc(involved, ncap * sizeof(*n));
                            if (!n) {
                                abort();
                            }
                            involved = n;
                            involved_cap = ncap;
                        }
                        involved[involved_count++] = id;
                    }
                }
            }
        }

        for (size_t k = 0; k < involved_count; ++k) {
            upscale_ensure_kite(env, &map, &map_count, &map_cap, involved[k], block);
        }
        // Sync existing map entries to the carried end positions: old = current.
        // New entries were already initialized to the stored block start.
        // For existing entries the current center already holds the previous
        // block end, which is the correct start here (stale stored starts are
        // ignored, fixing pre-playback scrub positions as well).
        for (size_t k = 0; k < involved_count; ++k) {
            Upscale_Kite *uk = upscale_find_kite(map, map_count, involved[k]);
            if (uk) {
                uk->kite.old_center = uk->kite.center;
                uk->kite.old_angle = uk->kite.angle;
            }
        }

        if (N <= 1 || D <= 0) {
            // Keep as-is, but advance the chain so later blocks start correctly.
            if (has_motion) {
                upscale_apply_block_instant(map, map_count, block);
            }
            Frames kept = tkbc_deep_copy_frames(&script->space, block);
            kept.frames_index = upscaled.count;
            for (size_t i = 0; i < kept.count; ++i) {
                kept.elements[i].index = i;
            }
            space_dap(&script->space, &upscaled, kept);
            free(involved);
            continue;
        }

        // N > 1: bake per-kite start snapshots and iterative cursors.
        float step = D / (float) N;
        typedef struct {
            Id id;
            Vector2 start_pos;
            float start_angle;
            Vector2 cur_pos;
            float cur_angle;
            Kite geo;
        } Cursor;
        Cursor *cursors = malloc(involved_count * sizeof(*cursors));
        if (involved_count && !cursors) {
            abort();
        }
        for (size_t k = 0; k < involved_count; ++k) {
            Upscale_Kite *uk = upscale_find_kite(map, map_count, involved[k]);
            cursors[k].id = involved[k];
            cursors[k].start_pos = uk->kite.center;
            cursors[k].start_angle = uk->kite.angle;
            cursors[k].cur_pos = uk->kite.center;
            cursors[k].cur_angle = uk->kite.angle;
            cursors[k].geo = uk->kite;
        }

        // Precompute per (frame,kite) absolute travel for rotations.
        for (size_t s = 0; s < N; ++s) {
            Frames slice = {0};
            // Slice start positions for scrubbing.
            for (size_t k = 0; k < involved_count; ++k) {
                Kite_Position kp = {
                    .kite_id = cursors[k].id,
                    .position = cursors[k].cur_pos,
                    .angle = cursors[k].cur_angle,
                };
                space_dap(&script->space, &slice.kite_frame_positions, kp);
            }

            for (size_t i = 0; i < block->count; ++i) {
                Frame *f = &block->elements[i];
                float O = f->original_duration > 0 ? f->original_duration : f->duration;
                if (O <= 0) {
                    if (s == 0) {
                        // Instant frame: keep once in the first slice.
                        if (f->kind == ACTION_KITE_WAIT || f->kind == ACTION_KITE_QUIT) {
                            upscale_push_wait_frame(&script->space, &slice, f->kind, 0);
                        } else {
                            for (size_t j = 0; j < f->kite_id_array.count; ++j) {
                                upscale_push_single_id_frame(&script->space, &slice, f->kind, f->action, 0,
                                                             f->kite_id_array.elements[j]);
                            }
                        }
                    }
                    continue;
                }
                size_t Ni = (size_t) ceilf(O / step - 1e-6f);
                if (Ni < 1) {
                    Ni = 1;
                }
                if (Ni > N) {
                    Ni = N;
                }
                if (s >= Ni) {
                    continue;
                }
                float dur = (s == Ni - 1) ? (O - (float) (Ni - 1) * step) : step;
                if (dur <= 0) {
                    dur = step;
                }
                if (f->kind == ACTION_KITE_WAIT || f->kind == ACTION_KITE_QUIT) {
                    upscale_push_wait_frame(&script->space, &slice, f->kind, dur);
                    continue;
                }
                float elapsed_next = (s == Ni - 1) ? O : (float) (s + 1) * step;
                for (size_t j = 0; j < f->kite_id_array.count; ++j) {
                    Id kid = f->kite_id_array.elements[j];
                    Cursor *cu = NULL;
                    for (size_t k = 0; k < involved_count; ++k) {
                        if (cursors[k].id == kid) {
                            cu = &cursors[k];
                            break;
                        }
                    }
                    if (!cu) {
                        continue;
                    }
                    // Block start for this kite (for absolute lerps).
                    Vector2 Spos = {0};
                    float Sangle = 0;
                    {
                        Upscale_Kite *uk0 = upscale_find_kite(map, map_count, kid);
                        // map currently holds block start (old == start). Use the
                        // saved cursor start instead, which is exactly that.
                        Spos = cu->start_pos;
                        Sangle = cu->start_angle;
                        (void) uk0;
                    }
                    switch (f->kind) {
                    case ACTION_KITE_MOVE: {
                        float t = elapsed_next / O;
                        if (t > 1) {
                            t = 1;
                        }
                        Vector2 target = Vector2Lerp(Spos, f->action.as_move.position, t);
                        Action a = {0};
                        a.as_move.position = target;
                        upscale_push_single_id_frame(&script->space, &slice, ACTION_KITE_MOVE, a, dur, kid);
                        cu->cur_pos = target;
                    } break;
                    case ACTION_KITE_MOVE_ADD: {
                        Vector2 off = Vector2Scale(f->action.as_move_add.position, dur / O);
                        Action a = {0};
                        a.as_move_add.position = off;
                        upscale_push_single_id_frame(&script->space, &slice, ACTION_KITE_MOVE_ADD, a, dur, kid);
                        cu->cur_pos = Vector2Add(cu->cur_pos, off);
                        // If a tip frame shares this slice it already moved cu
                        // via geometry below in original order; the pure offset
                        // addition here matches sequential playback order.
                    } break;
                    case ACTION_KITE_ROTATION_ADD: {
                        float aj = f->action.as_rotation_add.angle * (dur / O);
                        Action a = {0};
                        a.as_rotation_add.angle = aj;
                        upscale_push_single_id_frame(&script->space, &slice, ACTION_KITE_ROTATION_ADD, a, dur, kid);
                        cu->cur_angle += aj;
                    } break;
                    case ACTION_KITE_ROTATION: {
                        float T = f->action.as_rotation.angle;
                        if (Ni == 1) {
                            Action a = {0};
                            a.as_rotation.angle = T;
                            upscale_push_single_id_frame(&script->space, &slice, ACTION_KITE_ROTATION, a, dur, kid);
                            cu->cur_angle = T;
                        } else if (s < Ni - 1) {
                            float Delta;
                            if (T == 0) {
                                Kite tmp = cu->geo;
                                tmp.old_angle = Sangle;
                                tmp.angle = Sangle;
                                Action tmpa = {0};
                                tmpa.as_rotation.angle = T;
                                Delta = tkbc_check_angle_zero(&tmp, ACTION_KITE_ROTATION, tmpa, O);
                            } else {
                                Delta = upscale_absolute_travel(Sangle, T);
                            }
                            float aj = Delta * (dur / O);
                            Action a = {0};
                            a.as_rotation_add.angle = aj;
                            upscale_push_single_id_frame(&script->space, &slice, ACTION_KITE_ROTATION_ADD, a, dur,
                                                         kid);
                            cu->cur_angle += aj;
                        } else {
                            Action a = {0};
                            a.as_rotation.angle = T;
                            upscale_push_single_id_frame(&script->space, &slice, ACTION_KITE_ROTATION, a, dur, kid);
                            cu->cur_angle = T;
                        }
                    } break;
                    case ACTION_KITE_TIP_ROTATION_ADD: {
                        float aj = f->action.as_tip_rotation_add.angle * (dur / O);
                        bool has_move = upscale_block_has_move(block, kid);
                        if (has_move && f->action.as_tip_rotation_add.angle != 0) {
                            // Keep geometry motion (tip moves center) so the
                            // final matches tip_end + move offsets. Playback
                            // runs tip+move sequentially per slice like here.
                        }
                        Action a = {0};
                        a.as_tip_rotation_add.angle = aj;
                        a.as_tip_rotation_add.tip = f->action.as_tip_rotation_add.tip;
                        upscale_push_single_id_frame(&script->space, &slice, ACTION_KITE_TIP_ROTATION_ADD, a, dur,
                                                     kid);
                        // Advance cursor via real tip geometry.
                        Kite tmp = cu->geo;
                        tmp.center = cu->cur_pos;
                        tmp.angle = cu->cur_angle;
                        tmp.old_center = cu->cur_pos;
                        tmp.old_angle = cu->cur_angle;
                        tkbc_kite_update_internal(&tmp);
                        tkbc_tip_rotation(&tmp, NULL, tmp.angle + aj, f->action.as_tip_rotation_add.tip);
                        cu->cur_pos = tmp.center;
                        cu->cur_angle = tmp.angle;
                    } break;
                    case ACTION_KITE_TIP_ROTATION: {
                        float T = f->action.as_tip_rotation.angle;
                        TIP tip = f->action.as_tip_rotation.tip;
                        bool has_move = upscale_block_has_move(block, kid);
                        // For absolute MOVE + TIP the live playback uses
                        // angle-only tip (no center motion) to avoid wobble.
                        // Mirror that by emitting ROTATION slices here.
                        bool angle_only = has_move;
                        // Detect absolute move presence: if the kite has any
                        // absolute MOVE frame in this block, use angle-only.
                        bool has_abs_move = false;
                        for (size_t q = 0; q < block->count; ++q) {
                            if (block->elements[q].kind == ACTION_KITE_MOVE &&
                                tkbc_contains_id(block->elements[q].kite_id_array, kid)) {
                                has_abs_move = true;
                                break;
                            }
                        }
                        if (has_abs_move) {
                            angle_only = true;
                        }
                        if (Ni == 1) {
                            if (angle_only) {
                                Action a = {0};
                                a.as_rotation.angle = T;
                                upscale_push_single_id_frame(&script->space, &slice, ACTION_KITE_ROTATION, a, dur,
                                                             kid);
                                cu->cur_angle = T;
                            } else {
                                Action a = {0};
                                a.as_tip_rotation.angle = T;
                                a.as_tip_rotation.tip = tip;
                                upscale_push_single_id_frame(&script->space, &slice, ACTION_KITE_TIP_ROTATION, a, dur,
                                                             kid);
                                Kite tmp = cu->geo;
                                tmp.center = Spos;
                                tmp.angle = Sangle;
                                tmp.old_center = Spos;
                                tmp.old_angle = Sangle;
                                tkbc_kite_update_internal(&tmp);
                                tkbc_tip_rotation(&tmp, &Spos, T, tip);
                                cu->cur_pos = tmp.center;
                                cu->cur_angle = tmp.angle;
                            }
                        } else if (s < Ni - 1) {
                            float Delta;
                            if (T == 0) {
                                Kite tmp = cu->geo;
                                tmp.old_angle = Sangle;
                                tmp.angle = Sangle;
                                Action tmpa = {0};
                                tmpa.as_tip_rotation.angle = T;
                                tmpa.as_tip_rotation.tip = tip;
                                Delta = tkbc_check_angle_zero(&tmp, ACTION_KITE_TIP_ROTATION, tmpa, O);
                            } else {
                                Delta = upscale_absolute_travel(Sangle, T);
                            }
                            float aj = Delta * (dur / O);
                            if (angle_only) {
                                Action a = {0};
                                a.as_rotation_add.angle = aj;
                                upscale_push_single_id_frame(&script->space, &slice, ACTION_KITE_ROTATION_ADD, a,
                                                             dur, kid);
                                cu->cur_angle += aj;
                            } else {
                                Action a = {0};
                                a.as_tip_rotation_add.angle = aj;
                                a.as_tip_rotation_add.tip = tip;
                                upscale_push_single_id_frame(&script->space, &slice, ACTION_KITE_TIP_ROTATION_ADD, a,
                                                             dur, kid);
                                Kite tmp = cu->geo;
                                tmp.center = cu->cur_pos;
                                tmp.angle = cu->cur_angle;
                                tmp.old_center = cu->cur_pos;
                                tmp.old_angle = cu->cur_angle;
                                tkbc_kite_update_internal(&tmp);
                                tkbc_tip_rotation(&tmp, NULL, tmp.angle + aj, tip);
                                cu->cur_pos = tmp.center;
                                cu->cur_angle = tmp.angle;
                            }
                        } else {
                            if (angle_only) {
                                Action a = {0};
                                a.as_rotation.angle = T;
                                upscale_push_single_id_frame(&script->space, &slice, ACTION_KITE_ROTATION, a, dur,
                                                             kid);
                                cu->cur_angle = T;
                            } else {
                                Action a = {0};
                                a.as_tip_rotation.angle = T;
                                a.as_tip_rotation.tip = tip;
                                upscale_push_single_id_frame(&script->space, &slice, ACTION_KITE_TIP_ROTATION, a, dur,
                                                             kid);
                                Kite tmp = cu->geo;
                                tmp.center = cu->cur_pos;
                                tmp.angle = cu->cur_angle;
                                tmp.old_center = Spos;
                                tmp.old_angle = Sangle;
                                tkbc_kite_update_internal(&tmp);
                                // Snap from the block start for exactness.
                                Kite snap = cu->geo;
                                snap.center = Spos;
                                snap.angle = Sangle;
                                snap.old_center = Spos;
                                snap.old_angle = Sangle;
                                tkbc_kite_update_internal(&snap);
                                tkbc_tip_rotation(&snap, &Spos, T, tip);
                                cu->cur_pos = snap.center;
                                cu->cur_angle = snap.angle;
                            }
                        }
                    } break;
                    default: break;
                    }
                }
            }

            slice.frames_index = upscaled.count;
            space_dap(&script->space, &upscaled, slice);
        }

        // Commit cursor ends to the chain map.
        for (size_t k = 0; k < involved_count; ++k) {
            Upscale_Kite *uk = upscale_find_kite(map, map_count, cursors[k].id);
            if (uk) {
                Vector2 p = cursors[k].cur_pos;
                float a = cursors[k].cur_angle;
                tkbc_center_rotation(&uk->kite, &p, a);
                uk->kite.old_center = uk->kite.center;
                uk->kite.old_angle = uk->kite.angle;
            }
        }
        free(cursors);
        free(involved);
    }

    // Replace the script body with the upscaled one, reindexed.
    script->elements = upscaled.elements;
    script->count = upscaled.count;
    script->capacity = upscaled.capacity;
    for (size_t i = 0; i < script->count; ++i) {
        script->elements[i].frames_index = i;
    }
    free(map);
}

/**
 * @brief Eagerly computes the true per-block start positions of a script.
 *
 * This is exactly the calculation that otherwise only happens implicitly
 * during the first full play (when each finished block overwrites the next
 * block's stored positions with the live kite state). Running it eagerly
 * means smooth scrubbing works immediately, with zero plays beforehand:
 * every block's kite_frame_positions becomes the deterministically simulated
 * end state of its predecessor, stepped with a fixed dt of TARGET_DT.
 *
 * Only stored positions are rewritten; actions, durations and the block
 * structure are untouched. Missing entries for involved kites are appended
 * (mirroring the patch helpers). Blocks that cannot involve kites
 * (WAIT/QUIT-only) keep their stored data as-is.
 *
 * @param env The global state, used for kite geometry templates.
 * @param script The script whose timeline gets baked in place.
 */
void tkbc_bake_script_timeline(Env *env, Script *script) {
    if (!env || !script || script->count == 0) {
        return;
    }
    const float dt = (float) TARGET_DT;

    Upscale_Kite *map = NULL;
    size_t map_count = 0;
    size_t map_cap = 0;

    for (size_t b = 0; b < script->count; ++b) {
        Frames *block = &script->elements[b];

        // Collect the kites this block moves.
        Id *involved = NULL;
        size_t involved_count = 0;
        size_t involved_cap = 0;
        for (size_t i = 0; i < block->count; ++i) {
            Frame *f = &block->elements[i];
            if (!upscale_frame_is_motion(f->kind)) {
                continue;
            }
            for (size_t j = 0; j < f->kite_id_array.count; ++j) {
                Id id = f->kite_id_array.elements[j];
                bool seen = false;
                for (size_t k = 0; k < involved_count; ++k) {
                    if (involved[k] == id) {
                        seen = true;
                        break;
                    }
                }
                if (!seen) {
                    if (involved_count >= involved_cap) {
                        size_t ncap = involved_cap ? involved_cap * 2 : 8;
                        Id *n = realloc(involved, ncap * sizeof(*n));
                        if (!n) {
                            abort();
                        }
                        involved = n;
                        involved_cap = ncap;
                    }
                    involved[involved_count++] = id;
                }
            }
        }

        for (size_t k = 0; k < involved_count; ++k) {
            upscale_ensure_kite(env, &map, &map_count, &map_cap, involved[k], block);
        }

        // Rewrite the stored starts from the simulated chain state, appending
        // entries for involved kites that have none yet.
        for (size_t i = 0; i < block->kite_frame_positions.count; ++i) {
            Kite_Position *kp = &block->kite_frame_positions.elements[i];
            Upscale_Kite *uk = upscale_find_kite(map, map_count, kp->kite_id);
            if (uk) {
                kp->position = uk->kite.center;
                kp->angle = uk->kite.angle;
            }
        }
        for (size_t k = 0; k < involved_count; ++k) {
            bool present = false;
            for (size_t i = 0; i < block->kite_frame_positions.count; ++i) {
                if (block->kite_frame_positions.elements[i].kite_id == involved[k]) {
                    present = true;
                    break;
                }
            }
            if (!present) {
                Upscale_Kite *uk = upscale_find_kite(map, map_count, involved[k]);
                if (uk) {
                    Kite_Position kp = {
                        .kite_id = involved[k],
                        .position = uk->kite.center,
                        .angle = uk->kite.angle,
                    };
                    space_dap(&script->space, &block->kite_frame_positions, kp);
                }
            }
        }

        // Old state equals the block start, like a live block entry.
        for (size_t k = 0; k < involved_count; ++k) {
            Upscale_Kite *uk = upscale_find_kite(map, map_count, involved[k]);
            if (uk) {
                uk->kite.old_center = uk->kite.center;
                uk->kite.old_angle = uk->kite.angle;
            }
        }

        if (block->count > 0) {
            // Simulate this block to completion on copies, so the script's
            // own runtime state (durations/finished) is never touched.
            Frame *tmp = malloc(block->count * sizeof(*tmp));
            if (!tmp) {
                abort();
            }
            for (size_t i = 0; i < block->count; ++i) {
                tmp[i] = block->elements[i];
                float orig = block->elements[i].original_duration;
                tmp[i].duration = orig > 0 ? orig : block->elements[i].duration;
                tmp[i].finished = false;
            }

            Kite_State *states = NULL;
            if (involved_count > 0) {
                states = malloc(involved_count * sizeof(*states));
                if (!states) {
                    abort();
                }
                for (size_t k = 0; k < involved_count; ++k) {
                    Upscale_Kite *uk = upscale_find_kite(map, map_count, involved[k]);
                    memset(&states[k], 0, sizeof(states[k]));
                    states[k].kite_id = involved[k];
                    states[k].kite = uk ? &uk->kite : NULL;
                }
            }

            Env tmpenv;
            memset(&tmpenv, 0, sizeof(tmpenv));
            tmpenv.kite_array.elements = states;
            tmpenv.kite_array.count = involved_count;
            tmpenv.kite_array.capacity = involved_count;
            Frames tmpblock = *block;
            tmpblock.elements = tmp;
            tmpenv.frames = &tmpblock;

            size_t guard = 0;
            for (;;) {
                for (size_t i = 0; i < tmpblock.count; ++i) {
                    if (!tmp[i].finished) {
                        tkbc_render_frame_with_dt(&tmpenv, &tmp[i], dt);
                    }
                }
                bool all = true;
                for (size_t i = 0; i < tmpblock.count; ++i) {
                    if (!tmp[i].finished) {
                        all = false;
                        break;
                    }
                }
                if (all) {
                    break;
                }
                if (++guard > 1000000) {
                    tkbc_fprintf(stderr, "WARNING", "Timeline bake: block %zu did not converge.\n", b);
                    break;
                }
            }

            free(tmp);
            free(states);
        }

        free(involved);
    }

    free(map);
}

/**
 * @brief This function adds a script to the global array located in the env.
 * It is needed to achieve stability for the raw frames and script pointers in
 * the env, they can be invalidated when the scripts array reallocates.
 *
 * @param env The global state of the application.
 * @param script The script to add.
 * @param evict_when_full When the scripts array reached the memory threshold
 * and this flag is true, the oldest script is evicted to keep the known scripts
 * bounded. Receiving scripts from the network have to pass false here, because
 * evicting them would make the client/server forget a script that it already
 * knows and the next time the script arrives it would be registered as new and
 * send/broadcast again.
 */
void tkbc_add_script(Env *env, Script script, bool evict_when_full) {
    UUID script_id = tkbc_uuid_nil();
    Index frames_index = 0;
    bool is_frames = false;
    bool is_script = false;

    if (env->frames) {
        is_frames = true;
        frames_index = env->frames->frames_index;
    }

    if (env->script) {
        is_script = true;
        script_id = env->script->id;
    }

    // This has to be twice as big as the default script amount to be able to remove the evict_when_full guard.
#define threshold_max_scripts_in_memory 10
    if (evict_when_full && env->scripts.count >= threshold_max_scripts_in_memory) {
        Script *first_script = &env->scripts.elements[0];
        // NOTE: this is actually slow because every other script just be moved
        // over in the array.
        tkbc_unload_script_from_memory(env, first_script->id);
    }

    // size_t bytes_count = tkbc_calculate_script_byte_size_allocated(script);

    {
        // NOTE: s_copy.space must survive the deep copy. tkbc_deep_copy_script
        // allocates into the passed Space but returns a Script with a zeroed
        // space field, so track allocations in a separate Space and move it
        // into the copy afterwards. Otherwise the assignment below would
        // discard the planet bookkeeping.
        Space s_space = {0};
        Space_Report report = {0};
        if (space_report_allocations(&env->scratch_buf_script.space, &report)) {
            space_init_capacity(&s_space, report.allocated_count);
        }
        Script s_copy = tkbc_deep_copy_script(&s_space, &script);
        s_copy.space = s_space;

        {
            s_copy.name_input.shadow_text = "";
            s_copy.name_input.key_constrained = NULL;
            s_copy.name_input.max_char = 255;
            s_copy.name_input.selection_start = SIZE_MAX;
            s_copy.name_input.is_active = false;
            s_copy.name_input.spacing = 2;
            s_copy.name_input.font = env->font;
            s_copy.name_input.text_color = TKBC_UI_BLACK;
            // The name buffer is already deep copied above into s_copy.space.
            if (!s_copy.name_input.text.elements) {
                tkbc_text_input_init(&s_copy.name_input, &s_copy.space, "");
            }
        }

        // Upscale long blocks into per-tick slices so the timeline can scrub
        // continuously at the animation fps, like a video. Already upscaled
        // blocks (durations <= 1/fps) are kept as-is, making this idempotent
        // for network re-receives.
        tkbc_upscale_script(env, &s_copy, (float) TARGET_FPS);

        // Eagerly bake the true timeline positions right away (this also runs
        // on the server for freshly received scripts): smooth scrubbing works
        // immediately without playing the script one time first.
        tkbc_bake_script_timeline(env, &s_copy);

        space_dap(&env->_scripts_space, &env->scripts, s_copy);

        // Rest the scratch buffers they got invalidated by resetting the space.
        memset(&env->scratch_buf_frames, 0, sizeof(env->scratch_buf_frames));
        // Rest only the rest of the fields and not the space inside of the
        // scratch_buf_script script to preserve memory for reuse.
        {
            space_reset_space(&env->scratch_buf_script.space);
            // Rest only the rest of the fields and not the space inside of the
            // scratch_buf_script script to preserve memory for reuse.
            Space saved_space = env->scratch_buf_script.space;
            memset(&env->scratch_buf_script, 0, sizeof(env->scratch_buf_script));
            env->scratch_buf_script.space = saved_space;
        }
    }

    if (is_script) {
        if (!tkbc_load_script_id(env, script_id, false)) {
            return;
        }
    }

    if (is_frames) {
        // The env->script pointer is now valid again, so we can use it.
        for (size_t i = 0; i < env->script->count; ++i) {
            if (env->script->elements[i].frames_index == frames_index) {
                env->frames = &env->script->elements[i];
                break;
            }
        }
    }
}

/**
 * @brief The function checks for the user input that is related to a script
 * execution. It can control the timeline and stop and start the execution.
 *
 * @param env The global state of the application.
 */
void tkbc_input_handler_script(Env *env) {
    // Offline (or standalone) mode: there is no server to inform about
    // script deletions, so drop pending delete requests. The local deletion
    // already happened in the script menu.
    env->pending_script_deletes.count = 0;

    // Hard reset to startposition angel 0
    // KEY_ENTER
    if (tkbc_check_keymaps_full(env->keymaps, KMH_SET_KITES_TO_START_POSITION, KEY_MAP_CHECK_KEY_PRESSED)) {
        tkbc_kite_array_start_position(env, &env->kite_array, env->window_width, env->window_height, true);
    }

    // KEY_SPACE
    if (tkbc_check_keymaps_full(env->keymaps, KMH_TOGGLE_SCRIPT_EXECUTION, KEY_MAP_CHECK_KEY_PRESSED)) {
        tkbc_toggle_script_execution(env);
    }

    // This guard just prevent it for one frame.
    // But it can't be blocked for longer because the user may actually want to
    // scrub that fast.
    // This is a bit of a bad design for trackpads but there
    // is no way around that.
    if (!env->script_loading) {
        tkbc_scrub_frames(env);
    }

    // TODO: Change condition to something like UI interaction.
    if (!env->keymaps_interaction && !env->script_menu_interaction && !env->colorizer) {
        env->script_loading = false;
    }
}

/**
 * @brief The function can be used to update all the kites positions and angle
 * that are registered in the currently loaded frame of the script before.
 *
 * @param env The global state of the application.
 */
void tkbc_set_kite_positions_from_kite_frames_positions(Env *env) {
    // TODO: Think about kites that are move in the previous frame but not in
    // the current one. The kites can end up in wired locations, because the
    // state is not exactly as if the script has executed from the beginning.
    assert(env->frames);
    for (size_t i = 0; i < env->frames->kite_frame_positions.count; ++i) {
        Id id = env->frames->kite_frame_positions.elements[i].kite_id;
        Kite *kite = tkbc_get_kite_by_id(env, id);
        assert(kite != NULL && "Unexpected data lose.");

        Vector2 position = env->frames->kite_frame_positions.elements[i].position;
        float angle = env->frames->kite_frame_positions.elements[i].angle;

        tkbc_center_rotation(kite, &position, angle);

        // For the correct recomputation of the action where the slider is set to.
        kite->old_angle = kite->angle;
        kite->old_center = kite->center;
    }
}

/**
 * @brief This function moves to the prev/next frame of a script and
 * recalculates the script_frames_positions
 *
 * @param env The global state of the application.
 * @param drag_left True, if the script should be moved forward, otherwise
 * false.
 */
void tkbc_execute_scrub_slide(Env *env, bool drag_left) {
    if (!env->script || !env->frames) {
        return;
    }
    size_t current = env->frames->frames_index;
    size_t target = current;
    // The indexes are assumed in order and at the corresponding index.
    // This is needed to avoid a down cast of size_t to long or int that can
    // hold ever value of size_t.
    if (drag_left) {
        if (current > 0) {
            target = current - 1;
        }
    } else {
        if (current + 1 < env->script->count) {
            target = current + 1;
        }
    }

    tkbc_scrub_to_index(env, target);
}

/**
 * @brief Jumps the timeline to an absolute block index (video-like scrub).
 *
 * Every scrub position is a slice start with correct remaining durations, so
 * resuming playback continues smoothly from there.
 *
 * @param env The global state of the application.
 * @param target_index The frames_index to jump to, clamped into range.
 */
void tkbc_scrub_to_index(Env *env, size_t target_index) {
    if (!env->script || !env->frames) {
        return;
    }
    if (env->script->count == 0) {
        return;
    }
    if (target_index >= env->script->count) {
        target_index = env->script->count - 1;
    }
    env->script_finished = true;
    env->frames = &env->script->elements[target_index];

    tkbc_restore_script_frame_states(env);
    tkbc_set_kite_positions_from_kite_frames_positions(env);
}

/**
 * @brief Toggles script playback between paused and playing.
 *
 * The toggle is clamped to the loaded script: resuming at the final slice
 * just plays out that slice and pauses again at the end, resuming at the
 * first slice just plays forward. It never wraps around and never leaves
 * script mode; only an explicit NO SCRIPT terminates execution.
 *
 * @param env The global state of the application.
 */
void tkbc_toggle_script_execution(Env *env) {
    if (!env->frames) {
        return;
    }
    env->script_finished = !env->script_finished;
}

/**
 * @brief Restores all frame durations and finished flags across the entire
 * script so that a replay from the beginning uses the original timing.
 *
 * @param env The global state of the application.
 */
void tkbc_restore_script_frame_states(Env *env) {
    if (!env->script) {
        return;
    }

    for (size_t i = 0; i < env->script->count; ++i) {
        Frames *frames = &env->script->elements[i];
        for (size_t j = 0; j < frames->count; ++j) {
            frames->elements[j].duration = frames->elements[j].original_duration;
            frames->elements[j].finished = false;
        }
    }

    env->global_quit.is_script_quit = false;
    env->global_quit.script_quit_duration = 0;
}

/**
 * @brief The function computes the frame state of the timeline and syncs up
 * the currently loaded frame. It computes mouse control of the timeline.
 *
 * @param env The global state of the application.
 */
void tkbc_scrub_frames(Env *env) {
    if (env->script == NULL) {
        return;
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && env->timeline_interaction) {
        int mouse_x = GetMouseX();
        // Video-like absolute scrub: map the mouse X onto the upscaled
        // per-tick timeline instead of stepping a single block per event.
        // With hundreds of slices the old stepwise slide would take seconds
        // to traverse the script.
        if (env->timeline_base.width > 0 && env->script->count > 0) {
            float t = ((float) mouse_x - env->timeline_base.x) / env->timeline_base.width;
            if (t < 0) {
                t = 0;
            }
            if (t > 1) {
                t = 1;
            }
            size_t target = (size_t) (t * (float) env->script->count);
            if (target >= env->script->count) {
                target = env->script->count - 1;
            }
            if (target != env->frames->frames_index) {
                tkbc_scrub_to_index(env, target);
            }
        } else {
            float slider = env->timeline_front.x + env->timeline_front.width;
            float c = mouse_x - slider;
            bool drag_left = c <= 0;

            tkbc_execute_scrub_slide(env, drag_left);
        }
    }
}

// ========================== SCRIPT HANDLER INTERNAL ========================

/**
 * @brief The function handles the computation of the new position of the kite
 * corresponding to the called move action.
 *
 * @param kite The kite where the new position is calculated for.
 * @param position The new position of the kite.
 * @param duration The time it should take to interpolate the kite to the new
 * position.
 * @return The amount that the center position has moved to the final
 * position.
 */
Vector2 tkbc_script_move_with_dt(Kite *kite, Vector2 position, float duration, float dt) {
    if (duration <= 0) {
        Vector2 result = Vector2Subtract(position, kite->center);
        tkbc_kite_update_position(kite, &position);
        return result;
    }

    if (Vector2Equals(kite->center, position)) {
        // NOTE:This might be just (0,0), because the  precision is not needed?
        Vector2 result = Vector2Subtract(position, kite->center);
        tkbc_kite_update_position(kite, &position);
        return result;
    }

    Vector2 d = Vector2Subtract(position, kite->old_center);
    Vector2 dnorm = Vector2Normalize(d);
    Vector2 dnormscale = Vector2Scale(dnorm, (Vector2Length(d) / duration * dt));

    if (Vector2Length(dnormscale) >= Vector2Length(Vector2Subtract(position, kite->center))) {
        tkbc_kite_update_position(kite, &position);
        return Vector2Subtract(position, kite->center);
    } else {
        Vector2 it = Vector2Add(kite->center, dnormscale);
        tkbc_kite_update_position(kite, &it);
        return dnormscale;
    }
}

/**
 * @brief The function handles the computation of the new rotation of the kite
 * corresponding to the called rotation action.
 *
 * @param env The global state of the application.
 * @param kite The kite that should be handled.
 * @param angle The new angle the kite should be rotated to or by depended on
 * the adding parameter.
 * @param duration The time it should take to interpolate the kite to the new
 * angle.
 * @param adding The parameter changes if the angle value is going to be added
 * to the kite or if the kite angle should change to the given angle.
 * @return The delta amount the kite angle changes.
 */
float tkbc_script_rotate_with_dt(Kite *kite, float angle, float duration, bool adding, float dt) {

    // NOTE: For the instant rotation the computation can be simpler by just
    // calling the direction angles, but for future line wrap calculation the
    // actual rotation direction is called instead.
    if (duration <= 0) {
        if (adding) {
            tkbc_kite_update_angle(kite, kite->old_angle + angle);
        } else {
            tkbc_kite_update_angle(kite, angle);
        }
        return fabsf(angle);
    }

    float d = fabsf(angle);
    float ds = d / duration * dt;

    if (ds >= fabsf(kite->old_angle) + d) {
        if (adding) {
            tkbc_kite_update_angle(kite, kite->old_angle + angle);
        } else {
            tkbc_kite_update_angle(kite, angle);
        }
        return fabsf(ds);
    }
    // NOTE: For the non adding version the finish detection will stop the
    // calculation at the correct point so the angle computation is not needed
    // her.
    if (signbit(angle) != 0) {
        tkbc_kite_update_angle(kite, kite->angle - ds);
    } else {
        tkbc_kite_update_angle(kite, kite->angle + ds);
    }
    return fabsf(ds);
}

/**
 * @brief The function handles the computation of the new tip rotation of the
 * kite corresponding to the called tip rotation action.
 *
 * @param env The global state of the application.
 * @param kite The kite that should be handled.
 * @param tip The tip of the leading kites edge.
 * @param angle The new angle the kite should be rotated to or by depended on
 * the adding parameter.
 * @param duration The time it should take to interpolate the kite to the new
 * angle.
 * @param adding The parameter changes if the angle value is going to be added
 * to the kite or if the kite angle should change to the given angle.
 * @return The delta amount the kite angle changes.
 */
float tkbc_script_rotate_tip_with_dt(Kite *kite, TIP tip, float angle, float duration, bool adding, float dt) {

    // NOTE: For the instant rotation the computation can be simpler by just
    // calling the direction angles, but for future line wrap calculation the
    // actual rotation direction is called instead.
    if (duration <= 0) {
        if (adding) {
            tkbc_tip_rotation(kite, NULL, kite->old_angle + angle, tip);
        } else {
            tkbc_tip_rotation(kite, NULL, angle, tip);
        }
        return fabsf(angle);
    }

    float d = fabsf(angle);
    float ds = d / duration * dt;

    if (ds >= fabsf(kite->old_angle) + d) {
        // Provided the old position, because the kite center moves as a circle
        // around the old fixed position.
        if (adding) {
            tkbc_tip_rotation(kite, &kite->old_center, kite->old_angle + angle, tip);
        } else {
            tkbc_tip_rotation(kite, &kite->old_center, angle, tip);
        }
        return fabsf(ds);
    }
    // NOTE: For the non adding version the finish detection will stop the
    // calculation at the correct point so the angle computation is not needed
    // her.
    if (signbit(angle) != 0) {
        tkbc_tip_rotation(kite, NULL, kite->angle - ds, tip);
    } else {
        tkbc_tip_rotation(kite, NULL, kite->angle + ds, tip);
    }
    return fabsf(ds);
}

Vector2 tkbc_script_move(Kite *kite, Vector2 position, float duration) {
    return tkbc_script_move_with_dt(kite, position, duration, tkbc_get_frame_time());
}

float tkbc_script_rotate(Kite *kite, float angle, float duration, bool adding) {
    return tkbc_script_rotate_with_dt(kite, angle, duration, adding, tkbc_get_frame_time());
}

float tkbc_script_rotate_tip(Kite *kite, TIP tip, float angle, float duration, bool adding) {
    return tkbc_script_rotate_tip_with_dt(kite, tip, angle, duration, adding, tkbc_get_frame_time());
}

/**
 * @brief The function resolves a zero angle to a concrete rotation value. This
 * is needed to determine the actual rotation direction when a frame with a
 * duration and an angle of zero is used for a kite rotation.
 *
 * @param kite The kite that is going to be rotated.
 * @param kind The kind of the rotation action.
 * @param action The action that contains the rotation to resolve.
 * @param duration The duration of the rotation frame.
 * @return The resolved rotation angle that should be used for the kite.
 */
float tkbc_check_angle_zero(Kite *kite, Action_Kind kind, Action action, float duration) {
    switch (kind) {
    case ACTION_KITE_ROTATION:
    case ACTION_KITE_ROTATION_ADD:
        if (action.as_rotation.angle != 0 || duration <= 0) {
            return action.as_rotation.angle;
        }

        if (signbit(action.as_rotation.angle) == 0) {
            /* Positive 0 */
            if (kite->old_angle < 0) {
                /* Negative angle 0 = -90 +90 */
                return fabsf(fmodf(kite->old_angle, 360));
            } else {
                /* Positive angle 0 = 90 +270 ->  270 = + 360 - (90) */
                /* The case where the old angle is already 0 is handled by the outer
                 * mod. */
                return fmodf(360 - fmodf(kite->old_angle, 360), 360);
            }
        } else {
            /* Negative 0 */
            if (kite->old_angle < 0) {
                /* Negative angle 0 = -90 -270 -> -270 = -360 + 90 = -(360 - (90))
                 */
                /* The case where the old angle is already 0 is handled by the outer
                 * mod. */
                return -fmodf(360 - fmodf(kite->old_angle, 360), 360);

            } else {
                /* Positive angle 0 = 90 -90 */
                return -fabsf(fmodf(kite->old_angle, 360));
            }
        }

    case ACTION_KITE_TIP_ROTATION:
    case ACTION_KITE_TIP_ROTATION_ADD:
        if (action.as_tip_rotation.angle != 0 || duration <= 0) {
            return action.as_tip_rotation.angle;
        }

        if (signbit(action.as_tip_rotation.angle) == 0) {
            if (kite->old_angle < 0) {
                return fabsf(fmodf(kite->old_angle, 360));
            } else {
                return fmodf(360 - fmodf(kite->old_angle, 360), 360);
            }
        } else {
            if (kite->old_angle < 0) {
                return -fmodf(360 - fmodf(kite->old_angle, 360), 360);
            } else {
                return -fabsf(fmodf(kite->old_angle, 360));
            }
        }
    default: assert(0 && "UNREACHABLE tkbc_check_angle_zero()");
    }

    return 0;
}
