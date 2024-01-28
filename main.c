#include "raylib.h"
#include <assert.h>
#include <complex.h>
#include <math.h>
#include <stdio.h>

#define TEAL                                                                   \
  CLITERAL(Color) { 0, 128, 128, 255 } // Teal
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

  float scale;
} Kite;

void draw_kite(Kite *kite, Vector2 *new__center_pos,
               float center_deg_rotation) {
  kite->center.x = new__center_pos->x;
  kite->center.y = new__center_pos->y;

  // TODO: This thing is not implemented for the triangles
  float cw = sqrtf((kite->right.v3.y - kite->left.v1.y) *
                       (kite->right.v3.y - kite->left.v1.y) +
                   (kite->right.v3.x - kite->left.v1.x) *
                       (kite->right.v3.x - kite->left.v1.x));

  float is = kite->inner_space;
  float o = kite->overlap;

  // if (center_deg_rotation == 0) {
  // if (e) {
  //   kite->left.v1.x = new__center_pos->x - (cw / 2.f);
  //   kite->left.v1.y = new__center_pos->y;
  //   kite->left.v2.x = new__center_pos->x - is;
  //   kite->left.v2.y = new__center_pos->y + is;
  //   kite->left.v3.x = new__center_pos->x + o;
  //   kite->left.v3.y = new__center_pos->y;

  //   kite->right.v1.x = new__center_pos->x - o;
  //   kite->right.v1.y = new__center_pos->y;
  //   kite->right.v2.x = new__center_pos->x + is;
  //   kite->right.v2.y = new__center_pos->y + is;
  //   kite->right.v3.x = new__center_pos->x + (cw / 2.f);
  //   kite->right.v3.y = new__center_pos->y;
  // }

  // Example
  // float r = new__center_pos->x + (cw / 2.f);
  // float phi = PI/4;
  // float x = r*cosf(phi);
  // float y = r*sinf(phi);

  //       |
  //       |
  //       |           x
  //       |         / |
  //       |        /  |
  //       |       /   |
  // ------|----------------
  //       |

  // The difference between the angle 0 and the default downward interpolation
  float bottom_right_interpolation = (PI * (360 - 42) / 180);
  float bottom_left_interpolation = bottom_right_interpolation;
  float phi = (PI * (center_deg_rotation) / 180);

  kite->left.v1.x = new__center_pos->x - crealf((cw / 2.f) * cexpf(I * phi));
  kite->left.v1.y = new__center_pos->y + cimagf((cw / 2.f) * cexpf(I * phi));

  kite->left.v2.x = new__center_pos->x -
                    crealf(is * cexpf(I * (phi - bottom_left_interpolation)));
  kite->left.v2.y = new__center_pos->y +
                    cimagf(is * cexpf(I * (phi - bottom_left_interpolation)));

  kite->left.v3.x = new__center_pos->x + crealf(o * cexpf(I * phi));
  kite->left.v3.y = new__center_pos->y + cimagf(o * cexpf(I * phi));

  kite->right.v1.x = new__center_pos->x - crealf(o * cexpf(I * phi));
  kite->right.v1.y = new__center_pos->y + cimagf(o * cexpf(I * phi));

  kite->right.v2.x = new__center_pos->x +
                     crealf(is * cexpf(I * (phi - bottom_right_interpolation)));
  kite->right.v2.y = new__center_pos->y +
                     cimagf(is * cexpf(I * (phi - bottom_right_interpolation)));

  kite->right.v3.x = new__center_pos->x + crealf((cw / 2.f) * cexpf(I * phi));
  kite->right.v3.y = new__center_pos->y + cimagf((cw / 2.f) * cexpf(I * phi));
  //  - + + + + - - + + + + -

  // Draw a color-filled triangle (vertex in counter-clockwise order!)
  DrawTriangle(kite->left.v1, kite->left.v2, kite->left.v3, kite->body_color);
  DrawTriangle(kite->right.v1, kite->right.v2, kite->right.v3,
               kite->body_color);

  kite->rec.height = kite->spread;
  kite->rec.width = cw + kite->spread;
  kite->rec.x = kite->left.v1.x - kite->spread / 2;
  kite->rec.y = kite->left.v1.y - kite->rec.height;

  Vector2 origin = {0};
  // Vector2 origin = {.x = kite->rec.width / 2.f, .y = 0};
  DrawRectanglePro(kite->rec, origin, -center_deg_rotation, kite->top_color);
}

void kite_init(Kite *kite, Vector2 center) {
  kite->center.x = center.x;
  kite->center.y = center.y;
  kite->body_color = TEAL;
  kite->top_color = DARKGRAY;
  kite->scale = 6.f;
  kite->overlap = 8.f;
  // kite->inner_space = 12.f;
  kite->inner_space = 20.f;
  kite->spread = 1.f;

  float cw = 20.f * kite->scale * 2;
  // kite->inner_space += kite->scale * kite->scale;
  kite->inner_space *= kite->scale;
  // kite->overlap += kite->scale * 4;
  kite->overlap *= kite->scale;
  kite->spread += kite->scale;

  kite->left.v1 = (Vector2){.x = center.x - (cw / 2.f), .y = center.y};
  kite->left.v2 = (Vector2){.x = center.x - kite->inner_space,
                            .y = center.y + kite->inner_space};
  kite->left.v3 = (Vector2){.x = center.x + kite->overlap, .y = center.y};

  kite->right.v1 = (Vector2){.x = center.x - kite->overlap, .y = center.y};
  kite->right.v2 = (Vector2){.x = center.x + kite->inner_space,
                             .y = center.y + kite->inner_space};
  kite->right.v3 = (Vector2){.x = center.x + (cw / 2.f), .y = center.y};

  cw = kite->right.v3.x - kite->left.v1.x;

  kite->rec.height = kite->spread;
  kite->rec.width = cw + kite->spread;
  kite->rec.x = kite->left.v1.x - kite->spread / 2;
  kite->rec.y = kite->left.v1.y - kite->rec.height;
}

void input_handler(Vector2 *pos) {

  float speed = 5;
  float velocity = 1;
  float friction = 100;

  if (IsKeyDown(KEY_J) || IsKeyDown(KEY_DOWN)) {
    pos->y += speed * velocity;
    if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT))
      pos->x += speed * velocity;
    if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT))
      pos->x -= speed * velocity;

  } else if (IsKeyDown(KEY_K) || IsKeyDown(KEY_UP)) {
    pos->y -= speed * velocity;
    if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT))
      pos->x += speed * velocity;
    if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT))
      pos->x -= speed * velocity;

  } else if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT)) {
    pos->x -= speed * velocity;
  } else if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT)) {
    pos->x += speed * velocity;
  }

  if (velocity <= speed / friction) {
    velocity *= velocity;
  }
}

int main(void) {

  const char *title = "TEAM KITE BALLETT CHOREOGRAPHER";

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, title);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  // SetTargetFPS(1);
  SetTargetFPS(60);

  Vector2 pos = {GetScreenWidth() / 2.f, GetScreenHeight() / 2.f};
  Vector2 pos1 = {GetScreenWidth() / 2.f, GetScreenHeight() / 2.f};

  Kite kite = {0};
  Kite kite1 = {0};
  kite_init(&kite, pos);
  kite_init(&kite1, pos1);
  kite1.body_color = RED;
  // TODO: Find a reasonable velocity

  printf("The POS:" VECTOR2_FMT "\n", Vector2_FMT_ARGS(pos));
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(SKYBLUE);

    input_handler(&pos);

    // draw_kite(&kite, &pos, 9);
    // draw_kite(&kite, &pos, 0);
    draw_kite(&kite, &pos, 0);
    draw_kite(&kite1, &pos1, 0);

    EndDrawing();
  };

  CloseWindow();
  return 0;
}
