#ifndef TKBC_H_
#define TKBC_H_

#include "../global/tkbc-types.h"
#include "raylib.h"

#define TEAL CLITERAL(Color){0, 128, 128, 255} // Teal

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

#define VECTOR2_FMT "(%f,%f)"
#define Vector2_FMT_ARGS(arg) (float)(arg).x, (float)(arg).y
#ifndef EPSILON
#define EPSILON 0.001f
#endif // EPSILON
static const int TARGET_FPS = 120;

// ===========================================================================
// ===========================================================================
// ========================== KITE DECLARATIONS ==============================
// ===========================================================================
// ===========================================================================

Env *tkbc_init_env(void);
Kite_State *tkbc_init_kite(void);
void tkbc_destroy_env(Env *env);
void tkbc_destroy_kite(Kite_State *state);
void tkbc_destroy_kite_array(Kite_States *kite_states);
void tkbc_kite_array_start_position(Kite_States *kite_states,
                                    size_t window_width, size_t window_height);

void tkbc_file_handler(Env *env);
void tkbc_set_kite_defaults(Kite *kite, bool is_generated);
void tkbc_set_kite_state_defaults(Kite_State *state);

// ========================== KITE POSITION ==================================

void tkbc_kite_update_internal(Kite *kite);
void tkbc_kite_update_position(Kite *kite, Vector2 *position);
void tkbc_kite_update_angle(Kite *kite, float center_deg_rotation);
void tkbc_center_rotation(Kite *kite, Vector2 *position,
                          float center_deg_rotation);
void tkbc_tip_rotation(Kite *kite, Vector2 *position, float tip_deg_rotation,
                       TIP tip);
void tkbc_circle_rotation(Kite *kite, Vector2 *position, float deg_rotation,
                          float radius, TIP tip, bool below);

// ========================== KITE DISPLAY ===================================

void tkbc_draw_kite(Kite *kite);
void tkbc_draw_kite_array(Kite_States *kite_states);
void tkbc_update_kites_for_resize_window(Env *env);

// ========================== UI DECLARATIONS ==============================

void tkbc_draw_ui(Env *env);
void tkbc_ui_timeline(Env *env, size_t block_index, size_t block_index_count);
Color tkbc_get_random_color();

#endif // TKBC_H_
