#include "../../external/space/space.h"
#include "../global/tkbc-types.h"
#include "../global/tkbc-utils.h"
#include "raylib.h"
#include <assert.h>
#include <stdio.h>

extern Assets assets;
static size_t gloabl_asset_id_factory = 0;

#include "../../assets/combind_assets.h"
#include "tkbc-asset-handler.h"

// Save and load kite designs from config files.

/**
 * @brief The function appends a new kite image to the global kite_images
 * collection. The image data is copied and stored.
 *
 * @param data The raw image data.
 * @param width The width of the image.
 * @param height The height of the image.
 * @param format The pixel format of the image.
 * @return The id assigned to the newly appended kite image.
 */
Id tkbc_append_kite_image(unsigned char *data, int width, int height,
                          int format) {
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

  Kite_Image kite_image = (Kite_Image){
      .normal = image_normal,
      .flipped = image_flipped,
  };

  space_dap(&assets.space, &assets,
            ((Asset){
                .type = ASSETS_KITE_DESIGN,
                .as.kite_image = kite_image,
                .id = gloabl_asset_id_factory,
            }));

  return gloabl_asset_id_factory++;
}

/**
 * @brief The function appends a new asset image to the global assets
 * collection. The image data is copied and stored.
 *
 * @param data The raw image data.
 * @param width The width of the image.
 * @param height The height of the image.
 * @param format The pixel format of the image.
 * @return The id assigned to the newly appended asset.
 */
Id tkbc_append_asset_image(unsigned char *data, int width, int height,
                           int format) {
  Image image = {
      .data = data,
      .width = width,
      .height = height,
      .mipmaps = 1,
      .format = format,
  };

  image = ImageCopy(image);

  space_dap(&assets.space, &assets,
            ((Asset){
                .type = ASSETS_IMAGE,
                .as.image = image,
                .id = gloabl_asset_id_factory,
            }));

  return gloabl_asset_id_factory++;
}

void tkbc_append_kite_image_pannels() {
  tkbc_append_asset_image(image_Skeleton, IMAGE_Skeleton_WIDTH,
                          IMAGE_Skeleton_HEIGHT, IMAGE_Skeleton_FORMAT);
  tkbc_append_asset_image(image_Filled_Panel, IMAGE_Filled_Panel_WIDTH,
                          IMAGE_Filled_Panel_HEIGHT, IMAGE_Filled_Panel_FORMAT);
  tkbc_append_asset_image(
      image_Skeleton_Leadingedge, IMAGE_Skeleton_Leadingedge_WIDTH,
      IMAGE_Skeleton_Leadingedge_HEIGHT, IMAGE_Skeleton_Leadingedge_FORMAT);
  tkbc_append_asset_image(image_Leadingedge, IMAGE_Leadingedge_WIDTH,
                          IMAGE_Leadingedge_HEIGHT, IMAGE_Leadingedge_FORMAT);
  tkbc_append_asset_image(image_Gaze, IMAGE_Gaze_WIDTH, IMAGE_Gaze_HEIGHT,
                          IMAGE_Gaze_FORMAT);

  tkbc_append_asset_image(image_Left_01_1, IMAGE_Left_01_1_WIDTH,
                          IMAGE_Left_01_1_HEIGHT, IMAGE_Left_01_1_FORMAT);
  tkbc_append_asset_image(image_Left_02_1, IMAGE_Left_02_1_WIDTH,
                          IMAGE_Left_02_1_HEIGHT, IMAGE_Left_02_1_FORMAT);
  tkbc_append_asset_image(image_Left_03_1, IMAGE_Left_03_1_WIDTH,
                          IMAGE_Left_03_1_HEIGHT, IMAGE_Left_03_1_FORMAT);
  tkbc_append_asset_image(image_Left_04_1, IMAGE_Left_04_1_WIDTH,
                          IMAGE_Left_04_1_HEIGHT, IMAGE_Left_04_1_FORMAT);
  tkbc_append_asset_image(image_Left_05_1, IMAGE_Left_05_1_WIDTH,
                          IMAGE_Left_05_1_HEIGHT, IMAGE_Left_05_1_FORMAT);
  tkbc_append_asset_image(image_Left_06_1, IMAGE_Left_06_1_WIDTH,
                          IMAGE_Left_06_1_HEIGHT, IMAGE_Left_06_1_FORMAT);
  tkbc_append_asset_image(image_Left_07_1, IMAGE_Left_07_1_WIDTH,
                          IMAGE_Left_07_1_HEIGHT, IMAGE_Left_07_1_FORMAT);
  tkbc_append_asset_image(image_Left_08_1, IMAGE_Left_08_1_WIDTH,
                          IMAGE_Left_08_1_HEIGHT, IMAGE_Left_08_1_FORMAT);
  tkbc_append_asset_image(image_Left_09_1, IMAGE_Left_09_1_WIDTH,
                          IMAGE_Left_09_1_HEIGHT, IMAGE_Left_09_1_FORMAT);
  tkbc_append_asset_image(image_Left_10_1, IMAGE_Left_10_1_WIDTH,
                          IMAGE_Left_10_1_HEIGHT, IMAGE_Left_10_1_FORMAT);
  tkbc_append_asset_image(image_Left_11_1, IMAGE_Left_11_1_WIDTH,
                          IMAGE_Left_11_1_HEIGHT, IMAGE_Left_11_1_FORMAT);
  tkbc_append_asset_image(image_Left_12_1, IMAGE_Left_12_1_WIDTH,
                          IMAGE_Left_12_1_HEIGHT, IMAGE_Left_12_1_FORMAT);
  tkbc_append_asset_image(image_Left_13_1, IMAGE_Left_13_1_WIDTH,
                          IMAGE_Left_13_1_HEIGHT, IMAGE_Left_13_1_FORMAT);
  tkbc_append_asset_image(image_Left_14_1, IMAGE_Left_14_1_WIDTH,
                          IMAGE_Left_14_1_HEIGHT, IMAGE_Left_14_1_FORMAT);
  tkbc_append_asset_image(image_Left_15_1, IMAGE_Left_15_1_WIDTH,
                          IMAGE_Left_15_1_HEIGHT, IMAGE_Left_15_1_FORMAT);

  tkbc_append_asset_image(image_Right_01_2, IMAGE_Right_01_2_WIDTH,
                          IMAGE_Right_01_2_HEIGHT, IMAGE_Right_01_2_FORMAT);
  tkbc_append_asset_image(image_Right_02_2, IMAGE_Right_02_2_WIDTH,
                          IMAGE_Right_02_2_HEIGHT, IMAGE_Right_02_2_FORMAT);
  tkbc_append_asset_image(image_Right_03_2, IMAGE_Right_03_2_WIDTH,
                          IMAGE_Right_03_2_HEIGHT, IMAGE_Right_03_2_FORMAT);
  tkbc_append_asset_image(image_Right_04_2, IMAGE_Right_04_2_WIDTH,
                          IMAGE_Right_04_2_HEIGHT, IMAGE_Right_04_2_FORMAT);
  tkbc_append_asset_image(image_Right_05_2, IMAGE_Right_05_2_WIDTH,
                          IMAGE_Right_05_2_HEIGHT, IMAGE_Right_05_2_FORMAT);
  tkbc_append_asset_image(image_Right_06_2, IMAGE_Right_06_2_WIDTH,
                          IMAGE_Right_06_2_HEIGHT, IMAGE_Right_06_2_FORMAT);
  tkbc_append_asset_image(image_Right_07_2, IMAGE_Right_07_2_WIDTH,
                          IMAGE_Right_07_2_HEIGHT, IMAGE_Right_07_2_FORMAT);
  tkbc_append_asset_image(image_Right_08_2, IMAGE_Right_08_2_WIDTH,
                          IMAGE_Right_08_2_HEIGHT, IMAGE_Right_08_2_FORMAT);
  tkbc_append_asset_image(image_Right_09_2, IMAGE_Right_09_2_WIDTH,
                          IMAGE_Right_09_2_HEIGHT, IMAGE_Right_09_2_FORMAT);
  tkbc_append_asset_image(image_Right_10_2, IMAGE_Right_10_2_WIDTH,
                          IMAGE_Right_10_2_HEIGHT, IMAGE_Right_10_2_FORMAT);
  tkbc_append_asset_image(image_Right_11_2, IMAGE_Right_11_2_WIDTH,
                          IMAGE_Right_11_2_HEIGHT, IMAGE_Right_11_2_FORMAT);
  tkbc_append_asset_image(image_Right_12_2, IMAGE_Right_12_2_WIDTH,
                          IMAGE_Right_12_2_HEIGHT, IMAGE_Right_12_2_FORMAT);
  tkbc_append_asset_image(image_Right_13_2, IMAGE_Right_13_2_WIDTH,
                          IMAGE_Right_13_2_HEIGHT, IMAGE_Right_13_2_FORMAT);
  tkbc_append_asset_image(image_Right_14_2, IMAGE_Right_14_2_WIDTH,
                          IMAGE_Right_14_2_HEIGHT, IMAGE_Right_14_2_FORMAT);
  tkbc_append_asset_image(image_Right_15_2, IMAGE_Right_15_2_WIDTH,
                          IMAGE_Right_15_2_HEIGHT, IMAGE_Right_15_2_FORMAT);

  tkbc_append_asset_image(image_Middle_05, IMAGE_Middle_05_WIDTH,
                          IMAGE_Middle_05_HEIGHT, IMAGE_Middle_05_FORMAT);
  tkbc_append_asset_image(image_Middle_09, IMAGE_Middle_09_WIDTH,
                          IMAGE_Middle_09_HEIGHT, IMAGE_Middle_09_FORMAT);
  tkbc_append_asset_image(image_Middle_12, IMAGE_Middle_12_WIDTH,
                          IMAGE_Middle_12_HEIGHT, IMAGE_Middle_12_FORMAT);
}

/**
 * @brief The function loads all the predefined kite assets into the
 * asset collection. This includes left, right, middle, skeleton,
 * leading edge, and other predefined kite images.
 */
void append_assets(void) {

  // This loading order has to be the exact same as in the
  // Asset_Kite_Design_Kind enum to correlate the place.
  // TODO: Make the asset loading depended on the enum.
  static_assert(
      ASSET_KITE_DESIGN_COUNT,
      "The static asset count has changed think about the order of appending");

  tkbc_append_asset_image(image_Logo, IMAGE_Logo_WIDTH, IMAGE_Logo_HEIGHT,
                          IMAGE_Logo_FORMAT);

  tkbc_append_kite_image(image_1, IMAGE_1_WIDTH, IMAGE_1_HEIGHT,
                         IMAGE_1_FORMAT);
  tkbc_append_kite_image(image_2, IMAGE_2_WIDTH, IMAGE_2_HEIGHT,
                         IMAGE_2_FORMAT);
  tkbc_append_kite_image(image_3, IMAGE_3_WIDTH, IMAGE_3_HEIGHT,
                         IMAGE_3_FORMAT);
  tkbc_append_kite_image(image_4, IMAGE_4_WIDTH, IMAGE_4_HEIGHT,
                         IMAGE_4_FORMAT);

  tkbc_append_kite_image_pannels();

  Image colorizer_image = tkbc_get_asset_image(IMAGE_FILLED_PANEL).as.image;
  tkbc_append_kite_image(colorizer_image.data, colorizer_image.width,
                         colorizer_image.height, colorizer_image.format);
}

#ifndef TKBC_SERVER
/**
 * @brief The function creates and appends a new kite texture from the given
 * kite image. The texture is loaded from the image for both normal and flipped
 * orientations.
 *
 * @param kite_image The kite image from which the texture should be created.
 */
void tkbc_load_kite_texture_from_kite_image(Kite_Image kite_image,
                                            Id asset_id) {
  Asset *asset = tkbc_find_asset_from_id(asset_id);
  if (!asset) {
    return;
  }

  tkbc_get_asset_kite_design(asset->id).as.kite_texture = (Kite_Texture){
      .normal = LoadTextureFromImage(kite_image.normal),
      .flipped = LoadTextureFromImage(kite_image.flipped),
  };

  GenTextureMipmaps(
      &tkbc_get_asset_kite_design(asset->id).as.kite_texture.normal);
  GenTextureMipmaps(
      &tkbc_get_asset_kite_design(asset->id).as.kite_texture.flipped);
}

/**
 * @brief The function loads all kite images and creates textures from them.
 * This function should only be called on the client side, not on the server.
 */
void tkbc_load_kite_images_and_textures(void) {

  // TODO: Make a type dependent image loader.
  append_assets();

  for (size_t i = 0; i < assets.count; ++i) {

    if (assets.elements[i].type == ASSETS_IMAGE) {
      if (!IsImageValid(tkbc_get_asset_image(i).as.image)) {
        tkbc_fprintf(stderr, "ERROR", "Could not load image asset: %zu.\n", i);
      }
      continue;
    }

    if (!IsImageValid(tkbc_get_asset_kite_design(i).as.kite_image.normal)) {
      tkbc_fprintf(stderr, "ERROR", "Could not load normal kite image: %zu.\n",
                   i);
    }
    if (!IsImageValid(tkbc_get_asset_kite_design(i).as.kite_image.flipped)) {
      tkbc_fprintf(stderr, "ERROR", "Could not load flipped kite image: %zu.\n",
                   i);
    }

    if (tkbc_get_asset_kite_design(i).id >= IMAGE_PANNEL_PARTS_BEGIN &&
        tkbc_get_asset_kite_design(i).id <= IMAGE_PANNEL_PARTS_END) {
      continue;
    }

    tkbc_load_kite_texture_from_kite_image(
        tkbc_get_asset_kite_design(i).as.kite_image,
        tkbc_get_asset_kite_design(i).id);

    if (!IsTextureValid(tkbc_get_asset_kite_design(i).as.kite_texture.normal)) {
      tkbc_fprintf(stderr, "ERROR",
                   "Could not load normal kite texture: %zu.\n", i);
    }
    if (!IsTextureValid(
            tkbc_get_asset_kite_design(i).as.kite_texture.flipped)) {
      tkbc_fprintf(stderr, "ERROR",
                   "Could not load flipped kite texture: %zu.\n", i);
    }
  }
}
#endif

/**
 * @brief The function destroys all kite assets including images and textures.
 */
void tkbc_assets_destroy(void) {
  for (size_t i = 0; i < assets.count; ++i) {
    switch (assets.elements[i].type) {
    case ASSETS_IMAGE:
      UnloadImage(tkbc_get_asset_image(i).as.image);
      break;
    case ASSETS_KITE_DESIGN:
      UnloadImage(tkbc_get_asset_kite_design(i).as.kite_image.normal);
      UnloadImage(tkbc_get_asset_kite_design(i).as.kite_image.flipped);

#ifndef TKBC_SERVER
      UnloadTexture(tkbc_get_asset_kite_design(i).as.kite_texture.normal);
      UnloadTexture(tkbc_get_asset_kite_design(i).as.kite_texture.flipped);
#endif
      break;
    case ASSETS_KIND_COUNT:
    default:
      assert(false && "tkbc_assets_destroy: UNREACHABLE");
    }
  }
}

/**
 * @brief The function tries to find a specific asset by an id.
 *
 * @param id The id of the assets to search for.
 * @return The found asset, if not found NULL.
 */
Asset *tkbc_find_asset_from_id(Id id) {
  for (size_t i = 0; i < assets.count; ++i) {
    if (assets.elements[i].id == id) {
      return &assets.elements[i];
    }
  }
  return NULL;
}

/**
 * @brief This function calculates the current number of assets that is related
 * to the kite designs.
 *
 * @return The count of the currently registered assets related to kite design.
 */
size_t tkbc_get_current_kite_design_count() {
  size_t result = 0;
  for (size_t i = 0; i < assets.count; ++i) {
    if (assets.elements[i].type == ASSETS_KITE_DESIGN) {
      result++;
    }
  }
  return result;
}

/**
 * @brief Appends a kite image to the asset list and loads its texture.
 *
 * @param data The raw pixel data of the kite image.
 * @param width The width of the image in pixels.
 * @param height The height of the image in pixels.
 * @param format The pixel format of the image data.
 * @return Id The asset id of the appended kite image.
 */
Id tkbc_append_kite_image_and_kite_texture(unsigned char *data, int width,
                                           int height, int format) {

  Id id = tkbc_append_kite_image(data, width, height, format);
  // This is just for compilation the function is not used in
  // the server at all. Just the files in this dir are all
  // passed to the server compilations as well.
#ifndef TKBC_SERVER
  Kite_Image kite_image =
      tkbc_get_asset_kite_design(assets.count - 1).as.kite_image;

  tkbc_load_kite_texture_from_kite_image(kite_image, id);
#endif
  return id;
}
