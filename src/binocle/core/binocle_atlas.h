//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_ATLAS_H
#define BINOCLE_ATLAS_H

#include <stdbool.h>

struct binocle_subtexture;
struct sg_image;

/**
 * The format of the atlas. We currently support libGDX and TexturePacker
 */
typedef enum binocle_atlas_file_format {
  BINOCLE_ATLAS_FILE_FORMAT_TEXTUREPACKER,
  BINOCLE_ATLAS_FILE_FORMAT_LIBGDX
} binocle_atlas_file_format;

typedef struct binocle_atlas_animation {
  const char *name;
  const char *direction;
  int from;
  int to;
} binocle_atlas_animation;

/**
 * Load a TexturePacker JSON atlas
 * @param filename the JSON file path and name
 * @param texture the corresponding texture that we already loaded
 * @param subtextures an array of subtextures to store those coming from the JSON
 * @param num_subtextures the number of subtextures that have been loaded by this function
 * @param include_animations if true, animations will be read and set in `animations` and `num_animations`
 * @param animations a pointer to a non-initialized binocle_atlas_animation struct. This function will allocate the memory needed and retain ownership. Call binocle_atlas_animation_destroy to free its usage. If this is set to NULL, animations will be ignored.
 * @param num_animations a pointer to a size_t value that will be filled in by this function with the number of animations.
 */
void binocle_atlas_load_texturepacker(char *filename, struct sg_image *texture,
                                      struct binocle_subtexture *subtextures, int *num_subtextures,
                                      bool include_animations,
                                      binocle_atlas_animation **animations, size_t *num_animations);

/**
 * Load a libGDX atlas
 * \note Not yet implemented
 * @param filename the JSON file path and name
 * @param texture the corresponding texture that we already loaded
 * @param subtextures an array of subtextures to store those coming from the JSON
 * @param num_subtextures the number of subtextures that have been loaded by this function
 */
void binocle_atlas_load_libgdx(char *filename, struct sg_image *texture, struct binocle_subtexture *subtextures,
                               int *num_subtextures);

/**
 * Frees the memory allocated while loading animations
 * @param animations a pointer to the binocle_atlas_animation struct to free
 * @param num_animations the number of animations previously loaded
 */
void binocle_atlas_animation_destroy(binocle_atlas_animation *animations, size_t num_animations);

#endif //BINOCLE_ATLAS_H
