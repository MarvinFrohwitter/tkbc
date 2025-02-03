#include <assert.h>

#include "../global/tkbc-types.h"
#include "../global/tkbc-utils.h"
#include "raylib.h"
#include "tkbc-keymaps.h"
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

  tkbc_ui_keymaps(env);

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
    if (env->rendering) {
      DrawRectangleRec(env->timeline_front, ColorBrightness(RED, 0.1));
    } else {
      DrawRectangleRec(env->timeline_front, ColorBrightness(TEAL, 0.1));
    }
  }
}

void tkbc_ui_keymaps(Env *env) {
  // KEY_F1
  if (IsKeyPressed(tkbc_hash_to_key(*env->keymaps, 1000))) {
    env->keymaps_interaction = !env->keymaps_interaction;
  }
  if (!env->keymaps_interaction) {
    return;
  }
  Rectangle rec = {0, 0, env->window_width * 0.4, env->window_height};
  DrawRectangleRec(rec, ColorAlpha(GRAY, 0.5));

  int padding = 10;
  float box_height = 80;
  size_t count = (env->window_height / box_height);

  rec.height = box_height;
  for (size_t box = 0; box < count - 1 && box < env->keymaps->count; ++box) {
    if (CheckCollisionPointRec(GetMousePosition(), rec) &&
        !env->keymaps_mouse_interaction) {
      DrawRectangleRec(rec, ColorAlpha(TEAL, 0.5));
    }
    if (env->keymaps_mouse_interaction &&
        box == env->keymaps_mouse_interaction_box) {
      DrawRectangleRec(rec, ColorAlpha(TEAL, 0.5));
    }

    int font_size = 22;
    Vector2 font_box;
    do {
      font_box =
          MeasureTextEx(GetFontDefault(),
                        env->keymaps->elements[box].description, font_size, 0);
      font_size -= 1;
    } while (font_box.x + 2 * padding > rec.width &&
             font_box.y + 2 * padding > box_height / 2.0);

    DrawText(env->keymaps->elements[box].description, rec.x + padding,
             rec.y + padding, font_size, BLACK);

    Rectangle mainkey_box = {
        .x = rec.x + padding,
        .y = rec.y + box_height / 2.0,
        .width = rec.width * 0.75,
        .height = box_height / 2.0 - padding,
    };

    DrawRectangleRec(mainkey_box, LIGHTGRAY);

    if (CheckCollisionPointRec(GetMousePosition(), mainkey_box)) {
      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        env->keymaps_mouse_interaction = true;
        env->keymaps_mouse_interaction_box = box;
      }
      if (!env->keymaps_mouse_interaction) {
        DrawRectangleRec(mainkey_box, DARKPURPLE);
      }
    }
    if (env->keymaps_mouse_interaction &&
        box == env->keymaps_mouse_interaction_box) {
      DrawRectangleRec(mainkey_box, PURPLE);
    }

    if (env->keymaps_mouse_interaction) {
      int key = GetKeyPressed();
      if (key == 0) {
        goto key_change_skip;
      }
      if (key != KEY_ESCAPE) {
        env->keymaps->elements[env->keymaps_mouse_interaction_box].key = key;
        env->keymaps->elements[env->keymaps_mouse_interaction_box].key_str =
            tkbc_key_to_str(key);
      }
      env->keymaps_mouse_interaction = false;
    }

  key_change_skip:
    DrawText(env->keymaps->elements[box].key_str,
             mainkey_box.x + mainkey_box.width * 0.1,
             mainkey_box.y + mainkey_box.height * 0.1,
             (int)(mainkey_box.height - mainkey_box.height * 0.1), BLACK);

    rec.y += box_height;
  }

  rec.width = (env->window_width * 0.2) / 4.0;
  rec.height = (env->window_height * 0.15) / 4.0;
  rec.x = env->window_width * 0.2 - rec.width - rec.width * 0.2;
  rec.y = env->window_height - rec.height * 2;

  if (CheckCollisionPointRec(GetMousePosition(), rec)) {
    DrawRectangleRec(rec, PURPLE);
  } else {
    DrawRectangleRec(rec, ColorAlpha(TEAL, 0.5));
  }
  if (!env->keymaps_mouse_interaction) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(GetMousePosition(), rec)) {
      tkbc_save_keymaps_to_file(*env->keymaps, ".tkbc-keymaps");
    }
  }

  DrawText("SAVE", rec.x + rec.width * 0.1, rec.y + rec.height * 0.2, 30,
           BLACK);

  rec.width = (env->window_width * 0.2) / 4.0;
  rec.height = (env->window_height * 0.15) / 4.0;
  rec.x = rec.width * 0.2;
  rec.y = env->window_height - rec.height * 2;

  if (CheckCollisionPointRec(GetMousePosition(), rec)) {
    DrawRectangleRec(rec, PURPLE);
  } else {
    DrawRectangleRec(rec, ColorAlpha(TEAL, 0.5));
  }
  if (!env->keymaps_mouse_interaction) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(GetMousePosition(), rec)) {
      tkbc_load_keymaps_from_file(env->keymaps, ".tkbc-keymaps");
      tkbc_setup_keymaps(env->keymaps);
    }
  }

  DrawText("LOAD", rec.x + rec.width * 0.1, rec.y + rec.height * 0.2, 30,
           BLACK);
}
