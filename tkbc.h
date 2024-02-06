#ifndef TKBC_H_
#define TKBC_H_

#include "raylib.h"

#define TEAL                                                                   \
  CLITERAL(Color) { 0, 128, 128, 255 } // Teal #define WINDOW_SCALE 120
#define WINDOW_SCALE 120
#define SCREEN_WIDTH 16 * WINDOW_SCALE
#define SCREEN_HEIGHT 9 * WINDOW_SCALE

#define VECTOR2_FMT "(%f,%f)"
#define Vector2_FMT_ARGS(arg) (float)(arg).x, (float)(arg).y

typedef struct Triangle {
  Vector2 v1;
  Vector2 v2;
  Vector2 v3;
} Triangle;

typedef struct Kite {
  Vector2 center;

  Color body_color;
  Triangle left;
  Triangle right;
  float overlap;
  float inner_space;

  Color top_color;
  Rectangle rec;
  float spread;

  float width;
  float height;
  float scale;
  float center_rotation;

  float speed;
} Kite;

void kite_init(Kite *k);
void kite_update(Kite *k, Vector2 position, float center_deg_rotation);
void draw_kite(Kite *k, Vector2 position, float center_deg_rotation);
Vector2 input_handler(Kite *k);

#endif // TKBC_H_
