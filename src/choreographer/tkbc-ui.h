#ifndef TKBC_UI_H
#define TKBC_UI_H

#include "../global/tkbc-types.h"
void tkbc_draw_ui(Env *env);
void tkbc_ui_timeline(Env *env, size_t block_index, size_t block_index_count);
void tkbc_ui_keymaps(Env *env);

#endif // TKBC_UI_H
