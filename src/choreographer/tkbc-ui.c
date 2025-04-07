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

  if (env->frames) {
    // A script is currently executing.
#ifdef TKBC_CLIENT
    tkbc_ui_timeline(env, env->server_script_block_index,
                     env->server_script_frames_in_script_count);

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

/**
 * @brief The function sets the dest_key and it's corresponding string when the
 * given key is not equal to KEY_DELETE. For that case the keybind is deleted.
 *
 * @param dest_key The key to sets is value to key_value.
 * @param dest_str The key str that holds the string representation of the key.
 * @param key_value The value that the dest_key should be set to.
 */
void tkbc_set_key_or_delete(int *dest_key, const char **dest_str,
                            int key_value) {
  if (key_value == KEY_DELETE) {
    *dest_key = KEY_NULL;
  } else {
    *dest_key = key_value;
  }
  *dest_str = tkbc_key_to_str(*dest_key);
}

/**
 * @brief The function sets the new keybinding corresponding to the given
 * rectangle. The handling of the drawing of the text and the box color is also
 * dynamically changed.
 *
 * @param env The global state of the application.
 * @param rectangle The rectangle of the key box that should be handled.
 * @param iteration The number of the key box within a keymap.
 * @param cur_major_box The base rectangle number of a single keymap.
 */
void tkbc_draw_key_box(Env *env, Rectangle rectangle, Key_Box iteration,
                       size_t cur_major_box) {
  DrawRectangleRec(rectangle, TKBC_UI_LIGHTGRAY_ALPHA);

  if (CheckCollisionPointRec(GetMousePosition(), rectangle)) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      env->keymaps_mouse_interaction = true;
      env->keymaps_mouse_interaction_box = cur_major_box;
      env->keymaps_interaction_rec_number = iteration;
    }

    if (!env->keymaps_mouse_interaction) {
      DrawRectangleRec(rectangle, TKBC_UI_DARKPURPLE_ALPHA);
    }
  }

  if (env->keymaps_mouse_interaction &&
      cur_major_box == env->keymaps_mouse_interaction_box &&
      env->keymaps_interaction_rec_number == iteration) {
    DrawRectangleRec(rectangle, TKBC_UI_PURPLE_ALPHA);
  }

  if (!env->keymaps_mouse_interaction) {
    goto key_change_skip;
  }

  int key = GetKeyPressed();
  if (key == 0) {
    goto key_change_skip;
  }
  // NOTE: This KEY_ESCAPE disables the ability to set KEY_ESCAPE for any
  // keymap.
  if (key != KEY_ESCAPE) {
    Key_Map *km = &env->keymaps->elements[env->keymaps_mouse_interaction_box];
    switch (env->keymaps_interaction_rec_number) {
    case BOX_MOD_KEY:
      tkbc_set_key_or_delete(&km->mod_key, &km->mod_key_str, key);
      break;
    case BOX_MOD_CO_KEY:
      tkbc_set_key_or_delete(&km->mod_co_key, &km->mod_co_key_str, key);
      break;
    case BOX_SELECTION_KEY1:
      tkbc_set_key_or_delete(&km->selection_key1, &km->selection_key1_str, key);
      break;
    case BOX_SELECTION_KEY2:
      tkbc_set_key_or_delete(&km->selection_key2, &km->selection_key2_str, key);
      break;
    case BOX_KEY:
      tkbc_set_key_or_delete(&km->key, &km->key_str, key);
      break;
    default:
      assert(0 && "UNREACHABLE tkbc_draw_key_box()");
    }

    if (km->hash == KMH_QUIT_PROGRAM) {
      SetExitKey(tkbc_hash_to_key(*env->keymaps, KMH_QUIT_PROGRAM));
    }

    env->keymaps_mouse_interaction = false;
    env->keymaps_interaction_rec_number = -1;
  }

key_change_skip: {}
  Vector2 text_size = {0};
  int font_size = 18;

  const char *str;
  switch (iteration) {
  case BOX_MOD_KEY:
    str = env->keymaps->elements[cur_major_box].mod_key_str;
    break;
  case BOX_MOD_CO_KEY:
    str = env->keymaps->elements[cur_major_box].mod_co_key_str;
    break;
  case BOX_SELECTION_KEY1:
    str = env->keymaps->elements[cur_major_box].selection_key1_str;
    break;
  case BOX_SELECTION_KEY2:
    str = env->keymaps->elements[cur_major_box].selection_key2_str;
    break;
  case BOX_KEY:
    str = env->keymaps->elements[cur_major_box].key_str;
    break;
  default:
    assert(0 && "UNREACHABLE tkbc_draw_key_box()");
  }

  // TODO: Made the text scale
  if (strcmp(str, tkbc_key_to_str(KEY_NULL)) == 0) {
    str = "---";
  }
  text_size = MeasureTextEx(GetFontDefault(), str, font_size, 0);
  DrawText(str, rectangle.x + rectangle.width * 0.5 - text_size.x * 0.5,
           rectangle.y + rectangle.height * 0.5 - text_size.y * 0.5, font_size,
           TKBC_UI_BLACK);
}

/**
 * @brief The function displays the keymaps settings and handles the interaction
 * with it. That includes loading and saving them to a file as well as setting
 * all the new defined keybinds from the user or even resetting them to the
 * defaults.
 *
 * @param env The global state of the application.
 */
void tkbc_ui_keymaps(Env *env) {
  // This will ensure that the settings can always be left regardless to which
  // keybinding is set. For opening and closing.
  if (IsKeyPressed(KEY_ESCAPE) &&
      tkbc_hash_to_key(*env->keymaps, KMH_CHANGE_KEY_MAPPINGS) != KEY_ESCAPE) {
    env->keymaps_interaction = false;
  }
  // KEY_ESCAPE
  if (IsKeyPressed(tkbc_hash_to_key(*env->keymaps, KMH_CHANGE_KEY_MAPPINGS))) {
    env->keymaps_interaction = !env->keymaps_interaction;
    env->keymaps_mouse_interaction = false;
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

  if (env->keymaps_scollbar_interaction) {
    Vector2 mouse_pos = GetMousePosition();
    float offset_height = env->keymaps_inner_scrollbar.height / 2;
    float sb_center_y = env->keymaps_inner_scrollbar.y + offset_height;

    if (mouse_pos.y - offset_height > sb_center_y) {

      if (env->keymaps_top_interaction_box <
          env->keymaps->count - env->screen_items) {
        env->keymaps_top_interaction_box += 1;
      }

    } else if (mouse_pos.y + offset_height < sb_center_y) {
      if (env->keymaps_top_interaction_box > 0) {
        env->keymaps_top_interaction_box -= 1;
      }
    }
  }

  if (GetMouseWheelMove()) {
    if (GetMouseWheelMoveV().y < 0) {
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
  Vector2 text_size;
  for (size_t box = env->keymaps_top_interaction_box;
       box < env->screen_items + env->keymaps_top_interaction_box &&
       box < env->keymaps->count;
       ++box) {

    size_t key_box_count = 5;
    Rectangle key_box = {
        .x = env->keymaps_base.x + padding,
        .y = env->keymaps_base.y + env->box_height / 2.0,

        .width = (env->keymaps_base.width - (padding * (key_box_count + 1))) /
                 key_box_count,
        .height = env->box_height / 2.0 - padding,
    };

    if (CheckCollisionPointRec(GetMousePosition(), env->keymaps_base) &&
        !env->keymaps_mouse_interaction) {
      DrawRectangleRec(env->keymaps_base, TKBC_UI_TEAL_ALPHA);
    }
    if (env->keymaps_mouse_interaction &&
        box == env->keymaps_mouse_interaction_box) {
      DrawRectangleRec(env->keymaps_base, TKBC_UI_TEAL_ALPHA);
    }

    int font_size = 22;
    do {
      text_size =
          MeasureTextEx(GetFontDefault(),
                        env->keymaps->elements[box].description, font_size, 0);
      font_size -= 1;
    } while (text_size.x + 2 * padding > env->keymaps_base.width &&
             text_size.y + 2 * padding > env->box_height / 2.0);

    DrawText(env->keymaps->elements[box].description,
             env->keymaps_base.x + padding, env->keymaps_base.y + padding,
             font_size, TKBC_UI_BLACK);

    for (size_t i = 0; i < key_box_count; ++i) {
      // BOX_MOD_KEY
      // BOX_MOD_CO_KEY
      // BOX_SELECTION_KEY1
      // BOX_SELECTION_KEY2
      // BOX_KEY
      tkbc_draw_key_box(env, key_box, i, box);

      key_box.x += key_box.width + padding;
    }

    env->keymaps_base.y += env->box_height;
  }

  //
  // Displayment of the load and save buttons.
  size_t interaction_buttons_count = 3;
  env->keymaps_base.width =
      (env->keymaps_base.width - (padding * (interaction_buttons_count * 1))) /
      interaction_buttons_count;
  env->keymaps_base.height = env->box_height * 0.5;
  env->keymaps_base.x += padding;
  env->keymaps_base.y =
      env->box_height * env->screen_items + env->keymaps_base.height;
  size_t font_size = env->keymaps_base.height * 0.75;

  if (CheckCollisionPointRec(GetMousePosition(), env->keymaps_base)) {
    DrawRectangleRec(env->keymaps_base, TKBC_UI_DARKPURPLE_ALPHA);
  } else {
    DrawRectangleRec(env->keymaps_base, TKBC_UI_TEAL_ALPHA);
  }
  if (!env->keymaps_mouse_interaction) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(GetMousePosition(), env->keymaps_base)) {
      DrawRectangleRec(env->keymaps_base, TKBC_UI_PURPLE_ALPHA);
      tkbc_load_keymaps_from_file(env->keymaps, ".tkbc-keymaps");
      tkbc_setup_keymaps_strs(env->keymaps);
      SetExitKey(tkbc_hash_to_key(*env->keymaps, KMH_QUIT_PROGRAM));
    }
  }
  const char *load = "LOAD";
  text_size = MeasureTextEx(GetFontDefault(), load, font_size, 0);
  DrawText(
      load,
      env->keymaps_base.x + env->keymaps_base.width * 0.5 - text_size.x * 0.5,
      env->keymaps_base.y + env->keymaps_base.height * 0.5 - text_size.y * 0.5,
      font_size, TKBC_UI_BLACK);

  env->keymaps_base.x += padding + env->keymaps_base.width;
  if (CheckCollisionPointRec(GetMousePosition(), env->keymaps_base)) {
    DrawRectangleRec(env->keymaps_base, TKBC_UI_DARKPURPLE_ALPHA);
  } else {
    DrawRectangleRec(env->keymaps_base, TKBC_UI_TEAL_ALPHA);
  }
  if (!env->keymaps_mouse_interaction) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(GetMousePosition(), env->keymaps_base)) {
      DrawRectangleRec(env->keymaps_base, TKBC_UI_PURPLE_ALPHA);
      tkbc_set_keymaps_defaults(env->keymaps);
      SetExitKey(tkbc_hash_to_key(*env->keymaps, KMH_QUIT_PROGRAM));
    }
  }
  const char *reset = "RESET";
  text_size = MeasureTextEx(GetFontDefault(), reset, font_size, 0);
  DrawText(
      reset,
      env->keymaps_base.x + env->keymaps_base.width * 0.5 - text_size.x * 0.5,
      env->keymaps_base.y + env->keymaps_base.height * 0.5 - text_size.y * 0.5,
      font_size, TKBC_UI_BLACK);

  env->keymaps_base.x += padding + env->keymaps_base.width;
  if (CheckCollisionPointRec(GetMousePosition(), env->keymaps_base)) {
    DrawRectangleRec(env->keymaps_base, TKBC_UI_DARKPURPLE_ALPHA);
  } else {
    DrawRectangleRec(env->keymaps_base, TKBC_UI_TEAL_ALPHA);
  }
  if (!env->keymaps_mouse_interaction) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(GetMousePosition(), env->keymaps_base)) {
      DrawRectangleRec(env->keymaps_base, TKBC_UI_PURPLE_ALPHA);
      tkbc_save_keymaps_to_file(*env->keymaps, ".tkbc-keymaps");
    }
  }
  const char *save = "SAVE";
  text_size = MeasureTextEx(GetFontDefault(), save, font_size, 0);
  DrawText(
      save,
      env->keymaps_base.x + env->keymaps_base.width * 0.5 - text_size.x * 0.5,
      env->keymaps_base.y + env->keymaps_base.height * 0.5 - text_size.y * 0.5,
      font_size, TKBC_UI_BLACK);
}
