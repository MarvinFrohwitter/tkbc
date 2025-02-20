#ifndef TKBC_KEYMAPS_H
#define TKBC_KEYMAPS_H

#include <raylib.h>
#include <stddef.h>

#include "../global/tkbc-types.h"

int tkbc_load_keymaps_from_file(Key_Maps *keymaps, const char *filename);
bool tkbc_save_keymaps_to_file(Key_Maps keymaps, const char *filename);
void tkbc_init_keymaps_defaults(Key_Maps *keymaps);
void tkbc_set_keymaps_defaults(Key_Maps *keymaps);
void tkbc_setup_keymaps_strs(Key_Maps *keymaps);
const char *tkbc_key_to_str(int key);
int tkbc_hash_to_key(Key_Maps keymaps, int hash);
Key_Map tkbc_hash_to_keymap(Key_Maps keymaps, int hash);

#endif // TKBC_KEYMAPS_H
