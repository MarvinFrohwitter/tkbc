#include "tkbc-popup.h"
#include "../choreographer/tkbc.h"
#include "../global/tkbc-utils.h"

/**
 * @brief The function checks if the given popup has a click interaction.
 *
 * @param popup The popup that should be checked.
 * @return 0 if no interaction has happen, -1 if the cross was clicked, greater
 * then 0 for the corresponding option interaction.
 */
int tkbc_check_popup_interaction(Popup *popup) {
  if (!popup->active) {
    return 0;
  }
  if (CheckCollisionPointRec(GetMousePosition(), popup->cross)) {
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      return -1;
    }
  }
  if (CheckCollisionPointRec(GetMousePosition(), popup->option1)) {
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      return 1;
    }
  }
  return 0;
}

/**
 * @brief The function updates/sets new values for the given disconnect popup.
 *
 * @param popup The popup where the values should be updated.
 */
void tkbc_popup_resize(Popup *popup) {
  if (!popup->active) {
    return;
  }
  const int win_width = tkbc_get_screen_width();
  const int win_height = tkbc_get_screen_height();

  const float padding = 10;
  const float scalar = win_width / ((popup->text_width + padding) * 9);

  const float width = win_width / (9 * scalar);
  const float height = win_height / (16 * scalar);

  popup->base.x = win_width / 2.0f - width / 2.0f;
  popup->base.y = win_height / 2.0f - height / 2.0f;
  popup->base.width = width;
  popup->base.height = height;

  const int cross_box_size = width * 0.1;
  popup->cross.x = popup->base.x + popup->base.width - cross_box_size;
  popup->cross.y = popup->base.y;
  popup->cross.width = cross_box_size;
  popup->cross.height = cross_box_size;

  const int option1_box_size = cross_box_size * 2;
  popup->option1.x = popup->base.x + popup->base.width - option1_box_size;
  popup->option1.y = popup->base.y + popup->base.height - cross_box_size;
  popup->option1.width = option1_box_size;
  popup->option1.height = cross_box_size;

  popup->option1_text = "QUIT";
}

/**
 * @brief The function draws the given popup with respect to its internal
 * values.
 *
 * @param popup The popup that should be displayed on the screen.
 * @return True if the popup could be displayed; in respect to that if it is
 * active, otherwise false.
 */
bool tkbc_draw_popup(Popup *popup) {
  if (!popup->active) {
    return false;
  }
  const Color save_cross_color = popup->cross_color;
  if (CheckCollisionPointRec(GetMousePosition(), popup->cross)) {
    popup->cross_color = ColorBrightness(popup->cross_color, -0.2);
  }
  const Color save_opiton1_color = popup->option1_color;
  if (CheckCollisionPointRec(GetMousePosition(), popup->option1)) {
    popup->option1_color = ColorBrightness(popup->option1_color, -0.2);
  }

  DrawRectangleRec(popup->base, popup->base_color);
  DrawRectangleRec(popup->option1, popup->option1_color);

  if (popup->option1_text == NULL) {
    popup->option1_text = "OK";
  }

  int opt1_font_size = popup->option1.height;
  Vector2 opt1_text_size = tkbc_reduce_str_to_fit_box(
      popup->font, popup->option1_text, &opt1_font_size, popup->option1);

  Vector2 p = {0};
  p.x = popup->option1.x + popup->option1.width / 2.0 - opt1_text_size.x / 2.0;
  p.y = popup->option1.y + popup->option1.height / 2.0 - opt1_text_size.y / 2.0;
  DrawTextEx(popup->font, popup->option1_text, p, opt1_font_size, 2,
             TKBC_UI_BLACK);

  DrawRectangleRec(popup->cross, popup->cross_color);
  Vector2 start_pos = {.x = popup->cross.x, .y = popup->cross.y};
  Vector2 end_pos = {.x = popup->cross.x + popup->cross.width,
                     .y = popup->cross.y + popup->cross.height};

  const float thick = 3;
  DrawLineEx(start_pos, end_pos, thick, TKBC_UI_BLACK);
  start_pos.x = popup->cross.x + popup->cross.width;
  start_pos.y = popup->cross.y;
  end_pos.x = popup->cross.x;
  end_pos.y = popup->cross.y + popup->cross.height;
  DrawLineEx(start_pos, end_pos, thick, TKBC_UI_BLACK);

  p.x = popup->base.x + popup->base.width / 2.0 - popup->text_width / 2.0;
  p.y = popup->base.y + popup->base.height / 2.0 - popup->font_size / 2.0;
  DrawTextEx(popup->font, popup->text, p, popup->font_size, 2,
             popup->text_color);

  popup->cross_color = save_cross_color;
  popup->option1_color = save_opiton1_color;
  return true;
}

/**
 * @brief The function creates a popup and sets the given message as its main
 * text.
 *
 * @param font The font to use.
 * @param message The text that should be displayed in the main bounding box
 * @return The new created popup.
 */
Popup tkbc_popup_message(Font font, const char *message) {
  const int font_size = 30;
  const Vector2 size = MeasureTextEx(font, message, font_size, 0);

  Popup frame = {
      .active = false,
      .base_color = TKBC_UI_LIGHTGRAY,
      .cross_color = TKBC_UI_RED,
      .option1_color = TKBC_UI_TEAL,
      .text = message,
      .font_size = font_size,
      .text_width = size.x,
      .text_color = TKBC_UI_BLACK,
      .font = font,
  };

  return frame;
}
