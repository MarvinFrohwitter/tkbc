#include <assert.h>

#include "../global/tkbc-types.h"
#include "raylib.h"
#include "tkbc-ui.h"
#include "tkbc.h"

/**
 * @brief The function wraps all the UI-elements to a single draw handler that
 * computes the position and size of the UI-components.
 *
 * @param env The global state of the application.
 */
void tkbc_draw_ui(Env *env) {

  env->window_height = GetScreenHeight();
  env->window_width = GetScreenWidth();

  if (env->block_frame->count > 0) {
    // A script is currently executing.
    tkbc_ui_timeline(env);
  }

  if (env->rendering) {
    DrawCircleV(CLITERAL(Vector2){20, 20}, 10, RED);
  }
  DrawFPS(env->window_width / 2, 10);
}

/**
 * @brief The function provides the timeline UI slider to change the current
 * displayed frame of a script.
 *
 * @param env The global state of the application.
 */
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
  assert(env->timeline_segments <= env->block_frame->count);
  assert(env->block_frame->count != 0);

  env->timeline_segment_width =
      env->timeline_base.width / env->block_frame->count;

  env->timeline_segments_width =
      env->timeline_segment_width * env->timeline_segments;

  if ((mouse_pos.x >= env->timeline_base.x + env->timeline_base.width) ||
      (env->timeline_segments >= env->block_frame->count)) {
    // Just for save UI drawing if the mouse is outside the right bounding box
    // of the timeline, the alignment should always be filled completely.
    env->timeline_front.width = env->timeline_base.width;

  } else {

    // If there are frames to display provide a segment in the timeline
    if (env->timeline_segments > 0) {
      env->timeline_front.width = env->timeline_segments_width;
    } else {

      /* TODO: Consider a width of just 0 in that case where there is no frame
      to display. This is likely not going to happen, because the scripts have
      the init frame.
       */
      env->timeline_front.width = mouse_pos.x - env->timeline_base.x;
    }
  }

  if (env->timeline_hoverover || env->timeline_interaction) {
    DrawRectangleRec(env->timeline_base, ColorBrightness(BLACK, 0.3));
    DrawRectangleRec(env->timeline_front, ColorBrightness(TEAL, 0.1));
  }
}