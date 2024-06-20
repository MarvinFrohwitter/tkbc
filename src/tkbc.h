#ifndef TKBC_H_
#define TKBC_H_

#include "tkbc-types.h"
#include <stdio.h>

#define TEAL                                                                   \
  CLITERAL(Color) { 0, 128, 128, 255 } // Teal

#define VECTOR2_FMT "(%f,%f)"
#define Vector2_FMT_ARGS(arg) (float)(arg).x, (float)(arg).y
#define EPSILON 0.001f

// ===========================================================================
// ===========================================================================
// ========================== KITE DECLARATIONS ==============================
// ===========================================================================
// ===========================================================================

Env *kite_init_env();
Kite_State *kite_init_kite();
void kite_destroy_env(Env *env);
void kite_destroy_kite(Kite_State *state);
void kite_destroy_kite_array(Env *env);
void kite_kite_array_generate(Env *env, size_t kite_count);
void kite_kite_array_start_position(Env *env);

void kite_set_kite_defaults(Kite *kite, bool is_generated);
void kite_set_kite_state_defaults(Kite_State *state);

// ========================== KITE POSITION ==================================

void kite_center_rotation(Kite *kite, Vector2 *position,
                          float center_deg_rotation);
void kite_tip_rotation(Kite *kite, Vector2 *position, float tip_deg_rotation,
                       TIP tip);
void kite_circle_rotation(Kite *kite, Vector2 *position, float deg_rotation,
                          TIP tip, bool below);

// ========================== KITE DISPLAY ===================================

void kite_draw_kite(Kite *kite);
void kite_draw_kite_array(Env *env);

// ===========================================================================
// ===========================================================================
// ========================== HEADER DECLARATIONS ============================
// ===========================================================================
// ===========================================================================

// ========================== KEYBOARD INPUT =================================

#ifdef TKBC_INPUT_HANDLER_IMPLEMENTATION
#undef TKBC_INPUT_HANDLER_IMPLEMENTATION
#endif // TKBC_INPUT_HANDLER_IMPLEMENTATION
#include "tkbc-input-handler.h"

// ========================== SCRIPT API =====================================

#ifdef TKBC_SCRIPT_API_IMPLEMENTATION
#undef TKBC_SCRIPT_API_IMPLEMENTATION
#endif // TKBC_SCRIPT_API_IMPLEMENTATION
#include "tkbc-script-api.h"

// ========================== TEAM FIGURES API ===============================

#ifdef TKBC_TEAM_FIGURES_API_IMPLEMENTATION
#undef TKBC_TEAM_FIGURES_API_IMPLEMENTATION
#endif // TKBC_TEAM_FIGURES_API_IMPLEMENTATION
#include "tkbc-team-figures-api.h"

// ========================== SCRIPT HANDLER =================================

#ifdef TKBC_SCRIPT_HANDLER_IMPLEMENTATION
#undef TKBC_SCRIPT_HANDLER_IMPLEMENTATION
#endif // TKBC_SCRIPT_HANDLER_IMPLEMENTATION
#include "tkbc-script-handler.h"

// ========================== Sound Handler ==================================

#ifdef TKBC_SOUND_IMPLEMENTATION
#undef TKBC_SOUND_IMPLEMENTATION
#endif // TKBC_SOUND_IMPLEMENTATION
#include "tkbc-sound-handler.h"

// ========================== KITE UTILS =====================================

#ifdef TKBC_UTILS_IMPLEMENTATION
#undef TKBC_UTILS_IMPLEMENTATION
#endif // TKBC_UTILS_IMPLEMENTATION
#include "tkbc-utils.h"

// ===========================================================================

#endif // TKBC_H_
