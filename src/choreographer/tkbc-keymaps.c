#include "tkbc-keymaps.h"
#include "../../external/lexer/tkbc-lexer.h"
#include "../global/tkbc-types.h"
#include "../global/tkbc-utils.h"
#include <stddef.h>
#include <stdio.h>

#include "../config.h"

int tkbc_load_keymaps_from_file(KeyMaps *keymaps, const char *filename) {
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

check:
  lexer_del(lexer);
  return ok;
}

bool tkbc_save_keymaps_to_file(KeyMaps keymaps, const char *filename) {
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

void tkbc_set_keymaps_defaults(KeyMaps *keymaps) {
  for (size_t i = 0; i < ARRAY_LENGTH(default_keymaps); ++i) {
    tkbc_dap(keymaps, default_keymaps[i]);
  }
}

void tkbc_setup_keymaps(KeyMaps *keymaps) {
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

int tkbc_hash_to_key(KeyMaps keymaps, int hash) {
  for (size_t i = 0; i < keymaps.count; ++i) {
    if (hash == keymaps.elements[i].hash) {
      return keymaps.elements[i].key;
    }
  }
  return 0;
}

KeyMap tkbc_hash_to_keymap(KeyMaps keymaps, int hash) {
  for (size_t i = 0; i < keymaps.count; ++i) {
    if (hash == keymaps.elements[i].hash) {
      return keymaps.elements[i];
    }
  }
  return (KeyMap){0};
}

const char *tkbc_key_to_str(int key) {
  switch (key) {
  case KEY_NULL: // Key: NULL, used for no key pressed
    return "key_null";
  // Alphanumeric keys
  case KEY_APOSTROPHE: // Key: '
    return "key_apostrophe";
  case KEY_COMMA: // Key: ,
    return "key_comma";
  case KEY_MINUS: // Key: -
    return "key_minus";
  case KEY_PERIOD: // Key: .
    return "key_period";
  case KEY_SLASH: // Key: /
    return "key_slash";
  case KEY_ZERO: // Key: 0
    return "key_zero";
  case KEY_ONE: // Key: 1
    return "key_one";
  case KEY_TWO: // Key: 2
    return "key_two";
  case KEY_THREE: // Key: 3
    return "key_three";
  case KEY_FOUR: // Key: 4
    return "key_four";
  case KEY_FIVE: // Key: 5
    return "key_five";
  case KEY_SIX: // Key: 6
    return "key_six";
  case KEY_SEVEN: // Key: 7
    return "key_seven";
  case KEY_EIGHT: // Key: 8
    return "key_eight";
  case KEY_NINE: // Key: 9
    return "key_nine";
  case KEY_SEMICOLON: // Key: ;
    return "key_semicolon";
  case KEY_EQUAL: // Key: =
    return "key_equal";
  case KEY_A: // Key: A | a
    return "key_a";
  case KEY_B: // Key: B | b
    return "key_b";
  case KEY_C: // Key: C | c
    return "key_c";
  case KEY_D: // Key: D | d
    return "key_d";
  case KEY_E: // Key: E | e
    return "key_e";
  case KEY_F: // Key: F | f
    return "key_f";
  case KEY_G: // Key: G | g
    return "key_g";
  case KEY_H: // Key: H | h
    return "key_h";
  case KEY_I: // Key: I | i
    return "key_i";
  case KEY_J: // Key: J | j
    return "key_j";
  case KEY_K: // Key: K | k
    return "key_k";
  case KEY_L: // Key: L | l
    return "key_l";
  case KEY_M: // Key: M | m
    return "key_m";
  case KEY_N: // Key: N | n
    return "key_n";
  case KEY_O: // Key: O | o
    return "key_o";
  case KEY_P: // Key: P | p
    return "key_p";
  case KEY_Q: // Key: Q | q
    return "key_q";
  case KEY_R: // Key: R | r
    return "key_r";
  case KEY_S: // Key: S | s
    return "key_s";
  case KEY_T: // Key: T | t
    return "key_t";
  case KEY_U: // Key: U | u
    return "key_u";
  case KEY_V: // Key: V | v
    return "key_v";
  case KEY_W: // Key: W | w
    return "key_w";
  case KEY_X: // Key: X | x
    return "key_x";
  case KEY_Y: // Key: Y | y
    return "key_y";
  case KEY_Z: // Key: Z | z
    return "key_z";
  case KEY_LEFT_BRACKET: // Key: [
    return "key_left_bracket";
  case KEY_BACKSLASH: // Key: '\'
    return "key_backslash";
  case KEY_RIGHT_BRACKET: // Key: ]
    return "key_right_bracket";
  case KEY_GRAVE: // Key: `
    return "key_grave";
  // Function keys
  case KEY_SPACE: // Key: Space
    return "key_space";
  case KEY_ESCAPE: // Key: Esc
    return "key_escape";
  case KEY_ENTER: // Key: Enter
    return "key_enter";
  case KEY_TAB: // Key: Tab
    return "key_tab";
  case KEY_BACKSPACE: // Key: Backspace
    return "key_backspace";
  case KEY_INSERT: // Key: Ins
    return "key_insert";
  case KEY_DELETE: // Key: Del
    return "key_delete";
  case KEY_RIGHT: // Key: Cursor right
    return "key_right";
  case KEY_LEFT: // Key: Cursor left
    return "key_left";
  case KEY_DOWN: // Key: Cursor down
    return "key_down";
  case KEY_UP: // Key: Cursor up
    return "key_up";
  case KEY_PAGE_UP: // Key: Page up
    return "key_page_up";
  case KEY_PAGE_DOWN: // Key: Page down
    return "key_page_down";
  case KEY_HOME: // Key: Home
    return "key_home";
  case KEY_END: // Key: End
    return "key_end";
  case KEY_CAPS_LOCK: // Key: Caps lock
    return "key_caps_lock";
  case KEY_SCROLL_LOCK: // Key: Scroll down
    return "key_scroll_lock";
  case KEY_NUM_LOCK: // Key: Num lock
    return "key_num_lock";
  case KEY_PRINT_SCREEN: // Key: Print screen
    return "key_print_screen";
  case KEY_PAUSE: // Key: Pause
    return "key_pause";
  case KEY_F1: // Key: F1
    return "key_f1";
  case KEY_F2: // Key: F2
    return "key_f2";
  case KEY_F3: // Key: F3
    return "key_f3";
  case KEY_F4: // Key: F4
    return "key_f4";
  case KEY_F5: // Key: F5
    return "key_f5";
  case KEY_F6: // Key: F6
    return "key_f6";
  case KEY_F7: // Key: F7
    return "key_f7";
  case KEY_F8: // Key: F8
    return "key_f8";
  case KEY_F9: // Key: F9
    return "key_f9";
  case KEY_F10: // Key: F10
    return "key_f10";
  case KEY_F11: // Key: F11
    return "key_f11";
  case KEY_F12: // Key: F12
    return "key_f12";
  case KEY_LEFT_SHIFT: // Key: Shift left
    return "key_left_shift";
  case KEY_LEFT_CONTROL: // Key: Control left
    return "key_left_control";
  case KEY_LEFT_ALT: // Key: Alt left
    return "key_left_alt";
  case KEY_LEFT_SUPER: // Key: Super left
    return "key_left_super";
  case KEY_RIGHT_SHIFT: // Key: Shift right
    return "key_right_shift";
  case KEY_RIGHT_CONTROL: // Key: Control right
    return "key_right_control";
  case KEY_RIGHT_ALT: // Key: Alt right
    return "key_right_alt";
  case KEY_RIGHT_SUPER: // Key: Super right
    return "key_right_super";
  case KEY_KB_MENU: // Key: KB menu
    return "key_kb_menu";
  // Keypad keys
  case KEY_KP_0: // Key: Keypad 0
    return "key_kp_0";
  case KEY_KP_1: // Key: Keypad 1
    return "key_kp_1";
  case KEY_KP_2: // Key: Keypad 2
    return "key_kp_2";
  case KEY_KP_3: // Key: Keypad 3
    return "key_kp_3";
  case KEY_KP_4: // Key: Keypad 4
    return "key_kp_4";
  case KEY_KP_5: // Key: Keypad 5
    return "key_kp_5";
  case KEY_KP_6: // Key: Keypad 6
    return "key_kp_6";
  case KEY_KP_7: // Key: Keypad 7
    return "key_kp_7";
  case KEY_KP_8: // Key: Keypad 8
    return "key_kp_8";
  case KEY_KP_9: // Key: Keypad 9
    return "key_kp_9";
  case KEY_KP_DECIMAL: // Key: Keypad .
    return "key_kp_decimal";
  case KEY_KP_DIVIDE: // Key: Keypad /
    return "key_kp_divide";
  case KEY_KP_MULTIPLY: // Key: Keypad *
    return "key_kp_multiply";
  case KEY_KP_SUBTRACT: // Key: Keypad -
    return "key_kp_subtract";
  case KEY_KP_ADD: // Key: Keypad +
    return "key_kp_add";
  case KEY_KP_ENTER: // Key: Keypad Enter
    return "key_kp_enter";
  case KEY_KP_EQUAL: // Key: Keypad =
    return "key_kp_equal";
  // Android key buttons
  case KEY_BACK: // Key: Android back button
    return "key_back";
  case KEY_MENU: // Key: Android menu button
    return "key_menu";
  case KEY_VOLUME_UP: // Key: Android volume up button
    return "key_volume_up";
  case KEY_VOLUME_DOWN: // Key: Android volume down button
    return "key_volume_down";
  default:
    assert(0 && "UNKNOWN KEY");
  }
}
