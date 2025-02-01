# Team Kite Ballett Choreographer

- The choreography creator.

- The dependency is version 5.5:
- Original Raylib 5.5 Source Code: <https://github.com/raysan5/raylib/releases/tag/5.5>
- For commits older than fb53c95 the used library is:
- Original Raylib 5.0 Source Code: <https://github.com/raysan5/raylib/releases/tag/5.0>

### Build with cb

```Shell
$ cc -o cb cb.c
static linking raylib
$ ./cb static
or dynamic linking raylib
$ ./cb dynamic
```

### Build with make

```Shell
make
./build/tkbc
```

## SCRIPT API in C

To use the direct C functions the file content of the example file has to be
replaced with the new created script. This file will be automatically included
in the compilation and the scope will be called to load the script.

Note: The env pointer is automatically inserted into the scope.

Step 1 Initialization

```C
// Create the maximum amount of kites that should be used in the script.
Kite_Ids ki = tkbc_kite_array_generate(env, 4);
```

Step 2 Set up the script scope

```C
// Create the script scope.
tkbc_script_input {
  tkbc_script_begin
    // Here goes the script.
  tkbc_script_end
}
```

Multiple script can be provided with another begin and end call.

```C
// Create more scripts.
tkbc_script_input {
  tkbc_script_begin
    // Here goes the first script.
  tkbc_script_end

  tkbc_script_begin
    // Here goes the second script.
  tkbc_script_end
}
```

Step 3 Script calls

```C
// Some useful definitions can be:
size_t duration = 3; // In seconds.
Kite_Ids custom_ids = tkbc_indexs_append(0,1,2,3);
Kite_Ids custom_ids_range = tkbc_indexs_range(int start, int end); // The end is exclusive.
Kite kite = *env->vanilla_kite; // The default kite that is available for getting values; as an example that could be the dimensions.
int space = 100;
```

The simple versions:

COLLECTION and SET do the same thing.

```C
// The possible primitive script calls are:
COLLECTION(KITE_QUIT(duration));
SET(KITE_WAIT(duration));

SET(
    KITE_MOVE_ADD(ID(0), kite.width + 2 * space + kite.width / 2.0,
                  -kite.width - space - space, move_duration),
);

SET(
    KITE_MOVE(ID(0), env->window_width / 2.0 - kite.width - space,
              env->window_height - kite.height - space, 0),
);

SET(
    KITE_ROTATION_ADD(tkbc_indexs_append(0), -270, rotation_duration)
);

SET(
    KITE_ROTATION(ki, 0, 0),
);

SET(
    KITE_TIP_ROTATION_ADD(ID(0), 315, LEFT_TIP, rotation_duration)
);

SET(
    KITE_TIP_ROTATION(ID(0), 315, RIGHT_TIP, rotation_duration)
);
```

The more explicit versions that do the same thing:

```C
// The possible primitive script calls are:
tkbc_register_frames(env, tkbc_script_wait(duration));
tkbc_register_frames(env, tkbc_script_frames_quit(duration));

tkbc_register_frames(env, tkbc_frame_generate(KITE_MOVE_ADD, custom_ids,
                                              &((Move_Add_Action){
                                                  .position.x = 0,
                                                  .position.y = -300,
                                              }),
                                              duration));

tkbc_register_frames(env, tkbc_frame_generate(KITE_MOVE, ki,
                                              &((Move_Action){
                                                  .position.x = 0,
                                                  .position.y = -300,
                                              }),
                                              duration));

tkbc_register_frames(env, tkbc_frame_generate(KITE_ROTATION_ADD, ki,
                               &((Rotation_Add_Action){.angle = -0}), duration));

tkbc_register_frames(env, tkbc_frame_generate(KITE_ROTATION, ki,
                               &((Rotation_Action){.angle = 0}), duration));

  tkbc_register_frames(env, tkbc_frame_generate(KITE_TIP_ROTATION_ADD, ki,
                                           &((Tip_Rotation_Add_Action){
                                               .angle = 90, .tip = LEFT_TIP}),
                                           duration));

  tkbc_register_frames(env, tkbc_frame_generate(KITE_TIP_ROTATION, ki,
                                           &((Tip_Rotation_Action){
                                               .angle = 90, .tip = RIGHT_TIP}),
                                           duration));
```

Every primitive action an be nested to achieve not only linear time motion
control, but also parallel motions. That could me needed if a kite should move
forward and rotate at the same time.
This could look like this:

```C
// simple:
  SET(

      KITE_MOVE(ID(0), env->window_width / 2.0 - kite.width - space,
                env->window_height - kite.height - space, 0),
      KITE_MOVE(ID(1),
                env->window_width / 2.0 + kite.width + space,
                env->window_height - kite.height - space, 0),
      KITE_ROTATION(ki, 0, 0),

      // Just for documentation
      KITE_QUIT(duration)

  );

// explicit:
  tkbc_register_frames(env,
                       tkbc_frame_generate(KITE_TIP_ROTATION_ADD, ki,
                                           &((Tip_Rotation_Add_Action){
                                               .angle = 90, .tip = LEFT_TIP}),
                                           duration),
                       tkbc_frame_generate(KITE_MOVE_ADD, ki,
                                           &(CLITERAL(Move_Add_Action){
                                               .position.x = 0,
                                               .position.y = -300,
                                           }),
                                           5),
                       tkbc_script_wait(1.5),
                       tkbc_script_frames_quit(7)
                       );
```

More examples could be found in ./tkbc_scripts/.

## SCRIPT TEAM API in C

The primitive types can additionally combined with the calls to the
TEAM-FIGURES API. This function call can not be nested into the parallel
visualization.

Note: The env pointer is automatically inserted into the scope.

Available team calls are:

```C
bool tkbc_script_team_line(Env *env, Kite_Ids kite_index_array,
                           Vector2 position, Vector2 offset, float h_padding,
                           float move_duration);
bool tkbc_script_team_grid(Env *env, Kite_Ids kite_index_array,
                           Vector2 position, Vector2 offset, float v_padding,
                           float h_padding, size_t rows, size_t columns,
                           float move_duration);

bool tkbc_script_team_ball(Env *env, Kite_Ids kite_index_array,
                           Vector2 position, Vector2 offset, float radius,
                           float move_duration, float rotation_duration);

bool tkbc_script_team_mountain(Env *env, Kite_Ids kite_index_array,
                               Vector2 position, Vector2 offset,
                               float v_padding, float h_padding,
                               float move_duration, float rotation_duration);

bool tkbc_script_team_valley(Env *env, Kite_Ids kite_index_array,
                             Vector2 position, Vector2 offset, float v_padding,
                             float h_padding, float move_duration,
                             float rotation_duration);

bool tkbc_script_team_arc(Env *env, Kite_Ids kite_index_array, Vector2 position,
                          Vector2 offset, float v_padding, float h_padding,
                          float angle, float move_duration,
                          float rotation_duration);
bool tkbc_script_team_mouth(Env *env, Kite_Ids kite_index_array,
                            Vector2 position, Vector2 offset, float v_padding,
                            float h_padding, float angle, float move_duration,
                            float rotation_duration);

void tkbc_script_team_box(Env *env, Kite_Ids kite_index_array,
                          DIRECTION direction, float angle, float box_size,
                          float move_duration, float rotation_duration);
void tkbc_script_team_box_left(Env *env, Kite_Ids kite_index_array,
                               float box_size, float move_duration,
                               float rotation_duration);
void tkbc_script_team_box_right(Env *env, Kite_Ids kite_index_array,
                                float box_size, float move_duration,
                                float rotation_duration);

bool tkbc_script_team_split_box_up(Env *env, Kite_Ids kite_index_array,
                                   ODD_EVEN odd_even, float box_size,
                                   float move_duration,
                                   float rotation_duration);

void tkbc_script_team_diamond(Env *env, Kite_Ids kite_index_array,
                             DIRECTION direction, float angle, float box_size,
                             float move_duration, float rotation_duration);
void tkbc_script_team_diamond_left(Env *env, Kite_Ids kite_index_array,
                                  float box_size, float move_duration,
                                  float rotation_duration);
void tkbc_script_team_diamond_right(Env *env, Kite_Ids kite_index_array,
                                   float box_size, float move_duration,
                                   float rotation_duration);
```

---

## SCRIPT API in .kite files

The .kite files can be loaded dynamic at runtime via drag and drop.

Step 1 Initialization

Create the maximum amount of kites that should be used in the script.

```Scala
KITES 4
```

Step 2

Set up the enclosing script tags BEGIN and END.

```Scala
BEGIN
END
```

Step 3

For definition of the kite ids that should be effected by the primitive call
they can be specified in parentheses, or if every generated kite is effected
the keyword KITES can be used. The order of the arguments separated by a space
is the same as in the C calls.

Provide the primitive script calls, those contain:

```Scala
WAIT 2
QUIT 100
// action|>ids|>x|>y|>duration
MOVE_ADD KITES 0 -300 5
// action|>ids|>x|>y|>duration
MOVE (0 1 2 3) 0 -300 5

// action|>ids|>angle|>duration
ROTATION KITES 180 2
// action|>ids|>angle|>duration
ROTATION_ADD KITES 180 2

// action|>ids|>angle|>TIP|>duration
TIP_ROTATION KITES 180 LEFT 2
// action|>ids|>angle|>TIP|>duration
TIP_ROTATION_ADD KITES 180 RIGHT 2
```

For parallel visualization use braces to make the block that should be executed together.

```Scala
ROTATION KITES 180 2
{
    ROTATION_ADD KITES 180 2
    WAIT 2
    QUIT 100
}
```

Example:

```Scala
KITES 4

BEGIN
WAIT 0.000000
MOVE_ADD (0 1 2 3) 0.000000 -300.000000 5.000000
{
    MOVE (0) 718.799988 540.000000 2.000000
    MOVE (1) 879.599976 540.000000 2.000000
    MOVE (2) 1040.400024 540.000000 2.000000
    MOVE (3) 1201.199951 540.000000 2.000000
}
WAIT 1.000000
ROTATION (0 1 2 3) -90.000000 1.000000
WAIT 1.000000
ROTATION (0 1 2 3) -0.000000 2.000000
WAIT 3.000000
{
    MOVE_ADD (0 1 2 3) 0.000000 -300.000000 9.000000
    WAIT 1.500000
    QUIT 7.000000
}
WAIT 0.500000
WAIT 0.000000
END
```

A full example is provided in ./tkbc_scripts/first.kite.

The Team calls can be used in the '.kite' files in the following way.
The exact types of the variables can be assumed as in the C declarations above.
They are mostly float except rows and cols, but the types could maybe change so
the C declarations are up to date.
```JS
EXTERN TEAM_LINE KITES position.x position.y offset.x offset.y h_padding move_duration
EXTERN TEAM_GRID KITES position.x position.y offset.x offset.y v_padding h_padding rows columns move_duration
EXTERN TEAM_BALL KITES position.x position.y offset.x offset.y radius move_duration rotation_duration
EXTERN TEAM_MOUNTAIN KITES position.x position.y offset.x offset.y v_padding h_padding move_duration rotation_duration
EXTERN TEAM_VALLEY KITES position.x position.y offset.x offset.y v_padding h_padding move_duration rotation_duration
EXTERN TEAM_ARC KITES position.x position.y offset.x offset.y v_padding h_padding angle move_duration rotation_duration
EXTERN TEAM_MOUTH KITES position.x position.y offset.x offset.y v_padding h_padding angle move_duration rotation_duration
EXTERN TEAM_BOX KITES DIRECTION direction angle box_size move_duration rotation_duration
EXTERN TEAM_BOX_LEFT KITES box_size move_duration rotation_duration
EXTERN TEAM_BOX_RIGHT KITES box_size move_duration rotation_duration
EXTERN TEAM_SPLIT_BOX_UP KITES odd_even box_size move_duration rotation_duration
EXTERN TEAM_DIAMOND KITES direction angle box_size move_duration rotation_duration
EXTERN TEAM_DIAMOND_LEFT KITES box_size move_duration rotation_duration
EXTERN TEAM_DIAMOND_RIGHT KITES box_size move_duration rotation_duration
```
A more specific example would be the following.
```Scala
KITES 4
BEGIN
ROTATION (0 1 2 3) -90.000000 1.000000
EXTERN TEAM_BOX KITES LEFT -90 100 5.1 3.4
EXTERN TEAM_BOX (0 1 2 3) RIGHT -90 100 5.1 3.4
{
    MOVE_ADD (0 1 2 3) 0.000000 -300.000000 9.000000
    WAIT 1.500000
    QUIT 7.000000
}
EXTERN TEAM_SPLIT_BOX_UP KITES ODD 100 5 3
EXTERN TEAM_SPLIT_BOX_UP KITES EVEN 100 5 3
END
```
---

## Mappings

**`:Key:` `:Function Description:`**

`KEY_ESCAPE` Quits the Program.

`KEY_B` Take a Screenshot.

`KEY_V` Start recording the screen.

`SHIFT + KEY_V` End recording the screen.

`KEY_ENTER` Set all kites in start position face up (angle = 0).

`KEY_1|...|KEY_9` Toggles the selection of a kite.

`KEY_X` Rotate all selected kites face up (angle = 0).

`KEY_R` Rotate all selected kites at the center clockwise.

`SHIFT + KEY_R` Rotate all selected kites at the center anticlockwise.

`[KEY_H | KEY_L] + KEY_T` Rotate all selected kites at the Tip clockwise.

`[KEY_H | KEY_L] + SHIFT + KEY_T` Rotate all selected kites at the Tip anticlockwise.

`KEY_F` Switches between a fixed and a smooth rotation (toggle).

`KEY_LEFT | KEY_H` Moves the selected kites by the set speed to the left.

`KEY_DOWN | KEY_J` Moves the selected kites by the set speed to the down.

`KEY_UP | KEY_K` Moves the selected kites by the set speed to the up.

`KEY_RIGHT | KEY_L` Moves the selected kites by the set speed to the right.

`KEY_P` Increase the fly speed.

`SHIFT + KEY_P` Reduce the fly speed.

`KEY_O` Increase the turn speed.

`SHIFT + KEY_O` Reduce the turn speed.

`KEY_SPACE` Toggles the interruption that controls the execution of a script.

`KEY_TAB` Switches to the next loaded script.

`KEY_N` Plays the currently loaded sound.

`SHIFT + KEY_N` Stops the current sound track.

`KEY_M` Pauses the current sound track.

`SHIFT + KEY_M` Resumes the currently loaded sound track.

---
