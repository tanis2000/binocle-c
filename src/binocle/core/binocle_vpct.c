//
// Created by Valerio Santinelli on 19/04/18.
//

#include "binocle_vpct.h"

binocle_vpct binocle_vpct_new(kmVec2 pos, binocle_color color, kmVec2 tex) {
    binocle_vpct res = {
        .pos = pos,
        .color = color,
        .tex = tex
    };
    return res;
}