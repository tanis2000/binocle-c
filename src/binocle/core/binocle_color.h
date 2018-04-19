//
// Created by Valerio Santinelli on 13/04/18.
//

#ifndef BINOCLE_BINOCLE_COLOR_H
#define BINOCLE_BINOCLE_COLOR_H

typedef struct binocle_color {
  //! Red, green, blue, alpha (translucency)
  float r;
  float g;
  float b;
  float a;
} binocle_color;

static binocle_color binocle_color_white() {
  binocle_color res = {};
  res.r = 1;
  res.g = 1;
  res.b = 1;
  res.a = 1;
  return res;
}

static binocle_color binocle_color_azure() {
  binocle_color res = {};
  res.r = 191.0f/255.0f;
  res.g = 1;
  res.b = 1;
  res.a = 1;
  return res;
}

#endif //BINOCLE_BINOCLE_COLOR_H
