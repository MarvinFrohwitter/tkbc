#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../external/stb/stb_image_write.h"

#include "kite_image.h"
#include <assert.h>
#include <stdio.h>

int main(void) {

  const char *image_name1 = "1.png";
  if (stbi_write_png(image_name1, KITE_IMAGE1_WIDTH, KITE_IMAGE1_HEIGHT, 4,
                     kite_image1, 0) == 0) {
    printf("ERROR: Failed %s\n", image_name1);
    return 1;
  }
  printf("Successfully converted: %s\n", image_name1);
  const char *image_name2 = "2.png";
  if (stbi_write_png(image_name2, KITE_IMAGE2_WIDTH, KITE_IMAGE2_HEIGHT, 4,
                     kite_image2, 0) == 0) {
    printf("ERROR: Failed %s\n", image_name2);
    return 1;
  }
  printf("Successfully converted: %s\n", image_name2);
  const char *image_name3 = "3.png";
  if (stbi_write_png(image_name3, KITE_IMAGE3_WIDTH, KITE_IMAGE3_HEIGHT, 4,
                     kite_image3, 0) == 0) {
    printf("ERROR: Failed %s\n", image_name3);
    return 1;
  }
  printf("Successfully converted: %s\n", image_name3);

  return 0;
}
