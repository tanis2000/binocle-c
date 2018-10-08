//
// Created by Valerio Santinelli on 24/04/18.
//

#ifndef BINOCLE_BINOCLE_BITMAPFONT_H
#define BINOCLE_BINOCLE_BITMAPFONT_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <kazmath/kazmath.h>
#include "binocle_vpct.h"
#include "binocle_material.h"

#define BINOCLE_MAX_CHARACTERS 256
#define BINOCLE_MAX_FONT_VERTICES 65535

#define BINOCLE_MAKE_SQUARE_T(x1, y1, x2, y2, tx1, ty1, tx2, ty2) { \
(x1),(y1), (tx1),(ty1), \
(x1),(y2), (tx1),(ty2), \
(x2),(y2), (tx2),(ty2), \
(x2),(y1), (tx2),(ty1) \
}

struct binocle_texture;
struct binocle_gd;
struct binocle_color;

typedef struct binocle_bitmapfont_square_t{
  float vtlx, vtly, ttlx, ttly, vblx, vbly, tblx, tbly, vbrx, vbry, tbrx, tbry, vtrx, vtry, ttrx, ttry;
} binocle_bitmapfont_square_t;

typedef struct binocle_bitmapfont_character {
  // X location on the spritesheet
  int x;
  // Y location on the spritesheet
  int y;
  // Width of the character image
  int width;
  // Height of the character image
  int height;
  // The X amount the image should be offset when drawing the image
  int x_offset;
  // The Y amount the image should be offset when drawing the image
  int y_offset;
  // The amount to move the current position after drawing the character
  int x_advance;
} binocle_bitmapfont_character;

typedef struct binocle_bitmapfont {
  binocle_bitmapfont_character characters[BINOCLE_MAX_CHARACTERS];
  int kerning[BINOCLE_MAX_CHARACTERS][BINOCLE_MAX_CHARACTERS];
  int scale_w;
  int scale_h;
  int line_height;
  binocle_vpct vertexes[BINOCLE_MAX_FONT_VERTICES];
  size_t vertexes_count;
  struct binocle_material *material;
  bool flip;
} binocle_bitmapfont;

binocle_bitmapfont binocle_bitmapfont_new();
int binocle_bitmapfont_split (const char *str, char c, char ***arr);
bool binocle_bitmapfont_string_starts_with(const char *prefix, const char *str);
void binocle_bitmapfont_parse_character(binocle_bitmapfont *font, const char *line);
void binocle_bitmapfont_parse_kerning_entry(binocle_bitmapfont *font, const char *line);
void binocle_bitmapfont_parse_common_line(binocle_bitmapfont *font, const char *line, bool flip);
binocle_bitmapfont binocle_bitmapfont_from_file(const char *filename, bool flip);
float binocle_bitmapfont_get_width_of_string(binocle_bitmapfont font, const char *str, float height);
void binocle_bitmapfont_create_vertice_and_tex_coords_for_string(binocle_bitmapfont *font, const char *str, float height,
                                                                 kmMat4 transformation_matrix, binocle_color color);
void binocle_bitmapfont_draw_string(binocle_bitmapfont font, const char *str, float height, struct binocle_gd *gd, uint64_t x, uint64_t y, kmAABB2 viewport, binocle_color color, kmMat4 view_matrix);
binocle_bitmapfont_square_t binocle_bitmapfont_make_square_t(float x1, float y1, float x2, float y2, float tx1, float ty1, float tx2, float ty2);

#endif //BINOCLE_BINOCLE_BITMAPFONT_H
