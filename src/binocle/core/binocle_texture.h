//
// Created by Valerio Santinelli on 17/04/18.
//

#ifndef BINOCLE_BINOCLE_TEXTURE_H
#define BINOCLE_BINOCLE_TEXTURE_H

#include "binocle_sdl.h"

typedef struct binocle_texture {
  GLuint tex_id;
  uint64_t width;
  uint64_t height;
} binocle_texture;

binocle_texture binocle_texture_new();
void binocle_texture_from_image(binocle_texture texture, unsigned char *image, uint64_t w, uint64_t h);

#endif //BINOCLE_BINOCLE_TEXTURE_H
