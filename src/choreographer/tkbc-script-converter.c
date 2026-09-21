#include "tkbc-script-converter.h"
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
 * @param buffer The buffer where to print the kites.
 * @param ids The kite ids of a single frame that should be serialized.
 * @param all_ids The distinct kite ids of the whole script in first
 * appearance order, used for the index mapping.
 */
void tkbc_print_kites(Content *buffer, Kite_Ids ids, Kite_Ids all_ids) {
    if (ids.count > 0) {
        size_t index = 0;
        for (; index < all_ids.count; ++index) {
            if (all_ids.elements[index] == ids.elements[0]) {
                break;
            }
        }
        tkbc_dapf(buffer, "(%zu", index);
        for (size_t id = 1; id < ids.count; ++id) {
            index = 0;
            for (; index < all_ids.count; ++index) {
                if (all_ids.elements[index] == ids.elements[id]) {
                    break;
                }
            }
            tkbc_dapf(buffer, " %zu", index);
        }
    } else {
        tkbc_dapf(buffer, "(");
    }
    tkbc_dapf(buffer, ")");
}

/**
 * @brief The function serializes the provided script in memory form to a .kite
 * file.
 *
 * @param script A memory representation of a script.
 * @param filepath The file path where the script should be saved to.
 * @return 0 If the saving and serialization of the file has succeeded. 1 if
 * writing of the header failed and -1 if the writing of the main program has
 * failed.
 */
int tkbc_export_script_to_dot_kite_file_from_mem(Script *script, const char *filepath) {
    int ok = 0;
    Kite_Ids ids = {0};
    Content out = {0};

    // Collect all distinct kite ids first so every frame can be serialized
    // as file-local indices into this list.
    // TODO: Use hash function for this.
    for (size_t frames = 0; frames < script->count; ++frames) {
        for (size_t frame = 0; frame < script->elements[frames].count; ++frame) {
            Frame *f = &script->elements[frames].elements[frame];
            for (size_t i = 0; i < f->kite_id_array.count; ++i) {
                Id id = f->kite_id_array.elements[i];
                if (!tkbc_contains_id(ids, id)) {
                    tkbc_dap(&ids, id);
                }
            }
        }
    }

    tkbc_dapf(&out, "BEGIN\n");
    for (size_t frames = 0; frames < script->count; ++frames) {

        if (script->elements[frames].count > 1) {
            tkbc_dapf(&out, "{\n");
        }

        for (size_t frame = 0; frame < script->elements[frames].count; ++frame) {
            Frame *f = &script->elements[frames].elements[frame];

            switch (f->kind) {
            case ACTION_KITE_QUIT: {
                tkbc_dapf(&out, "QUIT");
            } break;

            case ACTION_KITE_WAIT: {
                tkbc_dapf(&out, "WAIT");
            } break;

            case ACTION_KITE_MOVE: {
                Move_Action action = f->action.as_move;
                tkbc_dapf(&out, "MOVE ");
                tkbc_print_kites(&out, f->kite_id_array, ids);
                tkbc_dapf(&out, " %G %G", action.position.x, action.position.y);

            } break;

            case ACTION_KITE_MOVE_ADD: {
                Move_Add_Action action = f->action.as_move_add;
                tkbc_dapf(&out, "MOVE_ADD ");
                tkbc_print_kites(&out, f->kite_id_array, ids);
                tkbc_dapf(&out, " %G %G", action.position.x, action.position.y);

            } break;

            case ACTION_KITE_ROTATION: {
                Rotation_Action action = f->action.as_rotation;
                tkbc_dapf(&out, "ROTATION ");
                tkbc_print_kites(&out, f->kite_id_array, ids);
                tkbc_dapf(&out, " %G", action.angle);

            } break;

            case ACTION_KITE_ROTATION_ADD: {
                Rotation_Add_Action action = f->action.as_rotation_add;
                tkbc_dapf(&out, "ROTATION_ADD ");
                tkbc_print_kites(&out, f->kite_id_array, ids);
                tkbc_dapf(&out, " %G", action.angle);

            } break;

            case ACTION_KITE_TIP_ROTATION: {
                Tip_Rotation_Action action = f->action.as_tip_rotation;
                tkbc_dapf(&out, "TIP_ROTATION ");
                tkbc_print_kites(&out, f->kite_id_array, ids);
                tkbc_dapf(&out, " %G %s", action.angle, action.tip == LEFT_TIP ? "LEFT" : "RIGHT");

            } break;

            case ACTION_KITE_TIP_ROTATION_ADD: {
                Tip_Rotation_Add_Action action = f->action.as_tip_rotation_add;
                tkbc_dapf(&out, "TIP_ROTATION_ADD ");
                tkbc_print_kites(&out, f->kite_id_array, ids);
                tkbc_dapf(&out, " %G %s", action.angle, action.tip == LEFT_TIP ? "LEFT" : "RIGHT");

            } break;

            default: assert(0 && "UNREACHABLE tkbc_export_script_to_dot_kite_file_from_mem");
            }

            tkbc_dapf(&out, " %G", f->duration);
            tkbc_dapf(&out, "\n");
        }

        if (script->elements[frames].count > 1) {
            tkbc_dapf(&out, "}\n");
        }
    }
    tkbc_dapf(&out, "END\n");

    char buf[32];
    snprintf(buf, sizeof(buf), "KITES %zu\n", ids.count);
    int err = tkbc_write_file(filepath, buf, strlen(buf));
    if (err) {
        check_return(-err);
    }
    err = tkbc_append_file(filepath, out.elements, out.count);
    check_return(err);

check:
    free(out.elements);
    free(ids.elements);
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
        err = tkbc_export_script_to_dot_kite_file_from_mem(&env->scripts.elements[i], buf);

        if (err) {
            break;
        }
    }
    space_reset_tspace();

    return err;
}
