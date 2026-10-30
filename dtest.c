tkbc_script_input {
    Kite kite = *env->vanilla_kite;

#define kite_count 2
    Kite_Ids ki = tkbc_kite_array_generate(env, kite_count);
    static_assert(kite_count >= 2, "");
    Id zero = ki.elements[0];
    Id one = ki.elements[1];

    tkbc_script_begin("SECOND");
    float rotation_duration = 1;
    SET(

        KITE_MOVE_ADD(ID(zero), -kite.width, 0, rotation_duration),
        KITE_MOVE_ADD(ID(one), kite.width, 0, rotation_duration),

        KITE_TIP_ROTATION_ADD(ID(zero), 180, LEFT_TIP, rotation_duration),
        KITE_TIP_ROTATION_ADD(ID(one), 180, LEFT_TIP, rotation_duration)

    );
    tkbc_script_end();

    free(ki.elements);
}
