//
// Created by Valerio Santinelli on 2019-05-13.
//

#include "binocle_render_state.h"
#include "binocle_texture.h"
#include "binocle_shader.h"

binocle_render_state binocle_render_state_new(binocle_texture *texture, binocle_shader *shader) {
  binocle_render_state res = {0};
  res.blend_mode = binocle_blend_new_default();
  kmMat4Identity(&res.transform);
  res.texture = texture;
  res.shader = shader;
  return res;
}