#include "../global/tkbc-types.h"
#include "../global/tkbc-utils.h"
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief The function prints the serialized form of the kite ids.
 *
 * @param file The file where to print the kites.
 * @param ids The kite ids that should be serialized.
 */
void tkbc_print_kites(FILE *file, Kite_Ids ids) {
  if (ids.count > 0) {
    fprintf(file, "(%zu", ids.elements[0]);
    for (size_t id = 1; id < ids.count; ++id) {
      fprintf(file, " %zu", ids.elements[id]);
    }
  } else {
    fprintf(file, "(");
  }
  fprintf(file, ")");
}

/**
 * @brief The function serializes the provided script in memory form to a .kite
 * file.
 *
 * @param script A memory representation of a script.
 * @param filename The file name where the script should be saved to.
 * @return 0 If the saving and serialization of the file has succeeded. -1 if
 * the file opening has failed and 1 if the fseek operation has failed.
 */
int tkbc_write_script_kite_from_mem(Script *script, const char *filename) {
  int ok = 0;
  size_t max_kites = 0;
  FILE *file = fopen(filename, "wb");
  if (file == NULL) {
    tkbc_fprintf(stderr, "ERROR", "%s:%d:%s\n", __FILE__, __LINE__,
                 strerror(errno));
    return -1;
  }

  fprintf(file, "KITES  \n");
  // Padding for Maximum number that can be inserted later for KITES.
  fprintf(file, "\n\n\n\n\n\n\n\n\n");
  fprintf(file, "BEGIN\n");
  for (size_t frames = 0; frames < script->count; ++frames) {

    if (script->elements[frames].count > 1) {
      fprintf(file, "{\n");
    }

    for (size_t frame = 0; frame < script->elements[frames].count; ++frame) {
      Frame *f = &script->elements[frames].elements[frame];
      if (f->kite_id_array.count) {
        max_kites = fmaxf(max_kites, f->kite_id_array.count);
      }

      switch (f->kind) {
      case KITE_QUIT: {
        fprintf(file, "QUIT");
      } break;

      case KITE_WAIT: {
        fprintf(file, "WAIT");
      } break;

      case KITE_MOVE: {
        Move_Action action = f->action.as_move;
        fprintf(file, "MOVE ");
        tkbc_print_kites(file, f->kite_id_array);
        fprintf(file, " %f %f", action.position.x, action.position.y);

      } break;

      case KITE_MOVE_ADD: {
        Move_Add_Action action = f->action.as_move_add;
        fprintf(file, "MOVE_ADD ");
        tkbc_print_kites(file, f->kite_id_array);
        fprintf(file, " %f %f", action.position.x, action.position.y);

      } break;

      case KITE_ROTATION: {
        Rotation_Action action = f->action.as_rotation;
        fprintf(file, "ROTATION ");
        tkbc_print_kites(file, f->kite_id_array);
        fprintf(file, " %f", action.angle);

      } break;

      case KITE_ROTATION_ADD: {
        Rotation_Add_Action action = f->action.as_rotation_add;
        fprintf(file, "ROTATION_ADD ");
        tkbc_print_kites(file, f->kite_id_array);
        fprintf(file, " %f", action.angle);

      } break;

      case KITE_TIP_ROTATION: {
        Tip_Rotation_Action action = f->action.as_tip_rotation;
        fprintf(file, "TIP_ROTATION ");
        tkbc_print_kites(file, f->kite_id_array);
        fprintf(file, " %f %s", action.angle,
                action.tip == LEFT_TIP ? "LEFT" : "RIGHT");

      } break;

      case KITE_TIP_ROTATION_ADD: {
        Tip_Rotation_Add_Action action = f->action.as_tip_rotation_add;
        fprintf(file, "TIP_ROTATION_ADD ");
        tkbc_print_kites(file, f->kite_id_array);
        fprintf(file, " %f %s", action.angle,
                action.tip == LEFT_TIP ? "LEFT" : "RIGHT");

      } break;

      default:
        assert(0 && "UNREACHABLE tkbc_script_kite_from_mem");
      }

      fprintf(file, " %f\n", f->duration);
    }

    if (script->elements[frames].count > 1) {
      fprintf(file, "}\n");
    }
  }
  fprintf(file, "END\n");

  if (fseek(file, 0, SEEK_SET) < 0) {
    fprintf(stderr, "ERROR: %s\n", strerror(errno));
    check_return(1);
  }
  fprintf(file, "KITES %zu\n", max_kites);

check:
  fclose(file);
  return ok;
}
