//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_ATLAS_H
#define BINOCLE_ATLAS_H

struct binocle_subtexture;
struct binocle_texture;

/**
 * The format of the atlas. We currently support libGDX and TexturePacker
 */
typedef enum binocle_atlas_file_format {
  BINOCLE_ATLAS_FILE_FORMAT_TEXTUREPACKER,
  BINOCLE_ATLAS_FILE_FORMAT_LIBGDX
} binocle_atlas_file_format;

/**
 * Load a TexturePacker JSON atlas
 * @param filename the JSON file path and name
 * @param texture the corresponding texture that we already loaded
 * @param subtextures an array of subtextures to store those coming from the JSON
 * @param num_subtextures the number of subtextures that have been loaded by this function
 */
void binocle_atlas_load_texturepacker(char *filename, struct binocle_texture *texture,
                                      struct binocle_subtexture *subtextures, int *num_subtextures);

/**
 * Load a libGDX atlas
 * \note Not yet implemented
 * @param filename the JSON file path and name
 * @param texture the corresponding texture that we already loaded
 * @param subtextures an array of subtextures to store those coming from the JSON
 * @param num_subtextures the number of subtextures that have been loaded by this function
 */
void binocle_atlas_load_libgdx(char *filename, struct binocle_texture *texture, struct binocle_subtexture *subtextures,
                               int *num_subtextures);

#endif //BINOCLE_ATLAS_H
