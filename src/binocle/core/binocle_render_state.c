//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_render_state.h"
#include "backend/binocle_texture.h"
#include "backend/binocle_shader.h"

binocle_render_state binocle_render_state_new(binocle_texture *texture, binocle_shader *shader) {
  binocle_render_state res = {0};
  res.blend_mode = binocle_blend_new_default();
  kmMat4Identity(&res.transform);
  res.texture = texture;
  res.shader = shader;
  return res;
}