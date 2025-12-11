#include "../global/tkbc-types.h"
#include "../global/tkbc-utils.h"
#include "tkbc-script-handler.h"
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief The function prints the serialized form of the kite ids.
 *
 * @param file The file where to print the kites.
 * @param ids The kite ids that should be serialized.
 */
void tkbc_print_kites(Content *buffer, Kite_Ids ids) {
  if (ids.count > 0) {
    tkbc_dapf(buffer, "(%zu", ids.elements[0]);
    for (size_t id = 1; id < ids.count; ++id) {
      tkbc_dapf(buffer, " %zu", ids.elements[id]);
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
 * @param filename The file name where the script should be saved to.
 * @return 0 If the saving and serialization of the file has succeeded. 1 if
 * writing of the header failed and -1 if the writing of the main program has
 * failed.
 */
int tkbc_export_script_to_dot_kite_file_from_mem(Script *script,
                                                 const char *filename) {
  int ok = 0;
  Kite_Ids ids = {0};
  Content out = {0};

  tkbc_dapf(&out, "BEGIN\n");
  for (size_t frames = 0; frames < script->count; ++frames) {

    if (script->elements[frames].count > 1) {
      tkbc_dapf(&out, "{\n");
    }

    for (size_t frame = 0; frame < script->elements[frames].count; ++frame) {
      Frame *f = &script->elements[frames].elements[frame];

      // TODO: Use hash function for this.
      for (size_t i = 0; i < f->kite_id_array.count; ++i) {
        Id id = f->kite_id_array.elements[i];
        if (!tkbc_contains_id(ids, id)) {
          tkbc_dap(&ids, id);
        }
      }

      switch (f->kind) {
      case KITE_QUIT: {
        tkbc_dapf(&out, "QUIT");
      } break;

      case KITE_WAIT: {
        tkbc_dapf(&out, "WAIT");
      } break;

      case KITE_MOVE: {
        Move_Action action = f->action.as_move;
        tkbc_dapf(&out, "MOVE ");
        tkbc_print_kites(&out, f->kite_id_array);
        tkbc_dapf(&out, " %f %f", action.position.x, action.position.y);

      } break;

      case KITE_MOVE_ADD: {
        Move_Add_Action action = f->action.as_move_add;
        tkbc_dapf(&out, "MOVE_ADD ");
        tkbc_print_kites(&out, f->kite_id_array);
        tkbc_dapf(&out, " %f %f", action.position.x, action.position.y);

      } break;

      case KITE_ROTATION: {
        Rotation_Action action = f->action.as_rotation;
        tkbc_dapf(&out, "ROTATION ");
        tkbc_print_kites(&out, f->kite_id_array);
        tkbc_dapf(&out, " %f", action.angle);

      } break;

      case KITE_ROTATION_ADD: {
        Rotation_Add_Action action = f->action.as_rotation_add;
        tkbc_dapf(&out, "ROTATION_ADD ");
        tkbc_print_kites(&out, f->kite_id_array);
        tkbc_dapf(&out, " %f", action.angle);

      } break;

      case KITE_TIP_ROTATION: {
        Tip_Rotation_Action action = f->action.as_tip_rotation;
        tkbc_dapf(&out, "TIP_ROTATION ");
        tkbc_print_kites(&out, f->kite_id_array);
        tkbc_dapf(&out, " %f %s", action.angle,
                  action.tip == LEFT_TIP ? "LEFT" : "RIGHT");

      } break;

      case KITE_TIP_ROTATION_ADD: {
        Tip_Rotation_Add_Action action = f->action.as_tip_rotation_add;
        tkbc_dapf(&out, "TIP_ROTATION_ADD ");
        tkbc_print_kites(&out, f->kite_id_array);
        tkbc_dapf(&out, " %f %s", action.angle,
                  action.tip == LEFT_TIP ? "LEFT" : "RIGHT");

      } break;

      default:
        assert(0 && "UNREACHABLE tkbc_script_kite_from_mem");
      }

      tkbc_dapf(&out, " %f\n", f->duration);
    }

    if (script->elements[frames].count > 1) {
      tkbc_dapf(&out, "}\n");
    }
  }
  tkbc_dapf(&out, "END\n");

  char buf[32];
  snprintf(buf, sizeof(buf), "KITES %zu\n", ids.count);
  int err = tkbc_write_file(filename, buf, strlen(buf));
  if (err) {
    check_return(-err);
  }
  err = tkbc_append_file(filename, out.elements, out.count);
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
 * @return 0 If the saving and serialization of all the files has succeeded. If
 * an error occurred the positive "script_id" of the first failing script is
 * returned, if writing the header of that script has failed and the negative
 * "-scirpt_id" will be returned, if writing the main program has failed.
 * The first initial KITES count in the script is considered to be the header.
 */
int tkbc_export_all_scripts_to_dot_kite_file_from_mem(Env *env) {
  int err = 0;
  size_t id = 1;
  for (size_t i = 0; i < env->scripts.count; ++i) {
    char buf[32];
    if (env->scripts.elements[i].name) {
      sprintf(buf, "%s.kite", env->scripts.elements[i].name);
    } else {
      sprintf(buf, "Script%zu.kite", i);
    }

    err = tkbc_export_script_to_dot_kite_file_from_mem(
        &env->scripts.elements[i], buf);

    if (err) {
      id = env->scripts.elements[i].script_id;
      break;
    }
  }

  return err * id;
}
