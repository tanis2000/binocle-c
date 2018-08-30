//
// Created by Valerio Santinelli on 19/04/18.
//

#ifndef BINOCLE_BINOCLE_VPCT_H
#define BINOCLE_BINOCLE_VPCT_H

#include "binocle_math.h"
#include "binocle_color.h"

typedef struct binocle_vpct {
    kmVec2 pos;
    binocle_color color;
    kmVec2 tex;
} binocle_vpct;

binocle_vpct binocle_vpct_new(kmVec2 pos, binocle_color color, kmVec2 tex);

#endif //BINOCLE_BINOCLE_VPCT_H