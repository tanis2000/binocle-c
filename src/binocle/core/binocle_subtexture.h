//
// Created by Valerio Santinelli on 17/04/18.
//

#ifndef BINOCLE_BINOCLE_SUBTEXTURE_H
#define BINOCLE_BINOCLE_SUBTEXTURE_H

#include <kazmath/kazmath.h>
#include <stdint.h>

struct binocle_texture;

typedef struct binocle_subtexture {
  struct binocle_texture *texture;
  kmAABB2 rect;
} binocle_subtexture;

binocle_subtexture binocle_subtexture_with_texture(struct binocle_texture *texture, uint64_t x, uint64_t y, uint64_t width, uint64_t height);

#endif //BINOCLE_BINOCLE_SUBTEXTURE_H
