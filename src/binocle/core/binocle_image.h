//
// Created by Valerio Santinelli on 19/04/18.
//

#ifndef BINOCLE_IMAGE_H
#define BINOCLE_IMAGE_H

#include "sokol_gfx.h"
#include <stdint.h>
#include "binocle_fs.h"

/**
 * \brief an image
 */
// typedef struct binocle_image {
//   /// The raw data of the image
//   unsigned char *data; // NOTE: this might be better as void*
//   /// The image width
//   uint64_t width;
//   /// The image height
//   uint64_t height;
//   // TODO: might as well add mipmap levels and pixel format
// } binocle_image;

typedef struct binocle_image_load_desc {
  /// the full filename of the image we want to load
  const char *filename;
  /// the filter to use (linear or nearest)
  sg_filter filter;
  sg_wrap wrap;
  binocle_fs_supported fs;
} binocle_image_load_desc;

/**
 * \brief Loads an image file (.png or .jpg) through stb image.
 * It applies linear filtering and edge clamping by default.
 * It uses SDL as the backing filesystem.
 * @param filename the full filename of the image we want to load
 * @return the actual image data
 */
sg_image binocle_image_load(const char *filename);

/**
 * \brief Loads an image file (.png or .jpg) through stb image
 * @param binocle_image_load_desc the descriptor with the data needed to load the image
 * @return the actual image data
 */
sg_image binocle_image_load_with_desc(binocle_image_load_desc *desc);

/**
 * \brief Frees the memory allocated for the image
 * @param image the image to destroy
 */
void binocle_image_destroy(sg_image image);

/**
 * \brief Loads an image from a memory stream
 * @param image_data the memory stream of the image
 * @param width the image width
 * @param height the image height
 * @param filter the filter to use (linear or nearest)
 * @return the image descriptor
 */
sg_image binocle_image_load_from_memory(const unsigned char *image_data,
                                        int width, int height,
                                        sg_filter filter);

#endif // BINOCLE_IMAGE_H
