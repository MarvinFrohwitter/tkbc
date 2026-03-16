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

void tkbc_append_kite_image(unsigned char *data, int width, int height,
                            int format);

#ifndef TKBC_SERVER
void tkbc_append_kite_texture(Kite_Image kite_image);
void tkbc_load_kite_images_and_textures(void);

void tkbc_assets_destroy_kite_textures();
void tkbc_assets_destroy();
#endif

void append_assets();
void tkbc_assets_destroy_kite_images();

#endif // TKBC_ASSET_HANDLER_H_
