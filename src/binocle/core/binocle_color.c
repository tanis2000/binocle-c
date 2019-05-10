//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_color.h"

binocle_color binocle_color_new(float r, float g, float b, float a) {
  binocle_color res;
  res.r = r;
  res.g = g;
  res.b = b;
  res.a = a;
  return res;
}