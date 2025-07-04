#ifndef TKBC_ASSET_HANDLER_H_
#define TKBC_ASSET_HANDLER_H_

extern Space kite_images_space;
extern Kite_Images kite_images;

#include "../../assets/kite_image.h"

static inline void tkbc_load_kite_images(void) {

  Image image_normal = {
      .data = kite_image1,
      .width = KITE_IMAGE1_WIDTH,
      .height = KITE_IMAGE1_HEIGHT,
      .mipmaps = 1,
      .format = KITE_IMAGE1_FORMAT,
  };
  Image image_flipped = ImageCopy(image_normal);
  ImageFlipHorizontal(&image_flipped);

  space_dap(&kite_images_space, &kite_images,
            ((Kite_Image){
                .normal = image_normal,
                .flipped = image_flipped,
            }));

  for (size_t i = 0; i < kite_images.count; ++i) {
    if (!IsImageValid(kite_images.elements[i].normal)) {
      tkbc_fprintf(stderr, "ERROR", "Could not load normal kite image.\n");
    }
    if (!IsImageValid(kite_images.elements[i].flipped)) {
      tkbc_fprintf(stderr, "ERROR", "Could not load flipped kite image.\n");
    }
  }
}

#endif // TKBC_ASSET_HANDLER_H_
