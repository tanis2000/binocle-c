//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_array.h"
#include "binocle_sdl.h"
#include "binocle_math.h"

void *binocle_array_set_capacity_internal(void *arr, uint64_t new_capacity, uint64_t item_size)
{
  uint8_t *p = arr ? (uint8_t *)binocle_array_header(arr) : 0;
  const uint64_t extra = sizeof(binocle_array_header_t);
  const uint64_t size = binocle_array_size(arr);
  const uint64_t bytes_before = arr ? item_size * binocle_array_capacity(arr) + extra : 0;
  const uint64_t bytes_after = new_capacity ? item_size * new_capacity + extra : 0;
  if (p && !binocle_array_capacity(arr)) {
    // Static allocation
    uint8_t *old_p = p;
    p = (uint8_t *)SDL_realloc(0, bytes_after);
    const uint64_t static_bytes = item_size * size + extra;
    const uint64_t to_copy = static_bytes > bytes_after ? bytes_after : static_bytes;
    memcpy(p, old_p, to_copy);
  } else
    p = (uint8_t *)SDL_realloc(p, bytes_after);
  void *new_a = p ? p + extra : p;
  if (new_a) {
    binocle_array_header(new_a)->size = size;
    binocle_array_header(new_a)->capacity = new_capacity;
  }
  return new_a;
}

void *binocle_array_grow_internal(void *arr, uint64_t to_at_least, uint64_t item_size)
{
  const uint64_t capacity = arr ? binocle_array_capacity(arr) : 0;
  if (capacity >= to_at_least)
    return arr;
  const uint64_t min_new_capacity = capacity ? capacity * 2 : 16;
  const uint64_t new_capacity = MAX(min_new_capacity, to_at_least);
  return binocle_array_set_capacity_internal(arr, new_capacity, item_size);
}
