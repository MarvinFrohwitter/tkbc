#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../global/tkbc-types.h"
#include "../global/tkbc-utils.h"
#include "raylib.h"
#include "tkbc-keymaps.h"
#include "tkbc-script-handler.h"
#include "tkbc-ui.h"
#include "tkbc.h"

extern Kite_Textures kite_textures;

/**
 * @brief The function wraps all the UI-elements to a single draw handler that
 * computes the position and size of the UI-components.
 *
 * @param env The global state of the application.
 */
void tkbc_draw_ui(Env *env) {

  env->window_height = tkbc_get_screen_height();
  env->window_width = tkbc_get_screen_width();

  if (env->frames &&
      !(env->script_menu_interaction || env->keymaps_interaction)) {
    // A script is currently executing.
#ifdef TKBC_CLIENT
    tkbc_ui_timeline(env, env->server_script_block_index,
                     env->server_script_frames_in_script_count);

#else
    tkbc_ui_timeline(env, env->frames->block_index, env->block_frame->count);
#endif // TKBC_CLIENT
  }

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

  tkbc_ui_script_menu(env);

  if (!env->script_menu_interaction) {
    tkbc_ui_keymaps(env);
    tkbc_ui_color_picker(env);
  }

  if (!env->keymaps_interaction && !env->script_menu_interaction) {
    tkbc_display_kite_information(env);
  }
}

/**
 * @brief The function is responsible for managing and displaying a given
 * scrollbar. The bar will automatically scaled and moved according to the given
 * items_count and its currently first displayed item.
 *
 * @param env The global state of the application.
 * @param scrollbar The scrollbar that contains the interaction information and
 * it's geometry.
 * @param outer_container The rectangle where the scrollbar should be placed in.
 * @param items_count The maximum items that should be represented by the
 * scrollbar position.
 * @param top_interaction_box The first item that is displayed currently on the
 * screen, that must not necessary be the first item of the item_count.
 */
void tkbc_scrollbar(Env *env, Scrollbar *scrollbar, Rectangle outer_container,
                    size_t items_count, size_t *top_interaction_box) {
  //
  // The scroll_bar handle
  // -1 for the left out box at the bottom
  env->screen_items = (env->window_height / env->box_height) - 1;

  scrollbar->base.width = outer_container.width * 0.025;
  scrollbar->base.height = env->box_height * env->screen_items;

  scrollbar->base.x =
      outer_container.x + outer_container.width - scrollbar->base.width;

  DrawRectangleRounded(scrollbar->base, 1, 10, TKBC_UI_LIGHTGRAY_ALPHA);

  //
  // The inner scroll_bar handle
  scrollbar->inner_scrollbar.x =
      outer_container.x + outer_container.width - scrollbar->base.width;
  scrollbar->inner_scrollbar.width = scrollbar->base.width;

  size_t minimum_handle_height = (size_t)scrollbar->base.height >> 2;
  scrollbar->inner_scrollbar.height =
      minimum_handle_height +
      scrollbar->base.height / (float)(items_count - env->screen_items + 1);

  {
    // Enable the lerping for extra smooth scrolling.
    // float before = scrollbar->inner_scrollbar.y;

    scrollbar->inner_scrollbar.y =
        (scrollbar->base.height - scrollbar->inner_scrollbar.height) /
        (items_count - env->screen_items) * *top_interaction_box;

    // float after = scrollbar->inner_scrollbar.y;

    // scrollbar->inner_scrollbar.y =
    //     before + (after - before) * tkbc_get_frame_time();
  }

  if (items_count <= env->screen_items) {
    scrollbar->inner_scrollbar = scrollbar->base;
    DrawRectangleRounded(scrollbar->inner_scrollbar, 1, 10,
                         TKBC_UI_DARKPURPLE_ALPHA);
    return;
  }

  // This is just needed for window resizing problems. When the list of items
  // is scrolled to the bottom in a small window and then the window gets
  // resized to a lager one, the scrollbar should not be outside of the base
  // scroll container. The list it self may float to the top but that is not a
  // bug in it self list can handle a scrolloff. Below the list there is
  // nothing to display so it just empty space there is no need to
  // recallculate the position of the items for the lager window. --
  // M.Frohwitter 07.04.2025
  if (scrollbar->inner_scrollbar.y >
      scrollbar->base.height - scrollbar->inner_scrollbar.height) {
    scrollbar->inner_scrollbar.y =
        scrollbar->base.height - scrollbar->inner_scrollbar.height;
  }

  DrawRectangleRounded(scrollbar->inner_scrollbar, 1, 10,
                       TKBC_UI_DARKPURPLE_ALPHA);

  if (IsMouseButtonUp(MOUSE_BUTTON_LEFT)) {
    scrollbar->interaction = false;
  }
  if (CheckCollisionPointRec(GetMousePosition(), scrollbar->inner_scrollbar) &&
      IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    scrollbar->interaction = true;
  }

  if (CheckCollisionPointRec(GetMousePosition(), scrollbar->base) &&
      IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    scrollbar->interaction = true;
  }

  if (scrollbar->interaction) {
    Vector2 mouse_pos = GetMousePosition();
    float offset_height = scrollbar->inner_scrollbar.height / 2;
    float sb_center_y = scrollbar->inner_scrollbar.y + offset_height;

    if (mouse_pos.y - offset_height > sb_center_y) {

      if (*top_interaction_box < items_count - env->screen_items) {
        *top_interaction_box += 1;
      }

    } else if (mouse_pos.y + offset_height < sb_center_y) {
      if (*top_interaction_box > 0) {
        *top_interaction_box -= 1;
      }
    }
  }

  if (GetMouseWheelMove()) {
    if (GetMouseWheelMoveV().y < 0) {
      if (*top_interaction_box < items_count - env->screen_items) {
        *top_interaction_box += 1;
      }
    } else {
      if (*top_interaction_box > 0) {
        *top_interaction_box -= 1;
      }
    }
  }
}

/**
 * @brief This function is responsible for displaying all in memory loaded
 * scripts as a list. The user can then choose a script that should be executed.
 *
 * @param env The global state of the application.
 * @return True if menu was force closed, otherwise false.
 */
bool tkbc_ui_script_menu(Env *env) {
  if (IsKeyPressed(tkbc_hash_to_key(env->keymaps, KMH_CHANGE_KEY_MAPPINGS))) {
    env->script_menu_interaction = false;
    return true;
  }

  if (IsKeyPressed(tkbc_hash_to_key(env->keymaps, KMH_SWITCHES_NEXT_SCRIPT))) {
    env->script_menu_interaction = !env->script_menu_interaction;
    env->script_menu_mouse_interaction = false;
  }

  if (!env->script_menu_interaction) {
    return false;
  }

  env->script_menu_base =
      (Rectangle){0, 0, env->window_width * 0.4, env->window_height};

  size_t scripts_count = env->block_frames->count;
  float padding = 10;
  Rectangle script_box = {
      .x = env->script_menu_base.x + padding,
      .y = env->script_menu_base.y + padding / 2,
      .width = env->script_menu_base.width -
               env->script_menu_scrollbar.base.width - 2 * padding,
      .height = env->box_height - padding,
  };
  Rectangle outer_script_box = {
      .x = env->script_menu_base.x,
      .y = env->script_menu_base.y,
      .width =
          env->script_menu_base.width - env->script_menu_scrollbar.base.width,
      .height = env->box_height,
  };

  tkbc_scrollbar(env, &env->script_menu_scrollbar, env->script_menu_base,
                 scripts_count, &env->script_menu_top_interaction_box);

  int font_size = 22;
  Vector2 text_size;
  char i_name[64] = {0};
  for (size_t box = env->script_menu_top_interaction_box;
       box < env->screen_items + env->script_menu_top_interaction_box &&
       box < scripts_count;
       ++box) {

    if (CheckCollisionPointRec(GetMousePosition(), outer_script_box) &&
        !env->script_menu_mouse_interaction) {
      DrawRectangleRec(outer_script_box, TKBC_UI_TEAL_ALPHA);
    }
    if (env->script_menu_mouse_interaction &&
        box == env->script_menu_mouse_interaction_box) {
      DrawRectangleRec(outer_script_box, TKBC_UI_TEAL_ALPHA);
    }

    DrawRectangleRounded(script_box, 1, 10, TKBC_UI_LIGHTGRAY_ALPHA);

    if (CheckCollisionPointRec(GetMousePosition(), script_box)) {
      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        env->script_menu_mouse_interaction = true;
        env->script_menu_mouse_interaction_box = box;
      }

      if (!env->script_menu_mouse_interaction) {
        DrawRectangleRounded(script_box, 1, 10, TKBC_UI_DARKPURPLE_ALPHA);
      }
    }

    if (env->script_menu_mouse_interaction &&
        box == env->script_menu_mouse_interaction_box) {
      DrawRectangleRounded(script_box, 1, 10, TKBC_UI_PURPLE_ALPHA);
    }

    if (env->block_frames->elements[box].name) {
      const char *name = env->block_frames->elements[box].name;
      text_size = MeasureTextEx(GetFontDefault(), name, 12, 10);

      DrawText(name, script_box.x + script_box.width / 2 - text_size.x / 2,
               script_box.y + script_box.height / 2 - text_size.y / 2,
               font_size, TKBC_UI_BLACK);
    } else {
      snprintf(i_name, sizeof(i_name) - 1, "Script: %zu", box + 1);
      text_size = MeasureTextEx(GetFontDefault(), i_name, font_size, 0);

      DrawText(i_name, script_box.x + script_box.width / 2 - text_size.x / 2,
               script_box.y + script_box.height / 2 - text_size.y / 2,
               font_size, TKBC_UI_BLACK);
      memset(i_name, 0, sizeof(i_name));
    }

    script_box.y += script_box.height + padding;
    outer_script_box.y += env->box_height;
  }

  /* ------------------------- Confirm key --------------------------------- */

  size_t interaction_buttons_count = 3;
  outer_script_box.width =
      (outer_script_box.width - (padding * interaction_buttons_count)) /
      interaction_buttons_count;
  outer_script_box.height = env->box_height * 0.5;

  outer_script_box.x += padding + (interaction_buttons_count - 1) *
                                      (padding + outer_script_box.width);

  outer_script_box.y =
      env->box_height * env->screen_items + env->box_height / 2.f;

  if (CheckCollisionPointRec(GetMousePosition(), outer_script_box)) {
    DrawRectangleRounded(outer_script_box, 1, 10, TKBC_UI_DARKPURPLE_ALPHA);
  } else {
    DrawRectangleRounded(outer_script_box, 1, 10, TKBC_UI_TEAL_ALPHA);
  }

  if (env->script_menu_mouse_interaction) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(GetMousePosition(), outer_script_box)) {
      DrawRectangleRounded(outer_script_box, 1, 10, TKBC_UI_PURPLE_ALPHA);
      // Script ids start from 1 so +1 is needed.
      tkbc_load_script_id(env, env->script_menu_mouse_interaction_box + 1);
      env->script_menu_interaction = false;
      env->new_script_selected = true;
    }
  }
  const char *confirm = "CONFIRM";
  text_size = MeasureTextEx(GetFontDefault(), confirm, font_size, 0);
  DrawText(
      confirm,
      outer_script_box.x + outer_script_box.width * 0.5 - text_size.x * 0.5,
      outer_script_box.y + outer_script_box.height * 0.5 - text_size.y * 0.5,
      font_size, TKBC_UI_BLACK);

  return false;
}

/**
 * @brief The function is a way of getting information of the first selected
 * kite on the screen. This contains things like fly_speed and turn_speed.
 *
 * @param env The global state of the application.
 */
void tkbc_display_kite_information(Env *env) {
  for (size_t i = 0; i < env->kite_array->count; ++i) {
    if (env->kite_array->elements[i].is_kite_input_handler_active) {
      tkbc_display_kite_information_speeds(&env->kite_array->elements[i]);
      return;
    }
  }
}

/**
 * @brief The function is responsible for managing the displayment of the
 * fly_speed and turn_speed of the given kite.
 *
 * @param kite_state The kite for which the current speed values should be
 * displayed on the screen.
 */
void tkbc_display_kite_information_speeds(Kite_State *kite_state) {
  char buf[64] = {0};
  sprintf(buf, "Turn Speed: %.0f \t Fly Speed: %.0f",
          kite_state->kite->turn_speed, kite_state->kite->fly_speed);
  Vector2 pos = {10, 10};
  DrawText(buf, pos.x, pos.y, 20, TKBC_UI_TEAL);
}

/**
 * @brief The function determines if the given key can be represented as a hex
 * digit.
 *
 * @param key The value of the key pressed on the keyboard.
 * @return True if the given key would pass the isxdigit() function, otherwise
 * false.
 */
bool is_key_valid_part_of_hex_number(int key) {
  return (key >= KEY_ZERO && key <= KEY_NINE) || (key >= KEY_A && key <= KEY_F);
}

/**
 * @brief The function manages and displays the color picker where the user can
 * select new colors for the currently selected kites.
 *
 * @param env The global state of the application.
 */
void tkbc_ui_color_picker(Env *env) {
  if (env->script_setup) {
    return;
  }

  // KEY_ESCAPE
  if (IsKeyPressed(tkbc_hash_to_key(env->keymaps, KMH_CHANGE_KEY_MAPPINGS))) {
    env->color_picker_interaction = !env->color_picker_interaction;
  }
  if (!env->color_picker_interaction) {
    return;
  }

  float color_picker_width = env->window_width * 0.2;
  env->color_picker_base =
      (Rectangle){env->window_width - color_picker_width, 0, color_picker_width,
                  env->window_height};
  // DrawRectangleRec(env->color_picker_base, TKBC_UI_GRAY_ALPHA);

  int padding = 10;
  int font_size = 22;
  const char *description = "Enter a hex color code.";
  Vector2 text_size =
      MeasureTextEx(GetFontDefault(), description, font_size, 0);
  float description_height = text_size.y + padding;

  Rectangle input_box;
  input_box.height = env->box_height * 0.5;
  input_box.width = env->color_picker_base.width * 0.8;
  input_box.x = env->color_picker_base.x + env->color_picker_base.width / 2 -
                input_box.width / 2;
  input_box.y = padding + description_height;

  DrawText(description, input_box.x, env->color_picker_base.y + padding,
           font_size, TKBC_UI_BLACK);
  DrawRectangleRounded(input_box, 50, 20, WHITE);

  size_t char_amount = strlen(env->color_picker_input_text);

  int key = GetKeyPressed();
  Vector2 mouse = GetMousePosition();
  if (CheckCollisionPointRec(mouse, input_box) &&
      IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    env->color_picker_input_mouse_interaction = true;
  }

  if (key == KEY_ENTER ||
      key == tkbc_hash_to_key(env->keymaps, KMH_CHANGE_KEY_MAPPINGS)) {
    env->color_picker_input_mouse_interaction = false;
    goto key_skip;
  }

  if (!env->color_picker_input_mouse_interaction) {
    goto key_skip;
  }

  if (key == KEY_BACKSPACE) {
    if (char_amount > 1) {
      env->color_picker_input_text[char_amount - 1] = '\0';
    }
  } else if (is_key_valid_part_of_hex_number(key)) {
    if (char_amount > 8) {
      goto key_skip;
    }
    env->color_picker_input_text[char_amount] = key;
  }

key_skip:
  text_size = MeasureTextEx(GetFontDefault(), env->color_picker_input_text,
                            font_size, 0);
  DrawText(env->color_picker_input_text, input_box.x + padding,
           input_box.y + input_box.height / 2 - text_size.y / 2, font_size,
           TKBC_UI_GRAY);

  if (env->color_picker_input_text[1] == '\0') {
    const char *shadow_text = "  008080FF";
    text_size = MeasureTextEx(GetFontDefault(), shadow_text, font_size, 0);
    DrawText(shadow_text, input_box.x + padding,
             input_box.y + input_box.height / 2 - text_size.y / 2, font_size,
             TKBC_UI_LIGHTGRAY);
    // Rest so the cursor does not add the text_size, when the shadow text is
    // displayed.
    text_size.x = 0;
    text_size.y = 0;
  }

  Rectangle cursor = input_box;
  cursor.width = 2;
  cursor.height = input_box.height * 0.9;
  cursor.y = input_box.y + input_box.height / 2 - cursor.height / 2;
  float font_correction_factor = 1;
  cursor.x = input_box.x + (padding >> 1) + (padding << 1) +
             font_correction_factor * text_size.x;
  DrawRectangleRec(cursor, TKBC_UI_BLACK);

  if (strlen(env->color_picker_input_text) == 8 + 1) {
    env->last_selected_color =
        GetColor(strtol(env->color_picker_input_text + 1, NULL, 16));
  }

  Rectangle color_box;
  color_box.height = env->box_height;
  color_box.width = input_box.width;
  color_box.x = input_box.x;
  color_box.y = padding + input_box.y + input_box.height;
  // This is for allowing correct displayment of the alpha.
  DrawRectangleRounded(color_box, 2, 20, WHITE);
  DrawRectangleRounded(color_box, 2, 20, env->last_selected_color);
  DrawRectangleRoundedLinesEx(color_box, 2, 20, 1, BLACK);

  if (CheckCollisionPointRec(mouse, color_box) &&
      IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    tkbc_set_color_for_selected_kites(env, env->last_selected_color);

    env->favorite_colors.elements[env->current_favorite_colors_index++ %
                                  env->favorite_colors.count] =
        env->last_selected_color;
  }

  float color_circle_radius = color_box.height / 2;
  float left_circle_center =
      env->color_picker_base.x + color_circle_radius + padding;

  Vector2 color_circle = {
      .x = left_circle_center,
      .y = color_box.y + color_box.height + padding,
  };

  // Handle favorite colors circles.
  color_circle.y += 2 * color_circle_radius + padding;
  for (size_t i = 0; i < env->favorite_colors.count; i++) {
    if (CheckCollisionPointCircle(mouse, color_circle, color_circle_radius) &&
        IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      env->last_selected_color = env->favorite_colors.elements[i];
    }

    DrawCircleV(color_circle, color_circle_radius,
                env->favorite_colors.elements[i]);
    DrawCircleLinesV(color_circle, color_circle_radius, BLACK);
    color_circle.x += 2 * color_circle_radius + padding;
  }

  ////////////////////////////////////////////////////////////////////

  Rectangle forward = {
      .x = left_circle_center - color_circle_radius,
      .y = color_circle.y + color_circle_radius + padding +
           color_circle_radius * 0.5,
      .width = 4 * color_circle_radius,
      .height = color_circle_radius,
  };

  if (CheckCollisionPointRec(mouse, forward)) {
    DrawRectangleRounded(forward, 1, 10, TKBC_UI_DARKPURPLE_ALPHA);
  } else {
    DrawRectangleRounded(forward, 1, 10, TKBC_UI_TEAL_ALPHA);
  }

  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
      CheckCollisionPointRec(mouse, forward)) {
    DrawRectangleRounded(forward, 1, 10, TKBC_UI_PURPLE_ALPHA);
    env->color_picker_display_designs = !env->color_picker_display_designs;
  }

  const char *designs = NULL;
  if (env->color_picker_display_designs) {
    designs = "COLORS";
  } else {
    designs = "DESIGNS";
  }
  text_size = MeasureTextEx(GetFontDefault(), designs, font_size, 0);
  DrawText(designs, forward.x + forward.width * 0.5 - text_size.x * 0.5,
           forward.y + forward.height * 0.5 - text_size.y * 0.5, font_size,
           TKBC_UI_BLACK);

  if (env->color_picker_display_designs) {
    forward.y += forward.height + padding + color_circle_radius * 0.5;
    Vector2 display_position = {
        .x = forward.x,
        .y = forward.y,
    };

    for (size_t i = 0; i < kite_textures.count; ++i) {
      Texture2D t = kite_textures.elements[i].normal;
      float scale = env->color_picker_base.width * 0.5 / t.width;
      DrawTextureEx(t, display_position, 0, scale, WHITE);

      Rectangle collision_rectangle = {
          .x = display_position.x,
          .y = display_position.y,
          .width = t.width,
          .height = t.height,
      };

      if (CheckCollisionPointRec(mouse, collision_rectangle)) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
          tkbc_set_texture_for_selected_kites(env, &kite_textures.elements[i]);
          tkbc_set_color_for_selected_kites(env, BLANK);
        }
      }

      display_position.y +=
          2 * kite_textures.elements[i].normal.height + padding;
    }

    return;
  }

  // Handle default colors circles.
  color_circle.x = left_circle_center;
  color_circle.y += 2 * color_circle_radius + padding;

  Color colors[] = {
      LIGHTGRAY, GRAY,  DARKGRAY,  YELLOW,  GOLD,   ORANGE,
      PINK,      RED,   MAROON,    GREEN,   LIME,   DARKGREEN,
      SKYBLUE,   BLUE,  DARKBLUE,  PURPLE,  VIOLET, DARKPURPLE,
      BEIGE,     BROWN, DARKBROWN, MAGENTA, TEAL,
  };

  for (size_t i = 0; i < ARRAY_LENGTH(colors); ++i) {
    if (i % 4 == 0) {
      color_circle.x = left_circle_center;
      color_circle.y += 2 * color_circle_radius + padding;
    } else {
      color_circle.x += 2 * color_circle_radius + padding;
    }
    DrawCircleV(color_circle, color_circle_radius, colors[i]);
    DrawCircleLinesV(color_circle, color_circle_radius, BLACK);

    if (CheckCollisionPointCircle(mouse, color_circle, color_circle_radius)) {
      env->last_selected_color = colors[i];
      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        tkbc_set_color_for_selected_kites(env, env->last_selected_color);
      }
    }
  }
}

/**
 * @brief The function can be used to change the color of all currently selected
 * kites to the new specified one.
 *
 * @param env The global state of the application.
 * @param color The new color that should be assigned.
 */
void tkbc_set_color_for_selected_kites(Env *env, Color color) {
  for (size_t i = 0; i < env->kite_array->count; ++i) {
    if (env->kite_array->elements[i].is_kite_input_handler_active) {
      env->kite_array->elements[i].kite->body_color = color;
    }
  }
}

/**
 * @brief The function assignees the given texture pair, containing normal and
 * flipped versions, to the currently selected kites.
 *
 * @param env The global state of the application.
 * @param kite_texture The new pair of textures that should be assigned to all
 * the selected kites.
 */
void tkbc_set_texture_for_selected_kites(Env *env, Kite_Texture *kite_texture) {
  for (size_t i = 0; i < env->kite_array->count; ++i) {
    if (env->kite_array->elements[i].is_kite_input_handler_active) {
      tkbc_set_kite_texture(env->kite_array->elements[i].kite, kite_texture);
    }
  }
}

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

  float margin = 10;
  env->timeline_base.width = env->window_width / 2.0f;
  env->timeline_base.height = env->window_height / 42.0f;
  env->timeline_base.x = env->timeline_base.width / 2.0f;
  env->timeline_base.y =
      env->window_height - env->timeline_base.height - margin;

  env->timeline_front.height = env->timeline_base.height;
  env->timeline_front.x = env->timeline_base.x;
  env->timeline_front.y = env->timeline_base.y;

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
 * @brief The function sets the dest_key and it's corresponding string when
 * the given key is not equal to KEY_DELETE. For that case the keybind is
 * deleted.
 *
 * @param dest_key The key to sets is value to key_value.
 * @param dest_str The key str that holds the string representation of the
 * key.
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
 * rectangle. The handling of the drawing of the text and the box color is
 * also dynamically changed.
 *
 * @param env The global state of the application.
 * @param rectangle The rectangle of the key box that should be handled.
 * @param iteration The number of the key box within a keymap.
 * @param cur_major_box The base rectangle number of a single keymap.
 */
void tkbc_draw_key_box(Env *env, Rectangle rectangle, Key_Box iteration,
                       size_t cur_major_box) {
  DrawRectangleRounded(rectangle, 1, 10, TKBC_UI_LIGHTGRAY_ALPHA);

  if (CheckCollisionPointRec(GetMousePosition(), rectangle)) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      env->keymaps_mouse_interaction = true;
      env->keymaps_mouse_interaction_box = cur_major_box;
      env->keymaps_interaction_rec_number = iteration;
    }

    if (!env->keymaps_mouse_interaction) {
      DrawRectangleRounded(rectangle, 1, 10, TKBC_UI_DARKPURPLE_ALPHA);
    }
  }

  if (env->keymaps_mouse_interaction &&
      cur_major_box == env->keymaps_mouse_interaction_box &&
      env->keymaps_interaction_rec_number == iteration) {
    DrawRectangleRounded(rectangle, 1, 10, TKBC_UI_PURPLE_ALPHA);
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
    Key_Map *km = &env->keymaps.elements[env->keymaps_mouse_interaction_box];
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
      SetExitKey(tkbc_hash_to_key(env->keymaps, KMH_QUIT_PROGRAM));
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
    str = env->keymaps.elements[cur_major_box].mod_key_str;
    break;
  case BOX_MOD_CO_KEY:
    str = env->keymaps.elements[cur_major_box].mod_co_key_str;
    break;
  case BOX_SELECTION_KEY1:
    str = env->keymaps.elements[cur_major_box].selection_key1_str;
    break;
  case BOX_SELECTION_KEY2:
    str = env->keymaps.elements[cur_major_box].selection_key2_str;
    break;
  case BOX_KEY:
    str = env->keymaps.elements[cur_major_box].key_str;
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
 * @brief The function displays the keymaps settings and handles the
 * interaction with it. That includes loading and saving them to a file as
 * well as setting all the new defined keybinds from the user or even
 * resetting them to the defaults.
 *
 * @param env The global state of the application.
 */
void tkbc_ui_keymaps(Env *env) {
  // This will ensure that the settings can always be left regardless to which
  // keybinding is set. For opening and closing.
  if (IsKeyPressed(KEY_ESCAPE) &&
      tkbc_hash_to_key(env->keymaps, KMH_CHANGE_KEY_MAPPINGS) != KEY_ESCAPE) {
    env->keymaps_interaction = false;
  }
  // KEY_ESCAPE
  if (IsKeyPressed(tkbc_hash_to_key(env->keymaps, KMH_CHANGE_KEY_MAPPINGS))) {
    env->keymaps_interaction = !env->keymaps_interaction;
    env->keymaps_mouse_interaction = false;
  }
  if (!env->keymaps_interaction) {
    return;
  }
  env->keymaps_base =
      (Rectangle){0, 0, env->window_width * 0.4, env->window_height};
  // DrawRectangleRec(env->keymaps_base, TKBC_UI_GRAY_ALPHA);

  tkbc_scrollbar(env, &env->keymaps_scrollbar, env->keymaps_base,
                 env->keymaps.count, &env->keymaps_top_interaction_box);

  // TODO: Think about UI scaling.
  //
  // The displayment key bind boxes.
  int padding = 10;
  env->keymaps_base.height = env->box_height;
  env->keymaps_base.width -= env->keymaps_scrollbar.base.width;
  Vector2 text_size;
  for (size_t box = env->keymaps_top_interaction_box;
       box < env->screen_items + env->keymaps_top_interaction_box &&
       box < env->keymaps.count;
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
                        env->keymaps.elements[box].description, font_size, 0);
      font_size -= 1;
    } while (text_size.x + 2 * padding > env->keymaps_base.width &&
             text_size.y + 2 * padding > env->box_height / 2.0);

    DrawText(env->keymaps.elements[box].description,
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
    DrawRectangleRounded(env->keymaps_base, 1, 10, TKBC_UI_DARKPURPLE_ALPHA);
  } else {
    DrawRectangleRounded(env->keymaps_base, 1, 10, TKBC_UI_TEAL_ALPHA);
  }
  if (!env->keymaps_mouse_interaction) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(GetMousePosition(), env->keymaps_base)) {
      DrawRectangleRounded(env->keymaps_base, 1, 10, TKBC_UI_PURPLE_ALPHA);
      tkbc_load_keymaps_from_file(&env->keymaps, ".tkbc-keymaps");
      tkbc_setup_keymaps_strs(&env->keymaps);
      SetExitKey(tkbc_hash_to_key(env->keymaps, KMH_QUIT_PROGRAM));
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
    DrawRectangleRounded(env->keymaps_base, 1, 10, TKBC_UI_DARKPURPLE_ALPHA);
  } else {
    DrawRectangleRounded(env->keymaps_base, 1, 10, TKBC_UI_TEAL_ALPHA);
  }
  if (!env->keymaps_mouse_interaction) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(GetMousePosition(), env->keymaps_base)) {
      DrawRectangleRounded(env->keymaps_base, 1, 10, TKBC_UI_PURPLE_ALPHA);
      tkbc_set_keymaps_defaults(&env->keymaps);
      SetExitKey(tkbc_hash_to_key(env->keymaps, KMH_QUIT_PROGRAM));
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
    DrawRectangleRounded(env->keymaps_base, 1, 10, TKBC_UI_DARKPURPLE_ALPHA);
  } else {
    DrawRectangleRounded(env->keymaps_base, 1, 10, TKBC_UI_TEAL_ALPHA);
  }
  if (!env->keymaps_mouse_interaction) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(GetMousePosition(), env->keymaps_base)) {
      DrawRectangleRounded(env->keymaps_base, 1, 10, TKBC_UI_PURPLE_ALPHA);
      tkbc_save_keymaps_to_file(env->keymaps, ".tkbc-keymaps");
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
