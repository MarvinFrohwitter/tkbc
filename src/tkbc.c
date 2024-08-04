#include <assert.h>
#include <complex.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "tkbc.h"

/**
 * @brief The function initializes a new Env.
 *
 * @return The new allocated Env.
 */
Env *tkbc_init_env(void) {
  Env *env = calloc(1, sizeof(*env));
  if (env == NULL) {
    fprintf(stderr, "ERROR: No more memory can be allocated.\n");
    return NULL;
  }
  env->kite_array = calloc(1, sizeof(*env->kite_array));
  if (env->kite_array == NULL) {
    fprintf(stderr, "ERROR: No more memory can be allocated.\n");
    return NULL;
  }
  env->frames = calloc(1, sizeof(*env->frames));
  if (env->frames == NULL) {
    fprintf(stderr, "ERROR: No more memory can be allocated.\n");
    return NULL;
  }

  env->frames->kite_frame_positions =
      calloc(1, sizeof(*env->frames->kite_frame_positions));
  if (env->frames->kite_frame_positions == NULL) {
    fprintf(stderr, "ERROR: No more memory can be allocated.\n");
    return NULL;
  }

  env->index_blocks = calloc(1, sizeof(*env->index_blocks));
  if (env->index_blocks == NULL) {
    fprintf(stderr, "ERROR: No more memory can be allocated.\n");
    return NULL;
  }

  env->block_frames = calloc(1, sizeof(*env->block_frames));
  if (env->block_frames == NULL) {
    fprintf(stderr, "ERROR: No more memory can be allocated.\n");
    return NULL;
  }

  env->scratch_buf_frames = calloc(1, sizeof(*env->scratch_buf_frames));
  if (env->scratch_buf_frames == NULL) {
    fprintf(stderr, "ERROR: No more memory can be allocated.\n");
    return NULL;
  }

  env->scratch_buf_frames->kite_frame_positions =
      calloc(1, sizeof(*env->scratch_buf_frames->kite_frame_positions));
  if (env->scratch_buf_frames->kite_frame_positions == NULL) {
    fprintf(stderr, "ERROR: No more memory can be allocated.\n");
    return NULL;
  }

  env->script_setup = true;
  env->window_width = GetScreenWidth();
  env->window_height = GetScreenHeight();
  env->script_interrupt = false;
  env->script_finished = false;
  env->global_block_index = 0;
  env->max_block_index = 0;
  env->attempts_block_index = 0;
  env->recording = false;
  env->rendering = false;
  env->pipe = 0;
  env->fps = TARGET_FPS;
  env->sound_file_name = NULL;

  float margin = 10;
  env->timeline_base.width = env->window_width / 2.0f;
  env->timeline_base.height = env->window_height / 42.0f;
  env->timeline_base.x = env->window_width / 4.0f;
  env->timeline_base.y =
      env->window_height - env->timeline_base.height - margin;

  env->timeline_front.width = 0;
  env->timeline_front.height = env->timeline_base.height;
  env->timeline_front.x = env->window_width / 4.0f;
  env->timeline_front.y =
      env->window_height - env->timeline_base.height - margin;

  env->timeline_segment_width = 0;
  env->timeline_segments_width = 0;
  env->timeline_segments = 0;
  env->timeline_hoverover = false;
  env->timeline_interaction = false;

  return env;
}

/**
 * @brief The function allocates the memory for a kite and it's corresponding
 * state and gives back the state structure.
 *
 * @return state The new allocated state.
 */
Kite_State *tkbc_init_kite(void) {
  Kite_State *state = calloc(1, sizeof(*state));
  if (state == NULL) {
    fprintf(stderr, "ERROR: No more memory can be allocated.\n");
    return NULL;
  }

  tkbc_set_kite_state_defaults(state);
  state->kite = calloc(1, sizeof(*state->kite));
  if (state->kite == NULL) {
    fprintf(stderr, "ERROR: No more memory can be allocated.\n");
    return NULL;
  }
  tkbc_set_kite_defaults(state->kite, true);

  int viewport_padding = state->kite->width > state->kite->height
                             ? state->kite->width / 2
                             : state->kite->height;

  Vector2 start_pos = {.y = GetScreenHeight() - 2 * viewport_padding,
                       .x = state->kite->center.x};
  tkbc_center_rotation(state->kite, &start_pos, state->kite->center_rotation);
  return state;
}

/**
 * @brief The function frees the memory for the given env state.
 *
 * @param env The global state of the application.
 */
void tkbc_destroy_env(Env *env) {
  tkbc_destroy_kite_array(env);
  free(env->kite_array->elements);
  free(env->kite_array);

  free(env->index_blocks->elements);

  // The frames are just a mapped in version of block_frames no need to handle
  // them separately.
  for (size_t i = 0; i < env->block_frames->count; ++i) {
    tkbc_destroy_frames(&env->block_frames->elements[i]);
  }
  free(env->block_frames->elements);
  free(env->block_frames);

  tkbc_destroy_frames(env->scratch_buf_frames);
  free(env->scratch_buf_frames->kite_frame_positions);
  free(env->scratch_buf_frames->elements);
  free(env->scratch_buf_frames);
  free(env);
}

/**
 * @brief The function frees the memory for the given state.
 *
 * @param state The current state of a kite.
 */
void tkbc_destroy_kite(Kite_State *state) { free(state->kite); }

/**
 * @brief The function frees all the kites that are currently in the global
 * kite_array.
 *
 * @param env The global state of the application.
 */
void tkbc_destroy_kite_array(Env *env) {
  for (size_t i = 0; i < env->kite_array->count; ++i) {
    tkbc_destroy_kite(&env->kite_array->elements[i]);
  }
}

/**
 * @brief The function initializes the amount of kites that are provided in the
 * arguments and inserts them in the global kite_array. It also sets a different
 * color for each kite, rather than the default color.
 *
 * @param env The global state of the application.
 */
void tkbc_kite_array_generate(Env *env, size_t kite_count) {

  Color color_array[] = {BLUE, GREEN, PURPLE, RED, TEAL};

  for (size_t i = 0; i < kite_count; ++i) {
    tkbc_dap(env->kite_array, *tkbc_init_kite());
    env->kite_array->elements[i].id = i;
    env->kite_array->elements[i].kite->body_color =
        color_array[i % ARRAY_LENGTH(color_array)];
  }

  tkbc_kite_array_start_position(env);
}

/**
 * @brief The function computes the spaced start positions for the kite_array
 * and set the kites back to the default state values.
 *
 * @param env The global state of the application.
 */
void tkbc_kite_array_start_position(Env *env) {

  assert(env->kite_array->count != 0);

  size_t kites_count = env->kite_array->count;
  float kite_width = env->kite_array->elements[0].kite->width;
  float kite_heigt = env->kite_array->elements[0].kite->height;
  int viewport_padding = kite_width > kite_heigt ? kite_width / 2 : kite_heigt;

  Vector2 start_pos = {.x = env->window_width / 2.0f -
                            kites_count * kite_width + kite_width / 2.0f,
                       .y = env->window_height - 2 * viewport_padding};

  for (size_t i = 0; i < kites_count; ++i) {
    tkbc_set_kite_state_defaults(&env->kite_array->elements[i]);
    tkbc_set_kite_defaults(env->kite_array->elements[i].kite, false);
    // TODO: Consider calling the internal center_rotation function that is
    // called in the kite_defaults. Just pass the start_pos there.
    tkbc_center_rotation(env->kite_array->elements[i].kite, &start_pos, 0);
    start_pos.x += 2 * kite_width;
  }
}

/**
 * @brief The function sets all the internal default of the kite and computes
 * the internal corner points of the kite.
 *
 * @param kite The kite that is going to be modified.
 * @param is_generated Chooses the information if the function is called by a
 * generator or as a reset of the values.
 */
void tkbc_set_kite_defaults(Kite *kite, bool is_generated) {
  if (is_generated) {
    kite->center.x = 0;
    kite->center.y = 0;

    kite->center.x = GetScreenWidth() / 2.0f;
    kite->center.y = GetScreenHeight() / 2.0f;
  }

  kite->fly_speed = 30;
  kite->turn_speed = 30;

  if (is_generated) {
    kite->body_color = TEAL;
  }
  kite->overlap = 8.0f;
  kite->inner_space = 20.f;

  kite->top_color = DARKGRAY;
  kite->spread = 0.2f;

  kite->width = 20.0f;
  kite->height = 0.0f;
  kite->scale = 4.0f;
  kite->center_rotation = 0;

  kite->overlap *= kite->scale;
  kite->inner_space *= kite->scale;
  kite->spread *= kite->scale;
  kite->width *= kite->scale * 2;

  tkbc_center_rotation(kite, NULL, kite->center_rotation);

  kite->height = fabsf(kite->left.v1.y - kite->left.v2.y);

  kite->old_center = kite->center;
  kite->old_angle = kite->center_rotation;
  kite->angle_sum = 0;
  kite->remaining_angle = 0;
  kite->segment_size = 0;
}

/**
 * @brief The function sets all the default settings for a kite.
 *
 * @param state The state for which the values will be changed to defaults.
 */
void tkbc_set_kite_state_defaults(Kite_State *state) {

  state->id = 0;
  state->kite_input_handler_active = false;
  state->fly_velocity = 10;
  state->turn_velocity = 10;
  state->iscenter = false;
  state->fixed = true;
  state->interrupt_movement = false;
  state->interrupt_smoothness = false;
}

// ===========================================================================
// ========================== KITE POSITION ==================================
// ===========================================================================

/**
 * @brief The function computes all the internal points for the kite and it's
 * new position as well as the angle.
 *
 * @param kite The kite that is going to be modified.
 * @param position The new position for the kite at the center of the leading
 * edge or NULL for internal center position of the kite structure.
 * @param center_deg_rotation The rotation of the kite.
 */
void tkbc_center_rotation(Kite *kite, Vector2 *position,
                          float center_deg_rotation) {
  Vector2 pos = {0};
  if (position != NULL) {
    pos.x = position->x;
    pos.y = position->y;
  } else {
    pos.x = kite->center.x;
    pos.y = kite->center.y;
  }

  kite->center.x = pos.x;
  kite->center.y = pos.y;

  kite->center_rotation = center_deg_rotation;
  float cw = kite->width;
  float is = kite->inner_space;
  float o = kite->overlap;
  float_t length = (kite->width / 2.f + kite->spread);
  length = floorf(length);

  // The difference between the angle 0 and the default downward interpolation
  float angle = 42;
  float bl_angle = (PI * (360 - (90 - angle)) / 180);
  float br_angle = (PI * (360 + (90 - angle)) / 180);
  float phi = (PI * (kite->center_rotation) / 180);

  // TODO: check for floorf as in the rotation function
  // LEFT Triangle
  // Correct
  kite->left.v1.x = pos.x - ceilf(crealf((cw / 2.f) * cexpf(I * phi)));
  kite->left.v1.y = pos.y + floorf(cimagf((cw / 2.f) * cexpf(I * phi)));
  kite->left.v2.x = pos.x - ceilf(crealf(is * cexpf(I * (phi - bl_angle))));
  kite->left.v2.y = pos.y + floorf(cimagf(is * cexpf(I * (phi - bl_angle))));
  kite->left.v3.x = pos.x + ceilf(crealf(o * cexpf(I * phi)));
  kite->left.v3.y = pos.y - floorf(cimagf(o * cexpf(I * phi)));

  // RIGHT Triangle
  // Correct
  kite->right.v1.x = pos.x - ceilf(crealf(o * cexpf(I * phi)));
  kite->right.v1.y = pos.y + floorf(cimagf(o * cexpf(I * phi)));
  kite->right.v2.x = pos.x + ceilf(crealf(is * cexpf(I * (phi - br_angle))));
  kite->right.v2.y = pos.y - floorf(cimagf(is * cexpf(I * (phi - br_angle))));
  kite->right.v3.x = pos.x + ceilf(crealf((cw / 2.f) * cexpf(I * phi)));
  kite->right.v3.y = pos.y - floorf(cimagf((cw / 2.f) * cexpf(I * phi)));

  // Just an random suitable height and width that fits the scaling and
  // spread. k->rec.height = 2 * PI * PI * logf(k->spread * k->spread);
  // k->rec.height = 2 * PI * k->spread;
  kite->rec.height = 2 * PI * logf(kite->scale);
  kite->rec.width = 2 * length;
  kite->rec.x = pos.x - ceilf(crealf(length * cexpf(I * phi)));
  kite->rec.y = pos.y + floorf(cimagf(length * cexpf(I * phi)));
}

/**
 * @brief The function computes the new position of the kite and its
 * corresponding structure values with a tip rotation.
 *
 * @param kite The kite that is going to be modified.
 * @param position The new position for the kite at the center of the leading
 * edge or NULL for internal center position of the kite structure.
 * @param tip_deg_rotation The angle in degrees.
 * angle.
 * @param tip The tip chosen left or right around where the kite is turning.
 */
void tkbc_tip_rotation(Kite *kite, Vector2 *position, float tip_deg_rotation,
                       TIP tip) {

  if (position != NULL) {
    tkbc_center_rotation(kite, position, kite->center_rotation);
  }

  float_t length = (kite->width / 2.f + kite->spread);
  length = floorf(length);
  float phi = (PI * (tip_deg_rotation) / 180);

  Vector2 pos = {0};
  switch (tip) {
  case LEFT_TIP: {

    // Move the rotation position to the left tip
    pos.x = kite->left.v1.x;
    pos.y = kite->left.v1.y;
    // Then rotate
    pos.x += ceilf(crealf((length)*cexpf(I * phi)));
    pos.y -= floorf(cimagf((length)*cexpf(I * phi)));
  } break;
  case RIGHT_TIP: {

    // Move the rotation position to the right tip
    pos.x = kite->right.v3.x;
    pos.y = kite->right.v3.y;
    // Then rotate
    pos.x -= ceilf(crealf((length)*cexpf(I * phi)));
    pos.y += floorf(cimagf((length)*cexpf(I * phi)));

  } break;
  default:
    assert(0 && "The chosen TIP is not valid!");
    break;
  }

  // Just compute a center rotation instead at the new found position
  tkbc_center_rotation(kite, &pos, tip_deg_rotation);
}

/**
 * @brief The function computes the rotation as a ball below and above.
 *
 * @param kite The kite for which the calculation will happen.
 * @param position The new position for the kite at the center of the leading
 * edge or NULL for internal center position of the kite structure.
 * @param deg_rotation The kite rotation in degrees.
 * @param tip The tip that will be chosen to calculate the beginning of the
 * circle.
 * @param below The area where the rotation will happen.
 */
void tkbc_circle_rotation(Kite *kite, Vector2 *position, float deg_rotation,
                          TIP tip, bool below) {

  if (position != NULL) {
    tkbc_center_rotation(kite, position, kite->center_rotation);
  }

  Vector2 pos = {0};
  pos = kite->center;

  // TODO: Change back to full circle size
  // float_t length = k->height;
  float_t length = kite->height / 2;
  float phi = (PI * (deg_rotation) / 180);
  float center_angle = 0;
  if (below) {
    center_angle = (PI * (360 - 270) / 180);
  } else {
    center_angle = (PI * (360 - 90) / 180);
  }

  // center rotation point;
  pos.x += ceilf(crealf((length)*cexpf(I * center_angle)));
  pos.y += floorf(cimagf((length)*cexpf(I * center_angle)));

  switch (tip) {
  case LEFT_TIP: {

    // With out it just flies a circle
    // pos->x -= ceilf(length);

    // Move the rotation position to the left tip
    pos.x = kite->left.v1.x;
    pos.y = kite->left.v1.y;
    // Then rotate
    pos.x += ceilf(crealf((length)*cexpf(I * phi)));
    pos.y -= floorf(cimagf((length)*cexpf(I * phi)));
  } break;
  case RIGHT_TIP: {

    // Move the rotation position to the right tip
    pos.x = kite->right.v3.x;
    pos.y = kite->right.v3.y;
    // Then rotate
    pos.x -= ceilf(crealf((length)*cexpf(I * phi)));
    pos.y += floorf(cimagf((length)*cexpf(I * phi)));

  } break;
  default:
    assert(0 && "The chosen TIP is not valid!");
  }
}

// ===========================================================================
// ========================== KITE DISPLAY ===================================
// ===========================================================================

/**
 * @brief The function draws all the components of the kite.
 *
 * @param kite The kite that is going to be modified.
 */
void tkbc_draw_kite(Kite *kite) {
  Vector2 origin = {0};

  // Draw a color-filled triangle (vertex in counter-clockwise order!)
  DrawTriangle(kite->left.v1, kite->left.v2, kite->left.v3, kite->body_color);
  DrawTriangle(kite->right.v1, kite->right.v2, kite->right.v3,
               kite->body_color);
  DrawRectanglePro(kite->rec, origin, -kite->center_rotation, kite->top_color);
}

/**
 * @brief The function draws every kite with its corresponding position on the
 * canvas.
 *
 * @param env The global state of the application.
 */
void tkbc_draw_kite_array(Env *env) {
  for (size_t i = 0; i < env->kite_array->count; ++i) {
    tkbc_draw_kite(env->kite_array->elements[i].kite);
  }
}
