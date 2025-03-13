#include "../src/choreographer/tkbc-script-api.h"
#include "../src/choreographer/tkbc-team-figures-api.h"

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

      KITE_MOVE_ADD(ID(0), 3 * kite.width, -3 * kite.width + kite.width,
                    move_duration),
      KITE_MOVE_ADD(ID(1), -3 * kite.width, -3 * kite.width, move_duration)

  );
  SET(

      KITE_ROTATION_ADD(ID(0), -270, rotation_duration),
      KITE_ROTATION_ADD(ID(1), 270, rotation_duration)

  );
  SET(KITE_WAIT(wait_time));
  SET(

      KITE_MOVE_ADD(ID(0), -kite.width, -2 * kite.width, move_duration),
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

      KITE_MOVE_ADD(ID(0), 8 * kite.width,
                    8 * kite.width + kite.width - lift_up, 3 * move_duration),
      KITE_MOVE_ADD(ID(1), -8 * kite.width, 8 * kite.width - lift_up,
                    3 * move_duration)

  );
  SET(KITE_WAIT(wait_time));
}

void face_in_center(Env *env) {
  SET(

      KITE_TIP_ROTATION_ADD(ID(0), -90, LEFT_TIP, rotation_duration),
      KITE_TIP_ROTATION_ADD(ID(1), 90, RIGHT_TIP, rotation_duration)

  );
  SET(

      KITE_MOVE_ADD(ID(0), -5 * kite.width - kite.width / 2, 0,
                    4 * move_duration),
      KITE_MOVE_ADD(ID(1), 5 * kite.width + kite.width / 2, 0,
                    4 * move_duration)

  );
}

void roll_up(Env *env, Kite_Ids ki) {
  tkbc_script_team_roll_split_up(env, ki, EVEN, 4, 0, 360, 2);
}

void edges_180(Env *env) {
  SET(

      KITE_MOVE_ADD(ID(0), -5 * kite.width + kite.width / 2, 0,
                    4 * move_duration),
      KITE_MOVE_ADD(ID(1), 5 * kite.width - kite.width / 2, 0,
                    4 * move_duration)

  );
  SET(

      KITE_TIP_ROTATION_ADD(ID(0), -180, RIGHT_TIP, rotation_duration),
      KITE_TIP_ROTATION_ADD(ID(1), 180, LEFT_TIP, rotation_duration)

  );
}

void change_90(Env *env) {
  SET(

      KITE_MOVE_ADD(ID(0), 4 * kite.width + kite.width / 2, 0,
                    4 * move_duration),
      KITE_MOVE_ADD(ID(1), -4 * kite.width - kite.width / 2, 0,
                    4 * move_duration)

  );
  SET(

      KITE_TIP_ROTATION_ADD(ID(0), 90, LEFT_TIP, rotation_duration),
      KITE_TIP_ROTATION_ADD(ID(1), 90, LEFT_TIP, rotation_duration)

  );
  SET(

      KITE_MOVE_ADD(ID(0), 0, -2.5 * kite.width, move_duration),
      KITE_MOVE_ADD(ID(1), 0, 2.5 * kite.width, move_duration)

  );

  SET(

      // TODO: Do the tip rotation instead, when it is working.
      // Turn in adaption
      // KITE_MOVE_ADD(ID(0), -kite.width, 0, rotation_duration),
      // KITE_MOVE_ADD(ID(1), kite.width, 0, rotation_duration),

      // KITE_TIP_ROTATION_ADD(ID(0), 180, LEFT_TIP, rotation_duration),
      // KITE_TIP_ROTATION_ADD(ID(1), 180, LEFT_TIP, rotation_duration)

      KITE_MOVE_ADD(ID(0), -kite.width / 2, 0, rotation_duration),
      KITE_MOVE_ADD(ID(1), kite.width / 2, 0, rotation_duration),

      KITE_ROTATION_ADD(ID(0), 180, rotation_duration),
      KITE_ROTATION_ADD(ID(1), 180, rotation_duration)

  );

  SET(

      KITE_MOVE_ADD(ID(0), 0, 2.5 * kite.width, move_duration),
      KITE_MOVE_ADD(ID(1), 0, -2.5 * kite.width, move_duration)

  );
}

void rollkiss_90_break(Env *env) {
  SET(

      KITE_ROTATION_ADD(ID(0), 270, rotation_duration),
      KITE_ROTATION_ADD(ID(1), 270, rotation_duration),

      KITE_MOVE_ADD(ID(0), -kite.height / 2, 0.5 * kite.width, move_duration),
      KITE_MOVE_ADD(ID(1), kite.height / 2, -0.5 * kite.width, move_duration)

  );
}

void edges_turn_in(Env *env) {
  SET(

      KITE_MOVE_ADD(ID(0), -5 * kite.width, 0, move_duration),
      KITE_MOVE_ADD(ID(1), 5 * kite.width, 0, move_duration)

  );

  SET(

      // TODO: Do the tip rotation instead, when it is working.
      // Turn in adaption
      // KITE_MOVE_ADD(ID(0), -kite.width, 0, rotation_duration),
      // KITE_MOVE_ADD(ID(1), kite.width, 0, rotation_duration),

      // KITE_TIP_ROTATION_ADD(ID(0), 180, RIGHT_TIP, rotation_duration),
      // KITE_TIP_ROTATION_ADD(ID(1), 180, RIGHT_TIP, rotation_duration)

      KITE_MOVE_ADD(ID(0), 0, -kite.width / 2, rotation_duration),
      KITE_MOVE_ADD(ID(1), 0, kite.width / 2, rotation_duration),

      KITE_ROTATION_ADD(ID(0), -180, rotation_duration),
      KITE_ROTATION_ADD(ID(1), -180, rotation_duration)

  );
  SET(

      KITE_MOVE_ADD(ID(0), 5 * kite.width, 0, move_duration),
      KITE_MOVE_ADD(ID(1), -5 * kite.width, 0, move_duration)

  );
}

void kiss_reverse(Env *env) {
  SET(

      KITE_MOVE_ADD(ID(0), kite.height / 2, 0, move_duration),
      KITE_MOVE_ADD(ID(1), -kite.height / 2, 0, move_duration)

  );

  SET(KITE_WAIT(wait_time));

  SET(

      KITE_MOVE_ADD(ID(0), -2 * kite.width, 0, move_duration),
      KITE_MOVE_ADD(ID(1), 2 * kite.width, 0, move_duration)

  );
}

void tip_turn_down_revers(Env *env) {
  SET(

      KITE_TIP_ROTATION_ADD(ID(0), -180, RIGHT_TIP, rotation_duration),
      KITE_TIP_ROTATION_ADD(ID(1), 180, LEFT_TIP, rotation_duration)

  );
  SET(

      KITE_MOVE_ADD(ID(0), 2 * kite.width - kite.height, 0, move_duration),
      KITE_MOVE_ADD(ID(1), -2 * kite.width + kite.height, 0, move_duration)

  );
}

void slide_down_forward(Env *env) {
  SET(

      KITE_MOVE_ADD(ID(0), 0, kite.width / 2, move_duration),
      KITE_MOVE_ADD(ID(1), 0, kite.width / 2, move_duration)

  );
  SET(

      KITE_MOVE_ADD(ID(0), -3 * kite.width + kite.height, 0, move_duration),
      KITE_MOVE_ADD(ID(1), 3 * kite.width - kite.height, 0, move_duration)

  );
}

void tip_turn_to_moon_walk(Env *env) {
  SET(

      KITE_TIP_ROTATION_ADD(ID(0), 90, LEFT_TIP, rotation_duration),
      KITE_TIP_ROTATION_ADD(ID(1), -90, RIGHT_TIP, rotation_duration)

  );
  // to face up
  SET(

      KITE_TIP_ROTATION_ADD(ID(0), 180, LEFT_TIP, rotation_duration),
      KITE_TIP_ROTATION_ADD(ID(1), -180, RIGHT_TIP, rotation_duration)

  );
  // to face down
  SET(

      KITE_TIP_ROTATION_ADD(ID(0), 180, RIGHT_TIP, rotation_duration),
      KITE_TIP_ROTATION_ADD(ID(1), -180, LEFT_TIP, rotation_duration)

  );
  // to face up
  SET(

      KITE_TIP_ROTATION_ADD(ID(0), 180, LEFT_TIP, rotation_duration),
      KITE_TIP_ROTATION_ADD(ID(1), -180, RIGHT_TIP, rotation_duration)

  );
}

void octagon(Env *env, Kite_Ids ki) {
  for (size_t i = 0; i < 4; ++i) {
    SET(KITE_TIP_ROTATION_ADD(ki, -135, RIGHT_TIP, rotation_duration));
    SET(KITE_TIP_ROTATION_ADD(ki, -135, LEFT_TIP, rotation_duration));
  }
}

void arc_to_diamond_out(Env *env) {
  SET(

      KITE_ROTATION_ADD(ID(0), 45, rotation_duration),
      KITE_ROTATION_ADD(ID(1), -45, rotation_duration)

  );

  SET(

      KITE_MOVE_ADD(ID(0), -2 * kite.width, -3 * kite.width, move_duration),
      KITE_MOVE_ADD(ID(1), 2 * kite.width, -3 * kite.width, move_duration)

  );

  SET(

      KITE_ROTATION_ADD(ID(0), 90, rotation_duration),
      KITE_ROTATION_ADD(ID(1), -90, rotation_duration)

  );

  SET(

      KITE_MOVE_ADD(ID(0), -2 * kite.width, 3 * kite.width, move_duration),
      KITE_MOVE_ADD(ID(1), 2 * kite.width, 3 * kite.width, move_duration)

  );

  SET(

      KITE_ROTATION_ADD(ID(0), 135, rotation_duration),
      KITE_ROTATION_ADD(ID(1), -135, rotation_duration)

  );

  SET(KITE_WAIT(wait_time));

  SET(

      KITE_MOVE_ADD(ID(0), 2 * kite.width, 0, move_duration),
      KITE_MOVE_ADD(ID(1), -2 * kite.width, 0, move_duration)

  );
}

void bicycle_in(Env *env) {
  SET(

      KITE_MOVE_ADD(ID(0), 2 * kite.width + kite.width / 2, 0, move_duration),
      KITE_MOVE_ADD(ID(1), -2 * kite.width - kite.width / 2, 0, move_duration),

      KITE_ROTATION_ADD(ID(0), -360, move_duration),
      KITE_ROTATION_ADD(ID(1), 360, move_duration)

  );

  SET(KITE_WAIT(wait_time));
}

void rollkiss_90_break_to_anti_split_position(Env *env) {
  SET(

      KITE_ROTATION_ADD(ID(0), -270, move_duration),
      KITE_ROTATION_ADD(ID(1), -270, move_duration),

      KITE_MOVE_ADD(ID(0), 0.5 * kite.width, -kite.height / 2, move_duration),
      KITE_MOVE_ADD(ID(1), -0.5 * kite.width, kite.height / 2, move_duration)

  );

  SET(

      KITE_ROTATION_ADD(ID(0), -90, move_duration),
      KITE_MOVE_ADD(ID(0), -kite.width / 2 + kite.height,
                    -kite.width - kite.width / 2 + kite.height, move_duration),

      KITE_TIP_ROTATION_ADD(ID(1), -90, LEFT_TIP, move_duration)

  );
}

void choreo(Env *env, Kite_Ids ki) {
  kite = *env->vanilla_kite;
  env->kite_array->elements[0].kite->body_color = DARKGREEN;
  env->kite_array->elements[1].kite->body_color = BLUE;
  tkbc_script_begin();
  bicycle_start(env, ki);
  diamond_stack_figure(env);
  extreme_window_slide(env);

  face_in_center(env);
  roll_up(env, ki);
  edges_180(env);

  change_90(env);
  rollkiss_90_break(env);
  edges_turn_in(env);
  kiss_reverse(env);
  tip_turn_down_revers(env);
  slide_down_forward(env);

  tip_turn_to_moon_walk(env);
  octagon(env, ki);
  arc_to_diamond_out(env);
  bicycle_in(env);
  rollkiss_90_break_to_anti_split_position(env);

  SET(KITE_WAIT(wait_time));
  tkbc_script_end();
}
