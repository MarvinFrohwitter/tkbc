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

size_t tkbc_append_kite_image(unsigned char *data, int width, int height,
                              int format);

#ifndef TKBC_SERVER
void tkbc_append_kite_texture(Kite_Image kite_image);
void tkbc_load_kite_images_and_textures(void);

void tkbc_assets_destroy_kite_textures(void);
void tkbc_assets_destroy(void);
#endif

void append_assets(void);
void tkbc_assets_destroy_kite_images(void);

bool tkbc_find_asset_id_in_kite_images(ssize_t id);
Kite_Image *tkbc_find_asset_in_kite_images(ssize_t id);

#ifndef TKBC_SERVER
Kite_Texture *tkbc_find_asset_in_kite_textures(ssize_t id);
#endif

#endif // TKBC_ASSET_HANDLER_H_
