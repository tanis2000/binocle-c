//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include <inttypes.h>
#include "binocle_image.h"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

#include "binocle_sdl.h"
#include "binocle_log.h"

binocle_image binocle_image_load_with_filter(const char *filename, binocle_filter filter) {
//  binocle_image *res = malloc(sizeof(binocle_image));
//  memset(res, 0, sizeof(*res));

  int width = 0;
  int height = 0;
  int bpp = 0;
  char *buffer;
  size_t size;
  binocle_image img = { 0 };

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
  unsigned char *data = stbi_load_from_memory(buffer, size, &width, &height, &bpp, STBI_rgb_alpha);
  if (data == NULL) {
    SDL_Log("Unable to load image %s", filename);
    return img;
  }

  //Get image dimensions
//  res->width = width;
//  res->height = height;

  binocle_log_info("Texture size: %" PRIu64 "x%" PRIu64", BPP: %d", width, height, bpp);
  binocle_image_desc desc = {
    .width = width,
    .height = height,
    .pixel_format = BINOCLE_PIXELFORMAT_RGBA8,
    .min_filter = filter,
    .mag_filter = filter,
    .data.subimage[0][0] = {
      .ptr = data,
      .size = width * height * 4 // forced to 4bpp as we use the same format all the time no matter what we read from the asset
    }
  };
  img = binocle_backend_make_image(&desc);
  stbi_image_free(data);
//  free(buffer); // if this is commented out, we leak memory. But on Windows it crashes if you uncomment this line.
  return img;
}

binocle_image binocle_image_load(const char *filename) {
  return binocle_image_load_with_filter(filename, BINOCLE_FILTER_LINEAR);
}

void binocle_image_destroy(binocle_image image) {
  binocle_backend_destroy_image(image);
}