//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_blend.h"

binocle_blend binocle_blend_new(binocle_blend_factor color_source_factor, binocle_blend_factor color_destination_factor,
                                binocle_blend_equation color_blend_equation, binocle_blend_factor alpha_source_factor,
                                binocle_blend_factor alpha_destination_factor,
                                binocle_blend_equation alpha_blend_equation) {
  binocle_blend res = {
      .color_src_factor = color_source_factor,
      .color_dst_factor = color_destination_factor,
      .color_equation = color_blend_equation,
      .alpha_src_factor = alpha_source_factor,
      .alpha_dst_factor = alpha_destination_factor,
      .alpha_equation = alpha_blend_equation
  };
  return res;
}

binocle_blend binocle_blend_new_simple(binocle_blend_factor source_factor, binocle_blend_factor destination_factor,
                                       binocle_blend_equation blend_equation) {
  binocle_blend res = {
      .color_src_factor = source_factor,
      .color_dst_factor = destination_factor,
      .color_equation = blend_equation,
      .alpha_src_factor = source_factor,
      .alpha_dst_factor = destination_factor,
      .alpha_equation = blend_equation
  };
  return res;
}

binocle_blend binocle_blend_new_default() {
  binocle_blend res = {
      .color_src_factor = BINOCLE_BLEND_SRCALPHA,
      .color_dst_factor = BINOCLE_BLEND_ONEMINUSSRCALPHA,
      .color_equation = BINOCLE_BLEND_ADD,
      .alpha_src_factor = BINOCLE_BLEND_ONE,
      .alpha_dst_factor = BINOCLE_BLEND_ONEMINUSSRCALPHA,
      .alpha_equation = BINOCLE_BLEND_ADD
  };
  return res;
}

