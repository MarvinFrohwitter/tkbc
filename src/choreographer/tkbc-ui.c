#include <assert.h>
#include <string.h>

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

  if (!env->rendering) {
    Color color = TKBC_UI_TEAL;
    int fps = GetFPS();
    if (fps < 25) {
      color = TKBC_UI_PURPLE;
    }

    char buf[16] = {0};
    sprintf(buf, "%2i FPS", fps);
    DrawText(buf, env->window_width / 2, 10, 20, color);
  }
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
    DrawRectangleRec(env->timeline_base, TKBC_UI_GRAY);
    if (env->rendering) {
      DrawRectangleRec(env->timeline_front, TKBC_UI_RED);
    } else {
      DrawRectangleRec(env->timeline_front, TKBC_UI_TEAL);
    }
  }
}

void tkbc_ui_keymaps(Env *env) {
  // KEY_F1
  if (IsKeyPressed(tkbc_hash_to_key(*env->keymaps, 1000))) {
    env->keymaps_interaction = !env->keymaps_interaction;
  }
  if (IsKeyPressed(KEY_ESCAPE)) {
    env->keymaps_interaction = false;
  }
  if (!env->keymaps_interaction) {
    return;
  }
  env->keymaps_base =
      (Rectangle){0, 0, env->window_width * 0.4, env->window_height};
  DrawRectangleRec(env->keymaps_base, TKBC_UI_GRAY_ALPHA);

  //
  // The scroll_bar handle
  env->screen_items = (env->window_height / env->box_height) - 1;
  env->scrollbar_width = env->keymaps_base.width * 0.025;

  env->keymaps_scrollbar.x =
      env->keymaps_base.x + env->keymaps_base.width - env->scrollbar_width;
  env->keymaps_scrollbar.width = env->scrollbar_width;
  env->keymaps_scrollbar.height = env->box_height * env->screen_items;
  DrawRectangleRec(env->keymaps_scrollbar, TKBC_UI_LIGHTGRAY_ALPHA);

  //
  // The inner scroll_bar handle
  env->keymaps_inner_scrollbar.x = env->keymaps_base.x +
                                   env->keymaps_base.width -
                                   env->keymaps_scrollbar.width;
  env->keymaps_inner_scrollbar.width = env->keymaps_scrollbar.width;
  env->keymaps_inner_scrollbar.height =
      env->keymaps_scrollbar.height /
      (float)(env->keymaps->count - env->screen_items + 1);

  float old_inner_sclolbar_y = env->keymaps_inner_scrollbar.y;
  float new_inner_sclolbar_y =
      (env->keymaps_inner_scrollbar.height) * env->keymaps_top_interaction_box;
  //
  // To lerp between the things it has to be move out side to be static
  env->keymaps_inner_scrollbar.y =
      old_inner_sclolbar_y +
      (new_inner_sclolbar_y - old_inner_sclolbar_y) * tkbc_get_frame_time();

  // Enable to remove LERP.
  env->keymaps_inner_scrollbar.y = new_inner_sclolbar_y;

  DrawRectangleRec(env->keymaps_inner_scrollbar, TKBC_UI_DARKPURPLE_ALPHA);

  if (IsMouseButtonUp(MOUSE_BUTTON_LEFT)) {
    env->keymaps_scollbar_interaction = false;
  }
  if (CheckCollisionPointRec(GetMousePosition(),
                             env->keymaps_inner_scrollbar) &&
      IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    env->keymaps_scollbar_interaction = true;
  }

  if (CheckCollisionPointRec(GetMousePosition(), env->keymaps_scrollbar) &&
      IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    env->keymaps_scollbar_interaction = true;
  }

  if (env->keymaps_scollbar_interaction || GetMouseWheelMove()) {
    Vector2 mouse_pos = GetMousePosition();
    float sb_center_y = env->keymaps_inner_scrollbar.y +
                        env->keymaps_inner_scrollbar.height / 2.0;
    if (mouse_pos.y > sb_center_y || GetMouseWheelMoveV().y > sb_center_y) {

      if (env->keymaps_top_interaction_box <
          env->keymaps->count - env->screen_items) {
        env->keymaps_top_interaction_box += 1;
      }

    } else {
      if (env->keymaps_top_interaction_box > 0) {
        env->keymaps_top_interaction_box -= 1;
      }
    }
  }

  // TODO: Think about UI scaling.
  //
  // The displayment key bind boxes.
  int padding = 10;
  env->keymaps_base.height = env->box_height;
  env->keymaps_base.width -= env->keymaps_scrollbar.width;
  for (size_t box = env->keymaps_top_interaction_box;
       box < env->screen_items + env->keymaps_top_interaction_box &&
       box < env->keymaps->count;
       ++box) {
    if (CheckCollisionPointRec(GetMousePosition(), env->keymaps_base) &&
        !env->keymaps_mouse_interaction) {
      DrawRectangleRec(env->keymaps_base, TKBC_UI_TEAL_ALPHA);
    }
    if (env->keymaps_mouse_interaction &&
        box == env->keymaps_mouse_interaction_box) {
      DrawRectangleRec(env->keymaps_base, TKBC_UI_TEAL_ALPHA);
    }

    int font_size = 22;
    Vector2 font_box;
    do {
      font_box =
          MeasureTextEx(GetFontDefault(),
                        env->keymaps->elements[box].description, font_size, 0);
      font_size -= 1;
    } while (font_box.x + 2 * padding > env->keymaps_base.width &&
             font_box.y + 2 * padding > env->box_height / 2.0);

    DrawText(env->keymaps->elements[box].description,
             env->keymaps_base.x + padding, env->keymaps_base.y + padding,
             font_size, TKBC_UI_BLACK);

    Rectangle mainkey_box = {
        .x = env->keymaps_base.x + padding,
        .y = env->keymaps_base.y + env->box_height / 2.0,
        .width = env->keymaps_base.width * 0.75,
        .height = env->box_height / 2.0 - padding,
    };

    DrawRectangleRec(mainkey_box, TKBC_UI_LIGHTGRAY_ALPHA);

    if (CheckCollisionPointRec(GetMousePosition(), mainkey_box)) {
      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        env->keymaps_mouse_interaction = true;
        env->keymaps_mouse_interaction_box = box;
      }
      if (!env->keymaps_mouse_interaction) {
        DrawRectangleRec(mainkey_box, TKBC_UI_DARKPURPLE_ALPHA);
      }
    }
    if (env->keymaps_mouse_interaction &&
        box == env->keymaps_mouse_interaction_box) {
      DrawRectangleRec(mainkey_box, TKBC_UI_PURPLE_ALPHA);
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
             (int)(mainkey_box.height - mainkey_box.height * 0.1),
             TKBC_UI_BLACK);

    env->keymaps_base.y += env->box_height;
  }

  //
  // Displayment of the load and save buttons.
  env->keymaps_base.width = (env->window_width * 0.2) / 4.0;
  env->keymaps_base.height = (env->window_height * 0.15) / 4.0;
  env->keymaps_base.x = env->window_width * 0.2 - env->keymaps_base.width -
                        env->keymaps_base.width * 0.2;
  env->keymaps_base.y = env->window_height - env->keymaps_base.height * 2;

  if (CheckCollisionPointRec(GetMousePosition(), env->keymaps_base)) {
    DrawRectangleRec(env->keymaps_base, TKBC_UI_PURPLE_ALPHA);
  } else {
    DrawRectangleRec(env->keymaps_base, TKBC_UI_TEAL_ALPHA);
  }
  if (!env->keymaps_mouse_interaction) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(GetMousePosition(), env->keymaps_base)) {
      tkbc_save_keymaps_to_file(*env->keymaps, ".tkbc-keymaps");
    }
  }

  DrawText("SAVE", env->keymaps_base.x + env->keymaps_base.width * 0.1,
           env->keymaps_base.y + env->keymaps_base.height * 0.2, 30,
           TKBC_UI_BLACK);

  env->keymaps_base.width = (env->window_width * 0.2) / 4.0;
  env->keymaps_base.height = (env->window_height * 0.15) / 4.0;
  env->keymaps_base.x = env->keymaps_base.width * 0.2;
  env->keymaps_base.y = env->window_height - env->keymaps_base.height * 2;

  if (CheckCollisionPointRec(GetMousePosition(), env->keymaps_base)) {
    DrawRectangleRec(env->keymaps_base, TKBC_UI_PURPLE_ALPHA);
  } else {
    DrawRectangleRec(env->keymaps_base, TKBC_UI_TEAL_ALPHA);
  }
  if (!env->keymaps_mouse_interaction) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(GetMousePosition(), env->keymaps_base)) {
      tkbc_load_keymaps_from_file(env->keymaps, ".tkbc-keymaps");
      tkbc_setup_keymaps(env->keymaps);
    }
  }

  DrawText("LOAD", env->keymaps_base.x + env->keymaps_base.width * 0.1,
           env->keymaps_base.y + env->keymaps_base.height * 0.2, 30,
           TKBC_UI_BLACK);
}
