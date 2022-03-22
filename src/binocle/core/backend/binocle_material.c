//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "binocle_material.h"
#include "binocle_blend.h"

binocle_material *binocle_material_new() {
  binocle_material *res = malloc(sizeof(binocle_material));
  memset(res, 0, sizeof(*res));
  res->blend_mode = binocle_blend_new_default();
  res->albedo_texture.id = 0;
  res->normal_texture.id = 0;
  res->metallic_texture.id = 0;
  res->roughness_texture.id = 0;
  res->ao_texture.id = 0;
  res->shader.id = 0;
  return res;
}

void binocle_material_destroy(binocle_material *material) {
  free(material);
  material = NULL;
}