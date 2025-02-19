#include "global/tkbc-types.h"

static KeyMap default_keymaps[] = {
    {
        .description = "Change key mappings.",
        .key = KEY_ESCAPE,
        .hash = 1000,
    },

    {
        .description = "Quits the Program.",
        .key = KEY_Q,
        .hash = 1005,
    },

    {
        .description = "Take a Screenshot.",
        .key = KEY_B,
        .hash = 1007,
    },

    {
        .description = "Start recording the screen.",
        .key = KEY_V,
        .hash = 1008,
    },
    {
        .description = "End recording the screen.",
        .mod_key = KEY_LEFT_SHIFT,
        .mod_co_key = KEY_RIGHT_SHIFT,
        .key = KEY_V,
        .hash = 1009,
    },

    {
        .description = "Set all kites in start position angle 0.",
        .key = KEY_ENTER,
        .hash = 1010,
    },

    // KEY_1|...|KEY_9 Toggles the selection of a kite.

    {
        .description = "Rotate all selected kites angle 0.",
        .key = KEY_X,
        .hash = 1011,
    },
    {
        .description = "Rotate all selected kites at the center clockwise.",
        .key = KEY_R,
        .hash = 1012,
    },

    {
        .description = "Rotate all selected kites at the center anticlockwise.",
        .mod_key = KEY_LEFT_SHIFT,
        .mod_co_key = KEY_RIGHT_SHIFT,
        .key = KEY_R,
        .hash = 1013,
    },

    {
        .description = "Rotate all selected kites at the tip clockwise.",
        .selection_key1 = KEY_H,
        .selection_key2 = KEY_L,
        .key = KEY_T,
        .hash = 1014,
    },
    {
        .description = "Rotate all selected kites at the tip anticlockwise.",
        .mod_key = KEY_LEFT_SHIFT,
        .mod_co_key = KEY_RIGHT_SHIFT,
        .selection_key1 = KEY_H,
        .selection_key2 = KEY_L,
        .key = KEY_T,
        .hash = 1015,
    },

    {
        .description =
            "Switches between a fixed and a smooth rotation (toggle).",
        .key = KEY_F,
        .hash = 1016,
    },

    // TODO: More than one key?.
    {
        .description = "Moves the selected kites by the set "
                       "speed to the left.",
        .key = KEY_H,
        .hash = 1017,
    },
    {
        .description = "Moves the selected kites by the set "
                       "speed to the down.",
        .key = KEY_J,
        .hash = 1018,
    },

    {
        .description = "Moves the selected kites by the set "
                       "speed to the up.",
        .key = KEY_K,
        .hash = 1019,
    },
    {
        .description = "Moves the selected kites by the set "
                       "speed to the right.",
        .key = KEY_L,
        .hash = 1020,
    },

    {
        .description = "Increase the fly speed.",
        .key = KEY_P,
        .hash = 1021,
    },
    {
        .description = "Reduce the fly speed.",
        .mod_key = KEY_LEFT_SHIFT,
        .mod_co_key = KEY_RIGHT_SHIFT,
        .key = KEY_P,
        .hash = 1022,
    },

    {
        .description = "Increase the turn speed.",
        .key = KEY_O,
        .hash = 1023,
    },
    {
        .description = "Reduce the turn speed.",
        .mod_key = KEY_LEFT_SHIFT,
        .mod_co_key = KEY_RIGHT_SHIFT,
        .key = KEY_O,
        .hash = 1024,
    },

    {
        .description =
            "Toggles the interruption that controls the execution of a script.",
        .key = KEY_SPACE,
        .hash = 1025,
    },
    {
        .description = "Switches to the next loaded script.",
        .key = KEY_TAB,
        .hash = 1026,
    },

    {
        .description = "Plays the currently loaded sound.",
        .key = KEY_N,
        .hash = 1027,
    },
    {
        .description = "Stops the current sound track.",
        .mod_key = KEY_LEFT_SHIFT,
        .mod_co_key = KEY_RIGHT_SHIFT,
        .key = KEY_N,
        .hash = 1028,
    },

    {
        .description = "Pauses the current sound track.",
        .key = KEY_M,
        .hash = 1029,
    },
    {
        .description = "Resumes the currently loaded sound track.",
        .mod_key = KEY_LEFT_SHIFT,
        .mod_co_key = KEY_RIGHT_SHIFT,
        .key = KEY_M,
        .hash = 1030,
    },
    {
        .description = "Rotate kites around in a circle clockwise.",
        .selection_key1 = KEY_H,
        .selection_key2 = KEY_L,
        .key = KEY_C,
        .hash = 1031,
    },

    {
        .description = "Rotate kites around in a circle anticlockwise.",
        .mod_key = KEY_LEFT_SHIFT,
        .mod_co_key = KEY_RIGHT_SHIFT,
        .selection_key1 = KEY_H,
        .selection_key2 = KEY_L,
        .key = KEY_C,
        .hash = 1032,
    },

    {
        .description = "Switch to mouse constol movement.",
        .key = KEY_ZERO,
        .hash = 1033,
    },
    {
        .description = "Moves the selected kites by the set "
                       "speed towards the mouse.",
        .key = KEY_W,
        .hash = 1034,
    },
    {
        .description = "Moves the selected kites by the set "
                       "speed left to the mouse.",
        .key = KEY_A,
        .hash = 1035,
    },
    {
        .description = "Moves the selected kites by the set "
                       "speed away from the mouse.",
        .key = KEY_S,
        .hash = 1036,
    },
    {
        .description = "Moves the selected kites by the set "
                       "speed right to the mouse.",
        .key = KEY_D,
        .hash = 1037,
    },
};
