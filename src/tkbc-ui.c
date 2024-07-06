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
  env->hover_over_timeline =
      CheckCollisionPointRec(mouse_pos, env->timeline_base);

  if (IsMouseButtonUp(MOUSE_BUTTON_LEFT)) {
    env->timeline_interaction = false;
  }

  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    env->timeline_interaction = true;
    env->timeline_front.width =
        mouse_pos.x >= env->timeline_base.x + env->timeline_base.width
            ? env->timeline_base.width
            : mouse_pos.x - env->timeline_base.x;
  }

  if (env->hover_over_timeline || env->timeline_interaction) {
    DrawRectangleRec(env->timeline_base, ColorBrightness(BLACK, 0.3));
    DrawRectangleRec(env->timeline_front, ColorBrightness(TEAL, 0.1));
  }
}
