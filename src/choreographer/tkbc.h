#ifndef TKBC_H_
#define TKBC_H_

#include "../global/tkbc-types.h"
#include "raylib.h"

#define TEAL CLITERAL(Color){0, 128, 128, 255} // Teal

#define ICAREX_weiß       CLITERAL(Color){0xFF, 0xFF, 0xFF, 0xFF}
#define ICAREX_hellgrau   CLITERAL(Color){169,  169,  169,  0xFF}
#define ICAREX_dunkelgrau CLITERAL(Color){58,   58,   58,   0xFF}
#define ICAREX_schwarz    CLITERAL(Color){0,    0,    0,    0xFF}
#define ICAREX_rot        CLITERAL(Color){255,  0,    0,    0xFF}
#define ICAREX_orange     CLITERAL(Color){255,  101,  50,   0xFF}
#define ICAREX_gold       CLITERAL(Color){204,  153,  0,    0xFF}
#define ICAREX_gelb       CLITERAL(Color){255,  204,  50,   0xFF}

#define ICAREX_grün       CLITERAL(Color){0,    128,  0,    0xFF}
#define ICAREX_cedar      CLITERAL(Color){60,   179,  113,  0xFF}

#define ICAREX_teal       CLITERAL(Color){0,    153,  153,  0xFF}
#define ICAREX_caribbean  CLITERAL(Color){44,   164,  173,  0xFF}
#define ICAREX_slate      CLITERAL(Color){70,   130,  180,  79}
#define ICAREX_hellblau   CLITERAL(Color){50,   211,  255,  0xFF}

#define ICAREX_blau       CLITERAL(Color){30,   144,  255,  0xFF}
#define ICAREX_dunkelblau CLITERAL(Color){0,    0,    205,  0xFF}

#define ICAREX_plum       CLITERAL(Color){132,  68,   82,   0xFF}
#define ICAREX_aubergin   CLITERAL(Color){102,  0,    80,   0xFF}
#define ICAREX_milkalila1 CLITERAL(Color){147,  95,   255,  0xFF}
#define ICAREX_milkalila2 CLITERAL(Color){114,  84,   154,  0xFF}
#define ICAREX_lila       CLITERAL(Color){153,  0,    153,  0xFF}

#define ICAREX_rasberry   CLITERAL(Color){255,  0,    129,  0xFF}
#define ICAREX_zartrosa   CLITERAL(Color){255,  136,  153,  0xFF}
#define ICAREX_brown      CLITERAL(Color){139,  69,   0,    0xFF}

#define ICAREX_neongelb   CLITERAL(Color){199,  255,  0,    0xFF}
#define ICAREX_neonorange CLITERAL(Color){255,  65,   0,    0xFF}
#define ICAREX_neongrün   CLITERAL(Color){0,    255,  0,    0xFF}


#define TKBC_UI_WHITE ColorBrightness(WHITE, 0)
#define TKBC_UI_SKYBLUE ColorBrightness(SKYBLUE, 0)
#define TKBC_UI_PURPLE ColorBrightness(PURPLE, 0)
#define TKBC_UI_LIGHTGRAY ColorBrightness(LIGHTGRAY, 0)

#define TKBC_UI_TEAL ColorBrightness(TEAL, 0.1)
#define TKBC_UI_GRAY ColorBrightness(GRAY, -0.2)
#define TKBC_UI_RED ColorBrightness(RED, -0.3)
#define TKBC_UI_BLACK ColorBrightness(BLACK, -0.2)

#define ALPHA_RATIO 0.7
#define TKBC_UI_TEAL_ALPHA ColorAlpha(TKBC_UI_TEAL, ALPHA_RATIO)
#define TKBC_UI_PURPLE_ALPHA ColorAlpha(PURPLE, ALPHA_RATIO)
#define TKBC_UI_DARKPURPLE_ALPHA ColorAlpha(DARKPURPLE, ALPHA_RATIO)
#define TKBC_UI_LIGHTGRAY_ALPHA ColorAlpha(GRAY, ALPHA_RATIO - 0.2)
#define TKBC_UI_GRAY_ALPHA ColorAlpha(GRAY, ALPHA_RATIO - 0.2)
#define TKBC_UI_WHITE_ALPHA ColorAlpha(WHITE, ALPHA_RATIO)

#define VECTOR2_FMT "(%f,%f)"
#define Vector2_FMT_ARGS(arg) (float)(arg).x, (float)(arg).y
#ifndef EPSILON
#define EPSILON 0.001f
#endif // EPSILON
#define TARGET_FPS 60
#define TARGET_DT (1 / (double)TARGET_FPS)
// #define TARGET_DT 0

// ===========================================================================
// ===========================================================================
// ========================== KITE DECLARATIONS ==============================
// ===========================================================================
// ===========================================================================

Env *tkbc_init_env(void);
Kite_State tkbc_init_kite(void);
void tkbc_destroy_env(Env *env);
void tkbc_destroy_kite(Kite_State *state);
void tkbc_destroy_kite_array(Kite_States *kite_states);
bool tkbc_remove_kite_from_list(Kite_States *kite_array, size_t kite_id);
void tkbc_kite_array_start_position(Kite_States *kite_states,
                                    size_t window_width, size_t window_height);

void tkbc_file_handler(Env *env);
void tkbc_set_kite_defaults(Kite *kite, bool is_generated);
void tkbc_set_kite_state_defaults(Kite_State *state);
void tkbc_set_kite_internals(Kite *kite, float fly_speed, float turn_speed,
                             Color body_color, Color top_color, float overlap,
                             float inner_space, float spread, float width,
                             float height, float angle, Vector2 center,
                             float scale);

// ========================== KITE POSITION ==================================

void tkbc_kite_update_internal(Kite *kite);
void tkbc_kite_update_scale(Kite *kite, float scale);
void tkbc_kite_update_position(Kite *kite, Vector2 *position);
void tkbc_kite_update_angle(Kite *kite, float center_deg_rotation);
void tkbc_center_rotation(Kite *kite, Vector2 *position,
                          float center_deg_rotation);
void tkbc_tip_rotation(Kite *kite, Vector2 *position, float tip_deg_rotation,
                       TIP tip);

// ========================== KITE DISPLAY ===================================

void tkbc_draw_kite(Kite_State *state);
void tkbc_draw_kite_array(Kite_States *kite_states);
void tkbc_update_kites_for_resize_window(Env *env);

bool tkbc_set_kite_texture(Kite *kite, Kite_Texture *kite_texture);
Color tkbc_get_random_color(void);

#endif // TKBC_H_
