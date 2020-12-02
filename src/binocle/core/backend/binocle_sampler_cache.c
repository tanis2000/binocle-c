//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_sampler_cache.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#define binocle_clamp(v,v0,v1) ((v<v0)?(v0):((v>v1)?(v1):(v)))

void binocle_smpcache_init(binocle_sampler_cache_t* cache, int capacity) {
  assert(cache && (capacity > 0));
  memset(cache, 0, sizeof(binocle_sampler_cache_t));
  cache->capacity = capacity;
  const int size = cache->capacity * sizeof(binocle_sampler_cache_item_t);
  cache->items = (binocle_sampler_cache_item_t*) malloc(size);
  memset(cache->items, 0, size);
}

void binocle_smpcache_discard(binocle_sampler_cache_t* cache) {
  assert(cache && cache->items);
  free(cache->items);
  cache->items = 0;
  cache->num_items = 0;
  cache->capacity = 0;
}

int binocle_smpcache_minlod_int(float min_lod) {
  return (int) (min_lod * 1000.0f);
}

int binocle_smpcache_maxlod_int(float max_lod) {
  return (int) (binocle_clamp(max_lod, 0.0f, 1000.0f) * 1000.0f);
}

int binocle_smpcache_find_item(const binocle_sampler_cache_t* cache, const binocle_image_desc* img_desc) {
  /* return matching sampler cache item index or -1 */
  assert(cache && cache->items);
  assert(img_desc);
  const int min_lod = binocle_smpcache_minlod_int(img_desc->min_lod);
  const int max_lod = binocle_smpcache_maxlod_int(img_desc->max_lod);
  for (int i = 0; i < cache->num_items; i++) {
    const binocle_sampler_cache_item_t* item = &cache->items[i];
    if ((img_desc->min_filter == item->min_filter) &&
        (img_desc->mag_filter == item->mag_filter) &&
        (img_desc->wrap_u == item->wrap_u) &&
        (img_desc->wrap_v == item->wrap_v) &&
        (img_desc->wrap_w == item->wrap_w) &&
        (img_desc->max_anisotropy == item->max_anisotropy) &&
        (img_desc->border_color == item->border_color) &&
        (min_lod == item->min_lod) &&
        (max_lod == item->max_lod))
    {
      return i;
    }
  }
  /* fallthrough: no matching cache item found */
  return -1;
}

void binocle_smpcache_add_item(binocle_sampler_cache_t* cache, const binocle_image_desc* img_desc, uintptr_t sampler_handle) {
  assert(cache && cache->items);
  assert(img_desc);
  assert(cache->num_items < cache->capacity);
  const int item_index = cache->num_items++;
  binocle_sampler_cache_item_t* item = &cache->items[item_index];
  item->min_filter = img_desc->min_filter;
  item->mag_filter = img_desc->mag_filter;
  item->wrap_u = img_desc->wrap_u;
  item->wrap_v = img_desc->wrap_v;
  item->wrap_w = img_desc->wrap_w;
  item->border_color = img_desc->border_color;
  item->max_anisotropy = img_desc->max_anisotropy;
  item->min_lod = binocle_smpcache_minlod_int(img_desc->min_lod);
  item->max_lod = binocle_smpcache_maxlod_int(img_desc->max_lod);
  item->sampler_handle = sampler_handle;
}

uintptr_t binocle_smpcache_sampler(binocle_sampler_cache_t* cache, int item_index) {
  assert(cache && cache->items);
  assert((item_index >= 0) && (item_index < cache->num_items));
  return cache->items[item_index].sampler_handle;
}