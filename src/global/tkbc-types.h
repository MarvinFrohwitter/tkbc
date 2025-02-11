#ifndef TKBC_TYPES_H_
#define TKBC_TYPES_H_

#include "raylib.h"
#include <stdio.h>
#include <sys/types.h>

// ===========================================================================
// ========================== TKBC KITE TYPES ================================
// ===========================================================================

typedef struct {
  const char *description;
  const char *mod_key_str;
  const char *mod_co_key_str;
  const char *selection_key1_str;
  const char *selection_key2_str;
  const char *key_str;
  int mod_key;
  int mod_co_key;
  int selection_key1;
  int selection_key2;
  int key;
  int hash;
} KeyMap;

typedef struct KeyMaps {
  KeyMap *elements;
  size_t count;
  size_t capacity;
} KeyMaps;

typedef struct {
  char *elements;
  size_t count;
  size_t capacity;
} Content; // A representation of a file content.

typedef struct { // A representation for an internal kite geometric.
  Vector2 v1;
  Vector2 v2;
  Vector2 v3;
} Triangle;

typedef size_t Index; // NOTE: Check for clang compiler issue in project.
typedef size_t Id;    // NOTE: Check for clang compiler issue in project.
typedef struct {
  Id kite_id;       // The universal id that is associated with one kite.
  Vector2 position; // The position that is located at the center of the top
                    // leading edge.
  float angle;      // The rotation is in degrees around the center position.
} Kite_Position;    // The combined position and rotation angle.

typedef struct {
  float old_angle; // The rotation angle before the frame interpolation has
                   // stated.
  Vector2
      old_center; // The old position before the frame interpolation has stated.

  // -------------------------------------------------------------------------

  float angle;    // The rotation is in degrees around the center position.
  Vector2 center; // The center position that is located at the center of
                  // the top leading edge.

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

  float fly_speed;  // Kite movement speed set from 0 to 100.
  float turn_speed; // Kite turn speed set from 0 to 100.
} Kite;             // The kite internal geometric design.

typedef struct {
  size_t kite_id; // The unique universal identifier for the kite.
  Kite *kite;     // The kite that holds its geometric and positioning stats.
  float fly_velocity;  // The base fly speed that holds the current combined
                       // value with the delta time and the variable fly_speed
                       // that is stored in the kite itself.
  float turn_velocity; // The base turn speed that holds the current combined
                       // value with the delta time and the variable turn_speed
                       // that is stored in the kite itself.

  bool mouse_control; // The indication if the kite follows the mouse position.
  bool interrupt_movement;   // The ability of the kite to move in user control.
  bool interrupt_smoothness; // The ability if the kite to turn smooth in user
                             // control.
  bool fixed; // The representation of the turn variant smooth or in fixed angle
              // steps.
  bool iscenter; // The representation of the active center rotation variant.

  bool kite_input_handler_active; // Representation of a manual user control
                                  // selection.
} Kite_State;                     // The current parametrized state of one kite.

typedef struct {
  Kite_State *elements; // The dynamic array collection for all generated kites.
  size_t count;         // The amount of elements in the array.
  size_t capacity; // The complete allocated space for the array represented as
                   // the number of collection elements of the array type.
} Kite_States; // The dynamic array that can hold kites and its corresponding
               // state.

typedef enum {
  LEFT_TIP,
  RIGHT_TIP
} TIP; // The left and right tip of the leading edge.
typedef enum {
  LEFT,
  RIGHT
} DIRECTION; // The Direction where the figure starts.
typedef enum {
  ODD,
  EVEN
} ODD_EVEN; // The kite group that is split default convention up or left.

typedef struct {
  float angle;         // The rotation angle the tip turn should have.
  TIP tip;             // The tip of the leading edge.
} Tip_Rotation_Action; // The action that is responsible for rotating the kite
                       // on one of the tips.

typedef struct {
  float angle;     // The rotation angle the center turn should have.
} Rotation_Action; // The action that is responsible for rotating the kite at
                   // the center.

typedef struct {
  Vector2 position; // The new position the move action should have.
} Move_Action;      // The action that is responsible for positioning the kite.

typedef struct {
  double starttime; // The time at the start of the frame representation.
} Wait_Action; // The action that is responsible for blocking a certain time.

typedef Tip_Rotation_Action
    Tip_Rotation_Add_Action; // The action that performs a addition to the
                             // current angle of the tip rotation.
typedef Rotation_Action Rotation_Add_Action; // The action that adds an angle to
                                             // the current rotation angle.
typedef Move_Action
    Move_Add_Action; // The action that adds a vector to the current position.
typedef Wait_Action Quit_Action; // The action that is responsible for force
                                 // quitting a frame after the specified time.

typedef union { // The collection of all the possible actions that can be used
                // in a script.
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
  KITE_ROTATION,
  KITE_ROTATION_ADD,
  KITE_TIP_ROTATION,
  KITE_TIP_ROTATION_ADD,
  ACTION_KIND_COUNT,
} Action_Kind; // A named listing of all the available action kinds.

typedef struct {
  Id *elements;    // The dynamic array collection for all kite indices.
  size_t count;    // The amount of elements in the array.
  size_t capacity; // The complete allocated space for the array represented as
                   // the number of collection elements of the array type.
} Kite_Ids;        // A dynamic array that can hold kite_ids.

typedef struct {
  Kite_Ids kite_id_array; // The collection of kite_ids that should be
                          // part of the performed action.
  Index index;            // The index of the current frame in the
  float duration;   // The time in seconds it should take to perform an action.
  Action_Kind kind; // A representation of the kind of the action pointer.
  void *action;     // The action the frame should be responsible for.
  bool finished;    // Represents the state of the currently handled frame.
} Frame;            // Combined action for the kites that are listed in the
                    // kite_id_array.

typedef struct {
  Kite_Position
      *elements;   // The dynamic array collection for all kite positions.
  size_t count;    // The amount of elements in the array.
  size_t capacity; // The complete allocated space for the array represented as
                   // the number of collection elements of the array type.
} Kite_Positions;  // The dynamic array of kite positions.

typedef struct {
  Frame *elements; // The dynamic array collection for all frames in the script.
  size_t count;    // The amount of elements in the array.
  size_t capacity; // The complete allocated space for the array represented as
                   // the number of collection elements of the array type.
  Index block_index; // The index in the block_frame array after registration.
  Kite_Positions *kite_frame_positions; // The start position of the kite in the
                                        // current frame.
} Frames; // A dynamic array collection that holds the type frame.

typedef struct {
  Frames *elements; // The dynamic array collection for all combined frames as a
                    // block frame.
  size_t count;     // The amount of elements in the array.
  size_t capacity;  // The complete allocated space for the array represented as
                    // the number of collection elements of the array type.
  Id script_id;     // The number of the loaded script starting from 1.
} Block_Frame; // A dynamic array collection that combined multiple frames to a
               // single kite draw representation.

typedef struct {
  Block_Frame
      *elements; // The dynamic array collection for all combined frame blocks.
  size_t count;  // The amount of elements in the array.
  size_t capacity; // The complete allocated space for the array represented as
                   // the number of collection elements of the array type.
} Block_Frames;    // A dynamic array collection that combined multiple frame
                   // blocks.

typedef struct Process Process;

typedef struct {
  Kite *vanilla_kite;      // A representation of all the default kite values.
  Kite_States *kite_array; // The kites that are generated for the current
                           // session of the application.
  size_t kite_id_counter;  // The identifier counter for the kite.

  Frames *frames;             // A view of the current active drawable frames.
  Block_Frame *block_frame;   // A view of all the frames that should be
                              // executed in a script.
  Block_Frames *block_frames; // The collection of all the parsed scripts.

  char *script_file_name; // The name of the script file '.kite'.
  size_t script_counter;  // Represents the amount of scripts that were loaded,
                          // starts with 1.
  size_t send_scripts;    // Represents the amount of scripts that where send to
                          // the peer partner starts; with 1.
  size_t server_script_block_index; // Represents of the block index the server
                                    // is currently executing.
  size_t server_script_block_index_count; // Represents of the current block
                                          // index count from the server.
  bool script_setup;     // The indication if the initial setup run is executed.
  bool script_interrupt; // The indication if a script is currently going to be
                         // loaded.
  bool script_finished;  // The indication a script has finished.
  bool free_bool_that_can_be_used_for_struct_packing;

  int fps;              // The fps of the application.
  size_t window_width;  // The window width of the application.
  size_t window_height; // The window height of the application.

  Frames *scratch_buf_frames; // A buffer that can be used to construct frames.
  Block_Frame *scratch_buf_block_frame; // A buffer that can be used to
                                        // construct a block_frame.

  // -------FFMPEG-------
  Sound sound;           // The current loaded sound.
  Process *ffmpeg;       // The pipe and pid of the ffmpeg subprocess.
  char *sound_file_name; // The name of the sound file that should be included
                         // in the rendered video.
  bool recording;        // The state if the recording of the window.
  bool rendering;        // The state of the rendering ffmpeg process

  // -------UI-------
  bool keymaps_interaction; // The status if the keymaps are currently edited.
  bool keymaps_mouse_interaction; // Checks if a keymap was clicked.
  float scrollbar_width; // The width of the scrollbar for the keymap settings.
  size_t box_height; // The height of one box that contains a keymap description
  size_t screen_items; // The amount of keymaps that can currently be displayed.
  size_t keymaps_mouse_interaction_box; // The id of the box the is clicked.
  size_t keymaps_top_interaction_box; // The id the current first displayed box.
  Rectangle keymaps_base;      // The base bounding box of the keymaps settings.
  Rectangle keymaps_scrollbar; // The base of the scrollbar for the keymaps.
  Rectangle
      keymaps_inner_scrollbar; // The indicator inside the keymaps scrollbar.
                               // and its keybinding selection box.
  KeyMaps *keymaps;            // The current keymaps
  bool keymaps_scollbar_interaction; // Checks if the scrollbar is currently
                                     // moved.

  bool timeline_hoverover;      // The status if the mouse is currently of the
                                // timeline.
  bool timeline_interaction;    // The status if the user controls the timeline.
  Rectangle timeline_base;      // The rectangle that is below the slider.
  Rectangle timeline_front;     // The rectangle that represents the slider.
  float timeline_segment_width; // The width of a single frame in the timeline.
  float timeline_segments_width; // The width of all the finished frames in the
                                 // timeline.
  size_t timeline_segments; // The amount of frames that the timeline displays.

} Env; // The global state of the application.

#endif // TKBC_TYPES_H_
