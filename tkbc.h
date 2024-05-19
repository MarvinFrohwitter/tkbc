#ifndef TKBC_H_
#define TKBC_H_

#include "raylib.h"
#include <stdio.h>

#define TEAL                                                                   \
  CLITERAL(Color) { 0, 128, 128, 255 } // Teal #define WINDOW_SCALE 120
#define WINDOW_SCALE 120
#define SCREEN_WIDTH 16 * WINDOW_SCALE
#define SCREEN_HEIGHT 9 * WINDOW_SCALE
#define TARGET_FPS 60

#define VECTOR2_FMT "(%f,%f)"
#define Vector2_FMT_ARGS(arg) (float)(arg).x, (float)(arg).y

typedef struct Triangle {
  Vector2 v1;
  Vector2 v2;
  Vector2 v3;
} Triangle;

typedef struct Kite {
  Vector2 center; // The center position that is located at the center of the
                  // top leading edge.

  Color body_color; // The color that is set for the kite body.
  Triangle left;    // The left Triangle that forms the kite body.
  Triangle right;   // The right Triangle that forms the kite body.

  float overlap;     // The overlap between the center position and the inner
                     // triangle tip.
  float inner_space; // The distance between the bottom two tips. Calculated as
                     // the length of the center position and the corresponding
                     // tip.

  Color top_color; // The color that is set for the leading edge.
  Rectangle rec;   // The leading edge.
  float spread;    // The overlap of the top leading edge to the side.

  float width;  // The width is dependent on the scale.
  float height; // The width is dependent on the scale.
  float scale;  // The scale is recommended to set between 0 and 10.
  float
      center_rotation; // The rotation is in degrees around the center position.

  float fly_speed;  // Kite movement speed set from 0 to 100.
  float turn_speed; // Kite turn speed set from 0 to 100.
} Kite;

typedef struct State {
  Kite *kite;
  float fly_velocity;
  float turn_velocity;

  bool interrupt_movement;
  bool interrupt_smoothness;
  bool fixed;
  bool iscenter;

  bool kite_input_handler_active;
  bool interrupt_script;
  int instruction_counter;
  int instruction_count;
} State;

typedef enum TIP { LEFT_TIP, RIGHT_TIP } TIP;
typedef enum Orientation { KITE_Y, KITE_X } Orientation;

// ===========================================================================
// ========================== Kite Declarations ==============================
// ===========================================================================

State *kite_init();
void kite_destroy(State *state);
void kite_set_state_defaults(State *state);
void kite_set_kite_defaults(Kite *kite, bool is_generated);

void kite_tip_rotation(Kite *kite, Vector2 *position, float tip_deg_rotation,
                       TIP tip);
void kite_center_rotation(Kite *kite, Vector2 *position,
                          float center_deg_rotation);
void kite_circle_rotation(Kite *kite, Vector2 *position, float deg_rotation,
                          TIP tip, bool below);
void kite_draw_kite(Kite *kite);
void kite_input_handler(State *state);
void kite_input_check_rotation(State *state);
void kite_input_check_tip_turn(State *state);
void kite_input_check_circle(State *state);
void kite_input_check_movement(State *state);
void kite_input_check_speed(State *state);
void kite_input_check_mouse(State *state);

float kite_clamp(float z, float a, float b);

// ===========================================================================
// ========================== Custom Kite Creation ===========================
// ===========================================================================

void kite_gen_kites(State *s1, State *s2, State *s3, State *s4);
void kite_array_destroy_kites();
void kite_draw_kite_array();
bool kite_array_check_interrupt_script();
void kite_array_input_handler();
void kite_array_start_pos();

// ===========================================================================
// ========================== Sound Handler ==================================
// ===========================================================================

void kite_sound_handler(Sound *kite_sound);
Sound kite_sound_init(size_t master_volume);
void kite_defer_sound(Sound sound);

// ===========================================================================
// ========================== Script Handler =================================
// ===========================================================================

typedef enum {
  FIXED,
  SMOOTH,
} PARAMETERS;

void kite_script_input(State *state);
void kite_script_begin(State *state);
void kite_script_end(State *state);

void kite_script_move(Kite *kite, float steps_x, float steps_y,
                      PARAMETERS parameters);
void kite_script_rotate(Kite *kite, float angle, PARAMETERS parameters);
void kite_script_rotate_tip(Kite *kite, TIP tip, float angle,
                            PARAMETERS parameters);

#endif // TKBC_H_
