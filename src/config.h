#include "global/tkbc-types.h"

static Key_Map default_keymaps[] = {
    {
        .description = "Change key mappings.",
        .key = KEY_ESCAPE,
        .hash = KMH_CHANGE_KEY_MAPPINGS,
    },

    {
        .description = "Quits the Program.",
        .key = KEY_BACKSPACE,
        .hash = KMH_QUIT_PROGRAM,
    },

    {
        .description = "Take a Screenshot.",
        .key = KEY_B,
        .hash = KMH_TAKE_SCREENSHOT,
    },

    {
        .description = "Begin recording the screen.",
        .key = KEY_V,
        .hash = KMH_BEGIN_RECORDING,
    },
    {
        .description = "End recording the screen.",
        .mod_key = KEY_LEFT_SHIFT,
        .mod_co_key = KEY_RIGHT_SHIFT,
        .key = KEY_V,
        .hash = KMH_END_RECORDING,
    },

    {
        .description = "Set all kites in start position angle 0.",
        .key = KEY_ENTER,
        .hash = KMH_SET_KITES_TO_START_POSITION,
    },

    // KEY_1|...|KEY_9 Toggles the selection of a kite.

    {
        .description = "Rotate all selected kites angle 0.",
        .key = KEY_X,
        .hash = KMH_ROTATE_KITES_ANGLE_ZERO,
    },

    {
        .description = "Rotate all selected kites at the tip clockwise.",
        .selection_key1 = KEY_H,
        .selection_key2 = KEY_L,
        .key = KEY_T,
        .hash = KMH_ROTATE_KITES_TIP_CLOCKWISE,
    },
    {
        .description = "Rotate all selected kites at the tip anticlockwise.",
        .mod_key = KEY_LEFT_SHIFT,
        .mod_co_key = KEY_RIGHT_SHIFT,
        .selection_key1 = KEY_H,
        .selection_key2 = KEY_L,
        .key = KEY_T,
        .hash = KMH_ROTATE_KITES_TIP_ANTICLOCKWISE,
    },

    {
        .description = "Rotate kites around in a circle clockwise.",
        .selection_key1 = KEY_H,
        .selection_key2 = KEY_L,
        .key = KEY_C,
        .hash = KMH_ROTATE_KITES_CIRCLE_CLOCKWISE,
    },
    {
        .description = "Rotate kites around in a circle anticlockwise.",
        .mod_key = KEY_LEFT_SHIFT,
        .mod_co_key = KEY_RIGHT_SHIFT,
        .selection_key1 = KEY_H,
        .selection_key2 = KEY_L,
        .key = KEY_C,
        .hash = KMH_ROTATE_KITES_CIRCLE_ANTICLOCKWISE,
    },

    {
        .description =
            "Switches between a fixed and a smooth rotation (toggle).",
        .key = KEY_F,
        .hash = KMH_TOGGLE_FIXED,
    },

    {
        .description = "Moves the selected kites by the set "
                       "speed to the left.",
        .key = KEY_A,
        .hash = KMH_MOVES_KITES_LEFT,
    },
    {
        .description = "Moves the selected kites by the set "
                       "speed to the down.",
        .key = KEY_S,
        .hash = KMH_MOVES_KITES_DOWN,
    },

    {
        .description = "Moves the selected kites by the set "
                       "speed to the up.",
        .key = KEY_W,
        .hash = KMH_MOVES_KITES_UP,
    },
    {
        .description = "Moves the selected kites by the set "
                       "speed to the right.",
        .key = KEY_D,
        .hash = KMH_MOVES_KITES_RIGHT,
    },

    {
        .description = "Increase the fly speed.",
        .key = KEY_P,
        .hash = KMH_INCREASE_FLY_SPEED,
    },
    {
        .description = "Reduce the fly speed.",
        .mod_key = KEY_LEFT_SHIFT,
        .mod_co_key = KEY_RIGHT_SHIFT,
        .key = KEY_P,
        .hash = KMH_REDUCE_FLY_SPEED,
    },

    {
        .description = "Increase the turn speed.",
        .key = KEY_O,
        .hash = KMH_INCREASE_TURN_SPEED,
    },
    {
        .description = "Reduce the turn speed.",
        .mod_key = KEY_LEFT_SHIFT,
        .mod_co_key = KEY_RIGHT_SHIFT,
        .key = KEY_O,
        .hash = KMH_REDUCE_TURN_SPEED,
    },

    {
        .description =
            "Toggles the interruption that controls the execution of a script.",
        .key = KEY_SPACE,
        .hash = KMH_TOGGLE_SCRIPT_EXECUTION,
    },
    {
        .description = "Switches to the next loaded script.",
        .key = KEY_TAB,
        .hash = KMH_SWITCHES_NEXT_SCRIPT,
    },

    {
        .description = "Plays the currently loaded sound.",
        .key = KEY_N,
        .hash = KMH_PLAYS_SOUND,
    },
    {
        .description = "Stops the current sound track.",
        .mod_key = KEY_LEFT_SHIFT,
        .mod_co_key = KEY_RIGHT_SHIFT,
        .key = KEY_N,
        .hash = KMH_STOPS_SOUND,
    },

    {
        .description = "Pauses the current sound track.",
        .key = KEY_M,
        .hash = KMH_PAUSES_SOUND,
    },
    {
        .description = "Resumes the currently loaded sound track.",
        .mod_key = KEY_LEFT_SHIFT,
        .mod_co_key = KEY_RIGHT_SHIFT,
        .key = KEY_M,
        .hash = KMH_RESUMES_SOUND,
    },

    // Alternative mouse controlling.
    {
        .description = "Switch to mouse control movement.",
        .key = KEY_ZERO,
        .hash = KMH_SWITCH_MOUSE_CONTOL_MOVEMENT,
    },
    {
        .description = "Lock the angle of the kite.",
        .mod_key = KEY_LEFT_SHIFT,
        .mod_co_key = KEY_RIGHT_SHIFT,
        .key = KEY_NULL,
        .hash = KMH_LOCK_KITE_ANGLE,
    },
    {
        .description = "Lock the angle for tip turn.",
        .mod_key = KEY_LEFT_CONTROL,
        .mod_co_key = KEY_RIGHT_CONTROL,
        .key = KEY_NULL,
        .hash = KMH_LOCK_KITE_TIP,
    },
    {
        .description = "Snaps the angle of the kite.",
        .key = KEY_R,
        .hash = KMH_SNAP_KITE_ANGLE,
    },

    {
        .description = "Rotate all selected kites at the center clockwise.",
        .key = KEY_E,
        .hash = KMH_ROTATE_KITES_CENTER_CLOCKWISE,
    },
    {
        .description = "Rotate all selected kites at the center anticlockwise.",
        .key = KEY_Q,
        .hash = KMH_ROTATE_KITES_CENTER_ANTICLOCKWISE,
    },

    {
        .description =
            "Moves the selected kites by the set speed towards the mouse.",
        .key = KEY_W,
        .hash = KMH_MOVES_KITES_TOWARDS_MOUSE,
    },
    {
        .description =
            "Moves the selected kites by the set speed left to the mouse.",
        .key = KEY_D,
        .hash = KMH_MOVES_KITES_LEFT_AROUND_MOUSE,
    },
    {
        .description =
            "Moves the selected kites by the set speed away from the mouse.",
        .key = KEY_S,
        .hash = KMH_MOVES_KITES_AWAY_MOUSE,
    },
    {
        .description =
            "Moves the selected kites by the set speed right to the mouse.",
        .key = KEY_A,
        .hash = KMH_MOVES_KITES_RIGHT_AROUND_MOUSE,
    },
};
