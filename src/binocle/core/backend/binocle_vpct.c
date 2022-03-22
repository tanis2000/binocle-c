//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_vpct.h"

binocle_vpct binocle_vpct_new(kmVec2 pos, sg_color color, kmVec2 tex) {
  binocle_vpct res = {
      .pos = pos,
      .color = color,
      .tex = tex
  };
  return res;
}