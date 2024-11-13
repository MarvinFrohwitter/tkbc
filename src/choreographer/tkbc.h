#ifndef TKBC_H_
#define TKBC_H_

#include "../global/tkbc-types.h"
#include "raylib.h"

#define TEAL CLITERAL(Color){0, 128, 128, 255} // Teal

#define VECTOR2_FMT "(%f,%f)"
#define Vector2_FMT_ARGS(arg) (float)(arg).x, (float)(arg).y
#define EPSILON 0.001f
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
void tkbc_kite_array_generate(Env *env, size_t kite_count);
void tkbc_kite_array_start_position(Kite_States *kite_states,
                                    size_t window_width, size_t window_height);

void tkbc_file_handler(Env *env);
void tkbc_set_kite_defaults(Kite *kite, bool is_generated);
void tkbc_set_kite_state_defaults(Kite_State *state);

// ========================== KITE POSITION ==================================

void tkbc_center_rotation(Kite *kite, Vector2 *position,
                          float center_deg_rotation);
void tkbc_tip_rotation(Kite *kite, Vector2 *position, float tip_deg_rotation,
                       TIP tip);
void tkbc_circle_rotation(Kite *kite, Vector2 *position, float deg_rotation,
                          TIP tip, bool below);

// ========================== KITE DISPLAY ===================================

void tkbc_draw_kite(Kite *kite);
void tkbc_draw_kite_array(Kite_States *kite_states);

// ========================== UI DECLARATIONS ==============================

void tkbc_draw_ui(Env *env);
void tkbc_ui_timeline(Env *env);

#endif // TKBC_H_
