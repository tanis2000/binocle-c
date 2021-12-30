//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include <stdint.h>
#include "binocle_subtexture.h"

binocle_subtexture binocle_subtexture_new() {
  binocle_subtexture res = {
      .texture = 0,
      .rect = {.min.x = 0, .min.y = 0, .max.x = 0, .max.y = 0}
  };
  return res;
}

binocle_subtexture
binocle_subtexture_with_texture(struct binocle_image *texture, float x, float y, float width, float height) {
  binocle_subtexture res = {
      .texture = texture,
      .rect = {.min.x = x, .min.y = y, .max.x = width, .max.y = height}
  };
  return res;
}

binocle_subtexture
binocle_subtexture_with_subtexture(binocle_subtexture *subtexture, float x, float y, float width, float height) {
  binocle_subtexture res = {
      .texture = subtexture->texture,
      .rect = {.min.x = x, .min.y = y, .max.x = width, .max.y = height}
  };
  return res;
}

uint64_t binocle_subtexture_get_x(binocle_subtexture subtexture) {
  return (uint64_t) subtexture.rect.min.x;
}

uint64_t binocle_subtexture_get_y(binocle_subtexture subtexture) {
  return (uint64_t) subtexture.rect.min.y;
}

uint64_t binocle_subtexture_get_width(binocle_subtexture *subtexture) {
  return (uint64_t) subtexture->rect.max.x;
}

uint64_t binocle_subtexture_get_height(binocle_subtexture *subtexture) {
  return (uint64_t) subtexture->rect.max.y;
}

kmVec2 binocle_subtexture_get_size(binocle_subtexture *subtexture) {
  kmVec2 res = {
      .x = binocle_subtexture_get_width(subtexture),
      .y = binocle_subtexture_get_height(subtexture)
  };
  return res;
}

kmVec2 binocle_subtexture_get_center(binocle_subtexture *subtexture) {
  kmVec2 res = {
      .x = binocle_subtexture_get_size(subtexture).x * 0.5f,
      .y = binocle_subtexture_get_size(subtexture).y * 0.5f
  };
  return res;
}

kmAABB2 binocle_subtexture_get_frame(binocle_subtexture subtexture, uint64_t index, uint64_t frame_width,
                                     uint64_t frame_height) {
  uint64_t x = index * frame_width;
  uint64_t y = (uint64_t) ((x / subtexture.rect.max.x) * frame_height);
  x %= (uint64_t) subtexture.rect.max.x;
  kmAABB2 res = {
      .min.x = binocle_subtexture_get_x(subtexture) + x,
      .min.y = binocle_subtexture_get_y(subtexture) + y,
      .max.x = frame_width,
      .max.y = frame_height
  };
  return res;
}

kmAABB2 binocle_subtexture_get_absolute_clip_rect(binocle_subtexture subtexture, kmAABB2 relative_clip_rect) {
  kmAABB2 res = {
      .min.x = relative_clip_rect.min.x + subtexture.rect.min.x,
      .min.y = relative_clip_rect.min.y + subtexture.rect.min.y,
      relative_clip_rect.max.x,
      relative_clip_rect.max.y
  };
  return res;
}
