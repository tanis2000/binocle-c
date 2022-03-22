//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_blend.h"
#include "sokol_gfx.h"

sg_blend_state binocle_blend_new(sg_blend_factor color_source_factor, sg_blend_factor color_destination_factor,
                                 sg_blend_op color_blend_equation, sg_blend_factor alpha_source_factor,
                                 sg_blend_factor alpha_destination_factor,
                                 sg_blend_op alpha_blend_equation) {
  sg_blend_state res = {
      .src_factor_rgb = color_source_factor,
      .dst_factor_rgb = color_destination_factor,
      .op_rgb = color_blend_equation,
      .src_factor_alpha = alpha_source_factor,
      .dst_factor_alpha = alpha_destination_factor,
      .op_alpha = alpha_blend_equation
  };
  return res;
}

sg_blend_state binocle_blend_new_simple(sg_blend_factor source_factor, sg_blend_factor destination_factor,
                                        sg_blend_op blend_equation) {
  sg_blend_state res = {
      .src_factor_rgb = source_factor,
      .dst_factor_rgb = destination_factor,
      .op_rgb = blend_equation,
      .src_factor_alpha = source_factor,
      .dst_factor_alpha = destination_factor,
      .op_alpha = blend_equation
  };
  return res;
}

sg_blend_state binocle_blend_new_default() {
  sg_blend_state res = {
      .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
      .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
      .op_rgb = SG_BLENDOP_ADD,
      .src_factor_alpha = SG_BLENDFACTOR_ONE,
      .dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
      .op_alpha = SG_BLENDOP_ADD
  };
  return res;
}

