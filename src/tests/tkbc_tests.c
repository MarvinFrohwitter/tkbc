#include "tkbc_test_geometrics.c"
#include "tkbc_test_script_handler.c"

#define eps 0.01
#define CASSERT_IMPLEMENTATION
#include "../../external/cassert/cassert.h"

Kite_Images kite_images;
Kite_Textures kite_textures;

int main() {
  cassert_tests {
    tkbc_test_geometrics(&tests);
    tkbc_test_script_handler(&tests);
  }

#ifdef SHORT_LOG
  cassert_short_print_tests(&tests);
#else
  cassert_print_tests(&tests);
#endif // SHORT_LOG

  cassert_free_tests(&tests);
  return 0;
}
