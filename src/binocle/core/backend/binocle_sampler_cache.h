//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_SAMPLER_CACHE_H
#define BINOCLE_SAMPLER_CACHE_H

#include "stdint.h"
#include "binocle_backend_types.h"

/*
    this is used by the Metal and WGPU backends to reduce the
    number of sampler state objects created through the backend API
*/
typedef struct {
  binocle_filter min_filter;
  binocle_filter mag_filter;
  binocle_wrap wrap_u;
  binocle_wrap wrap_v;
  binocle_wrap wrap_w;
  binocle_border_color border_color;
  uint32_t max_anisotropy;
  int min_lod;    /* orig min/max_lod is float, this is int(min/max_lod*1000.0) */
  int max_lod;
  uintptr_t sampler_handle;
} binocle_sampler_cache_item_t;

typedef struct {
  int capacity;
  int num_items;
  binocle_sampler_cache_item_t* items;
} binocle_sampler_cache_t;

void binocle_smpcache_init(binocle_sampler_cache_t* cache, int capacity);
void binocle_smpcache_discard(binocle_sampler_cache_t* cache);
uintptr_t binocle_smpcache_sampler(binocle_sampler_cache_t* cache, int item_index);
int binocle_smpcache_find_item(const binocle_sampler_cache_t* cache, const binocle_image_desc* img_desc);
void binocle_smpcache_add_item(binocle_sampler_cache_t* cache, const binocle_image_desc* img_desc, uintptr_t sampler_handle);

#endif // BINOCLE_SAMPLER_CACHE_H
