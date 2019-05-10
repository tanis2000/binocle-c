//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_COLOR_H
#define BINOCLE_COLOR_H

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
  binocle_color res = {.r = 191.0f / 255.0f, .g = 1, .b = 1, .a = 1};
  return res;
}

static binocle_color binocle_color_black() {
  binocle_color res = {.r = 0, .g = 0, .b = 0, .a = 1};
  return res;
}

/**
 * \brief Gets a new color
 * This is the struct used to represent colors all over the engine
 * @param r Red [0..1]
 * @param g Green [0..1]
 * @param b Blue [0..1]
 * @param a Alpha [0..1]
 * @return The color
 */
binocle_color binocle_color_new(float r, float g, float b, float a);

#endif //BINOCLE_COLOR_H
