//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_VPCT_H
#define BINOCLE_VPCT_H

#include "binocle_math.h"
#include "binocle_color.h"

typedef struct binocle_vpct {
  kmVec2 pos;
  binocle_color color;
  kmVec2 tex;
} binocle_vpct;

binocle_vpct binocle_vpct_new(kmVec2 pos, binocle_color color, kmVec2 tex);

#endif //BINOCLE_VPCT_H
