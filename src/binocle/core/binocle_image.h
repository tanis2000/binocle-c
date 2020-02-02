//
// Created by Valerio Santinelli on 19/04/18.
//

#ifndef BINOCLE_IMAGE_H
#define BINOCLE_IMAGE_H

#include <stdint.h>

/**
 * \brief an image
 */
typedef struct binocle_image {
  /// The raw data of the image
  unsigned char *data; // NOTE: this might be better as void*
  /// The image width
  uint64_t width;
  /// The image height
  uint64_t height;
  // TODO: might as well add mipmap levels and pixel format
} binocle_image;

/**
 * \brief Loads an image file (.png or .jpg) through stb image
 * @param filename the full filename of the image we want to load
 * @return the actual image data
 */
binocle_image *binocle_image_load(const char *filename);

#endif //BINOCLE_IMAGE_H
