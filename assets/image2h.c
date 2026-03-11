#define TKBC_UTILS_IMPLEMENTATION
#include "../src/global/tkbc-utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb/stb_image.h"

#include <stdio.h>

int main(int argc, char *argv[]) {
  const char *program_name = tkbc_shift_args(&argc, &argv);
  if (argc == 0) {
    printf("Error: Incorrect argument count given\n");
    printf("Usage: %s <image_file_path>\n", program_name);
    printf("Error: %d", argc);
    return 1;
  }

  while (argc > 0) {
    char *image_file_path = tkbc_shift_args(&argc, &argv);
    fprintf(stderr, "INFO: .................................\n");
    fprintf(stderr, "INFO: Converting %s ...\n", image_file_path);

    int x, y, comp;
    unsigned char *image_data = stbi_load(image_file_path, &x, &y, &comp, 4);
    if (image_data == NULL) {
      fprintf(stderr, "ERROR: could not load image data form file!\n");
      fprintf(stderr, "INFO: .................................\n");
      continue;
    }

    char *dot = strrchr(image_file_path, '.');
    if (dot == NULL) {
      fprintf(stderr, "ERROR: no valid file extension!\n");
      fprintf(stderr, "INFO: .................................\n");
      continue;
    }
    *dot = '\0';

    Content data = {0};
    tkbc_dapf(&data, "#define KITE_IMAGE%s_WIDTH %d\n", image_file_path, x);
    tkbc_dapf(&data, "#define KITE_IMAGE%s_HEIGHT %d\n", image_file_path, y);
    tkbc_dapf(&data, "#define KITE_IMAGE%s_FORMAT %s\n", image_file_path,
              STR(PIXELFORMAT_UNCOMPRESSED_R8G8B8A8));

    tkbc_dapf(&data,
              "\nstatic unsigned char "
              "kite_image%s[KITE_IMAGE%s_WIDTH*KITE_IMAGE%s_HEIGHT*%d] = {\n",
              image_file_path, image_file_path, image_file_path, comp);

    int line_comp = 15;
    for (size_t i = 0; i < (size_t)x * y * comp; ++i) {
      if (i == 0) {
        tkbc_dapf(&data, "    ");
      } else if (i % line_comp == 0) {
        tkbc_dapf(&data, "\n    ");
      }
      tkbc_dapf(&data, "0x%02X,", image_data[i]);
    }

    if (data.elements[data.count - 1] == ',') {
      data.count -= 1;
    } else if (data.elements[data.count - 1] == '\n') {
      data.count -= 2;
    }
    tkbc_dapf(&data, "\n};\n");

    char save[3];
    save[0] = *(dot + 0);
    save[1] = *(dot + 1);
    save[2] = *(dot + 2);

    *(dot + 0) = '.';
    *(dot + 1) = 'h';
    *(dot + 2) = '\0';
    if (tkbc_write_file(image_file_path, data.elements, data.count) < 0) {
      fprintf(stderr, "ERROR: could not write image data to file!\n");
      fprintf(stderr, "INFO: .................................\n");
      continue;
    } else {
      *(dot + 0) = save[0];
      *(dot + 1) = save[1];
      *(dot + 2) = save[2];

      printf("Successfully converted: %s\n", image_file_path);
      fprintf(stderr, "INFO: .................................\n");
    }
  }

  return 0;
}
