#include "tkbc-types.h"
#include "tkbc.h"
#include <raylib.h>

void tkbc_draw_ui(Env *env) {

  env->window_height = GetScreenHeight();
  env->window_width = GetScreenWidth();

  tkbc_ui_timeline(env);

  if (env->rendering) {
    DrawCircleV(CLITERAL(Vector2){20, 20}, 10, RED);
  }
  DrawFPS(env->window_width / 2, 10);
}

void tkbc_ui_timeline(Env *env) {
  Vector2 mouse_pos = GetMousePosition();
  if (IsKeyUp(KEY_LEFT_CONTROL) && IsKeyUp(KEY_RIGHT_CONTROL)) {
    env->timeline_interaction = false;
  }

  env->hover_over_timeline =
      CheckCollisionPointRec(mouse_pos, env->timeline_base);

  if (env->hover_over_timeline || env->timeline_interaction) {
    if (IsKeyDown(KEY_LEFT_CONTROL) ||IsKeyDown(KEY_RIGHT_CONTROL)) {
      env->timeline_interaction = true;
      env->timeline_front.width =
          mouse_pos.x >= env->timeline_base.x + env->timeline_base.width
              ? env->timeline_base.width
              : mouse_pos.x - env->timeline_base.x;
    }

    DrawRectangleRec(env->timeline_base, BLACK);
    DrawRectangleRec(env->timeline_front, RED);
    // DrawRectangleRec(env->timeline_base, ColorBrightness(BLACK, 0.3));
    // DrawRectangleRec(env->timeline_front, ColorBrightness(TEAL, 0.1));
  }
}
