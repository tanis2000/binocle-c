//
// Created by Valerio Santinelli on 11/04/24.
//

#ifndef GAME_TEMPLATE_BINOCLE_ASSET_H
#define GAME_TEMPLATE_BINOCLE_ASSET_H

#include "binocle_memory.h"


/*
 * Started thinking about how to implement an asset management system.
 * It should be able to load all different kind of assets and provide handles to the assets
 *
 * A plus would be to load the assets in an async manner.
 *
 * In theory, the binocle_assets struct should be part of your global game_state struct. Still not sure it is a good
 * idea, though,
 */

/// A unique handle to identify the asset
typedef uint32_t binocle_asset_handle;

typedef enum binocle_asset_type {
  BINOCLE_ASSET_TYPE_IMAGE,
  BINOCLE_ASSET_TYPE_AUDIO,
} binocle_asset_type;

/// The representation of an asset
typedef struct binocle_asset {
  binocle_asset_handle handle;
  binocle_asset_type asset_type;
} binocle_asset;

/// The representation of the original file
typedef struct binocle_asset_file {
  int dummy;
} binocle_asset_file;

typedef struct binocle_asset_source_file {
  int dummy;
} binocle_asset_source_file;

typedef struct binocle_assets {
  binocle_memory_arena non_restored_memory;

  uint32_t files_capacity;
  uint32_t files_count;
  binocle_asset_file *files;

  uint32_t assets_capacity;
  uint32_t assets_count;
  binocle_asset *assets;
} binocle_assets;

/**
 * \brief Initialize the Assets Manager
 * \return The instance of the Assets Manager
 */
binocle_assets *binocle_assets_new();

binocle_asset_handle binocle_asset_load_image(binocle_assets *assets, const char *filename);

#endif //GAME_TEMPLATE_BINOCLE_ASSET_H
