# Team Kite Ballett Choreographer
- The choreography creator.

- The dependency is version 5.5:
- Original Raylib 5.5 Source Code: https://github.com/raysan5/raylib/releases/tag/5.5
- For commits older than fb53c95 the used library is:
- Original Raylib 5.0 Source Code: https://github.com/raysan5/raylib/releases/tag/5.0

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
$ make
$ ./build/tkbc
```
-----------------------------------------------------------------------------
## Mappings

__```:Key:``` ```:Function Description:```__

```KEY_ESCAPE``` Quits the Program.

```KEY_B``` Take a Screenshot.

```KEY_V``` Start recording the screen.

```SHIFT + KEY_V``` End recording the screen.

```KEY_ENTER``` Set all kites in start position face up (angle = 0).

```KEY_1|...|KEY_9``` Toggles the selection of a kite.

```KEY_N``` Rotate all selected kites face up (angle = 0).

```KEY_R``` Rotate all selected kites at the center clockwise.

```SHIFT + KEY_R``` Rotate all selected kites at the center anticlockwise.

```[KEY_H | KEY_L] + KEY_T``` Rotate all selected kites at the Tip clockwise.

```[KEY_H | KEY_L] + SHIFT + KEY_T``` Rotate all selected kites at the Tip anticlockwise.

```KEY_F``` Switches between a fixed and a smooth rotation (toggle).

```KEY_LEFT | KEY_H``` Moves the selected kites by the set speed to the left.

```KEY_DOWN | KEY_J``` Moves the selected kites by the set speed to the down.

```KEY_UP | KEY_K``` Moves the selected kites by the set speed to the up.

```KEY_RIGHT | KEY_L``` Moves the selected kites by the set speed to the right.

```KEY_P``` Increase the fly speed.

```SHIFT + KEY_P``` Reduce the fly speed.

```KEY_O``` Increase the turn speed.

```SHIFT + KEY_O``` Reduce the turn speed.

```KEY_SPACE``` Toggles the interruption that controls the execution of a script.

```KEY_TAB``` Switches to the next loaded script.

```KEY_S``` Plays the currently loaded sound.

```SHIFT + KEY_S``` Stops the current sound track.

```KEY_M``` Pauses the current sound track.

```SHIFT + KEY_M``` Resumes the currently loaded sound track.

-----------------------------------------------------------------------------
