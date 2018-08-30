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
  binocle_color res = {.r = 1, .g = 1, .b = 1, .a = 1};
  return res;
}

static binocle_color binocle_color_azure() {
  binocle_color res = { .r = 191.0f / 255.0f,.g = 1, .b = 1, .a = 1 };
  return res;
}

static binocle_color binocle_color_black() {
  binocle_color res = {.r = 0, .g = 0, .b = 0, .a = 1};
  return res;
}

#endif //BINOCLE_BINOCLE_COLOR_H
