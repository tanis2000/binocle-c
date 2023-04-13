//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include <inttypes.h>
#include "binocle_image.h"
#include "binocle_sdl.h" // needed to define Uint16 and Uint8 when using emscripten's port of stb_image

#if !defined(__EMSCRIPTEN__)
#define STB_IMAGE_IMPLEMENTATION
#endif

#include "stb_image.h"

#include "binocle_sdl.h"
#include "binocle_log.h"

sg_image binocle_image_load_with_filter(const char *filename, sg_filter filter) {
//  binocle_image *res = malloc(sizeof(binocle_image));
//  memset(res, 0, sizeof(*res));

  int width = 0;
  int height = 0;
  int bpp = 0;
  char *buffer;
  size_t size;
  sg_image img = { 0 };

#if defined(BINOCLE_GL) || defined(BINOCLE_METAL)
  // Flip the y-coordinate because OpenGL expects the 0.0 coordinate on the y-axis to be on the bottom side of the
  // image, but images usually have 0.0 at the top of the y-axis
  stbi_set_flip_vertically_on_load(true);
#endif
  // TODO: check the file format
  if (!binocle_sdl_load_binary_file(filename, &buffer, &size)) {
    binocle_log_error("Unable to load image file %s", filename);
    return img;
  }
  unsigned char *data = stbi_load_from_memory(buffer, (int)size, &width, &height, &bpp, STBI_rgb_alpha);
  if (data == NULL) {
    SDL_Log("Unable to load image %s", filename);
    return img;
  }

  //Get image dimensions
//  res->width = width;
//  res->height = height;

  binocle_log_info("Texture size: %" PRId32 "x%" PRId32", BPP: %d", width, height, bpp);
  sg_image_desc desc = {
    .width = width,
    .height = height,
    .pixel_format = SG_PIXELFORMAT_RGBA8,
    .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
    .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
    .wrap_w = SG_WRAP_CLAMP_TO_EDGE,
    .min_filter = filter,
    .mag_filter = filter,
    .data.subimage[0][0] = {
      .ptr = data,
      .size = width * height * 4 // forced to 4bpp as we use the same format all the time no matter what we read from the asset
    }
  };
  img = sg_make_image(&desc);
  stbi_image_free(data);
//  free(buffer); // if this is commented out, we leak memory. But on Windows it crashes if you uncomment this line.
  return img;
}

sg_image binocle_image_load(const char *filename) {
  return binocle_image_load_with_filter(filename, SG_FILTER_LINEAR);
}

sg_image binocle_image_load_from_memory(const unsigned char *image_data, int width, int height, sg_filter filter) {
  sg_image img = { 0 };

  binocle_log_info("Texture size: %" PRId32 "x%" PRId32, width, height);
  sg_image_desc desc = {
    .width = width,
    .height = height,
    .pixel_format = SG_PIXELFORMAT_RGBA8,
    .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
    .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
    .wrap_w = SG_WRAP_CLAMP_TO_EDGE,
    .min_filter = filter,
    .mag_filter = filter,
    .data.subimage[0][0] = {
      .ptr = image_data,
      .size = width * height * sizeof(uint32_t)
    },
    .label = "nuklear-font-image"
  };
  img = sg_make_image(&desc);
  return img;
}

void binocle_image_destroy(sg_image image) {
  sg_destroy_image(image);
}