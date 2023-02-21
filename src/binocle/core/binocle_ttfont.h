//
// Created by Valerio Santinelli on 25/03/22.
//

#ifndef BINOCLE_TTFONT_H
#define BINOCLE_TTFONT_H

#include "backend/binocle_vpct.h"
#include "stb_truetype/stb_truetype.h"

#define BINOCLE_MAX_TTF_CHARACTERS 256
#define BINOCLE_MAX_TTF_VERTICES 65535

struct binocle_camera;
struct binocle_gd;

typedef struct binocle_ttfont {
  stbtt_bakedchar cdata[96];
  unsigned char*bitmap;
  sg_image image;
  int texture_width;
  int texture_height;
  binocle_vpct vertexes[BINOCLE_MAX_TTF_VERTICES];
  size_t vertexes_count;
  struct binocle_material *material;
} binocle_ttfont;

binocle_ttfont *binocle_ttfont_new();

void binocle_ttfont_destroy(binocle_ttfont *font);

binocle_ttfont *binocle_ttfont_from_file(const char *filename, float size, int texture_width, int texture_height, sg_shader shader);

void binocle_ttfont_draw_string(binocle_ttfont *font, const char *str, struct binocle_gd *gd,
                                float x, float y, kmAABB2 viewport, sg_color color, struct binocle_camera *camera, float depth);

float binocle_ttfont_get_string_width(binocle_ttfont *font, const char *str);

#endif //BINOCLE_TTFONT_H
