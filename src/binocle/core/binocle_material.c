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
  res->texture = NULL;
  res->shader = NULL;
  return res;
}