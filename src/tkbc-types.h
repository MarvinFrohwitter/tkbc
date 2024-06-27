#ifndef TKBC_TYPES_H_
#define TKBC_TYPES_H_

#include <raylib.h>
#include <stdio.h>
#include <sys/types.h>

// ===========================================================================
// ========================== TKBC KITE TYPES =================================
// ===========================================================================

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

typedef enum { FIXED, SMOOTH } PARAMETERS;
typedef enum { LEFT_TIP, RIGHT_TIP } TIP;
typedef enum { KITE_Y, KITE_X } ORIENTATION;
typedef enum { LEFT, RIGHT } DIRECTION;
typedef enum { ODD, EVEN } ODD_EVEN;

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
  int fps;

  Frames *scratch_buf_frames;

  // -------FFMPEG-------
  bool recording;
  bool rendering;
  int pipe;
  pid_t pid;
  char *sound_file_name;

} Env;

#endif // TKBC_TYPES_H_
