#ifndef TKBC_UI_H
#define TKBC_UI_H

#include "../global/tkbc-types.h"
void tkbc_draw_ui(Env *env);
void tkbc_ui_timeline(Env *env, size_t block_index, size_t block_index_count);

void tkbc_set_key_or_delete(int *dest_key, const char **dest_str,
                            int key_value);
void tkbc_draw_key_box(Env *env, Rectangle rectangle, Key_Box iteration,
                       size_t cur_major_box);
void tkbc_ui_keymaps(Env *env);

#endif // TKBC_UI_H
