//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_COLOR_H
#define BINOCLE_COLOR_H

#include "sokol_gfx.h"

static sg_color binocle_color_white() {
  sg_color res = {.r = 1, .g = 1, .b = 1, .a = 1};
  return res;
}

static sg_color binocle_color_azure() {
  sg_color res = {.r = 191.0f / 255.0f, .g = 1, .b = 1, .a = 1};
  return res;
}

static sg_color binocle_color_black() {
  sg_color res = {.r = 0, .g = 0, .b = 0, .a = 1};
  return res;
}

static sg_color binocle_color_green() {
  sg_color res = {.r = 0, .g = 1, .b = 0, .a = 1};
  return res;
}

static sg_color binocle_color_green_translucent() {
  sg_color res = {.r = 0, .g = 1, .b = 0, .a = 0.5f};
  return res;
}

static sg_color binocle_color_red() {
  sg_color res = {.r = 1, .g = 0, .b = 0, .a = 1};
  return res;
}

static sg_color binocle_color_red_translucent() {
  sg_color res = {.r = 1, .g = 0, .b = 0, .a = 0.5f};
  return res;
}

static sg_color binocle_color_blue() {
  sg_color res = {.r = 0, .g = 0, .b = 1, .a = 1};
  return res;
}

static sg_color binocle_color_orange() {
  sg_color res = {.r = 1, .g = 0.5f, .b = 0, .a = 1};
  return res;
}

static sg_color binocle_color_yellow() {
  sg_color res = {.r = 1, .g = 1, .b = 0, .a = 1};
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
sg_color binocle_color_new(float r, float g, float b, float a);

#endif //BINOCLE_COLOR_H
