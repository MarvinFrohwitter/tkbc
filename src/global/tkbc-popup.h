#ifndef TKBC_POPUP_H_
#define TKBC_POPUP_H_

#include "raylib.h"

typedef struct {
  bool active; // Gives the information if the pop-up is currently displayed.

  Rectangle base; // The rectangle that represents the background bounding box.
  Color base_color; // The color of the base background bounding box.

  Rectangle cross;   // The
  Color cross_color; // The background color of the cross that normally closes
                     // the pop-up.
  Rectangle
      option1; // The bounding box that represents the first selection option.
  Color option1_color; // The background color of the first option bounding box.
  const char
      *option1_text; // The text that should be displayed in the first option.

  const char
      *text; // The text that is displayed in the base container of the pop-up.
             // It is usually the main text positioned in the center.
  int font_size;    // The font size of the main text.
  int text_width;   // The text width of the main text of the pop-up.
  Color text_color; // The text foreground color of the main text.
} Popup; // The structure that contains all the information to display a pop-up.

int tkbc_check_popup_interaction(Popup *popup);
void tkbc_popup_resize_disconnect(Popup *popup);
bool tkbc_draw_popup(Popup *popup);
Popup tkbc_popup_message(const char *message);

#endif // TKBC_POPUP_H_
