#ifndef TKBC_H_
#define TKBC_H_

#include "raylib.h"
#include <stdio.h>

#define TEAL                                                                   \
  CLITERAL(Color) { 0, 128, 128, 255 } // Teal #define WINDOW_SCALE 120

#define VECTOR2_FMT "(%f,%f)"
#define Vector2_FMT_ARGS(arg) (float)(arg).x, (float)(arg).y

typedef struct {
  Vector2 v1;
  Vector2 v2;
  Vector2 v3;
} Triangle;

typedef struct {
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

typedef struct {
  size_t id;
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

typedef struct {
  State *items;
  size_t count;
  size_t capacity;
} States;

typedef enum {
  KITE_ACTION,
  KITE_MOVE,
  KITE_ROTATION,
  KITE_TIP_ROTATION
} Action_Kind;
typedef enum {
  FIXED,
  SMOOTH,
} PARAMETERS;
typedef enum { LEFT_TIP, RIGHT_TIP } TIP;
typedef enum { KITE_Y, KITE_X } ORIENTATION;

typedef struct {
  float angle;
  PARAMETERS parameters;
  TIP tip;

} Tip_Rotation_Action;

typedef struct {
  float angle;
  PARAMETERS parameters;

} Rotation_Action;

typedef struct {
  Vector2 position;
  PARAMETERS parameters;

} Move_Action;


typedef struct {
  size_t *items;
  size_t count;
  size_t capacity;
} Kite_Indexs;

typedef struct {
  Kite_Indexs *kite_index_array;
  size_t index;
  float duration;
  Action_Kind kind;
  void *action;
} Frame;

typedef struct {
  Frame *items;
  size_t count;
  size_t capacity;

  size_t frame_counter;
} Frames;

typedef struct {
  Frames *frames;
  States *kite_array;
  size_t window_width;
  size_t window_height;

} Env;

// ===========================================================================
// ========================== Kite Declarations ==============================
// ===========================================================================

Env *kite_env_init();
void kite_env_destroy(Env *env);
State *kite_kite_init();
void kite_kite_destroy(State *state);
void kite_set_state_defaults(State *state);
void kite_set_kite_defaults(Kite *kite, bool is_generated);

void kite_tip_rotation(Kite *kite, Vector2 *position, float tip_deg_rotation,
                       TIP tip);
void kite_center_rotation(Kite *kite, Vector2 *position,
                          float center_deg_rotation);
void kite_circle_rotation(Kite *kite, Vector2 *position, float deg_rotation,
                          TIP tip, bool below);
void kite_draw_kite(Kite *kite);
void kite_input_handler(Env *env, State *state);
void kite_input_check_rotation(State *state);
void kite_input_check_tip_turn(State *state);
void kite_input_check_circle(State *state);
void kite_input_check_movement(State *state);
void kite_input_check_speed(State *state);
void kite_input_check_mouse(State *state);

int kite_check_boundary(Kite *kite, ORIENTATION orientation);
float kite_clamp(float z, float a, float b);

// ===========================================================================
// ========================== Custom Kite Creation ===========================
// ===========================================================================

void kite_gen_kites(Env *env, size_t kite_count);
void kite_array_destroy_kites(Env *env);
void kite_draw_kite_array(Env *env);
bool kite_array_check_interrupt_script(Env *env);
void kite_array_input_handler(Env *env);
void kite_array_start_pos(Env *env);

// ===========================================================================
// ========================== Sound Handler ==================================
// ===========================================================================

Sound kite_sound_init(size_t master_volume);
void kite_defer_sound(Sound sound);
void kite_sound_handler(Sound *kite_sound);

// ===========================================================================
// ========================== Script Handler =================================
// ===========================================================================

Frame *kite_frame_init();
Frame *kite_gen_frame(Action_Kind kind, Kite_Indexs kite_indexs,
                      void *raw_action, float duration);
void kite_frame_reset(Frame *frame);
void kite_register_frame(Env *env, Frame *frame);
void kite_render_frame(Env *env, Frame *frame);

void kite_update_frames(Env *env);
void kite_array_destroy_frames(Env *env);

void kite_script_input(State *state);
void kite_script_begin(State *state);
void kite_script_end(State *state);

void kite_script_move(Kite *kite, float steps_x, float steps_y,
                      PARAMETERS parameters);
void kite_script_rotate(Kite *kite, float angle, PARAMETERS parameters);
void kite_script_rotate_tip(Kite *kite, TIP tip, float angle,
                            PARAMETERS parameters);

#endif // TKBC_H_
