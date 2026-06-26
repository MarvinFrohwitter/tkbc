#ifndef TKBC_ASSET_HANDLER_H_
#define TKBC_ASSET_HANDLER_H_

#include "../../external/space/space.h"
#include "../global/tkbc-types.h"
#include "../global/tkbc-utils.h"
#include "raylib.h"
#include <assert.h>
#include <stdio.h>

size_t tkbc_append_kite_image(unsigned char *data, int width, int height,
                              int format);

#ifndef TKBC_SERVER
void tkbc_load_kite_texture_from_kite_image(Kite_Image kite_image, Id asset_id);
void tkbc_load_assets(void);
#endif

void append_assets(void);
void tkbc_assets_destroy(void);
Asset *tkbc_find_asset_from_id(Id id);
size_t tkbc_get_current_kite_design_count();
Id tkbc_append_kite_image_and_kite_texture(unsigned char *data, int width,
                                           int height, int format);

#define tkbc_get_asset_image(kind)                                             \
  assets.elements[(assert(assets.count > 0),                                   \
                   assert(assets.elements[kind].type == ASSETS_IMAGE), kind)]

#define tkbc_get_asset_kite_design(kind)                                       \
  assets.elements[(assert(assets.count > 0),                                   \
                   assert(assets.elements[(kind)].type == ASSETS_KITE_DESIGN), \
                   (kind))]

#define tkbc_get_asset(kind) assets.elements[(assert(assets.count > 0), (kind))]

#endif // TKBC_ASSET_HANDLER_H_
