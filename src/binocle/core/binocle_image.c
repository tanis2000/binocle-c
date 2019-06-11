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

binocle_image binocle_image_load(const char *filename) {
  binocle_image res = {0};

  int width = 0;
  int height = 0;
  int bpp = 0;
  char *buffer;
  size_t size;

  // Flip the y-coordinate because OpenGL expects the 0.0 coordinate on the y-axis to be on the bottom side of the
  // image, but images usually have 0.0 at the top of the y-axis
  stbi_set_flip_vertically_on_load(true);
  // TODO: check the file format
  if (!binocle_sdl_load_binary_file(filename, &buffer, &size)) {
    binocle_log_error("Unable to load image file %s", filename);
    return res;
  }
  res.data = stbi_load_from_memory(buffer, size, &width, &height, &bpp, STBI_rgb_alpha);
  //res.data = stbi_load(filename, &width, &height, &bpp, STBI_rgb_alpha);
  if (res.data == NULL) {
    SDL_Log("Unable to load image %s", filename);
    return res;
  }

  //Get image dimensions
  res.width = width;
  res.height = height;

  binocle_log_info("Texture size: %" PRIu64 "x%" PRIu64", BPP: %d", res.width, res.height, bpp);
  return res;
}
