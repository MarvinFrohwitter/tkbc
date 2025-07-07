#ifndef TKBC_ASSET_HANDLER_H_
#define TKBC_ASSET_HANDLER_H_

#include "raylib.h"
#include <stdio.h>
#include "../global/tkbc-types.h"
#include "../../external/space/space.h"
#include "../global/tkbc-utils.h"

extern Space kite_images_space;
extern Kite_Images kite_images;

extern Space kite_textures_space;
extern Kite_Textures kite_textures;

#include "../../assets/kite_image.h"

static inline void tkbc_append_kite_image(unsigned char *data, int width,
                                          int height, int format) {
  Image image_normal = {
      .data = data,
      .width = width,
      .height = height,
      .mipmaps = 1,
      .format = format,
  };

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

static inline void tkbc_load_kite_images_and_textures(void) {

  tkbc_append_kite_image(kite_image1, KITE_IMAGE1_WIDTH, KITE_IMAGE1_HEIGHT,
                         KITE_IMAGE1_FORMAT);
  tkbc_append_kite_image(kite_image2, KITE_IMAGE2_WIDTH, KITE_IMAGE2_HEIGHT,
                         KITE_IMAGE2_FORMAT);
  tkbc_append_kite_image(kite_image3, KITE_IMAGE3_WIDTH, KITE_IMAGE3_HEIGHT,
                         KITE_IMAGE3_FORMAT);

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

#endif // TKBC_ASSET_HANDLER_H_
