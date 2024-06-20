#ifndef TKBC_H_
#define TKBC_H_

#include "raylib.h"
#include <stdio.h>

#define TEAL                                                                   \
  CLITERAL(Color) { 0, 128, 128, 255 } // Teal

#define VECTOR2_FMT "(%f,%f)"
#define Vector2_FMT_ARGS(arg) (float)(arg).x, (float)(arg).y
#define EPSILON 0.001f

typedef struct {
  Vector2 v1;
  Vector2 v2;
  Vector2 v3;
} Triangle;

typedef struct {
  // Script only use
  float segment_size; // The rotation steps.
  float angle_sum; // The sum of angles of all frames where the kite is part of.
  float remaining_angle; // The remaining rotation angle.
  float old_angle; // The rotation angle before the frame interpolation has
                   // stated.
  Vector2
      old_center; // The old position before the frame interpolation has stated.

  // ------------------------------------------------------------------------
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
  float height; // The height is dependent on the scale.
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
} Kite_State;

typedef struct {
  Kite_State *elements;
  size_t count;
  size_t capacity;
} Kite_States;

typedef enum { FIXED, SMOOTH, } PARAMETERS;
typedef enum { LEFT_TIP, RIGHT_TIP } TIP;
typedef enum { KITE_Y, KITE_X } ORIENTATION;
typedef enum { LEFT, RIGHT } DIRECTION;

typedef struct {
  float angle;
  TIP tip;
} Tip_Rotation_Action;

typedef struct {
  float angle;
} Rotation_Action;

typedef struct {
  Vector2 position;
} Move_Action;

typedef Move_Action Move_Add_Action;

typedef struct {
  double starttime;
} Wait_Action;

typedef Wait_Action Quit_Action;

typedef union {
  Tip_Rotation_Action tip_rotation_action;
  Rotation_Action rotation_action;
  Move_Action move_action;

  Tip_Rotation_Action tip_rotation_add_action;
  Rotation_Action rotation_add_action;
  Move_Action move_add_action;

  Wait_Action wait_action;
  Quit_Action quit_action;
} Action;

typedef enum {
  KITE_ACTION,
  KITE_QUIT,
  KITE_WAIT,
  KITE_MOVE,
  KITE_MOVE_ADD,
  KITE_ROTATION_ADD,
  KITE_TIP_ROTATION
} Action_Kind;

typedef size_t Index;
typedef struct {
  Index *elements;
  size_t count;
  size_t capacity;
} Kite_Indexs;

typedef struct {
  Kite_Indexs *kite_index_array;
  size_t index;
  float duration;
  Action_Kind kind;
  void *action;
  bool finished;
} Frame;

typedef struct {
  Frame *elements;
  size_t count;
  size_t capacity;

  size_t block_index;
} Frames;

typedef struct {
  Index *elements;
  size_t count;
  size_t capacity;

} Index_Blocks;

typedef struct {
  Kite_States *kite_array;

  Frames *frames;
  Index_Blocks *index_blocks;
  size_t global_block_index;
  size_t max_block_index;
  size_t attempts_block_index;

  bool script_interrupt;
  bool script_finished;

  size_t window_width;
  size_t window_height;

  Frames *scratch_buf_frames;
} Env;

// ===========================================================================
// ========================== SCRIPT API =====================================
// ===========================================================================

void kite_script_input(Env *env);
void kite_script_begin(Env *env);
void kite_script_end(Env *env);

void kite_script_update_frames(Env *env);
bool kite_script_finished(Env *env);

// ===========================================================================
// ========================== SCRIPT HANDLER API =============================
// ===========================================================================

Frame *kite_script_wait(float duration);
Frame *kite_script_frames_quit(float duration);
Frame *kite__frame_generate(Env *env, Action_Kind kind, Kite_Indexs kite_indexs,
                            void *raw_action, float duration);
#define kite_frame_generate(kind, kite_indexs, raw_action, duration)           \
  kite__frame_generate(env, kind, kite_indexs, raw_action, duration)
void kite__register_frames(Env *env, ...);
#define kite_register_frames(env, ...)                                         \
  kite__register_frames(env, __VA_ARGS__, NULL)
void kite_register_frames_array(Env *env, Frames *frames);

Kite_Indexs kite__indexs_append(size_t _, ...);
#define kite_indexs_append(...) kite__indexs_append(0, __VA_ARGS__, INT_MAX)
Kite_Indexs kite_indexs_range(int start, int end);
#define kite_indexs_generate(count) kite_indexs_range(0, count)

// ========================== Script Team Figures ============================

void kite_script_team_line(Env *env, Kite_Indexs kite_index_array,
                           size_t h_padding, Vector2 offset, float duration);
bool kite_script_team_grid(Env *env, Kite_Indexs kite_index_array, size_t rows,
                           size_t columns, size_t v_padding, size_t h_padding,
                           Vector2 offset, float duration);

bool kite_script_team_ball(Env *env, Kite_Indexs kite_index_array,
                           Vector2 position, Vector2 offset, float radius,
                           float duration);

bool kite_script_team_mountain(Env *env, Kite_Indexs kite_index_array,
                               size_t v_padding, size_t h_padding,
                               Vector2 offset, float duration);
bool kite_script_team_valley(Env *env, Kite_Indexs kite_index_array,
                             size_t v_padding, size_t h_padding, Vector2 offset,
                             float duration);

bool kite_script_team_arc(Env *env, Kite_Indexs kite_index_array,
                          size_t v_padding, size_t h_padding, Vector2 offset,
                          float angle, float duration);
bool kite_script_team_mouth(Env *env, Kite_Indexs kite_index_array,
                            size_t v_padding, size_t h_padding, Vector2 offset,
                            float angle, float duration);

void kite_script_team_box(Env *env, Kite_Indexs kite_index_array,
                          DIRECTION direction, float angle, float box_size,
                          float duration);
void kite_script_team_box_left(Env *env, Kite_Indexs kite_index_array,
                               float box_size, float duration);
void kite_script_team_box_right(Env *env, Kite_Indexs kite_index_array,
                                float box_size, float duration);

void kite_script_team_dimond(Env *env, Kite_Indexs kite_index_array,
                             DIRECTION direction, float angle, float box_size,
                             float duration);
void kite_script_team_dimond_left(Env *env, Kite_Indexs kite_index_array,
                                  float box_size, float duration);
void kite_script_team_dimond_right(Env *env, Kite_Indexs kite_index_array,
                                   float box_size, float duration);

// ========================== SCRIPT API END =================================

// ===========================================================================
// ========================== KITE DECLARATIONS ==============================
// ===========================================================================

Env *kite_init_env();
Kite_State *kite_init_kite();
void kite_destroy_env(Env *env);
void kite_destroy_kite(Kite_State *state);
void kite_destroy_kite_array(Env *env);
void kite_kite_array_generate(Env *env, size_t kite_count);
void kite_kite_array_start_position(Env *env);

void kite_set_kite_defaults(Kite *kite, bool is_generated);
void kite_set_state_defaults(Kite_State *state);

// ========================== KITE POSITION ==================================

void kite_tip_rotation(Kite *kite, Vector2 *position, float tip_deg_rotation,
                       TIP tip);
void kite_center_rotation(Kite *kite, Vector2 *position,
                          float center_deg_rotation);
void kite_circle_rotation(Kite *kite, Vector2 *position, float deg_rotation,
                          TIP tip, bool below);

// ========================== KITE DISPLAY ===================================

void kite_draw_kite(Kite *kite);
void kite_draw_kite_array(Env *env);

// ========================== KITE KEYBOARD INPUT ============================

void kite_input_handler(Env *env, Kite_State *state);
void kite_input_handler_kite_array(Env *env);
void kite_input_check_rotation(Kite_State *state);
void kite_input_check_tip_turn(Kite_State *state);
void kite_input_check_circle(Kite_State *state);
void kite_input_check_movement(Kite_State *state);
void kite_input_check_speed(Kite_State *state);
void kite_input_check_mouse(Kite_State *state);

// ========================== KITE UTILS =====================================

int kite_check_boundary(Kite *kite, ORIENTATION orientation);
float kite_clamp(float z, float a, float b);
float kite_lerp(float a, float b, float t);
int kite_max(int a, int b);

// ========================== SCRIPT HANDLER =================================

Frame *kite_init_frame();
void kite_register_frame(Env *env, Frame *frame);
void kite_destroy_frames(Frames *frames);
void kite_frame_reset(Frame *frame);
void kite_render_frame(Env *env, Frame *frame);

bool kite_check_finished_frames(Env *env);
size_t kite_check_finished_frames_count(Env *env);

// ========================== SCRIPT HANDLER INTERNAL ========================

void kite_script_move(Kite *kite, Vector2 position, float duration);
void kite_script_rotate(Kite *kite, float angle, float duration);
void kite_script_rotate_tip(Kite *kite, TIP tip, float angle, float duration);

// ===========================================================================
// ========================== Sound Handler ==================================
// ===========================================================================

Sound kite_init_sound(size_t master_volume);
void kite_sound_destroy(Sound sound);
void kite_sound_handler(Sound *kite_sound);

// ===========================================================================

#endif // TKBC_H_
