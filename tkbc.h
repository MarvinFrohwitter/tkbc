#ifndef TKBC_H_
#define TKBC_H_

#include "raylib.h"

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

  float speed; // Kite movement speed set from 0 to 100.
} Kite;

typedef struct State {
  Kite *kite;
  float velocity;

  bool interrupt_movement;
  bool interrupt_smoothness;
  bool fixed;
  bool iscenter;
  bool interrupt_script;
} State;

typedef enum TIP { LEFT_TIP, RIGHT_TIP } TIP;
typedef enum Orientation { KITE_Y, KITE_X } Orientation;

// ===========================================================================
// ========================== Kite Declarations ==============================
// ===========================================================================

State *kite_init();
void kite_destroy(State *state);
void kite_tip_rotation(Kite *k, Vector2 *position, float tip_deg_rotation,
                       TIP tip);
void kite_center_rotation(Kite *k, Vector2 *position,
                          float center_deg_rotation);
void kite_circle_rotation(Kite *k, Vector2 *position, float deg_rotation,
                          TIP tip, bool below);
void kite_draw_kite(Kite *k);
void kite_input_handler(State *s);
void kite_input_check_rotation(State *s);
void kite_input_check_tip_turn(State *s);
void kite_input_check_circle(State *s);
void kite_input_check_movement(State *s);
void kite_input_check_speed(State *s);

float kite_clamp(float z, float a, float b);

// ===========================================================================
// ========================== Script Handler =================================
// ===========================================================================

typedef enum PARAMETERS {
  FIXED,
  SMOOTH,
} PARAMETERS;

void kite_script_input(State *state);
void kite_script_begin(State *state);
void kite_script_end(State *state);

void kite_script_move(Kite *kite, float steps_x, float steps_y,
                      PARAMETERS parameters);

#endif // TKBC_H_
