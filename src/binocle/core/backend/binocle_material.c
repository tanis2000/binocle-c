//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "binocle_material.h"

binocle_material *binocle_material_new() {
  binocle_material *res = malloc(sizeof(binocle_material));
  memset(res, 0, sizeof(*res));
  res->blend_mode = binocle_blend_new_default();
  res->albedo_texture = NULL;
  res->normal_texture = NULL;
  res->metallic_texture = NULL;
  res->roughness_texture = NULL;
  res->ao_texture = NULL;
  res->shader = NULL;
  return res;
}

void binocle_material_destroy(binocle_material *material) {
  free(material);
  material = NULL;
}