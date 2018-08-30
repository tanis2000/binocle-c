//
// Created by Valerio Santinelli on 19/04/18.
//

#ifndef BINOCLE_BINOCLE_SPRITE_H
#define BINOCLE_BINOCLE_SPRITE_H

#include <kazmath/kazmath.h>
#include "binocle_subtexture.h"

struct binocle_material;
struct binocle_gd;

typedef struct binocle_sprite {
  binocle_subtexture subtexture;
  struct binocle_material *material;
  kmVec2 origin;
} binocle_sprite;

binocle_sprite binocle_sprite_from_material(struct binocle_material *material);
void binocle_sprite_draw(binocle_sprite sprite, struct binocle_gd *gd, int64_t x, int64_t y, kmAABB2 viewport, float rotation, float scale);

#endif //BINOCLE_BINOCLE_SPRITE_H
