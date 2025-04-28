#include <assert.h>
#include <errno.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../global/tkbc-types.h"
#include "../global/tkbc-utils.h"
#include "tkbc-keymaps.h"
#include "tkbc-parser.h"
#include "tkbc-script-handler.h"
#include "tkbc.h"

/**
 * @brief The function initializes a new Env.
 *
 * @return The new allocated Env.
 */
Env *tkbc_init_env(void) {
  Env *env = malloc(sizeof(*env));
  if (env == NULL) {
    tkbc_fprintf(stderr, "ERROR", "No more memory can be allocated.\n");
    return NULL;
  }
  // This is very important to zero init the complete struct.
  // Most of the computation relies on zero initialization.
  memset(env, 0, sizeof(*env));

  env->vanilla_kite = malloc(sizeof(*env->vanilla_kite));
  if (env->vanilla_kite == NULL) {
    tkbc_fprintf(stderr, "ERROR", "No more memory can be allocated.\n");
    return NULL;
  }
  memset(env->vanilla_kite, 0, sizeof(*env->vanilla_kite));

  env->kite_array = malloc(sizeof(*env->kite_array));
  if (env->kite_array == NULL) {
    tkbc_fprintf(stderr, "ERROR", "No more memory can be allocated.\n");
    return NULL;
  }
  memset(env->kite_array, 0, sizeof(*env->kite_array));

  env->block_frames = malloc(sizeof(*env->block_frames));
  if (env->block_frames == NULL) {
    tkbc_fprintf(stderr, "ERROR", "No more memory can be allocated.\n");
    return NULL;
  }
  memset(env->block_frames, 0, sizeof(*env->block_frames));

  env->keymaps = malloc(sizeof(*env->keymaps));
  if (env->keymaps == NULL) {
    tkbc_fprintf(stderr, "ERROR", "No more memory can be allocated.\n");
    return NULL;
  }
  memset(env->keymaps, 0, sizeof(*env->keymaps));

  tkbc_init_keymaps_defaults(env->keymaps);

  tkbc_set_kite_defaults(env->vanilla_kite, true);
  env->kite_id_counter = 0;
  env->script_setup = true;
  env->window_width = tkbc_get_screen_width();
  env->window_height = tkbc_get_screen_height();
  env->script_finished = true;
  env->fps = TARGET_FPS;

  env->box_height = 80;
  env->keymaps_interaction_rec_number = -1;

  env->color_picker_input_text = calloc(10, sizeof(char));
  if (env->color_picker_input_text == NULL) {
    tkbc_fprintf(stderr, "ERROR", "No more memory can be allocated.\n");
    return NULL;
  }

  env->max_favorite_colors = 4;
  for (size_t i = 0; i < env->max_favorite_colors; i++) {
    tkbc_dap(&env->favorite_colors, BLANK);
  }

  env->last_selected_color = GetColor(0x008080FF);
  env->color_picker_input_text[0] = '#';

  return env;
}

/**
 * @brief The function allocates the memory for a kite and its corresponding
 * state and gives back the new initialized state structure.
 *
 * @return state The new allocated state.
 */
Kite_State *tkbc_init_kite(void) {
  Kite_State *state = malloc(sizeof(*state));
  if (state == NULL) {
    tkbc_fprintf(stderr, "ERROR", "No more memory can be allocated.\n");
    return NULL;
  }
  memset(state, 0, sizeof(*state));

  state->kite = malloc(sizeof(*state->kite));
  if (state->kite == NULL) {
    tkbc_fprintf(stderr, "ERROR", "No more memory can be allocated.\n");
    return NULL;
  }
  memset(state->kite, 0, sizeof(*state->kite));

  tkbc_set_kite_state_defaults(state);
  tkbc_set_kite_defaults(state->kite, true);

  int viewport_padding = state->kite->width > state->kite->height
                             ? state->kite->width / 2
                             : state->kite->height;

  Vector2 start_pos = {.y = tkbc_get_screen_height() - 2 * viewport_padding,
                       .x = state->kite->center.x};
  tkbc_kite_update_position(state->kite, &start_pos);
  return state;
}

/**
 * @brief The function frees the memory for the given env state.
 *
 * @param env The global state of the application.
 */
void tkbc_destroy_env(Env *env) {

  if (env->keymaps->elements != NULL) {
    free(env->keymaps->elements);
    env->keymaps->elements = NULL;
  }
  if (env->keymaps != NULL) {
    free(env->keymaps);
    env->keymaps = NULL;
  }

  free(env->color_picker_input_text);
  free(env->favorite_colors.elements);
  free(env->sound_file_name);
  free(env->script_file_name);
  free(env->vanilla_kite);
  env->vanilla_kite = NULL;
  tkbc_destroy_kite_array(env->kite_array);

  for (size_t j = 0; j < env->block_frames->count; ++j) {
    // The frames are just a mapped in version of block_frame no need to handle
    // them separately.
    for (size_t i = 0; i < env->block_frames->elements[j].count; ++i) {
      tkbc_destroy_frames_internal_data(
          &env->block_frames->elements[j].elements[i]);
    }
  }

  free(env->block_frames->elements);
  env->block_frames->elements = NULL;
  free(env->block_frames);
  env->block_frames = NULL;

  for (size_t i = 0; i < env->scratch_buf_block_frame.count; ++i) {
    tkbc_destroy_frames_internal_data(
        &env->scratch_buf_block_frame.elements[i]);
  }

  tkbc_destroy_frames_internal_data(&env->scratch_buf_frames);
  free(env);
  env = NULL;
}

/**
 * @brief The function frees the memory for the given kite state.
 *
 * @param state The current state of a kite.
 */
void tkbc_destroy_kite(Kite_State *state) {
  free(state->kite);
  state->kite = NULL;
  free(state);
  state = NULL;
}

/**
 * @brief The function frees all the kites that are registered in the given
 * kite_array.
 *
 * @param kite_states The given kite array.
 */
void tkbc_destroy_kite_array(Kite_States *kite_states) {
  for (size_t i = 0; i < kite_states->count; ++i) {
    free(kite_states->elements[i].kite);
    kite_states->elements[i].kite = NULL;
  }
  free(kite_states->elements);
  kite_states->elements = NULL;
  free(kite_states);
  kite_states = NULL;
}

/**
 * @brief The function can be used to remove a kite from the given list by the
 * given kite_id.
 *
 * @param kite_array The list of kite Kite_State that should may contain the
 * kite.
 * @param kite_id The id of the kite that should be removed.
 * @return True if the kite is removed successfully, otherwise false.
 */
bool tkbc_remove_kite_from_list(Kite_States *kite_array, size_t kite_id) {
  if (kite_array == NULL) {
    return false;
  }
  for (size_t i = 0; i < kite_array->count; ++i) {
    if (kite_array->elements[i].kite_id == kite_id) {
      Kite_State ks_temp = kite_array->elements[i];
      kite_array->elements[i] = kite_array->elements[kite_array->count - 1];
      kite_array->elements[kite_array->count - 1] = ks_temp;
      free(kite_array->elements[kite_array->count - 1].kite);
      kite_array->elements[kite_array->count - 1].kite = NULL;
      kite_array->count -= 1;
      return true;
    }
  }
  return false;
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
          env->script_file_name = NULL;
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
        env->sound_file_name = NULL;
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
  kite->scale = 3.7f;
  kite->angle = 0;

  kite->overlap *= kite->scale;
  kite->inner_space *= kite->scale;
  kite->spread *= kite->scale;
  kite->width *= kite->scale * 2;

  tkbc_kite_update_internal(kite);

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

  state->is_active = true;
  state->is_kite_input_handler_active = false;
  state->fly_velocity = 10;
  state->turn_velocity = 10;
  state->is_center_rotation = false;
  state->is_fixed_rotation = true;
  state->interrupt_movement = false;
  state->interrupt_smoothness = false;
  state->is_mouse_control = true;
}

// ===========================================================================
// ========================== KITE POSITION ==================================
// ===========================================================================

/**
 * @brief The function updates all the internal geometric values according to
 * the current set position and angle. This can be used to set the internal
 * position and angle and then update the rest of the geometric values that are
 * responsible for the kite shape.
 *
 * @param kite The kite that is going to be modified.
 */
void tkbc_kite_update_internal(Kite *kite) {
  tkbc_center_rotation(kite, NULL, kite->angle);
}

/**
 * @brief The function updates all the internal values according to the new
 position with the current angle. This can be used in terms of positioning
 * the kite and for updating the (internal) geometric
 * values that are responsible for the kite shape.
 *
 * @param kite The kite that is going to be modified.
 * @param position The new position for the kite at the center of the leading
 * edge or NULL for internal center position of the kite structure.
 */
void tkbc_kite_update_position(Kite *kite, Vector2 *position) {
  tkbc_center_rotation(kite, position, kite->angle);
}

/**
 * @brief The function updates all the internal values according to the new
 * angle with the current position. This can be used to update the angle and set
 * the values for the (internal) geometric shape of the kite.
 *
 * @param kite The kite that is going to be modified.
 * @param center_deg_rotation The rotation of the kite that is set to the given
 * rotation.
 */
void tkbc_kite_update_angle(Kite *kite, float center_deg_rotation) {
  tkbc_center_rotation(kite, NULL, center_deg_rotation);
}

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
    tkbc_kite_update_position(kite, position);
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
    if (kite_states->elements[i].is_active) {
      tkbc_draw_kite(kite_states->elements[i].kite);
    }
  }
}

/**
 * @brief The function can be used to move all registered kites from there
 * original position to the new one with the new screen size. It is useful if
 * the screen size changes, than the kites are possible out of the view space
 * and with this function they will be responsive inside the view port.
 *
 * @param env The global state of the application.
 */
void tkbc_update_kites_for_resize_window(Env *env) {
  // TODO: Decide what to do with the dynamic simulations. They are bound
  // sometimes to fixed sizes in the script and can't be moved that easy to the
  // new dimensions.
  if (!env->script_finished) {
    return;
  }

  size_t width = tkbc_get_screen_width();
  size_t height = tkbc_get_screen_height();

  if (env->window_width != width || env->window_height != height) {
    for (size_t i = 0; i < env->kite_array->count; ++i) {
      Kite *kite = env->kite_array->elements[i].kite;
      kite->center.x = kite->center.x / (float)env->window_width * (float)width;
      kite->center.y =
          kite->center.y / (float)env->window_height * (float)height;
      kite->old_center.x =
          kite->old_center.x / (float)env->window_width * (float)width;
      kite->old_center.y =
          kite->old_center.y / (float)env->window_height * (float)height;
      tkbc_kite_update_internal(kite);
    }

    env->window_width = width;
    env->window_height = height;
  }
}

/**
 * @brief The function returns one of the defined raylib colors.
 *
 * @return color definition from raylib.
 */
Color tkbc_get_random_color() {
  Color colors[] = {
      LIGHTGRAY, GRAY,  DARKGRAY,  YELLOW,  GOLD,   ORANGE,
      PINK,      RED,   MAROON,    GREEN,   LIME,   DARKGREEN,
      SKYBLUE,   BLUE,  DARKBLUE,  PURPLE,  VIOLET, DARKPURPLE,
      BEIGE,     BROWN, DARKBROWN, MAGENTA, TEAL,
  };

  return colors[rand() % ARRAY_LENGTH(colors)];
}
