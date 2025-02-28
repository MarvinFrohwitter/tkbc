#include "../src/choreographer/tkbc-script-api.h"

static Kite kite;
static float wait_time = 0.5;
static float move_duration = 1;
static float rotation_duration = 1;

void bicycle_start(Env *env, Kite_Ids ki) {

  int bottom_padding = kite.height;

  COLLECTION(

      KITE_MOVE(ID(0), env->window_width / 2.0 - kite.width,
                env->window_height - kite.height - bottom_padding, 0),
      KITE_MOVE(ID(1), env->window_width / 2.0 + kite.width,
                env->window_height - kite.height - bottom_padding, 0),
      KITE_ROTATION(ki, 0, 0),

      // Just for documentation
      KITE_QUIT(wait_time)

  );
  SET(KITE_WAIT(4 * wait_time));
  SET(

      KITE_MOVE_ADD(ID(0), 2 * kite.width, -1.5 * kite.width, move_duration),
      KITE_ROTATION_ADD(tkbc_indexs_append(0), -270, rotation_duration)

  );
  SET(

      KITE_MOVE_ADD(ID(1), -2 * kite.width, -1.5 * kite.width, move_duration),
      KITE_ROTATION_ADD(tkbc_indexs_append(1), 270, rotation_duration)

  );
  SET(

      KITE_MOVE_ADD(ID(1), -(-2 * kite.width), 1.5 * kite.width, move_duration),
      KITE_ROTATION_ADD(tkbc_indexs_append(1), -315, rotation_duration)

  );
  SET(

      KITE_MOVE_ADD(ID(0), -(2 * kite.width), 1.5 * kite.width, move_duration),
      KITE_ROTATION_ADD(tkbc_indexs_append(0), 315, rotation_duration)

  );
  SET(KITE_WAIT(wait_time));
}

void diamond_stack_figure(Env *env) {
  SET(

      KITE_MOVE_ADD(ID(0), -2 * kite.width, -2 * kite.width, move_duration),
      KITE_MOVE_ADD(ID(1), 2 * kite.width, -2 * kite.width, move_duration)

  );
  SET(

      KITE_ROTATION_ADD(ID(0), 270, rotation_duration),
      KITE_ROTATION_ADD(ID(1), -270, rotation_duration)

  );
  SET(KITE_WAIT(wait_time));
  SET(

      KITE_MOVE_ADD(ID(0), 3 * kite.width, -3 * kite.width, move_duration),
      KITE_MOVE_ADD(ID(1), -3 * kite.width, -3 * kite.width + kite.width,
                    move_duration)

  );
  SET(

      KITE_ROTATION_ADD(ID(0), -270, rotation_duration),
      KITE_ROTATION_ADD(ID(1), 270, rotation_duration)

  );
  SET(KITE_WAIT(wait_time));
  SET(

      KITE_MOVE_ADD(ID(0), -kite.width, -kite.width, move_duration),
      KITE_MOVE_ADD(ID(1), kite.width, -kite.width, move_duration)

  );
  SET(

      KITE_ROTATION_ADD(ID(0), 45, rotation_duration),
      KITE_ROTATION_ADD(ID(1), -45, rotation_duration)

  );
  SET(

      KITE_MOVE_ADD(ID(0), -1.5 * kite.width, 0, move_duration),
      KITE_MOVE_ADD(ID(1), 1.5 * kite.width, 0, move_duration)

  );
}

void extreme_window_slide(Env *env) {
  int lift_up = 4 * kite.width;
  SET(

      KITE_TIP_ROTATION_ADD(ID(0), 90, LEFT_TIP, rotation_duration),
      KITE_TIP_ROTATION_ADD(ID(1), -90, RIGHT_TIP, rotation_duration)

  );
  SET(

      KITE_MOVE_ADD(ID(0), 8 * kite.width, 8 * kite.width - lift_up,
                    3 * move_duration),
      KITE_MOVE_ADD(ID(1), -8 * kite.width, 8 * kite.width - lift_up,
                    3 * move_duration)

  );
  SET(KITE_WAIT(wait_time));
}

void edges_180(Env *env) {
  SET(

      KITE_TIP_ROTATION_ADD(ID(0), -90, LEFT_TIP, rotation_duration),
      KITE_TIP_ROTATION_ADD(ID(1), 90, RIGHT_TIP, rotation_duration)

  );
  SET(

      KITE_MOVE_ADD(ID(0), -10 * kite.width, 0, 4 * move_duration),
      KITE_MOVE_ADD(ID(1), 10 * kite.width, 0, 4 * move_duration)

  );
  SET(

      KITE_TIP_ROTATION_ADD(ID(0), -180, RIGHT_TIP, rotation_duration),
      KITE_TIP_ROTATION_ADD(ID(1), 180, LEFT_TIP, rotation_duration)

  );
  SET(

      KITE_MOVE_ADD(ID(0), 10 * kite.width, 0, 4 * move_duration),
      KITE_MOVE_ADD(ID(1), -10 * kite.width, 0, 4 * move_duration)

  );
  SET(

      KITE_TIP_ROTATION_ADD(ID(0), 180, LEFT_TIP, rotation_duration),
      KITE_TIP_ROTATION_ADD(ID(1), -180, RIGHT_TIP, rotation_duration)

  );
}

void change_90(Env *env) {
  SET(

      KITE_MOVE_ADD(ID(0), -5 * kite.width - kite.width / 2, 0,
                    4 * move_duration),
      KITE_MOVE_ADD(ID(1), 5 * kite.width + kite.width / 2, 0,
                    4 * move_duration)

  );
  SET(

      KITE_TIP_ROTATION_ADD(ID(0), 90, LEFT_TIP, rotation_duration),
      KITE_TIP_ROTATION_ADD(ID(1), 90, LEFT_TIP, rotation_duration)

  );
  SET(

      KITE_MOVE_ADD(ID(0), 0, 3.5 * kite.width, 1 * move_duration),
      KITE_MOVE_ADD(ID(1), 0, -2.5 * kite.width, 1 * move_duration)

  );

  SET(

      // Turn in adaption
      KITE_MOVE_ADD(ID(0), -kite.width, 0, rotation_duration),
      KITE_MOVE_ADD(ID(1), kite.width, 0, rotation_duration),

      KITE_TIP_ROTATION_ADD(ID(0), 180, LEFT_TIP, rotation_duration),
      KITE_TIP_ROTATION_ADD(ID(1), 180, LEFT_TIP, rotation_duration)

  );
}

void choreo(Env *env, Kite_Ids ki) {
  kite = *env->vanilla_kite;
  tkbc_script_begin();
  bicycle_start(env, ki);
  diamond_stack_figure(env);
  extreme_window_slide(env);
  edges_180(env);
  change_90(env);
  SET(KITE_WAIT(wait_time));
  tkbc_script_end();
}
