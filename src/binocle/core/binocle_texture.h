//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_TEXTURE_H
#define BINOCLE_TEXTURE_H

#include "binocle_sdl.h"

struct binocle_image;

/**
 * A texture representation
 */
typedef struct binocle_texture {
  GLuint tex_id;
  uint64_t width;
  uint64_t height;
} binocle_texture;

/**
 * \brief Creates a new texture
 * @return the new texture
 */
binocle_texture *binocle_texture_new();

/**
 * \brief Creates a texture from image data and size
 * @param image the pointer to the image data
 * @param w the width of the image
 * @param h the height of the image
 * @return the texture
 */
binocle_texture *binocle_texture_from_image_data(unsigned char *image, uint64_t w, uint64_t h);

/**
 * \brief Creates a texture from a Binocle image
 * @param image the image
 * @return the texture
 */
binocle_texture *binocle_texture_from_image(struct binocle_image *image);

#endif //BINOCLE_TEXTURE_H
