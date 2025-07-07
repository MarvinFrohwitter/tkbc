#ifndef TKBC_UI_H
#define TKBC_UI_H

#include "../global/tkbc-types.h"
void tkbc_draw_ui(Env *env);
void tkbc_display_kite_information_speeds(Kite_State *kite_state);
void tkbc_display_kite_information(Env *env);

void tkbc_scrollbar(Env *env, Scrollbar *scrollbar, Rectangle outer_container,
                    size_t items_count, size_t *top_interaction_box);
bool tkbc_ui_script_menu(Env *env);
void tkbc_ui_timeline(Env *env, size_t block_index, size_t block_index_count);

void tkbc_set_key_or_delete(int *dest_key, const char **dest_str,
                            int key_value);
void tkbc_draw_key_box(Env *env, Rectangle rectangle, Key_Box iteration,
                       size_t cur_major_box);
void tkbc_ui_keymaps(Env *env);

void tkbc_ui_color_picker(Env *env);
void tkbc_set_color_for_selected_kites(Env *env, Color color);
void tkbc_set_texture_for_selected_kites(Env *env, Kite_Texture *kite_texture);

#endif // TKBC_UI_H
