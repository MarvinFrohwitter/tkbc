#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>

#include <libgen.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define TKBC_UTILS_IMPLEMENTATION
#include "../src/global/tkbc-utils.h"

void free_dir_entrys(Dir_Entries dir_entrys) {
  for (size_t i = 0; i < dir_entrys.count; ++i) {
    free(dir_entrys.elements[i].name);
    free(dir_entrys.elements[i].full_path);
  }
  free(dir_entrys.elements);
}

bool read_dir_impl(const char *path, Dir_Entries *list) {
  DIR *dir = opendir(path);
  if (!dir) {
    fprintf(stderr, "Error: Could not open dir %s: %s\n", path,
            strerror(errno));
    return false;
  }

  for (;;) {
    errno = 0;
    struct dirent *entry = readdir(dir);
    if (entry == NULL && errno == 0) {
      break;
    }
    // There should no error occur because EBADF can not happen.
    assert(errno == 0);
    if (strcmp(entry->d_name, ".") == 0) {
      continue;
    }
    if (strcmp(entry->d_name, "..") == 0) {
      continue;
    }

    char *real_path = realpath(path, NULL);
    int ret = snprintf(NULL, 0, "%s/%s", real_path, entry->d_name);
    if (ret < 0) {
      free(real_path);
      return false;
    }
    ret += 1;
    char *full_path = malloc(sizeof(char) * ret);
    ret = snprintf(full_path, ret, "%s/%s", real_path, entry->d_name);
    free(real_path);
    if (ret < 0) {
      return false;
    }

    tkbc_dap(list, ((Dir_Entry){
                       .name = strdup(entry->d_name),
                       .full_path = full_path,
                       .type = entry->d_type,
                   }));
  }

  int ok = closedir(dir);
  if (ok < 0) {
    printf("Error: Could not close dir %s: %s\n", path, strerror(errno));
    return false;
  }

  return true;
}

bool read_dir(const char *path, Dir_Entries *list) {
  if (!path) {
    return false;
  }
  bool result = read_dir_impl(path, list);
  if (result) {
    tkbc_dap(list, ((Dir_Entry){
                       .name = strdup("."),
                       .full_path = realpath(path, NULL),
                       .type = DT_DIR,
                   }));

    char *parent_full_path = realpath(path, NULL);
    parent_full_path = dirname(parent_full_path);

    tkbc_dap(list, ((Dir_Entry){
                       .name = strdup(".."),
                       .full_path = parent_full_path,
                       .type = DT_DIR,
                   }));
  }
  return result;
}

bool read_dir_recursive(const char *path, Dir_Entries *list) {
  bool ok = true;
  ok = read_dir(path, list);
  if (!ok) {
    return false;
  }

  for (size_t i = 0; i < list->count; ++i) {
    if (list->elements[i].type != DT_DIR) {
      continue;
    }
    if (strcmp(list->elements[i].name, ".") == 0) {
      continue;
    }
    if (strcmp(list->elements[i].name, "..") == 0) {
      continue;
    }

    ok = read_dir_impl(list->elements[i].full_path, list);
    if (!ok) {
      return false;
    }
  }

  return true;
}

#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb/stb_image.h"

bool transpile_png_to_h(const char *image_file_path, const char *name,
                        Content *result) {
  bool ok = true;
  fprintf(stderr, "INFO: .................................\n");
  fprintf(stderr, "INFO: Converting %s ...\n", image_file_path);

  int x, y, comp;
  unsigned char *image_data = stbi_load(image_file_path, &x, &y, &comp, 4);
  if (image_data == NULL) {
    fprintf(stderr, "ERROR: could not load image data form file!\n");
    fprintf(stderr, "INFO: .................................\n");
    check_return(false);
  }

  char *dot = (char *)strrchr(name, '.');
  if (dot == NULL) {
    fprintf(stderr, "ERROR: no valid file extension!\n");
    fprintf(stderr, "INFO: .................................\n");
    check_return(false);
  }
  *dot = '\0';

  Content data = {0};
  char *name_upper = tkbc_strtoupper(strdup(name));
  char *name_lower = tkbc_strtolower(strdup(name));
  tkbc_dapf(&data, "#define IMAGE_%s_WIDTH %d\n", name_upper, x);
  tkbc_dapf(&data, "#define IMAGE_%s_HEIGHT %d\n", name_upper, y);
  tkbc_dapf(&data, "#define IMAGE_%s_FORMAT %s\n", name_upper,
            STR(PIXELFORMAT_UNCOMPRESSED_R8G8B8A8));

  tkbc_dapf(&data,
            "\nstatic unsigned char "
            "asset_image_%s[IMAGE_%s_WIDTH*IMAGE_%s_HEIGHT*%d] = {\n",
            name_lower, name_upper, name_upper, comp);
  free(name_upper);
  free(name_lower);
  *dot = '.';

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

  dot = (char *)strrchr(image_file_path, '.');
  char save[3];
  save[0] = *(dot + 0);
  save[1] = *(dot + 1);
  save[2] = *(dot + 2);

  *(dot + 0) = '.';
  *(dot + 1) = 'h';
  *(dot + 2) = '\0';

  tkbc_dapc(result, data.elements, data.count);
  int res = tkbc_write_file(image_file_path, data.elements, data.count);
  if (res < 0) {
    fprintf(stderr, "ERROR: could not write image data to file!\n");
    fprintf(stderr, "INFO: .................................\n");
    *(dot + 0) = save[0];
    *(dot + 1) = save[1];
    *(dot + 2) = save[2];
    check_return(false);
  } else {
    fprintf(stderr, "Successfully converted: %s\n", image_file_path);
    fprintf(stderr, "INFO: .................................\n");
    *(dot + 0) = save[0];
    *(dot + 1) = save[1];
    *(dot + 2) = save[2];
  }

check:
  free(image_data);
  free(data.elements);
  return ok;
}

#define BUILD_PATH "build/"

int get_file_type(const char *path) {
  struct stat statbuf;
  if (lstat(path, &statbuf) < 0) {
    fprintf(stderr, "Error: could not get stat for file %s: %s\n", path,
            strerror(errno));
    return -1;
  }
  return statbuf.st_mode;
}

int main(int argc, char **argv) {
  int ok = 0;
  const char *program_name = shift(argv, argc);
  if (argc == 0) {
    fprintf(stderr, "Error: Incorrect argument count given\n");
    fprintf(stderr, "Usage: %s <path> [path..]\n", program_name);
    return 1;
  }
  bool append = false;
  while (argc > 0) {
    char *path = shift(argv, argc);
    if (strcmp(path, "") == 0) {
      continue;
    }

    int file_type = get_file_type(path);
    if (file_type < 0) {
      continue;
    }
    Dir_Entries entries = {0};
    if (0) {
    } else if (S_ISREG(file_type)) {
      tkbc_dap(&entries, ((Dir_Entry){
                             .name = strdup(basename(path)),
                             .full_path = strdup(path),
                             .type = DT_REG,
                         }));

      append = true;
    } else if (S_ISDIR(file_type)) {
      bool ok = read_dir_recursive(path, &entries);
      if (!ok) {
        free_dir_entrys(entries);
        continue;
      }
    } else {
      continue;
    }

    Content combined = {0};
    for (size_t i = 0; i < entries.count; ++i) {
      if (0) {
        if (strstr(entries.elements[i].full_path, ".h") != 0) {
          printf("removed: %s\n", entries.elements[i].full_path);
          // int rr = remove(dir_entrys.elements[i].full_path);
          // if (rr < 0) {
          //   printf("ERROR: could not remove %s: %s\n",
          //          dir_entrys.elements[i].full_path, strerror(errno));
          // }
        }
      }

      if (strstr(entries.elements[i].name, ".png") &&
          *entries.elements[i].name != '.') {
        transpile_png_to_h(entries.elements[i].full_path,
                           entries.elements[i].name, &combined);
      }
    }

    const char *combined_file_name = "combind_assets.h";
    char *parent_full_path = realpath(path, NULL);
    parent_full_path = dirname(parent_full_path);
    int n;
    if (append) {
      n = snprintf(NULL, 0, "%s/%s", parent_full_path, combined_file_name);
    } else {
      n = snprintf(NULL, 0, "%s/%s", path, combined_file_name);
    }
    if (n < 0) {
      free(combined.elements);
      free_dir_entrys(entries);
      continue;
    }
    n += 1;
    char *final_path = malloc(n * sizeof(char));
    if (append) {
      n = snprintf(final_path, n, "%s/%s", parent_full_path,
                   combined_file_name);
    } else {
      n = snprintf(final_path, n, "%s/%s", path, combined_file_name);
    }
    free(parent_full_path);
    if (n < 0) {
      free(final_path);
      free(combined.elements);
      free_dir_entrys(entries);
      continue;
    }

    int res = 0;
    if (append) {
      res = tkbc_append_file(final_path, combined.elements, combined.count);
    } else {
      res = tkbc_write_file(final_path, combined.elements, combined.count);
    }
    if (res < 0) {
      fprintf(stderr, "ERROR: could not write combined data to file!\n");
    } else {
      fprintf(stderr, "Successfully wrriten combined: %s\n", path);
    }

    free(final_path);
    free_dir_entrys(entries);
    free(combined.elements);
  }
  return ok;
}
