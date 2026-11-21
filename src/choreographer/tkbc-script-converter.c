#include "tkbc-script-converter.h"
#include "../../external/orbit/orbit.h"
#include "../global/tkbc-types.h"
#include "../global/tkbc-utils.h"
#include "tkbc-script-api.h"
#include "tkbc-script-handler.h"
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief The function prints the serialized form of the kite ids.
 *
 * Ids are serialized as file-local zero based indices into the global
 * distinct id list, so exported scripts stay portable even if the in memory
 * kite ids are higher than a fresh session knows (e.g. after networking or
 * previous script loads). The parser resolves them back to env kites.
 *
 * @param tspace The temporary space used for the output buffer allocations.
 * @param buffer The buffer where to print the kites.
 * @param ids The kite ids of a single frame that should be serialized.
 * @param all_ids Maps every distinct kite id of the whole script to its
 * file-local index (assigned in first appearance order).
 */
void tkbc_print_kites(Space *tspace, Content *buffer, Kite_Ids ids, Orbit_HT(Id, Id) * all_ids) {
    space_dapf(tspace, buffer, "(");
    for (size_t i = 0; i < ids.count; ++i) {
        if (i > 0) {
            space_dapf(tspace, buffer, " ");
        }
        Id *index = orbit_get(all_ids, ids.elements[i]);
        // Every id of a frame is collected before serialization, so a lookup
        // must succeed. Falling back to 0 keeps the output well formed.
        space_dapf(tspace, buffer, "%zu", index ? *index : 0);
    }
    space_dapf(tspace, buffer, ")");
}

static inline void *alloc_space_callback(void *alloc_data, size_t size) {
    return space_malloc(alloc_data, size);
}

/**
 * @brief The function serializes the provided script in memory form to a .kite
 * file.
 *
 * @param tspace The temporary space used for the serialization allocations.
 * @param script A memory representation of a script.
 * @param filepath The file path where the script should be saved to.
 * @return 0 If the saving and serialization of the file has succeeded. 1 if
 * writing of the header failed and -1 if the writing of the main program has
 * failed.
 */
int tkbc_export_script_to_dot_kite_file_from_mem(Space *tspace, Script *script, const char *filepath) {
    int ok = 0;
    Orbit_HT(Id, Id) ids = {
        .init_size = 16,
        .allocator.alloc = alloc_space_callback,
        .allocator.alloc_data = tspace,
    };
    Content out = {0};

    tkbc_redirect_script_elements(script);

    // Download writes the original non-upscaled script: the live timeline
    // may hold hundreds of per-tick slices, but the file keeps the authored
    // blocks they were baked from.

    // Collect all distinct kite ids first so every frame can be serialized
    // as file-local indices into this list.
    for (size_t frames = 0; frames < script->original_count; ++frames) {
        for (size_t frame = 0; frame < script->original_elements[frames].count; ++frame) {
            Frame *f = &script->original_elements[frames].elements[frame];
            for (size_t i = 0; i < f->kite_id_array.count; ++i) {
                Id id = f->kite_id_array.elements[i];
                if (NULL == orbit_get(&ids, id)) {
                    // The value is the file-local index in first appearance
                    // order; the hash table iteration order is unspecified.
                    orbit_put(&ids, id, ids.count);
                }
            }
        }
    }

    space_dapf(tspace, &out, "BEGIN\n");
    for (size_t frames = 0; frames < script->original_count; ++frames) {

        if (script->original_elements[frames].count > 1) {
            space_dapf(tspace, &out, "{\n");
        }

        for (size_t frame = 0; frame < script->original_elements[frames].count; ++frame) {
            Frame *f = &script->original_elements[frames].elements[frame];

            switch (f->kind) {
            case ACTION_KITE_QUIT: {
                space_dapf(tspace, &out, "QUIT");
            } break;

            case ACTION_KITE_WAIT: {
                space_dapf(tspace, &out, "WAIT");
            } break;

            case ACTION_KITE_MOVE: {
                Move_Action action = f->action.as_move;
                space_dapf(tspace, &out, "MOVE ");
                tkbc_print_kites(tspace, &out, f->kite_id_array, (void *) &ids);
                space_dapf(tspace, &out, " %G %G", action.position.x, action.position.y);

            } break;

            case ACTION_KITE_MOVE_ADD: {
                Move_Add_Action action = f->action.as_move_add;
                space_dapf(tspace, &out, "MOVE_ADD ");
                tkbc_print_kites(tspace, &out, f->kite_id_array, (void *) &ids);
                space_dapf(tspace, &out, " %G %G", action.position.x, action.position.y);

            } break;

            case ACTION_KITE_ROTATION: {
                Rotation_Action action = f->action.as_rotation;
                space_dapf(tspace, &out, "ROTATION ");
                tkbc_print_kites(tspace, &out, f->kite_id_array, (void *) &ids);
                space_dapf(tspace, &out, " %G", action.angle);

            } break;

            case ACTION_KITE_ROTATION_ADD: {
                Rotation_Add_Action action = f->action.as_rotation_add;
                space_dapf(tspace, &out, "ROTATION_ADD ");
                tkbc_print_kites(tspace, &out, f->kite_id_array, (void *) &ids);
                space_dapf(tspace, &out, " %G", action.angle);

            } break;

            case ACTION_KITE_TIP_ROTATION: {
                Tip_Rotation_Action action = f->action.as_tip_rotation;
                space_dapf(tspace, &out, "TIP_ROTATION ");
                tkbc_print_kites(tspace, &out, f->kite_id_array, (void *) &ids);
                space_dapf(tspace, &out, " %G %s", action.angle, action.tip == LEFT_TIP ? "LEFT" : "RIGHT");

            } break;

            case ACTION_KITE_TIP_ROTATION_ADD: {
                Tip_Rotation_Add_Action action = f->action.as_tip_rotation_add;
                space_dapf(tspace, &out, "TIP_ROTATION_ADD ");
                tkbc_print_kites(tspace, &out, f->kite_id_array, (void *) &ids);
                space_dapf(tspace, &out, " %G %s", action.angle, action.tip == LEFT_TIP ? "LEFT" : "RIGHT");

            } break;

            default: assert(0 && "UNREACHABLE tkbc_export_script_to_dot_kite_file_from_mem");
            }

            space_dapf(tspace, &out, " %G", f->duration);
            space_dapf(tspace, &out, "\n");
        }

        if (script->original_elements[frames].count > 1) {
            space_dapf(tspace, &out, "}\n");
        }
    }
    space_dapf(tspace, &out, "END\n");

    char buf[32];
    snprintf(buf, sizeof(buf), "KITES %zu\n", ids.count);
    int err = tkbc_write_file(filepath, buf, strlen(buf));
    if (err) {
        check_return(-err);
    }
    err = tkbc_append_file(filepath, out.elements, out.count);
    check_return(err);

check:

    tkbc_remove_redirect_script_elements(script);
    return ok;
}

/**
 * @brief The function serializes all the scripts from memory to a .kite file
 * with the corresponding script name, that is specified at the script
 * declaration time or it will get a custom generated name with an id, if no
 * name was specified.
 *
 * @param env The global state of the application.
 * @param path The path where the scripts should be exported to.
 * @return 0 If the saving and serialization of all the files has succeeded. 1
 * if the header of the first failing script could not be written, and -1 if
 * the main program of the first failing script could not be written.
 * The first initial KITES count in the script is considered to be the header.
 */
int tkbc_export_all_scripts_to_dot_kite_file_from_mem(Env *env, const char *path) {
    tkbc_make_dir_recursive_if_not_existis(path);

    int err = 0;
    for (size_t i = 0; i < env->scripts.count; ++i) {
        assert(tkbc_script_name(&env->scripts.elements[i])[0] != '\0');
        space_reset_tspace();
        const char *buf = space_tprintf("%s%s.kite", path, tkbc_script_name(&env->scripts.elements[i]));
        err = tkbc_export_script_to_dot_kite_file_from_mem(space_get_tspace(), &env->scripts.elements[i], buf);

        if (err) {
            break;
        }
    }
    space_reset_tspace();

    return err;
}
