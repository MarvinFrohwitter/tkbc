#ifndef TKBC_TYPES_H_
#define TKBC_TYPES_H_

#include "raylib.h"
#include <stdio.h>
#include <sys/types.h>

// ===========================================================================
// ========================== TKBC KITE TYPES ================================
// ===========================================================================

typedef struct {
  Image normal;
  Image flipped;
} Kite_Image;

typedef struct {
  Texture2D normal;
  Texture2D flipped;
} Kite_Texture;

typedef struct {
  Kite_Texture *elements;
  size_t count;
  size_t capacity;
} Kite_Textures;

typedef struct {
  Kite_Image *elements;
  size_t count;
  size_t capacity;
} Kite_Images;

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
} Key_Map;

typedef struct KeyMaps {
  Key_Map *elements;
  size_t count;
  size_t capacity;
} Key_Maps;

typedef enum {
  KMH_CHANGE_KEY_MAPPINGS = 1000,
  KMH_QUIT_PROGRAM,
  KMH_TAKE_SCREENSHOT,
  KMH_BEGIN_RECORDING,
  KMH_END_RECORDING,

  KMH_SET_KITES_TO_START_POSITION,
  KMH_ROTATE_KITES_ANGLE_ZERO,

  KMH_ROTATE_KITES_CENTER_CLOCKWISE,
  KMH_ROTATE_KITES_CENTER_ANTICLOCKWISE,
  KMH_ROTATE_KITES_TIP_CLOCKWISE,
  KMH_ROTATE_KITES_TIP_ANTICLOCKWISE,
  KMH_ROTATE_KITES_CIRCLE_CLOCKWISE,
  KMH_ROTATE_KITES_CIRCLE_ANTICLOCKWISE,

  KMH_TOGGLE_FIXED,

  KMH_MOVES_KITES_LEFT,
  KMH_MOVES_KITES_DOWN,
  KMH_MOVES_KITES_UP,
  KMH_MOVES_KITES_RIGHT,

  KMH_INCREASE_FLY_SPEED,
  KMH_REDUCE_FLY_SPEED,
  KMH_INCREASE_TURN_SPEED,
  KMH_REDUCE_TURN_SPEED,

  KMH_TOGGLE_SCRIPT_EXECUTION,
  KMH_SWITCHES_NEXT_SCRIPT,

  KMH_PLAYS_SOUND,
  KMH_STOPS_SOUND,
  KMH_PAUSES_SOUND,
  KMH_RESUMES_SOUND,

  KMH_SWITCH_MOUSE_CONTOL_MOVEMENT,
  KMH_LOCK_KITE_TIP,
  KMH_LOCK_KITE_ANGLE,
  KMH_SNAP_KITE_ANGLE,

  KMH_MOVES_KITES_TOWARDS_MOUSE,
  KMH_MOVES_KITES_AWAY_MOUSE,
  KMH_MOVES_KITES_LEFT_AROUND_MOUSE,
  KMH_MOVES_KITES_RIGHT_AROUND_MOUSE,

  KMH_KEY_KP_8,
  KMH_KEY_KP_9,
  KMH_KEY_KP_6,
  KMH_KEY_KP_3,
  KMH_KEY_KP_2,
  KMH_KEY_KP_7,
  KMH_KEY_KP_4,
  KMH_KEY_KP_1,

  KMH_KEY_KP_5,

  KMH_KEY_161,

  KMH_COUNT,
} Key_Map_Hash;

typedef enum {
  BOX_INVALID = -1,
  BOX_MOD_KEY = 0,
  BOX_MOD_CO_KEY = 1,
  BOX_SELECTION_KEY1 = 2,
  BOX_SELECTION_KEY2 = 3,
  BOX_KEY = 4,
} Key_Box;

typedef enum {
  LEFT_TIP = 1 << 0,
  RIGHT_TIP = 1 << 1,
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
  Texture2D kite_texture_normal; // The kite body texture that is used in when
                                 // normal flying mode is active.
  Texture2D
      kite_texture_flipped; // The kite body texture that is used when the kite
                            // has flipped and flies per default reverse.
  size_t texture_id; // The number that identifies the kite texture in the
                     // global kite_textures.
  float old_angle;   // The rotation angle before the frame interpolation has
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
  Kite *kite;     // The kite that holds its geometric and positioning stats.
  size_t kite_id; // The unique universal identifier for the kite.
  float fly_velocity;  // The base fly speed that holds the current combined
                       // value with the delta time and the variable fly_speed
                       // that is stored in the kite itself.
  float turn_velocity; // The base turn speed that holds the current combined
                       // value with the delta time and the variable turn_speed
                       // that is stored in the kite itself.
  int selected_tips;   // It represents if the kite leading edge angle can be
                       // freely mode with the rotation.
  bool is_active;      // If the kite should be drawn.
  bool is_kite_input_handler_active; // Representation of a manual user control
                                     // selection.
  bool is_fixed_rotation; // The representation of the turn variant smooth or in
                          // fixed angle steps.
  bool is_center_rotation;   // The representation of the active center rotation
                             // variant.
  bool interrupt_movement;   // The ability of the kite to move in user control.
  bool interrupt_smoothness; // The ability if the kite to turn smooth in user
                             // control.
  bool is_mouse_control;     // The indication if the kite follows the mouse
                             // position.
  bool is_mouse_in_dead_zone; // A space between the mouse and the kite that is
                              // not reachable by moving forward with the kite.
  bool is_snapping_to_angle;  // Represents if the kite snaps to the nearest 45
                              // degrees angle.
  bool is_angle_locked;       // It represents if the kite leading edge angle is
                        // calculated by the mouse position or it holds the
                        // current angle, (in mouse control mode).
  bool is_tip_locked;    // It represents if the kite leading edge angle can be
                         // freely mode with the rotation.
  bool is_rotating;      // Indicates if the kite currently rotates around
                         // the center.
  bool is_kite_reversed; // indicates if the kite faces away from the mouse.
} Kite_State;            // The current parametrized state of one kite.

typedef struct {
  Kite_State *elements; // The dynamic array collection for all generated kites.
  size_t count;         // The amount of elements in the array.
  size_t capacity; // The complete allocated space for the array represented as
                   // the number of collection elements of the array type.
} Kite_States; // The dynamic array that can hold kites and its corresponding
               // state.

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
  Tip_Rotation_Action as_tip_rotation;
  Rotation_Action as_rotation;
  Move_Action as_move;

  Tip_Rotation_Action as_tip_rotation_add;
  Rotation_Action as_rotation_add;
  Move_Action as_move_add;

  Wait_Action as_wait;
  Quit_Action as_quit;
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

  bool script_id_append; // Checks if the ids are heap allocated by the
                         // tkbc__indexs_append() method.
} Kite_Ids;              // A dynamic array that can hold kite_ids.

typedef struct {
  Kite_Ids kite_id_array; // The collection of kite_ids that should be
                          // part of the performed action.
  Index index;            // The index of the current frame in the
  float duration;   // The time in seconds it should take to perform an action.
  Action action;    // The action the frame should be responsible for.
  Action_Kind kind; // A representation of the kind of the action pointer.
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
  Index frames_index; // The index in the script array after registration.
  Kite_Positions kite_frame_positions; // The start position of the kite in the
                                       // current frame.
} Frames; // A dynamic array collection that holds the type frame.

typedef struct {
  Frames *elements; // The dynamic array collection for all combined frames as a
                    // script.
  size_t count;     // The amount of elements in the array.
  size_t capacity;  // The complete allocated space for the array represented as
                    // the number of collection elements of the array type.
  Id script_id;     // The number of the loaded script starting from 1.
  const char *name; // The name of the script.
} Script; // A dynamic array collection that combined multiple frames to a
          // single kite draw representation.

typedef struct {
  Script *elements; // The dynamic array collection for all combined scripts.
  size_t count;     // The amount of elements in the array.
  size_t capacity;  // The complete allocated space for the array represented as
                    // the number of collection elements of the array type.
} Scripts; // A dynamic array collection that combined multiple scripts.

typedef struct Process Process;

typedef struct {
  Color *elements; // The dynamic array collection for Colors.
  size_t count;    // The amount of elements in the array.
  size_t capacity; // The complete allocated space for the array represented as
                   // the number of collection elements of the array type.
} Colors;          // A dynamic array collection that holds the type Color.

typedef struct {
  Rectangle base;            // The base of the scrollbar..
  Rectangle inner_scrollbar; // The indicator inside the scrollbar.
  bool interaction;          // Checks if the scrollbar is currently
                             // moved.
} Scrollbar;

typedef struct {
  Kite *vanilla_kite;      // A representation of all the default kite values.
  Kite_States *kite_array; // The kites that are generated for the current
                           // session of the application.
  size_t kite_id_counter;  // The identifier counter for the kite.

  Frames *frames;   // A view of the current active drawable frames.
  Script *script;   // A view of all the frames that should be
                    // executed in a script.
  Scripts *scripts; // The collection of all the parsed scripts.

  char *script_file_name; // The name of the script file '.kite'.

  size_t send_scripts; // Represents the amount of scripts that where send to
                       // the peer partner starts; with 1.
  size_t server_script_frames_index; // Represents of the index of the frames
                                     // the server is currently executing.
  size_t server_script_frames_count; // Representation of the amount of
                                     // frames in the current script.
  size_t server_script_id; // Representation of the current script the server
                           // executes.
  size_t server_script_kite_max_count; // Represents the maximum kites that are
                                       // part of a complete script.

  bool new_script_selected; // Representation if a user has selected a new
                            // script in the UI.

  bool script_setup;     // The indication if the initial setup run is executed.
  bool script_interrupt; // The indication if a script is currently going to be
                         // loaded.
  bool script_finished;  // The indication a script has finished.

  int fps;              // The fps of the application.
  size_t window_width;  // The window width of the application.
  size_t window_height; // The window height of the application.

  Frames scratch_buf_frames; // A buffer that can be used to construct frames.
  Script scratch_buf_script; // A buffer that can be used to
                             // construct a script.

  // -------FFMPEG-------
  Sound sound;           // The current loaded sound.
  Process *ffmpeg;       // The pipe and pid of the ffmpeg subprocess.
  char *sound_file_name; // The name of the sound file that should be included
                         // in the rendered video.
  bool recording;        // The state if the recording of the window.
  bool rendering;        // The state of the rendering ffmpeg process

  // -------UI-------
  Scrollbar keymaps_scrollbar;
  bool keymaps_interaction; // The status if the keymaps are currently edited.
  bool keymaps_mouse_interaction;         // Checks if a keymap was clicked.
  Key_Box keymaps_interaction_rec_number; // Represents the box number by kind.

  size_t box_height; // The height of one box that contains a keymap description
  size_t screen_items; // The amount of keymaps that can currently be displayed.
  size_t keymaps_mouse_interaction_box; // The id of the box the is clicked.
  size_t keymaps_top_interaction_box; // The id the current first displayed box.
  Rectangle keymaps_base; // The base bounding box of the keymaps settings.
  Key_Maps keymaps;       // The current keymaps

  bool timeline_hoverover;      // The status if the mouse is currently of the
                                // timeline.
  bool timeline_interaction;    // The status if the user controls the timeline.
  Rectangle timeline_base;      // The rectangle that is below the slider.
  Rectangle timeline_front;     // The rectangle that represents the slider.
  float timeline_segment_width; // The width of a single frame in the timeline.
  float timeline_segments_width; // The width of all the finished frames in the
                                 // timeline.
  size_t timeline_segments; // The amount of frames that the timeline displays.

  Rectangle
      color_picker_base; // The base bounding box of the color selection menu.
  char *color_picker_input_text; // The base bounding box of the color
                                 // selection menu.
  bool color_picker_interaction; // The status if the color picker is currently
                                 // in use.
  bool color_picker_input_mouse_interaction; // Checks if the input box is
                                             // clicked.
  bool color_picker_display_designs; // If the color pallet or the designs are
                                     // displayed.
  Color last_selected_color; // The color that is displayed in the box below the
                             // input.
  size_t max_favorite_colors;
  size_t current_favorite_colors_index;
  Colors favorite_colors; // The current storage that holds the data for the
                          // color_picker favorite color circles.

  bool script_menu_interaction; // The status if the menu that displays all the
                                // available scripts is currently displays.
  Rectangle script_menu_base;
  Scrollbar script_menu_scrollbar;
  size_t script_menu_mouse_interaction_box; // The id of the box it is clicked.
  size_t script_menu_top_interaction_box; // The id the current first displayed
                                          // box.
  bool script_menu_mouse_interaction;

} Env; // The global state of the application.

#endif // TKBC_TYPES_H_
