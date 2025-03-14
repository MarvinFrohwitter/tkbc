#include "tkbc-keymaps.h"
#include "../../external/lexer/tkbc-lexer.h"
#include "../global/tkbc-types.h"
#include "../global/tkbc-utils.h"
#include <stddef.h>
#include <stdio.h>

#include "../config.h"

int tkbc_load_keymaps_from_file(Key_Maps *keymaps, const char *filename) {
  Content content = {0};
  int ok = tkbc_read_file(filename, &content);
  if (ok == -1) {
    return ok;
  }
  Lexer *lexer = lexer_new(filename, content.elements, content.count, 0);
  Token t = lexer_next(lexer);
  while (t.kind != EOF_TOKEN && t.kind != INVALID) {
    if (t.kind != NUMBER) {
      check_return(1);
    }
    int hash = atoi(lexer_token_to_cstr(lexer, &t));
    t = lexer_next(lexer);
    if (t.kind != NUMBER) {
      check_return(1);
    }
    int mod_key = atoi(lexer_token_to_cstr(lexer, &t));
    t = lexer_next(lexer);
    if (t.kind != NUMBER) {
      check_return(1);
    }
    int mod_co_key = atoi(lexer_token_to_cstr(lexer, &t));
    t = lexer_next(lexer);
    if (t.kind != NUMBER) {
      check_return(1);
    }
    int selection_key1 = atoi(lexer_token_to_cstr(lexer, &t));
    t = lexer_next(lexer);
    if (t.kind != NUMBER) {
      check_return(1);
    }
    int selection_key2 = atoi(lexer_token_to_cstr(lexer, &t));
    t = lexer_next(lexer);
    if (t.kind != NUMBER) {
      check_return(1);
    }
    int key = atoi(lexer_token_to_cstr(lexer, &t));
    t = lexer_next(lexer);

    for (size_t i = 0; i < keymaps->count; ++i) {
      if (hash == keymaps->elements[i].hash) {
        keymaps->elements[i].mod_key = mod_key;
        keymaps->elements[i].mod_co_key = mod_co_key;
        keymaps->elements[i].selection_key1 = selection_key1;
        keymaps->elements[i].selection_key2 = selection_key2;
        keymaps->elements[i].key = key;
        break;
      }
    }
  }
  tkbc_setup_keymaps_strs(keymaps);

check:
  lexer_del(lexer);
  return ok;
}

bool tkbc_save_keymaps_to_file(Key_Maps keymaps, const char *filename) {
  FILE *file = fopen(filename, "wb");
  if (file == NULL) {
    tkbc_fprintf(stderr, "ERROR", "%s:%d:%s\n", __FILE__, __LINE__,
                 strerror(errno));
    return false;
  }
  for (size_t i = 0; i < keymaps.count; ++i) {
    fprintf(file, "%d %d %d %d %d %d\n", keymaps.elements[i].hash,
            keymaps.elements[i].mod_key, keymaps.elements[i].mod_co_key,
            keymaps.elements[i].selection_key1,
            keymaps.elements[i].selection_key2, keymaps.elements[i].key);
  }

  fclose(file);
  return true;
}

void tkbc_init_keymaps_defaults(Key_Maps *keymaps) {
  for (size_t i = 0; i < ARRAY_LENGTH(default_keymaps); ++i) {
    tkbc_dap(keymaps, default_keymaps[i]);
  }
  tkbc_setup_keymaps_strs(keymaps);
}

void tkbc_set_keymaps_defaults(Key_Maps *keymaps) {
  size_t default_keymaps_count = ARRAY_LENGTH(default_keymaps);
  assert(keymaps->count == default_keymaps_count);
  for (size_t i = 0; i < default_keymaps_count; ++i) {
    for (size_t j = 0; j < keymaps->count; ++j) {
      if (keymaps->elements[j].hash == default_keymaps[i].hash) {
        keymaps->elements[j].mod_key = default_keymaps[i].mod_key;
        keymaps->elements[j].mod_co_key = default_keymaps[i].mod_co_key;
        keymaps->elements[j].selection_key1 = default_keymaps[i].selection_key1;
        keymaps->elements[j].selection_key2 = default_keymaps[i].selection_key2;
        keymaps->elements[j].key = default_keymaps[i].key;
      }
    }
  }
  tkbc_setup_keymaps_strs(keymaps);
}

void tkbc_setup_keymaps_strs(Key_Maps *keymaps) {
  for (size_t i = 0; i < keymaps->count; ++i) {
    keymaps->elements[i].mod_key_str =
        tkbc_key_to_str(keymaps->elements[i].mod_key);
    keymaps->elements[i].mod_co_key_str =
        tkbc_key_to_str(keymaps->elements[i].mod_co_key);
    keymaps->elements[i].selection_key1_str =
        tkbc_key_to_str(keymaps->elements[i].selection_key1);
    keymaps->elements[i].selection_key2_str =
        tkbc_key_to_str(keymaps->elements[i].selection_key2);
    keymaps->elements[i].key_str = tkbc_key_to_str(keymaps->elements[i].key);
  }
}

int tkbc_hash_to_key(Key_Maps keymaps, int hash) {
  for (size_t i = 0; i < keymaps.count; ++i) {
    if (hash == keymaps.elements[i].hash) {
      return keymaps.elements[i].key;
    }
  }
  return 0;
}

Key_Map tkbc_hash_to_keymap(Key_Maps keymaps, int hash) {
  for (size_t i = 0; i < keymaps.count; ++i) {
    if (hash == keymaps.elements[i].hash) {
      return keymaps.elements[i];
    }
  }
  return (Key_Map){0};
}

const char *tkbc_key_to_str(int key) {
  switch (key) {
  case KEY_NULL: // Key: NULL, used for no key pressed
    return "null";
  // Alphanumeric keys
  case KEY_APOSTROPHE: // Key: '
    return "apostrophe";
  case KEY_COMMA: // Key: ,
    return "comma";
  case KEY_MINUS: // Key: -
    return "minus";
  case KEY_PERIOD: // Key: .
    return "period";
  case KEY_SLASH: // Key: /
    return "slash";
  case KEY_ZERO: // Key: 0
    return "zero";
  case KEY_ONE: // Key: 1
    return "one";
  case KEY_TWO: // Key: 2
    return "two";
  case KEY_THREE: // Key: 3
    return "three";
  case KEY_FOUR: // Key: 4
    return "four";
  case KEY_FIVE: // Key: 5
    return "five";
  case KEY_SIX: // Key: 6
    return "six";
  case KEY_SEVEN: // Key: 7
    return "seven";
  case KEY_EIGHT: // Key: 8
    return "eight";
  case KEY_NINE: // Key: 9
    return "nine";
  case KEY_SEMICOLON: // Key: ;
    return "semicolon";
  case KEY_EQUAL: // Key: =
    return "equal";
  case KEY_A: // Key: A | a
    return "a";
  case KEY_B: // Key: B | b
    return "b";
  case KEY_C: // Key: C | c
    return "c";
  case KEY_D: // Key: D | d
    return "d";
  case KEY_E: // Key: E | e
    return "e";
  case KEY_F: // Key: F | f
    return "f";
  case KEY_G: // Key: G | g
    return "g";
  case KEY_H: // Key: H | h
    return "h";
  case KEY_I: // Key: I | i
    return "i";
  case KEY_J: // Key: J | j
    return "j";
  case KEY_K: // Key: K | k
    return "k";
  case KEY_L: // Key: L | l
    return "l";
  case KEY_M: // Key: M | m
    return "m";
  case KEY_N: // Key: N | n
    return "n";
  case KEY_O: // Key: O | o
    return "o";
  case KEY_P: // Key: P | p
    return "p";
  case KEY_Q: // Key: Q | q
    return "q";
  case KEY_R: // Key: R | r
    return "r";
  case KEY_S: // Key: S | s
    return "s";
  case KEY_T: // Key: T | t
    return "t";
  case KEY_U: // Key: U | u
    return "u";
  case KEY_V: // Key: V | v
    return "v";
  case KEY_W: // Key: W | w
    return "w";
  case KEY_X: // Key: X | x
    return "x";
  case KEY_Y: // Key: Y | y
    return "de_z";
  case KEY_Z: // Key: Z | z
    return "de_y";
  case KEY_LEFT_BRACKET: // Key: [
    return "left_bracket";
  case KEY_BACKSLASH: // Key: '\'
    return "backslash";
  case KEY_RIGHT_BRACKET: // Key: ]
    return "right_bracket";
  case KEY_GRAVE: // Key: `
    return "grave";
  // Function keys
  case KEY_SPACE: // Key: Space
    return "space";
  case KEY_ESCAPE: // Key: Esc
    return "escape";
  case KEY_ENTER: // Key: Enter
    return "enter";
  case KEY_TAB: // Key: Tab
    return "tab";
  case KEY_BACKSPACE: // Key: Backspace
    return "backspace";
  case KEY_INSERT: // Key: Ins
    return "insert";
  case KEY_DELETE: // Key: Del
    return "delete";
  case KEY_RIGHT: // Key: Cursor right
    return "right";
  case KEY_LEFT: // Key: Cursor left
    return "left";
  case KEY_DOWN: // Key: Cursor down
    return "down";
  case KEY_UP: // Key: Cursor up
    return "up";
  case KEY_PAGE_UP: // Key: Page up
    return "page_up";
  case KEY_PAGE_DOWN: // Key: Page down
    return "page_down";
  case KEY_HOME: // Key: Home
    return "home";
  case KEY_END: // Key: End
    return "end";
  case KEY_CAPS_LOCK: // Key: Caps lock
    return "caps_lock";
  case KEY_SCROLL_LOCK: // Key: Scroll down
    return "scroll_lock";
  case KEY_NUM_LOCK: // Key: Num lock
    return "num_lock";
  case KEY_PRINT_SCREEN: // Key: Print screen
    return "print_screen";
  case KEY_PAUSE: // Key: Pause
    return "pause";
  case KEY_F1: // Key: F1
    return "f1";
  case KEY_F2: // Key: F2
    return "f2";
  case KEY_F3: // Key: F3
    return "f3";
  case KEY_F4: // Key: F4
    return "f4";
  case KEY_F5: // Key: F5
    return "f5";
  case KEY_F6: // Key: F6
    return "f6";
  case KEY_F7: // Key: F7
    return "f7";
  case KEY_F8: // Key: F8
    return "f8";
  case KEY_F9: // Key: F9
    return "f9";
  case KEY_F10: // Key: F10
    return "f10";
  case KEY_F11: // Key: F11
    return "f11";
  case KEY_F12: // Key: F12
    return "f12";
  case KEY_LEFT_SHIFT: // Key: Shift left
    return "left_shift";
  case KEY_LEFT_CONTROL: // Key: Control left
    return "left_control";
  case KEY_LEFT_ALT: // Key: Alt left
    return "left_alt";
  case KEY_LEFT_SUPER: // Key: Super left
    return "left_super";
  case KEY_RIGHT_SHIFT: // Key: Shift right
    return "right_shift";
  case KEY_RIGHT_CONTROL: // Key: Control right
    return "right_control";
  case KEY_RIGHT_ALT: // Key: Alt right
    return "right_alt";
  case KEY_RIGHT_SUPER: // Key: Super right
    return "right_super";
  case KEY_KB_MENU: // Key: KB menu
    return "kb_menu";
  // Keypad keys
  case KEY_KP_0: // Key: Keypad 0
    return "kp_0";
  case KEY_KP_1: // Key: Keypad 1
    return "kp_1";
  case KEY_KP_2: // Key: Keypad 2
    return "kp_2";
  case KEY_KP_3: // Key: Keypad 3
    return "kp_3";
  case KEY_KP_4: // Key: Keypad 4
    return "kp_4";
  case KEY_KP_5: // Key: Keypad 5
    return "kp_5";
  case KEY_KP_6: // Key: Keypad 6
    return "kp_6";
  case KEY_KP_7: // Key: Keypad 7
    return "kp_7";
  case KEY_KP_8: // Key: Keypad 8
    return "kp_8";
  case KEY_KP_9: // Key: Keypad 9
    return "kp_9";
  case KEY_KP_DECIMAL: // Key: Keypad .
    return "kp_decimal";
  case KEY_KP_DIVIDE: // Key: Keypad /
    return "kp_divide";
  case KEY_KP_MULTIPLY: // Key: Keypad *
    return "kp_multiply";
  case KEY_KP_SUBTRACT: // Key: Keypad -
    return "kp_subtract";
  case KEY_KP_ADD: // Key: Keypad +
    return "kp_add";
  case KEY_KP_ENTER: // Key: Keypad Enter
    return "kp_enter";
  case KEY_KP_EQUAL: // Key: Keypad =
    return "kp_equal";
  // Android key buttons
  case KEY_BACK: // Key: Android back button
    return "back";
  case KEY_MENU: // Key: Android menu button
    return "menu";
  case KEY_VOLUME_UP: // Key: Android volume up button
    return "volume_up";
  case KEY_VOLUME_DOWN: // Key: Android volume down button
    return "volume_down";
  case 161:
    return "de_less_than";
  default:
    return "???";
  }
}
