//
// Created by Valerio Santinelli on 19/04/18.
//

#ifndef BINOCLE_IMAGE_H
#define BINOCLE_IMAGE_H

#include <stdint.h>

typedef struct binocle_image {
  unsigned char *data; // NOTE: this might be better as void*
  uint64_t width;
  uint64_t height;
  // TODO: might as well add mipmap levels and pixel format
} binocle_image;

binocle_image binocle_image_load(const char *filename);

#endif //BINOCLE_IMAGE_H
