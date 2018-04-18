//
// Created by Valerio Santinelli on 17/04/18.
//

#ifndef BINOCLE_BINOCLE_SUBTEXTURE_H
#define BINOCLE_BINOCLE_SUBTEXTURE_H

#include <kazmath.h>
struct binocle_texture;

typedef struct binocle_subtexture {
  struct binocle_texture *texture;
  kmAABB2 rect;
} binocle_subtexture;

#endif //BINOCLE_BINOCLE_SUBTEXTURE_H
