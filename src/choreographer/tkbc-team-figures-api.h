#ifndef TKBC_TEAM_FIGURES_API_H_
#define TKBC_TEAM_FIGURES_API_H_

#include "../global/tkbc-types.h"

// ===========================================================================
// ========================== Script Team Figures ============================
// ===========================================================================

bool tkbc_script_team_line(Env *env, Kite_Ids kite_index_array,
                           Vector2 position, Vector2 offset, float h_padding,
                           float move_duration);
bool tkbc_script_team_grid(Env *env, Kite_Ids kite_index_array,
                           Vector2 position, Vector2 offset, float v_padding,
                           float h_padding, size_t rows, size_t columns,
                           float move_duration);

bool tkbc_script_team_ball(Env *env, Kite_Ids kite_index_array,
                           Vector2 position, Vector2 offset, float radius,
                           float move_duration, float rotation_duration);

bool tkbc_script_team_mountain(Env *env, Kite_Ids kite_index_array,
                               Vector2 position, Vector2 offset,
                               float v_padding, float h_padding,
                               float move_duration, float rotation_duration);

bool tkbc_script_team_valley(Env *env, Kite_Ids kite_index_array,
                             Vector2 position, Vector2 offset, float v_padding,
                             float h_padding, float move_duration,
                             float rotation_duration);

bool tkbc_script_team_arc(Env *env, Kite_Ids kite_index_array, Vector2 position,
                          Vector2 offset, float v_padding, float h_padding,
                          float angle, float move_duration,
                          float rotation_duration);
bool tkbc_script_team_mouth(Env *env, Kite_Ids kite_index_array,
                            Vector2 position, Vector2 offset, float v_padding,
                            float h_padding, float angle, float move_duration,
                            float rotation_duration);

void tkbc_script_team_box(Env *env, Kite_Ids kite_index_array,
                          DIRECTION direction, float angle, float box_size,
                          float move_duration, float rotation_duration);
void tkbc_script_team_box_left(Env *env, Kite_Ids kite_index_array,
                               float box_size, float move_duration,
                               float rotation_duration);
void tkbc_script_team_box_right(Env *env, Kite_Ids kite_index_array,
                                float box_size, float move_duration,
                                float rotation_duration);

bool tkbc_script_team_split_box_up(Env *env, Kite_Ids kite_index_array,
                                   ODD_EVEN odd_even, float box_size,
                                   float move_duration,
                                   float rotation_duration);

void tkbc_script_team_diamond(Env *env, Kite_Ids kite_index_array,
                             DIRECTION direction, float angle, float box_size,
                             float move_duration, float rotation_duration);
void tkbc_script_team_diamond_left(Env *env, Kite_Ids kite_index_array,
                                  float box_size, float move_duration,
                                  float rotation_duration);
void tkbc_script_team_diamond_right(Env *env, Kite_Ids kite_index_array,
                                   float box_size, float move_duration,
                                   float rotation_duration);

#endif // TKBC_TEAM_FIGURES_API_H_
