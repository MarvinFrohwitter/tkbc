#include "tkbc-types.h"
#include "tkbc.h"
#include <raylib.h>

void tkbc_draw_ui(Env *env) {

  env->window_height = GetScreenHeight();
  env->window_width = GetScreenWidth();

  tkbc_ui_timeline(env);

  if (env->rendering) {
    DrawCircleV(CLITERAL(Vector2){20, 20}, 10, RED);
  }
  DrawFPS(env->window_width / 2, 10);
}

void tkbc_ui_timeline(Env *env) {
  if (env->script_setup) {
    return;
  }
  // if (env->script_finished) {
  //   return;
  // }

  Vector2 mouse_pos = GetMousePosition();
  env->timeline_hoverover =
      CheckCollisionPointRec(mouse_pos, env->timeline_base);

  if (IsMouseButtonUp(MOUSE_BUTTON_LEFT)) {
    env->timeline_interaction = false;
  }

  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    env->timeline_interaction = true;
  }

  env->timeline_segments = env->frames->block_index + 1;
  assert(env->timeline_segments <= env->index_blocks->count);
  assert(env->index_blocks->count != 0);

  env->timeline_segment_width =
      env->timeline_base.width / env->index_blocks->count;

  env->timeline_segments_width =
      env->timeline_segment_width * env->timeline_segments;

  env->timeline_front.width =

      // Just for save ui draw in if the mouse outside the right bounding box of
      // the timeline the alignment should always be filled completely.
      (mouse_pos.x >= env->timeline_base.x + env->timeline_base.width) ||
              (env->timeline_segments >= env->max_block_index)
          ? env->timeline_base.width

      // If there are frames to display provide a segment in the timeline
      : env->timeline_segments > 0 ? env->timeline_segments_width
                                   : mouse_pos.x - env->timeline_base.x;

  if (env->timeline_hoverover || env->timeline_interaction) {
    DrawRectangleRec(env->timeline_base, ColorBrightness(BLACK, 0.3));
    DrawRectangleRec(env->timeline_front, ColorBrightness(TEAL, 0.1));
  }
}
