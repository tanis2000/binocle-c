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


typedef struct binocle_asset {
  int dummy;
} binocle_asset;

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

#endif //GAME_TEMPLATE_BINOCLE_ASSET_H
