//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_SUBTEXTURE_H
#define BINOCLE_SUBTEXTURE_H

#include <kazmath/kazmath.h>
#include <stdint.h>

struct binocle_texture;

typedef struct binocle_subtexture {
  struct binocle_texture *texture;
  kmAABB2 rect;
  char name[1024];
} binocle_subtexture;

binocle_subtexture binocle_subtexture_with_texture(struct binocle_texture *texture, float x, float y, float width, float height);
binocle_subtexture binocle_subtexture_with_subtexture(binocle_subtexture *subtexture, float x, float y, float width, float height);
uint64_t binocle_subtexture_get_x(binocle_subtexture subtexture);
uint64_t binocle_subtexture_get_y(binocle_subtexture subtexture);
uint64_t binocle_subtexture_get_width(binocle_subtexture *subtexture);
uint64_t binocle_subtexture_get_height(binocle_subtexture *subtexture);
kmVec2 binocle_subtexture_get_size(binocle_subtexture *subtexture);
kmVec2 binocle_subtexture_get_center(binocle_subtexture *subtexture);
kmAABB2 binocle_subtexture_get_frame(binocle_subtexture subtexture, uint64_t index, uint64_t frame_width, uint64_t frame_height);
kmAABB2 binocle_subtexture_get_absolute_clip_rect(binocle_subtexture subtexture, kmAABB2 relative_clip_rect);

#endif //BINOCLE_SUBTEXTURE_H
