//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_ATLAS_H
#define BINOCLE_ATLAS_H

#include <stdbool.h>
#include <stdlib.h>
#include "kazmath/kazmath.h"

#define BINOCLE_MAX_ATLAS_FILENAME_LENGTH (1024)

struct binocle_sprite;
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
  bool repeat;
} binocle_atlas_animation;

typedef struct binocle_atlas_slice_key {
  int frame;
  kmAABB2 bounds;
  kmVec2 pivot;
} binocle_atlas_slice_key;

typedef struct binocle_atlas_slice {
  const char *name;
  const char *color;
  binocle_atlas_slice_key *keys;
  size_t num_keys;
} binocle_atlas_slice;

typedef struct binocle_atlas_tp_meta {
  char *app;
  char *version;
  char *image;
  char *format;
  char *scale;
  char *smartupdate;
  kmVec2 size;
  binocle_atlas_animation *frame_tags;
  size_t num_frame_tags;
  binocle_atlas_slice *slices;
  size_t num_slices;
} binocle_atlas_tp_meta;

typedef struct binocle_atlas_tp_frame {
  char *filename;
  kmAABB2 frame;
  bool rotated;
  bool trimmed;
  kmAABB2 sprite_source_size;
  kmVec2 source_size;
  kmVec2 pivot;
  int duration;
} binocle_atlas_tp_frame;

typedef struct binocle_atlas_texturepacker {
  char asset_filename[BINOCLE_MAX_ATLAS_FILENAME_LENGTH];
  binocle_atlas_tp_frame *frames;
  size_t num_frames;
  binocle_atlas_tp_meta meta;
} binocle_atlas_texturepacker;

/**
 * Load a TexturePacker JSON atlas
 * @param filename the JSON file path and name
 * @return an instance of binocle_atlas_texturepacker with the data of the atlas
 */
bool binocle_atlas_load_texturepacker(char *filename, binocle_atlas_texturepacker *atlas);

/**
 * Creates the subtexttures from the frames of an already loaded TexturePacker JSON atlas.
 * @param atlas an instance of binocle_atlas_texturepacker with the data of the atlas
 * @param texture the corresponding texture associated to this atlas. It has to be already loaded.
 * @param subtextures an array of subtextures to store those coming from the JSON
 * @param num_subtextures the number of subtextures that have been loaded by this function
 */
void binocle_atlas_texturepacker_create_subtextures(binocle_atlas_texturepacker *atlas, struct sg_image *texture,
                                                    struct binocle_subtexture *subtextures, int *num_subtextures);

/**
 * Frees the resources of the atlas
 * @param atlas an instance of binocle_atlas_texturepacker to release
 */
void binocle_atlas_destroy_texturepacker(binocle_atlas_texturepacker *atlas);

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

/**
 * Creates the animations in a given sprite based on the content of the atlas
 * @param atlas the TexturePacker atlas
 * @param texture the texture with the image of the atlas
 * @param sprite the sprite instance whose animations have to be created
 */
void binocle_atlas_texturepacker_create_animations(
  binocle_atlas_texturepacker *atlas,
  struct sg_image texture,
  struct binocle_sprite *sprite
);

#endif //BINOCLE_ATLAS_H
