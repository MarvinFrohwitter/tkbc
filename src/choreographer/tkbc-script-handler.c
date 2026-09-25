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
    new_frames.is_upscaled = frames->is_upscaled;

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

    if (script->original_elements && script->original_count > 0) {
        size_t n = script->original_count;
        Frames *new_non_upscaled = space_malloc(space, n * sizeof(*new_non_upscaled));
        if (!new_non_upscaled) {
            return (Script){0};
        }

        for (size_t i = 0; i < n; ++i) {
            Frames frames = tkbc_deep_copy_frames(space, &script->original_elements[i]);
            new_non_upscaled[i] = frames;
            // Not possible because alternative names of the dynamic array.
            // space_dap(space, &new_script.original_elements, frames);
        }

        new_script.original_elements = new_non_upscaled;
        new_script.original_count = n;
        new_script.original_capacity = n;
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

/**
 * @brief The function sets the kite_ids in a given script to new values
 * provided in the kite_ids array passed into the function.
 *
 * @param env The global state of the application.
 * @param script The script where the kite ids should be remapped to new values.
 * @param kite_ids The ids array that contain the new values.
 */
/**
 * @brief Rewrites kite ids inside a single frames block via the collected mapping.
 *
 * @param frames The block whose frame id arrays and stored positions get remapped.
 * @param current_kite_ids The distinct ids found in the script, in order.
 * @param kite_ids The replacement ids in the same order.
 */
void tkbc_remap_frames_kite_ids(Frames *frames, Kite_Ids current_kite_ids, Kite_Ids kite_ids) {
    assert(frames);
    assert(frames->elements || frames->count == 0);
    for (size_t new_id = 0; new_id < current_kite_ids.count; ++new_id) {
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

void tkbc_collect_script_kite_ids(Space *space, const Script *script, Kite_Ids *current_kite_ids) {
    for (size_t i = 0; i < script->count; ++i) {
        tkbc_collect_frames_kite_ids(space, &script->elements[i], current_kite_ids);
    }
}

void tkbc_collect_frames_kite_ids(Space *space, const Frames *frames, Kite_Ids *current_kite_ids) {
    // If the positions are available than it is faster to iterate the than looking into every frame individually. An
    // every kite id is part of the kite_frame_positions  if their are available.
    //
    // This assumes that every id is also part of the positions array => that is currently the case.
    if (frames->kite_frame_positions.elements) {
        for (size_t j = 0; j < frames->kite_frame_positions.count; ++j) {
            Id id = frames->kite_frame_positions.elements[j].kite_id;
            if (!tkbc_contains_id(*current_kite_ids, id)) {
                space_dap(space, current_kite_ids, id);
            }
        }
    } else {
        for (size_t j = 0; j < frames->count; ++j) {
            const Frame *frame = &frames->elements[j];
            for (size_t k = 0; k < frame->kite_id_array.count; ++k) {
                Kite_Ids ids = frame->kite_id_array;
                Id id = ids.elements[k];
                if (!tkbc_contains_id(*current_kite_ids, id)) {
                    space_dap(space, current_kite_ids, id);
                }
            }
        }
    }
}

void tkbc_remap_script_kite_id_arrays_to_kite_ids(Script *script, Kite_Ids current_kite_ids, Kite_Ids kite_ids) {
    assert(script);
    assert(script->count > 0);
    assert(current_kite_ids.count == kite_ids.count);
    assert(kite_ids.count > 0);
    for (size_t i = 0; i < script->count; ++i) {
        assert(script->elements);
        tkbc_remap_frames_kite_ids(&script->elements[i], current_kite_ids, kite_ids);
    }
    for (size_t i = 0; i < script->original_count; ++i) {
        assert(script->original_elements);
        tkbc_remap_frames_kite_ids(&script->original_elements[i], current_kite_ids, kite_ids);
    }
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
    // And use a space that is provided as a tspace this has to be passed in because we don't know if getting a tspace
    // and then releasing it will cause other data loss.

    Space space = {0};
    Kite_Ids current_kite_ids = {0};
    tkbc_collect_script_kite_ids(&space, script, &current_kite_ids);

    //
    // Activate the kites that belong to the script.
    for (size_t i = 0; i < env->kite_array.count; ++i) {
        env->kite_array.elements[i].is_active = false;
        env->kite_array.elements[i].is_kite_input_handler_active = false;

        for (size_t j = 0; j < current_kite_ids.count; ++j) {
            if (current_kite_ids.elements[j] == env->kite_array.elements[i].kite_id) {
                env->kite_array.elements[i].is_active = true;
                break;
            }
        }
    }

    space_free_space(&space);
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
            // TODO: Remove when the client in offline mode also generates kites for each script separately.
#ifdef TKBC_SERVER
            {  // Remove the kites that are used from the kite array.
                Kite_Ids current_kite_ids = {0};
                // Use the script space one last time.
                tkbc_collect_script_kite_ids(&env->scripts.elements[i].space, &env->scripts.elements[i],
                                             &current_kite_ids);
                for (size_t i = 0; i < current_kite_ids.count; ++i) {
                    tkbc_remove_kite_from_list(&env->kite_array, current_kite_ids.elements[i]);
                }
            }
#endif

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

/**
 * @brief Resets the scratch buffers used during script creation.
 *
 * Zeroes scratch_buf_frames and the scratch_buf_script struct while preserving
 * the scratch_buf_script.space allocator for reuse. Called after a script was
 * added to env->scripts and when adding is skipped because the script id is
 * already known, so the next script build starts clean.
 *
 * @param env The global state of the application.
 */
void tkbc_reset_script_scratch_creation(Env *env) {
    // Rest the scratch buffers they get invalidated by resetting the space.
    memset(&env->scratch_buf_frames, 0, sizeof(env->scratch_buf_frames));
    {
        space_reset_space(&env->scratch_buf_script.space);
        // Rest only the rest of the fields and not the space inside of the
        // scratch_buf_script script to preserve memory for reuse.
        Space saved_space = env->scratch_buf_script.space;
        memset(&env->scratch_buf_script, 0, sizeof(env->scratch_buf_script));
        env->scratch_buf_script.space = saved_space;
    }
}

/**
 * @brief This function adds a script to the global array located in the env.
 * It is needed to achieve stability for the raw frames and script pointers in
 * the env, they can be invalidated when the scripts array reallocates.
 *
 * A script whose id is already known is ignored instead of added again.
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
    // A script that is already known must not be added a second time.
    // Drop the scratch copy (reset like below) so the next build starts clean.
    if (tkbc_scripts_contains_id(env->scripts, script.id)) {
        tkbc_reset_script_scratch_creation(env);
        return;
    }

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
        tkbc_reset_script_scratch_creation(env);
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
 * @param drag_left True, if the script should be moved back, otherwise
 * false.
 */
void tkbc_execute_scrub_slide(Env *env, bool drag_left) {
    if (!env->script || !env->frames) {
        return;
    }
    size_t current = env->frames->frames_index;
    // The indexes are assumed in order and at the corresponding index.
    // This is needed to avoid a down cast of size_t to long or int that can
    // hold ever value of size_t.
    if (drag_left) {
        if (current > 0) {
            current -= 1;
        }
    } else {
        if (current + 1 < env->script->count) {
            current += 1;
        }
    }

    tkbc_scrub_to_index(env, current);
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
        if (env->script->count > 0 && env->timeline_base.width > 0) {
            float t = ((float) mouse_x - env->timeline_base.x) / env->timeline_base.width;
            t = tkbc_clamp(t, 0, 1);
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
