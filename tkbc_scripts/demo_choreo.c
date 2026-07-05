#include "../src/choreographer/tkbc-script-api.h"
#include "../src/choreographer/tkbc-team-figures-api.h"

static Kite kite;
static float wait_time = 0.5;
static float move_duration = 1;
static float rotation_duration = 1;
Id zero = 0;
Id one = 1;

void bicycle_start(Env *env, Kite_Ids ki) {
  int bottom_padding = kite.height;

  SET(

      KITE_MOVE(ID(zero), env->window_width / 2.0 - kite.width,
                env->window_height - kite.height - bottom_padding, 0),
      KITE_MOVE(ID(one), env->window_width / 2.0 + kite.width,
                env->window_height - kite.height - bottom_padding, 0),
      KITE_ROTATION(ki, 0, 0),

      // Just for documentation
      KITE_QUIT(wait_time)

  );
  SET(KITE_WAIT(wait_time));
  SET(

      KITE_MOVE_ADD(ID(zero), 2 * kite.width, -1.5 * kite.width, move_duration),
      KITE_ROTATION_ADD(tkbc_indexs_append(0), -270, rotation_duration)

  );
  SET(

      KITE_MOVE_ADD(ID(one), -2 * kite.width, -1.5 * kite.width, move_duration),
      KITE_ROTATION_ADD(tkbc_indexs_append(1), 270, rotation_duration)

  );
  SET(

      KITE_MOVE_ADD(ID(one), -(-2 * kite.width), 1.5 * kite.width,
                    move_duration),
      KITE_ROTATION_ADD(tkbc_indexs_append(1), -315, rotation_duration)

  );
  SET(

      KITE_MOVE_ADD(ID(zero), -(2 * kite.width), 1.5 * kite.width,
                    move_duration),
      KITE_ROTATION_ADD(tkbc_indexs_append(0), 315, rotation_duration)

  );
  SET(KITE_WAIT(wait_time));
}

void diamond_stack_figure(Env *env) {
  SET(

      KITE_MOVE_ADD(ID(zero), -2 * kite.width, -2 * kite.width, move_duration),
      KITE_MOVE_ADD(ID(one), 2 * kite.width, -2 * kite.width, move_duration)

  );
  SET(

      KITE_ROTATION_ADD(ID(zero), 270, rotation_duration),
      KITE_ROTATION_ADD(ID(one), -270, rotation_duration)

  );
  SET(KITE_WAIT(wait_time));
  SET(

      KITE_MOVE_ADD(ID(zero), 3 * kite.width, -3 * kite.width + kite.width,
                    move_duration),
      KITE_MOVE_ADD(ID(one), -3 * kite.width, -3 * kite.width, move_duration)

  );
  SET(

      KITE_ROTATION_ADD(ID(zero), -270, rotation_duration),
      KITE_ROTATION_ADD(ID(one), 270, rotation_duration)

  );
  SET(KITE_WAIT(wait_time));
  SET(

      KITE_MOVE_ADD(ID(zero), -kite.width, -2 * kite.width, move_duration),
      KITE_MOVE_ADD(ID(one), kite.width, -kite.width, move_duration)

  );
  SET(

      KITE_ROTATION_ADD(ID(zero), 45, rotation_duration),
      KITE_ROTATION_ADD(ID(one), -45, rotation_duration)

  );
  SET(

      KITE_MOVE_ADD(ID(zero), -1.5 * kite.width, 0, move_duration),
      KITE_MOVE_ADD(ID(one), 1.5 * kite.width, 0, move_duration)

  );
}

void extreme_window_slide(Env *env) {
  int lift_up = 4 * kite.width;
  SET(

      KITE_TIP_ROTATION_ADD(ID(zero), 90, LEFT_TIP, rotation_duration),
      KITE_TIP_ROTATION_ADD(ID(one), -90, RIGHT_TIP, rotation_duration)

  );
  SET(

      KITE_MOVE_ADD(ID(zero), 8 * kite.width,
                    8 * kite.width + kite.width - lift_up, 3 * move_duration),
      KITE_MOVE_ADD(ID(one), -8 * kite.width, 8 * kite.width - lift_up,
                    3 * move_duration)

  );
  SET(KITE_WAIT(wait_time));
}

void face_in_center(Env *env) {
  SET(

      KITE_TIP_ROTATION_ADD(ID(zero), -90, LEFT_TIP, rotation_duration),
      KITE_TIP_ROTATION_ADD(ID(one), 90, RIGHT_TIP, rotation_duration)

  );
  SET(

      KITE_MOVE_ADD(ID(zero), -5 * kite.width - kite.width / 2, 0,
                    4 * move_duration),
      KITE_MOVE_ADD(ID(one), 5 * kite.width + kite.width / 2, 0,
                    4 * move_duration)

  );
}

void roll_up(Env *env, Kite_Ids ki) {
  tkbc_script_team_roll_split_up(env, ki, EVEN, 4, 0, 360, 2);
}

void edges_180(Env *env) {
  SET(

      KITE_MOVE_ADD(ID(zero), -5 * kite.width + kite.width / 2, 0,
                    4 * move_duration),
      KITE_MOVE_ADD(ID(one), 5 * kite.width - kite.width / 2, 0,
                    4 * move_duration)

  );
  SET(

      KITE_TIP_ROTATION_ADD(ID(zero), -180, RIGHT_TIP, rotation_duration),
      KITE_TIP_ROTATION_ADD(ID(one), 180, LEFT_TIP, rotation_duration)

  );
}

void change_90(Env *env) {
  SET(

      KITE_MOVE_ADD(ID(zero), 4 * kite.width + kite.width / 2, 0,
                    4 * move_duration),
      KITE_MOVE_ADD(ID(one), -4 * kite.width - kite.width / 2, 0,
                    4 * move_duration)

  );
  SET(

      KITE_TIP_ROTATION_ADD(ID(zero), 90, LEFT_TIP, rotation_duration),
      KITE_TIP_ROTATION_ADD(ID(one), 90, LEFT_TIP, rotation_duration)

  );
  SET(

      KITE_MOVE_ADD(ID(zero), 0, -2.5 * kite.width, move_duration),
      KITE_MOVE_ADD(ID(one), 0, 2.5 * kite.width, move_duration)

  );

  SET(

      // TODO: Do the tip rotation instead, when it is working.
      // Turn in adaption
      // KITE_MOVE_ADD(ID(zero), -kite.width, 0, rotation_duration),
      // KITE_MOVE_ADD(ID(one), kite.width, 0, rotation_duration),

      // KITE_TIP_ROTATION_ADD(ID(zero), 180, LEFT_TIP, rotation_duration),
      // KITE_TIP_ROTATION_ADD(ID(one), 180, LEFT_TIP, rotation_duration)

      KITE_MOVE_ADD(ID(zero), -kite.width / 2, 0, rotation_duration),
      KITE_MOVE_ADD(ID(one), kite.width / 2, 0, rotation_duration),

      KITE_ROTATION_ADD(ID(zero), 180, rotation_duration),
      KITE_ROTATION_ADD(ID(one), 180, rotation_duration)

  );

  SET(

      KITE_MOVE_ADD(ID(zero), 0, 2.5 * kite.width, move_duration),
      KITE_MOVE_ADD(ID(one), 0, -2.5 * kite.width, move_duration)

  );
}

void rollkiss_90_break(Env *env) {
  SET(

      KITE_ROTATION_ADD(ID(zero), 270, rotation_duration),
      KITE_ROTATION_ADD(ID(one), 270, rotation_duration),

      KITE_MOVE_ADD(ID(zero), -kite.height / 2, 0.5 * kite.width,
                    move_duration),
      KITE_MOVE_ADD(ID(one), kite.height / 2, -0.5 * kite.width, move_duration)

  );
}

void edges_turn_in(Env *env) {
  SET(

      KITE_MOVE_ADD(ID(zero), -5 * kite.width, 0, move_duration),
      KITE_MOVE_ADD(ID(one), 5 * kite.width, 0, move_duration)

  );

  SET(

      // TODO: Do the tip rotation instead, when it is working.
      // Turn in adaption
      // KITE_MOVE_ADD(ID(zero), -kite.width, 0, rotation_duration),
      // KITE_MOVE_ADD(ID(one), kite.width, 0, rotation_duration),

      // KITE_TIP_ROTATION_ADD(ID(zero), 180, RIGHT_TIP, rotation_duration),
      // KITE_TIP_ROTATION_ADD(ID(one), 180, RIGHT_TIP, rotation_duration)

      KITE_MOVE_ADD(ID(zero), 0, -kite.width / 2, rotation_duration),
      KITE_MOVE_ADD(ID(one), 0, kite.width / 2, rotation_duration),

      KITE_ROTATION_ADD(ID(zero), -180, rotation_duration),
      KITE_ROTATION_ADD(ID(one), -180, rotation_duration)

  );
  SET(

      KITE_MOVE_ADD(ID(zero), 5 * kite.width, 0, move_duration),
      KITE_MOVE_ADD(ID(one), -5 * kite.width, 0, move_duration)

  );
}

void kiss_reverse(Env *env) {
  SET(

      KITE_MOVE_ADD(ID(zero), kite.height / 2, 0, move_duration),
      KITE_MOVE_ADD(ID(one), -kite.height / 2, 0, move_duration)

  );

  SET(KITE_WAIT(wait_time));

  SET(

      KITE_MOVE_ADD(ID(zero), -2 * kite.width, 0, move_duration),
      KITE_MOVE_ADD(ID(one), 2 * kite.width, 0, move_duration)

  );
}

void tip_turn_down_revers(Env *env) {
  SET(

      KITE_TIP_ROTATION_ADD(ID(zero), -180, RIGHT_TIP, rotation_duration),
      KITE_TIP_ROTATION_ADD(ID(one), 180, LEFT_TIP, rotation_duration)

  );
  SET(

      KITE_MOVE_ADD(ID(zero), 2 * kite.width - kite.height, 0, move_duration),
      KITE_MOVE_ADD(ID(one), -2 * kite.width + kite.height, 0, move_duration)

  );
}

void slide_down_forward(Env *env) {
  SET(

      KITE_MOVE_ADD(ID(zero), 0, kite.width / 2, move_duration),
      KITE_MOVE_ADD(ID(one), 0, kite.width / 2, move_duration)

  );
  SET(

      KITE_MOVE_ADD(ID(zero), -3 * kite.width + kite.height, 0, move_duration),
      KITE_MOVE_ADD(ID(one), 3 * kite.width - kite.height, 0, move_duration)

  );
}

void tip_turn_to_moon_walk(Env *env) {
  SET(

      KITE_TIP_ROTATION_ADD(ID(zero), 90, LEFT_TIP, rotation_duration),
      KITE_TIP_ROTATION_ADD(ID(one), -90, RIGHT_TIP, rotation_duration)

  );
  // to face up
  SET(

      KITE_TIP_ROTATION_ADD(ID(zero), 180, LEFT_TIP, rotation_duration),
      KITE_TIP_ROTATION_ADD(ID(one), -180, RIGHT_TIP, rotation_duration)

  );
  // to face down
  SET(

      KITE_TIP_ROTATION_ADD(ID(zero), 180, RIGHT_TIP, rotation_duration),
      KITE_TIP_ROTATION_ADD(ID(one), -180, LEFT_TIP, rotation_duration)

  );
  // to face up
  SET(

      KITE_TIP_ROTATION_ADD(ID(zero), 180, LEFT_TIP, rotation_duration),
      KITE_TIP_ROTATION_ADD(ID(one), -180, RIGHT_TIP, rotation_duration)

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

      KITE_ROTATION_ADD(ID(zero), 45, rotation_duration),
      KITE_ROTATION_ADD(ID(one), -45, rotation_duration)

  );

  SET(

      KITE_MOVE_ADD(ID(zero), -2 * kite.width, -3 * kite.width, move_duration),
      KITE_MOVE_ADD(ID(one), 2 * kite.width, -3 * kite.width, move_duration)

  );

  SET(

      KITE_ROTATION_ADD(ID(zero), 90, rotation_duration),
      KITE_ROTATION_ADD(ID(one), -90, rotation_duration)

  );

  SET(

      KITE_MOVE_ADD(ID(zero), -2 * kite.width, 3 * kite.width, move_duration),
      KITE_MOVE_ADD(ID(one), 2 * kite.width, 3 * kite.width, move_duration)

  );

  SET(

      KITE_ROTATION_ADD(ID(zero), 135, rotation_duration),
      KITE_ROTATION_ADD(ID(one), -135, rotation_duration)

  );

  SET(KITE_WAIT(wait_time));

  SET(

      KITE_MOVE_ADD(ID(zero), 2 * kite.width, 0, move_duration),
      KITE_MOVE_ADD(ID(one), -2 * kite.width, 0, move_duration)

  );
}

void bicycle_in(Env *env) {
  SET(

      KITE_MOVE_ADD(ID(zero), 2 * kite.width + kite.width / 2, 0,
                    move_duration),
      KITE_MOVE_ADD(ID(one), -2 * kite.width - kite.width / 2, 0,
                    move_duration),

      KITE_ROTATION_ADD(ID(zero), -360, move_duration),
      KITE_ROTATION_ADD(ID(one), 360, move_duration)

  );

  SET(KITE_WAIT(wait_time));
}

void rollkiss_90_break_to_anti_split_position(Env *env) {
  SET(

      KITE_ROTATION_ADD(ID(zero), -270, move_duration),
      KITE_ROTATION_ADD(ID(one), -270, move_duration),

      KITE_MOVE_ADD(ID(zero), 0.5 * kite.width, -kite.height / 2,
                    move_duration),
      KITE_MOVE_ADD(ID(one), -0.5 * kite.width, kite.height / 2, move_duration)

  );

  SET(

      KITE_ROTATION_ADD(ID(zero), -90, move_duration),
      KITE_MOVE_ADD(ID(zero), -kite.width / 2 + kite.height,
                    -kite.width - kite.width / 2 + kite.height, move_duration),

      KITE_TIP_ROTATION_ADD(ID(one), -90, LEFT_TIP, move_duration)

  );
}

void inverse_bicycle_change(Env *env) {
  SET(KITE_WAIT(wait_time));

  SET(

      KITE_ROTATION_ADD(ID(zero), -90, move_duration),
      KITE_MOVE_ADD(ID(zero), -kite.height / 2 - kite.width / 2, 0,
                    move_duration),

      KITE_ROTATION_ADD(ID(one), -90, move_duration),
      KITE_MOVE_ADD(ID(one), kite.height / 2 + kite.width / 2,
                    -kite.width - kite.height, move_duration)

  );

  SET(

      KITE_ROTATION_ADD(ID(zero), -90, move_duration),
      KITE_MOVE_ADD(ID(zero), -kite.height / 2 + kite.width / 2, 0,
                    move_duration),

      KITE_ROTATION_ADD(ID(one), -90, move_duration),
      KITE_MOVE_ADD(ID(one), kite.height / 2 - kite.width / 2,
                    -kite.width + kite.height, move_duration)

  );
}

void forward_bicycle_out(Env *env) {
  SET(

      KITE_ROTATION_ADD(ID(zero), 315, move_duration),
      KITE_MOVE_ADD(ID(zero), 0, 2 * kite.width, move_duration),

      KITE_ROTATION_ADD(ID(one), 315, move_duration),
      KITE_MOVE_ADD(ID(one), 0, -2 * kite.width, move_duration)

  );
}

void corner_turn_out(Env *env, Kite_Ids ki) {
  SET(

      KITE_MOVE_ADD(ID(zero), -4 * kite.width - kite.width / 2,
                    -4 * kite.width - kite.width / 2, move_duration),
      KITE_MOVE_ADD(ID(one), 4 * kite.width + kite.width / 2,
                    4 * kite.width + kite.width / 2, move_duration)

  );

  SET(KITE_TIP_ROTATION_ADD(ki, 180, LEFT_TIP, rotation_duration));
  SET(KITE_WAIT(wait_time));
}

// The actual center move correct position.
// void to_center_slide_in(Env *env) {
//   SET(

//       KITE_MOVE_ADD(ID(zero), 4 * kite.width - kite.width / 2,
//                     4 * kite.width - kite.width / 2, move_duration),

//       KITE_MOVE_ADD(ID(one), -4 * kite.width + kite.width / 2,
//                     -4 * kite.width + kite.width / 2, move_duration)

//   );

//   SET(

//       KITE_MOVE_ADD(ID(zero), kite.width + kite.width / 2 + kite.height / 2,
//                     -kite.width - kite.width / 2 - kite.height / 2,
//                     move_duration),
//       KITE_MOVE_ADD(ID(one), -kite.width - kite.width / 2 - kite.height / 2,
//                     kite.width + kite.width / 2 + kite.height / 2,
//                     move_duration)

//   );
// }

// void pair_change_90(Env *env, Kite_Ids ki) {
//   tkbc_script_team_roll_two_diffrent_angle(
//       env, ki, 1.3, 405, 585, 225, 405, rotation_duration,
//       rotation_duration);
// }

void to_center_slide_in(Env *env) {
  SET(

      // KITE_MOVE_ADD(ID(zero), 4 * kite.width - kite.width / 2, 4 * kite.width
      // - kite.width / 2, move_duration), KITE_MOVE_ADD(ID(one), -4 *
      // kite.width + kite.width / 2, -4 * kite.width + kite.width / 2,
      // move_duration)

      KITE_MOVE_ADD(ID(zero), 4 * kite.width - kite.height,
                    4 * kite.width - kite.height, move_duration),
      KITE_MOVE_ADD(ID(one), -4 * kite.width + kite.height,
                    -4 * kite.width + kite.height, move_duration)

  );

  SET(

      KITE_MOVE_ADD(ID(zero), kite.width + kite.width / 2 + kite.height / 2,
                    -kite.width - kite.width / 2 - kite.height / 2,
                    move_duration),
      KITE_MOVE_ADD(ID(one), -kite.width - kite.width / 2 - kite.height / 2,
                    kite.width + kite.width / 2 + kite.height / 2,
                    move_duration)

  );
}

void pair_change_90(Env *env) {
  SET(

      KITE_TIP_ROTATION_ADD(ID(zero), 90, LEFT_TIP, rotation_duration),
      KITE_TIP_ROTATION_ADD(ID(one), 90, LEFT_TIP, rotation_duration)

  );
}

void choreo(Env *env, Kite_Ids ki) {
  kite = *env->vanilla_kite;
  bool ok = tkbc_configure_kites(env, ki,
                                 (Kite_Config){
                                     .kite_id = zero,
                                     .body_color = RED,
                                     .top_color = kite.top_color,
                                 },
                                 (Kite_Config){
                                     .kite_id = one,
                                     .body_color = BLUE,
                                     .top_color = kite.top_color,
                                 });
  assert(ok);
  (void)ok;

  tkbc_script_begin("My Immortal");
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

  inverse_bicycle_change(env);
  forward_bicycle_out(env);
  corner_turn_out(env, ki);

  to_center_slide_in(env);
  // pair_change_90(env);
  SET(KITE_WAIT(wait_time));
  tkbc_script_end();
}
