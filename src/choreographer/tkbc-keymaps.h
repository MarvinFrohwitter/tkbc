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

bool tkbc_check_keymap(Key_Map keymap, Key_Map_Check_Config cfg, int kso);
bool tkbc_check_keymap_full(Key_Map keymap, Key_Map_Check_Config cfg);
bool tkbc_check_keymaps(Key_Maps keymaps, int hash, Key_Map_Check_Config cfg, int kso);
bool tkbc_check_keymaps_full(Key_Maps keymaps, int hash, Key_Map_Check_Config cfg);

#define KEY_CONFIG(value_key, value_mode_key, value_selection_key)             \
  ((Key_Map_Check_Config){.key = (value_key),                                  \
                          .mod_key = (value_mode_key),                         \
                          .selection_key = (value_selection_key)})
#define VA_KEY_CONFIG(...) ((Key_Map_Check_Config){__VA_ARGS__})

/*-------------- DEFAULT CONFIGS -------------------------------------------*/

static const Key_Map_Check_Config KEY_MAP_CHECK_DOWN = {
    .key = MODE_DOWN,
    .mod_key = MODE_DOWN,
    .selection_key = MODE_DOWN,
};

static const Key_Map_Check_Config KEY_MAP_CHECK_UP = {
    .key = MODE_UP,
    .mod_key = MODE_UP,
    .selection_key = MODE_UP,
};

static const Key_Map_Check_Config KEY_MAP_CHECK_PRESSED = {
    .key = MODE_PRESSED,
    .mod_key = MODE_PRESSED,
    .selection_key = MODE_PRESSED,
};

static const Key_Map_Check_Config KEY_MAP_CHECK_RELEASED = {
    .key = MODE_RELEASED,
    .mod_key = MODE_RELEASED,
    .selection_key = MODE_RELEASED,
};

static const Key_Map_Check_Config KEY_MAP_CHECK_KEY_PRESSED_MOD_DOWN = {
    .key = MODE_PRESSED,
    .mod_key = MODE_DOWN,
    .selection_key = MODE_DOWN, // Don't care for default behavior
};

static const Key_Map_Check_Config KEY_MAP_CHECK_KEY_PRESSED = {
    .key = MODE_PRESSED,
    .mod_key = MODE_DOWN,       // Don't care for default behavior
    .selection_key = MODE_DOWN, // Don't care for default behavior
};

#endif // TKBC_KEYMAPS_H
