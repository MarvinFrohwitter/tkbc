#include "../global/tkbc-types.h"
#include "../global/tkbc-utils.h"
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

bool tkbc_print_kites(FILE *file, Kite_Ids ids) {
  if (ids.count > 0) {
    fprintf(file, "(%zu", ids.elements[0]);
    for (size_t id = 1; id < ids.count; ++id) {
      fprintf(file, " %zu", ids.elements[id]);
    }
  } else {
    fprintf(file, "(");
  }
  fprintf(file, ")");

  return true;
}

bool tkbc_write_script_kite_from_mem(Block_Frame *block_frame,
                                     const char *filename) {
  bool ok = true;
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
  for (size_t frames = 0; frames < block_frame->count; ++frames) {

    if (block_frame->elements[frames].count > 1) {
      fprintf(file, "{\n");
    }

    for (size_t frame = 0; frame < block_frame->elements[frames].count;
         ++frame) {
      Frame *f = &block_frame->elements[frames].elements[frame];
      if (f->kite_id_array) {
        max_kites = fmaxf(max_kites, f->kite_id_array->count);
      }

      switch (f->kind) {
      case KITE_QUIT: {
        fprintf(file, "QUIT");
      } break;

      case KITE_WAIT: {
        fprintf(file, "WAIT");
      } break;

      case KITE_MOVE: {
        Move_Action *action = (Move_Action *)f->action;
        fprintf(file, "MOVE ");
        if (!tkbc_print_kites(file, *f->kite_id_array)) {
          check_return(false);
        }
        fprintf(file, " %f %f", action->position.x, action->position.y);

      } break;

      case KITE_MOVE_ADD: {
        Move_Add_Action *action = (Move_Add_Action *)f->action;
        fprintf(file, "MOVE_ADD ");
        if (!tkbc_print_kites(file, *f->kite_id_array)) {
          check_return(false);
        }
        fprintf(file, " %f %f", action->position.x, action->position.y);

      } break;

      case KITE_ROTATION: {
        Rotation_Action *action = (Rotation_Action *)f->action;
        fprintf(file, "ROTATION ");
        if (!tkbc_print_kites(file, *f->kite_id_array)) {
          check_return(false);
        }
        fprintf(file, " %f", action->angle);

      } break;

      case KITE_ROTATION_ADD: {
        Rotation_Add_Action *action = (Rotation_Add_Action *)f->action;
        fprintf(file, "ROTATION_ADD ");
        if (!tkbc_print_kites(file, *f->kite_id_array)) {
          check_return(false);
        }
        fprintf(file, " %f", action->angle);

      } break;

      case KITE_TIP_ROTATION: {
        Tip_Rotation_Action *action = (Tip_Rotation_Action *)f->action;
        fprintf(file, "TIP_ROTATION ");
        if (!tkbc_print_kites(file, *f->kite_id_array)) {
          check_return(false);
        }
        fprintf(file, " %f %s", action->angle,
                action->tip == LEFT_TIP ? "LEFT" : "RIGHT");

      } break;

      case KITE_TIP_ROTATION_ADD: {
        Tip_Rotation_Add_Action *action = (Tip_Rotation_Add_Action *)f->action;
        fprintf(file, "TIP_ROTATION_ADD ");
        if (!tkbc_print_kites(file, *f->kite_id_array)) {
          check_return(false);
        }
        fprintf(file, " %f %s", action->angle,
                action->tip == LEFT_TIP ? "LEFT" : "RIGHT");

      } break;

      default:
        assert(0 && "UNREACHABLE tkbc_script_kite_from_mem");
      }

      fprintf(file, " %f\n", f->duration);
    }

    if (block_frame->elements[frames].count > 1) {
      fprintf(file, "}\n");
    }
  }
  fprintf(file, "END\n");

  if (fseek(file, 0, SEEK_SET) < 0) {
    fprintf(stderr, "ERROR: %s\n", strerror(errno));
    check_return(false);
  }
  fprintf(file, "KITES %zu\n", max_kites);

check:
  fclose(file);
  return ok;
}
