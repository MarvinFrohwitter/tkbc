#ifndef TKBC_POPUP_H_
#define TKBC_POPUP_H_

#include "raylib.h"

typedef struct {
  bool active;

  Rectangle base;
  Color base_color;

  Rectangle cross;
  Color cross_color;

  Rectangle option1;
  Color option1_color;
  const char *option1_text;

  const char *text;
  int font_size;
  int text_width;
  Color text_color;
} Popup;

int tkbc_check_popup_interaction(Popup *popup);
void tkbc_popup_resize_disconnect(Popup *popup);
bool tkbc_draw_popup(Popup *popup);
Popup tkbc_popup_message(const char *message);

#endif // TKBC_POPUP_H_
