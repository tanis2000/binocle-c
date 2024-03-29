//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_render_state.h"
#include "backend/binocle_blend.h"

binocle_render_state binocle_render_state_new(sg_image *texture, sg_shader *shader) {
  binocle_render_state res = {0};
  res.blend_mode = binocle_blend_new_default();
  kmMat4Identity(&res.transform);
  res.texture = texture;
  res.shader = shader;
  return res;
}