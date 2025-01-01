#include <assert.h>
#include <errno.h>
#include <math.h>
#include <raylib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../global/tkbc-utils.h"
#include "tkbc-parser.h"
#include "tkbc-script-handler.h"
#include "tkbc.h"

/**
 * @brief The function initializes a new Env.
 *
 * @return The new allocated Env.
 */
Env *tkbc_init_env(void) {
  Env *env = calloc(1, sizeof(*env));
  if (env == NULL) {
    tkbc_fprintf(stderr, "ERROR", "No more memory can be allocated.\n");
    return NULL;
  }
  env->kite_array = calloc(1, sizeof(*env->kite_array));
  if (env->kite_array == NULL) {
    tkbc_fprintf(stderr, "ERROR", "No more memory can be allocated.\n");
    return NULL;
  }
  env->frames = calloc(1, sizeof(*env->frames));
  if (env->frames == NULL) {
    tkbc_fprintf(stderr, "ERROR", "No more memory can be allocated.\n");
    return NULL;
  }

  env->frames->kite_frame_positions =
      calloc(1, sizeof(*env->frames->kite_frame_positions));
  if (env->frames->kite_frame_positions == NULL) {
    tkbc_fprintf(stderr, "ERROR", "No more memory can be allocated.\n");
    return NULL;
  }

  env->block_frame = calloc(1, sizeof(*env->block_frame));
  if (env->block_frame == NULL) {
    tkbc_fprintf(stderr, "ERROR", "No more memory can be allocated.\n");
    return NULL;
  }

  env->block_frames = calloc(1, sizeof(*env->block_frames));
  if (env->block_frames == NULL) {
    tkbc_fprintf(stderr, "ERROR", "No more memory can be allocated.\n");
    return NULL;
  }

  env->scratch_buf_frames = calloc(1, sizeof(*env->scratch_buf_frames));
  if (env->scratch_buf_frames == NULL) {
    tkbc_fprintf(stderr, "ERROR", "No more memory can be allocated.\n");
    return NULL;
  }

  env->scratch_buf_block_frame =
      calloc(1, sizeof(*env->scratch_buf_block_frame));
  if (env->scratch_buf_block_frame == NULL) {
    tkbc_fprintf(stderr, "ERROR", "No more memory can be allocated.\n");
    return NULL;
  }

  env->scratch_buf_frames->kite_frame_positions =
      calloc(1, sizeof(*env->scratch_buf_frames->kite_frame_positions));
  if (env->scratch_buf_frames->kite_frame_positions == NULL) {
    tkbc_fprintf(stderr, "ERROR", "No more memory can be allocated.\n");
    return NULL;
  }

  env->kite_id_counter = 0;
  env->script_setup = true;
  env->window_width = tkbc_get_screen_width();
  env->window_height = tkbc_get_screen_height();
  env->script_interrupt = false;
  env->script_finished = true;
  env->script_counter = 0;
  env->recording = false;
  env->rendering = false;
  env->pipe = 0;
  env->fps = TARGET_FPS;
  env->sound_file_name = NULL;
  env->script_file_name = NULL;

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
 * @brief The function allocates the memory for a kite and its corresponding
 * state and gives back the new initialized state structure.
 *
 * @return state The new allocated state.
 */
Kite_State *tkbc_init_kite(void) {
  Kite_State *state = calloc(1, sizeof(*state));
  if (state == NULL) {
    tkbc_fprintf(stderr, "ERROR", "No more memory can be allocated.\n");
    return NULL;
  }

  tkbc_set_kite_state_defaults(state);
  state->kite = calloc(1, sizeof(*state->kite));
  if (state->kite == NULL) {
    tkbc_fprintf(stderr, "ERROR", "No more memory can be allocated.\n");
    return NULL;
  }
  tkbc_set_kite_defaults(state->kite, true);

  int viewport_padding = state->kite->width > state->kite->height
                             ? state->kite->width / 2
                             : state->kite->height;

  Vector2 start_pos = {.y = tkbc_get_screen_height() - 2 * viewport_padding,
                       .x = state->kite->center.x};
  tkbc_center_rotation(state->kite, &start_pos, state->kite->angle);
  return state;
}

/**
 * @brief The function frees the memory for the given env state.
 *
 * @param env The global state of the application.
 */
void tkbc_destroy_env(Env *env) {

  if (env->sound_file_name != NULL) {
    free(env->sound_file_name);
  }
  if (env->script_file_name != NULL) {
    free(env->script_file_name);
  }
  tkbc_destroy_kite_array(env->kite_array);
  free(env->kite_array->elements);
  free(env->kite_array);

  for (size_t j = 0; j < env->block_frames->count; ++j) {
    // The frames are just a mapped in version of block_frame no need to handle
    // them separately.
    for (size_t i = 0; i < env->block_frames->elements[j].count; ++i) {
      tkbc_destroy_frames(&env->block_frames->elements[j].elements[i]);
    }
    free(env->block_frames->elements[j].elements);
  }

  free(env->block_frames->elements);
  free(env->block_frames);

  for (size_t i = 0; i < env->scratch_buf_block_frame->count; ++i) {
    tkbc_destroy_frames(&env->scratch_buf_block_frame->elements[i]);
  }
  free(env->scratch_buf_block_frame->elements);
  free(env->scratch_buf_block_frame);

  tkbc_destroy_frames(env->scratch_buf_frames);
  free(env->scratch_buf_frames->kite_frame_positions);
  free(env->scratch_buf_frames->elements);
  free(env->scratch_buf_frames);
  free(env);
}

/**
 * @brief The function frees the memory for the given kite state.
 *
 * @param state The current state of a kite.
 */
void tkbc_destroy_kite(Kite_State *state) { free(state->kite); }

/**
 * @brief The function frees all the kites that are registered in the given
 * kite_array.
 *
 * @param kite_states The given kite array.
 */
void tkbc_destroy_kite_array(Kite_States *kite_states) {
  for (size_t i = 0; i < kite_states->count; ++i) {
    tkbc_destroy_kite(&kite_states->elements[i]);
  }
}

/**
 * @brief The function computes the spaced start positions for the kite_array
 * and set the kites back to the default state values.
 *
 * @param kite_states The given kite array.
 * @param window_width The width of the window.
 * @param window_height The height of the window.
 */
void tkbc_kite_array_start_position(Kite_States *kite_states,
                                    size_t window_width, size_t window_height) {

  assert(kite_states->count > 0);

  float kite_width = kite_states->elements[0].kite->width;
  float kite_height = kite_states->elements[0].kite->height;
  float viewport_padding =
      kite_width > kite_height ? kite_width / 2 : kite_height;

  Vector2 start_pos = {.x = window_width / 2.0f -
                            kite_states->count * kite_width + kite_width / 2.0f,
                       .y = window_height - 2 * viewport_padding};

  for (size_t i = 0; i < kite_states->count; ++i) {
    tkbc_set_kite_state_defaults(&kite_states->elements[i]);
    tkbc_set_kite_defaults(kite_states->elements[i].kite, false);
    tkbc_center_rotation(kite_states->elements[i].kite, &start_pos, 0);
    start_pos.x += 2 * kite_width;
  }
}

/**
 * @brief The function handles the drag and dropped files.
 *
 * @param env The global state of the application.
 */
void tkbc_file_handler(Env *env) {
  bool issound = false;

  if (IsFileDropped()) {
    FilePathList file_path_list = LoadDroppedFiles();
    if (file_path_list.count == 0) {
      return;
    }
    char *file_path;
    for (size_t i = 0; i < file_path_list.count && i < 1; ++i) {
      file_path = file_path_list.paths[i];
      tkbc_fprintf(stderr, "INFO", "FILE: PATH: %s\n", file_path);

      if (strstr(file_path, ".kite")) {
        if (env->script_file_name != NULL) {
          free(env->script_file_name);
        }
        env->script_file_name = strdup(file_path);
        if (env->script_file_name == NULL) {
          tkbc_fprintf(stderr, "ERROR",
                       "The allocation has failed in: %s: %d: %s\n", __FILE__,
                       __LINE__, strerror(errno));
        }
        tkbc_script_parser(env);
      } else {
        if (IsSoundValid(env->sound)) {
          StopSound(env->sound);
          UnloadSound(env->sound);
        }
        issound = true;
        env->sound = LoadSound(file_path);
      }
    }
    if (issound) {
      // Checks drag and dropped audio files.
      if (env->sound_file_name != NULL) {
        free(env->sound_file_name);
      }
      env->sound_file_name = strdup(file_path);
      if (env->sound_file_name == NULL) {
        tkbc_fprintf(stderr, "ERROR",
                     "The allocation has failed in: %s: %d: %s\n", __FILE__,
                     __LINE__, strerror(errno));
      }
    }

    UnloadDroppedFiles(file_path_list);
  }
}

/**
 * @brief The function sets all the internal defaults of the kite and computes
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

    kite->center.x = tkbc_get_screen_width() / 2.0f;
    kite->center.y = tkbc_get_screen_height() / 2.0f;
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
  kite->angle = 0;

  kite->overlap *= kite->scale;
  kite->inner_space *= kite->scale;
  kite->spread *= kite->scale;
  kite->width *= kite->scale * 2;

  tkbc_center_rotation(kite, NULL, kite->angle);

  // The computation is correct because of the previous given angle = 0.
  kite->height = fabsf(kite->left.v1.y - kite->left.v2.y);

  kite->old_center = kite->center;
  kite->old_angle = kite->angle;
}

/**
 * @brief The function sets all the default settings for a kite.
 *
 * @param state The kite state for which the values will be changed to defaults.
 */
void tkbc_set_kite_state_defaults(Kite_State *state) {

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
 * @brief The function computes all the internal points for the kite and its
 * new position as well as the angle. This can be used in terms of positioning
 * the kite and rotating it or just for updating the (internal) geometric
 * values that are responsible for the kite shape.
 *
 * @param kite The kite that is going to be modified.
 * @param position The new position for the kite at the center of the leading
 * edge or NULL for internal center position of the kite structure.
 * @param center_deg_rotation The rotation of the kite that is set to the given
 * rotation.
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

  kite->angle = center_deg_rotation;
  float cw = kite->width / 2.0f;
  float is = kite->inner_space;
  float o = kite->overlap;
  float_t length = cw + kite->spread;
  length = floorf(length);

  // The difference between the angle 0 and the default downward interpolation
  float angle = 42;
  float bl_angle = (PI * (360 - (90 - angle)) / 180);
  float br_angle = (PI * (360 + (90 - angle)) / 180);
  float phi = (PI * (kite->angle) / 180);

  float cosphi = cosf(phi);
  float sinphi = sinf(phi);

  // LEFT Triangle
  kite->left.v1.x = pos.x - cw * cosphi;
  kite->left.v1.y = pos.y + cw * sinphi;
  kite->left.v2.x = pos.x - is * cosf((phi - bl_angle));
  kite->left.v2.y = pos.y + is * sinf((phi - bl_angle));
  kite->left.v3.x = pos.x + o * cosphi;
  kite->left.v3.y = pos.y - o * sinphi;

  // RIGHT Triangle
  kite->right.v1.x = pos.x - o * cosphi;
  kite->right.v1.y = pos.y + o * sinphi;
  kite->right.v2.x = pos.x + is * cosf((phi - br_angle));
  kite->right.v2.y = pos.y - is * sinf((phi - br_angle));
  kite->right.v3.x = pos.x + cw * cosphi;
  kite->right.v3.y = pos.y - cw * sinphi;

  // Just an random suitable height and width that fits the scaling and
  // spread. k->rec.height = 2 * PI * PI * logf(k->spread * k->spread);
  kite->rec.height = 3 * PI * kite->spread;
  // kite->rec.height = 2 * PI * logf(kite->scale);
  kite->rec.width = 2 * length;
  kite->rec.x = pos.x - length * cosphi;
  kite->rec.y = pos.y + length * sinphi;
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
 * @param tip The tip chosen, left or right, where the kite is turning around.
 */
void tkbc_tip_rotation(Kite *kite, Vector2 *position, float tip_deg_rotation,
                       TIP tip) {

  if (position != NULL) {
    tkbc_center_rotation(kite, position, kite->angle);
  }

  float_t length = (kite->width / 2.f + kite->spread);
  length = floorf(length);
  float phi = (PI * (tip_deg_rotation) / 180);

  Vector2 pos = {0};
  switch (tip) {
  case LEFT_TIP: {

    // Move the rotation position to the left tip.
    pos.x = kite->left.v1.x;
    pos.y = kite->left.v1.y;
    // Then rotate.
    pos.x += length * cosf(phi);
    pos.y -= length * sinf(phi);

  } break;
  case RIGHT_TIP: {

    // Move the rotation position to the right tip.
    pos.x = kite->right.v3.x;
    pos.y = kite->right.v3.y;
    // Then rotate.
    pos.x -= length * cosf(phi);
    pos.y += length * sinf(phi);

  } break;
  default:
    assert(0 && "The chosen TIP is not valid!");
    break;
  }

  // Just compute a center rotation instead at the new shifted position.
  tkbc_center_rotation(kite, &pos, tip_deg_rotation);
}

/**
 * @brief The function computes the rotation as a ball below and above the
 * leading edge.
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
    tkbc_center_rotation(kite, position, kite->angle);
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
  pos.x += length * cosf(center_angle);
  pos.y += length * sinf(center_angle);

  switch (tip) {
  case LEFT_TIP: {

    // With out it just flies a circle
    // pos->x -= ceilf(length);

    // Move the rotation position to the left tip.
    pos.x = kite->left.v1.x;
    pos.y = kite->left.v1.y;
    // Then rotate
    pos.x += length * cosf(phi);
    pos.y -= length * sinf(phi);
  } break;
  case RIGHT_TIP: {

    // Move the rotation position to the right tip.
    pos.x = kite->right.v3.x;
    pos.y = kite->right.v3.y;
    // Then rotate
    pos.x -= length * cosf(phi);
    pos.y += length * sinf(phi);

  } break;
  default:
    assert(0 && "The chosen TIP is not valid!");
  }
}

// ===========================================================================
// ========================== KITE DISPLAY ===================================
// ===========================================================================

/**
 * @brief The function draws all the components of the given kite.
 *
 * @param kite The kite that is going to be drawn.
 */
void tkbc_draw_kite(Kite *kite) {
  Vector2 origin = {0};

  // Draw a color-filled triangle (vertex in counter-clockwise order!)
  DrawTriangle(kite->left.v1, kite->left.v2, kite->left.v3, kite->body_color);
  DrawTriangle(kite->right.v1, kite->right.v2, kite->right.v3,
               kite->body_color);
  DrawRectanglePro(kite->rec, origin, -kite->angle, kite->top_color);
}

/**
 * @brief The function draws every kite that is registered in given kite array
 * with its corresponding position on the screen.
 *
 * @param kite_states The given kite array.
 */
void tkbc_draw_kite_array(Kite_States *kite_states) {
  for (size_t i = 0; i < kite_states->count; ++i) {
    tkbc_draw_kite(kite_states->elements[i].kite);
  }
}
