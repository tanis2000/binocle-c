//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_VPCT_H
#define BINOCLE_VPCT_H

#include "../binocle_math.h"
#include "binocle_color.h"

/**
 * The vertex representation as position, color and texture coordinates
 */
typedef struct binocle_vpct {
  kmVec2 pos;
  binocle_color color;
  kmVec2 tex;
} binocle_vpct;

/**
 * \brief Creates a new vertex
 * @param pos the position
 * @param color the color
 * @param tex the texture coordinates
 * @return the vertex
 */
binocle_vpct binocle_vpct_new(kmVec2 pos, binocle_color color, kmVec2 tex);

typedef struct binocle_vpctn {
  kmVec3 pos;
  binocle_color color;
  kmVec2 tex;
  kmVec3 normal;
} binocle_vpctn;

binocle_vpctn binocle_vpctn_new(kmVec3 pos, binocle_color color, kmVec2 tex, kmVec3 normal);

#endif //BINOCLE_VPCT_H
