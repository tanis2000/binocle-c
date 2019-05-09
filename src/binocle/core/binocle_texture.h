//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_TEXTURE_H
#define BINOCLE_TEXTURE_H

#include "binocle_sdl.h"

struct binocle_image;

typedef struct binocle_texture {
  GLuint tex_id;
  uint64_t width;
  uint64_t height;
} binocle_texture;

binocle_texture binocle_texture_new();
binocle_texture binocle_texture_from_image_data(unsigned char *image, uint64_t w, uint64_t h);
binocle_texture binocle_texture_from_image(struct binocle_image image);

#endif //BINOCLE_TEXTURE_H
