#ifndef TKBC_ASSET_HANDLER_H_
#define TKBC_ASSET_HANDLER_H_

#include "../../external/space/space.h"
#include "../global/tkbc-types.h"
#include "../global/tkbc-utils.h"
#include "raylib.h"
#include <assert.h>
#include <stdio.h>

extern Space kite_images_space;
extern Kite_Images kite_images;

extern Space kite_textures_space;
extern Kite_Textures kite_textures;

#include "../../assets/combind_assets.h"

static inline void tkbc_append_kite_image(unsigned char *data, int width,
                                          int height, int format) {
  Image image_normal = {
      .data = data,
      .width = width,
      .height = height,
      .mipmaps = 1,
      .format = format,
  };

  image_normal = ImageCopy(image_normal);
  Image image_flipped = ImageCopy(image_normal);
  ImageFlipHorizontal(&image_flipped);

  space_dap(&kite_images_space, &kite_images,
            ((Kite_Image){
                .normal = image_normal,
                .flipped = image_flipped,
            }));
}

static inline void tkbc_append_kite_texture(Kite_Image kite_image) {
  space_dap(&kite_textures_space, &kite_textures,
            ((Kite_Texture){
                .normal = LoadTextureFromImage(kite_image.normal),
                .flipped = LoadTextureFromImage(kite_image.flipped),
            }));
}

static void append_assets() {
  tkbc_append_kite_image(image_1, IMAGE_1_WIDTH, IMAGE_1_HEIGHT, IMAGE_1_FORMAT);
  tkbc_append_kite_image(image_2, IMAGE_2_WIDTH, IMAGE_2_HEIGHT, IMAGE_2_FORMAT);
  tkbc_append_kite_image(image_3, IMAGE_3_WIDTH, IMAGE_3_HEIGHT, IMAGE_3_FORMAT);
  tkbc_append_kite_image(image_4, IMAGE_4_WIDTH, IMAGE_4_HEIGHT, IMAGE_4_FORMAT);

  tkbc_append_kite_image(image_Skeleton, IMAGE_Skeleton_WIDTH, IMAGE_Skeleton_HEIGHT, IMAGE_Skeleton_FORMAT);
  tkbc_append_kite_image(image_Filled_Panel, IMAGE_Filled_Panel_WIDTH, IMAGE_Filled_Panel_HEIGHT, IMAGE_Filled_Panel_FORMAT);
  tkbc_append_kite_image(image_Skeleton_Leadingedge, IMAGE_Skeleton_Leadingedge_WIDTH, IMAGE_Skeleton_Leadingedge_HEIGHT, IMAGE_Skeleton_Leadingedge_FORMAT);
  tkbc_append_kite_image(image_Leadingedge, IMAGE_Leadingedge_WIDTH, IMAGE_Leadingedge_HEIGHT, IMAGE_Leadingedge_FORMAT);
  tkbc_append_kite_image(image_Gaze, IMAGE_Gaze_WIDTH, IMAGE_Gaze_HEIGHT, IMAGE_Gaze_FORMAT);

  tkbc_append_kite_image(image_Left_01_1, IMAGE_Left_01_1_WIDTH, IMAGE_Left_01_1_HEIGHT, IMAGE_Left_01_1_FORMAT);
  tkbc_append_kite_image(image_Left_02_1, IMAGE_Left_02_1_WIDTH, IMAGE_Left_02_1_HEIGHT, IMAGE_Left_02_1_FORMAT);
  tkbc_append_kite_image(image_Left_03_1, IMAGE_Left_03_1_WIDTH, IMAGE_Left_03_1_HEIGHT, IMAGE_Left_03_1_FORMAT);
  tkbc_append_kite_image(image_Left_04_1, IMAGE_Left_04_1_WIDTH, IMAGE_Left_04_1_HEIGHT, IMAGE_Left_04_1_FORMAT);
  tkbc_append_kite_image(image_Left_05_1, IMAGE_Left_05_1_WIDTH, IMAGE_Left_05_1_HEIGHT, IMAGE_Left_05_1_FORMAT);
  tkbc_append_kite_image(image_Left_06_1, IMAGE_Left_06_1_WIDTH, IMAGE_Left_06_1_HEIGHT, IMAGE_Left_06_1_FORMAT);
  tkbc_append_kite_image(image_Left_07_1, IMAGE_Left_07_1_WIDTH, IMAGE_Left_07_1_HEIGHT, IMAGE_Left_07_1_FORMAT);
  tkbc_append_kite_image(image_Left_08_1, IMAGE_Left_08_1_WIDTH, IMAGE_Left_08_1_HEIGHT, IMAGE_Left_08_1_FORMAT);
  tkbc_append_kite_image(image_Left_09_1, IMAGE_Left_09_1_WIDTH, IMAGE_Left_09_1_HEIGHT, IMAGE_Left_09_1_FORMAT);
  tkbc_append_kite_image(image_Left_10_1, IMAGE_Left_10_1_WIDTH, IMAGE_Left_10_1_HEIGHT, IMAGE_Left_10_1_FORMAT);
  tkbc_append_kite_image(image_Left_11_1, IMAGE_Left_11_1_WIDTH, IMAGE_Left_11_1_HEIGHT, IMAGE_Left_11_1_FORMAT);
  tkbc_append_kite_image(image_Left_12_1, IMAGE_Left_12_1_WIDTH, IMAGE_Left_12_1_HEIGHT, IMAGE_Left_12_1_FORMAT);
  tkbc_append_kite_image(image_Left_13_1, IMAGE_Left_13_1_WIDTH, IMAGE_Left_13_1_HEIGHT, IMAGE_Left_13_1_FORMAT);
  tkbc_append_kite_image(image_Left_14_1, IMAGE_Left_14_1_WIDTH, IMAGE_Left_14_1_HEIGHT, IMAGE_Left_14_1_FORMAT);
  tkbc_append_kite_image(image_Left_15_1, IMAGE_Left_15_1_WIDTH, IMAGE_Left_15_1_HEIGHT, IMAGE_Left_15_1_FORMAT);


  tkbc_append_kite_image(image_Right_01_2, IMAGE_Right_01_2_WIDTH, IMAGE_Right_01_2_HEIGHT, IMAGE_Right_01_2_FORMAT);
  tkbc_append_kite_image(image_Right_02_2, IMAGE_Right_02_2_WIDTH, IMAGE_Right_02_2_HEIGHT, IMAGE_Right_02_2_FORMAT);
  tkbc_append_kite_image(image_Right_03_2, IMAGE_Right_03_2_WIDTH, IMAGE_Right_03_2_HEIGHT, IMAGE_Right_03_2_FORMAT);
  tkbc_append_kite_image(image_Right_04_2, IMAGE_Right_04_2_WIDTH, IMAGE_Right_04_2_HEIGHT, IMAGE_Right_04_2_FORMAT);
  tkbc_append_kite_image(image_Right_05_2, IMAGE_Right_05_2_WIDTH, IMAGE_Right_05_2_HEIGHT, IMAGE_Right_05_2_FORMAT);
  tkbc_append_kite_image(image_Right_06_2, IMAGE_Right_06_2_WIDTH, IMAGE_Right_06_2_HEIGHT, IMAGE_Right_06_2_FORMAT);
  tkbc_append_kite_image(image_Right_07_2, IMAGE_Right_07_2_WIDTH, IMAGE_Right_07_2_HEIGHT, IMAGE_Right_07_2_FORMAT);
  tkbc_append_kite_image(image_Right_08_2, IMAGE_Right_08_2_WIDTH, IMAGE_Right_08_2_HEIGHT, IMAGE_Right_08_2_FORMAT);
  tkbc_append_kite_image(image_Right_09_2, IMAGE_Right_09_2_WIDTH, IMAGE_Right_09_2_HEIGHT, IMAGE_Right_09_2_FORMAT);
  tkbc_append_kite_image(image_Right_10_2, IMAGE_Right_10_2_WIDTH, IMAGE_Right_10_2_HEIGHT, IMAGE_Right_10_2_FORMAT);
  tkbc_append_kite_image(image_Right_11_2, IMAGE_Right_11_2_WIDTH, IMAGE_Right_11_2_HEIGHT, IMAGE_Right_11_2_FORMAT);
  tkbc_append_kite_image(image_Right_12_2, IMAGE_Right_12_2_WIDTH, IMAGE_Right_12_2_HEIGHT, IMAGE_Right_12_2_FORMAT);
  tkbc_append_kite_image(image_Right_13_2, IMAGE_Right_13_2_WIDTH, IMAGE_Right_13_2_HEIGHT, IMAGE_Right_13_2_FORMAT);
  tkbc_append_kite_image(image_Right_14_2, IMAGE_Right_14_2_WIDTH, IMAGE_Right_14_2_HEIGHT, IMAGE_Right_14_2_FORMAT);
  tkbc_append_kite_image(image_Right_15_2, IMAGE_Right_15_2_WIDTH, IMAGE_Right_15_2_HEIGHT, IMAGE_Right_15_2_FORMAT);

  tkbc_append_kite_image(image_Middle_05, IMAGE_Middle_05_WIDTH, IMAGE_Middle_05_HEIGHT, IMAGE_Middle_05_FORMAT);
  tkbc_append_kite_image(image_Middle_09, IMAGE_Middle_09_WIDTH, IMAGE_Middle_09_HEIGHT, IMAGE_Middle_09_FORMAT);
  tkbc_append_kite_image(image_Middle_12, IMAGE_Middle_12_WIDTH, IMAGE_Middle_12_HEIGHT, IMAGE_Middle_12_FORMAT);


  Image colorizer_image = kite_images.elements[IMAGE_FILLED_PANEL].normal;
  tkbc_append_kite_image(colorizer_image.data, colorizer_image.width, colorizer_image.height, colorizer_image.format);
}

static inline void tkbc_load_kite_images_and_textures(void) {

  append_assets();

  for (size_t i = 0; i < kite_images.count; ++i) {
    if (!IsImageValid(kite_images.elements[i].normal)) {
      tkbc_fprintf(stderr, "ERROR", "Could not load normal kite image: %zu.\n",
                   i);
    }
    if (!IsImageValid(kite_images.elements[i].flipped)) {
      tkbc_fprintf(stderr, "ERROR", "Could not load flipped kite image: %zu.\n",
                   i);
    }

    tkbc_append_kite_texture(kite_images.elements[i]);
  }

  for (size_t i = 0; i < kite_textures.count; ++i) {
    if (!IsTextureValid(kite_textures.elements[i].normal)) {
      tkbc_fprintf(stderr, "ERROR",
                   "Could not load normal kite texture: %zu.\n", i);
    }
    if (!IsTextureValid(kite_textures.elements[i].flipped)) {
      tkbc_fprintf(stderr, "ERROR",
                   "Could not load flipped kite texture: %zu.\n", i);
    }
  }
}

static inline void tkbc_assets_destroy() {
  for (size_t i = 0; i < kite_textures.count; ++i) {
    UnloadTexture(kite_textures.elements[i].normal);
    UnloadTexture(kite_textures.elements[i].flipped);
  }

  for (size_t i = 0; i < kite_images.count; ++i) {
    UnloadImage(kite_images.elements[i].normal);
    UnloadImage(kite_images.elements[i].flipped);
  }
}

#endif // TKBC_ASSET_HANDLER_H_
