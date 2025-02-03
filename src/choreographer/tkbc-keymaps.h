#ifndef TKBC_KEYMAPS_H
#define TKBC_KEYMAPS_H

#include <raylib.h>
#include <stddef.h>

#include "../global/tkbc-types.h"

int tkbc_load_keymaps_from_file(KeyMaps *keymaps, const char *filename);
bool tkbc_save_keymaps_to_file(KeyMaps keymaps, const char *filename);
void tkbc_set_keymaps_defaults(KeyMaps *keymaps);
void tkbc_setup_keymaps(KeyMaps *keymaps);
const char *tkbc_key_to_str(int key);
int tkbc_hash_to_key(KeyMaps keymaps, int hash);
KeyMap tkbc_hash_to_keymap(KeyMaps keymaps, int hash);

#endif // TKBC_KEYMAPS_H
