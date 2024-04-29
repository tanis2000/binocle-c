//
// Created by Valerio Santinelli on 11/04/24.
//

#include "binocle_asset.h"

binocle_assets *binocle_assets_new() {
  binocle_assets *assets = binocle_memory_bootstrap_push_struct(binocle_assets, non_restored_memory, binocle_memory_non_restored_arena_bootstrap_params(), binocle_memory_default_arena_params());
  binocle_memory_arena *arena = &assets->non_restored_memory;

  // Reserve one null asset at the beginning
  binocle_memory_zero_struct(*(assets->assets));
  return assets;
}