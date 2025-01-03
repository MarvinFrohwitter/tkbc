#include <assert.h>

#include "../global/tkbc-types.h"
#include "../global/tkbc-utils.h"
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

  env->window_height = tkbc_get_screen_height();
  env->window_width = tkbc_get_screen_width();

  if (env->block_frame->count > 0) {
    // A script is currently executing.
#ifdef TKBC_CLIENT
    tkbc_ui_timeline(env, env->server_script_block_index,
                     env->server_script_block_index_count);

#else
    tkbc_ui_timeline(env, env->frames->block_index, env->block_frame->count);
#endif // TKBC_CLIENT
  }

  if (env->rendering) {
    DrawCircleV(CLITERAL(Vector2){20, 20}, 10, RED);
  }
  DrawFPS(env->window_width / 2, 10);
}

/**
 * @param env The global state of the application.
 */
/**
 * @brief The function provides the timeline UI slider to change the current
 * displayed frame of a script.
 *
 *
 * @param env The global state of the application.
 * @param block_index The current frames block_index.
 * @param block_index_count The maximum frames that registered.
 */
void tkbc_ui_timeline(Env *env, size_t block_index, size_t block_index_count) {
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

  env->timeline_segments = block_index + 1;

  assert(block_index_count != 0);
  assert(env->timeline_segments <= block_index_count);

  env->timeline_segment_width = env->timeline_base.width / block_index_count;

  env->timeline_segments_width =
      env->timeline_segment_width * env->timeline_segments;

  if ((mouse_pos.x >= env->timeline_base.x + env->timeline_base.width) ||
      (env->timeline_segments >= block_index_count)) {
    // Just for save UI drawing if the mouse is outside the right bounding box
    // of the timeline, the alignment should always be filled completely.
    env->timeline_front.width = env->timeline_base.width;

  } else {
    // If there are frames to display provide a segment in the timeline
    if (env->timeline_segments > 0) {
      env->timeline_front.width = env->timeline_segments_width;
    } else {
      env->timeline_front.width = 0;
    }
  }

  if (env->timeline_hoverover || env->timeline_interaction) {
    DrawRectangleRec(env->timeline_base, ColorBrightness(BLACK, 0.3));
    DrawRectangleRec(env->timeline_front, ColorBrightness(TEAL, 0.1));
  }
}
