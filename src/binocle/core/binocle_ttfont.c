//
// Created by Valerio Santinelli on 25/03/22.
//

#include "binocle_ttfont.h"
#include "binocle_sdl.h"
#include "binocle_log.h"
#include "binocle_gd.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype/stb_truetype.h"
#include "backend/binocle_material.h"

binocle_ttfont *binocle_ttfont_new() {
  binocle_ttfont *res = SDL_malloc(sizeof(binocle_ttfont));
  return res;
}

void binocle_ttfont_destroy(binocle_ttfont *font) {
  if (font != NULL) {
    SDL_free(font);
  }
}

binocle_ttfont *binocle_ttfont_from_file(const char *filename, float size, int texture_width, int texture_height, sg_shader shader) {
  binocle_ttfont *font = binocle_ttfont_new();
  font->texture_width = texture_width;
  font->texture_height = texture_height;

  char *buffer = NULL;
  size_t buffer_size = 0;
  bool loaded = binocle_sdl_load_binary_file(filename, &buffer, &buffer_size);

  if (!loaded) {
    binocle_log_error("Cannot open TTF file %s", filename);
    return font;
  }

  unsigned char *tmp_bitmap = (unsigned char*)SDL_malloc(texture_width * texture_height);

  int font_inited = stbtt_BakeFontBitmap(buffer,0, size, tmp_bitmap,texture_width,texture_height, 32,96, font->cdata);
  SDL_free(buffer);

  if (font_inited == 0) {
    binocle_log_error("Cannot initialize TTF %s", filename);
    return font;
  }

  if (font_inited > 0) {
    binocle_log_warning("Only %d characters fit in the texture", font_inited);
  }

  font->bitmap = (unsigned char*)malloc(texture_width * texture_height * 4);
  unsigned char *src = tmp_bitmap;
  unsigned char *dest = font->bitmap;
  size_t tot = texture_width * texture_height;
  for (int i = 0; i < tot; i++)
  {
    *dest++ = *dest++ = *dest++ = 0xFF;
    *dest++ = *src++;
  }
  SDL_free(tmp_bitmap)
  ;
  sg_image_desc desc = {
    .width = texture_width,
    .height = texture_height,
    .pixel_format = SG_PIXELFORMAT_RGBA8,
    .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
    .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
    .wrap_w = SG_WRAP_CLAMP_TO_EDGE,
    .min_filter = SG_FILTER_LINEAR,
    .mag_filter = SG_FILTER_LINEAR,
    .data.subimage[0][0] = {
      .ptr = font->bitmap,
      .size = texture_width * texture_height * 4 // forced to 4bpp as we use the same format all the time no matter what we read from the asset
    }
  };
  font->image = sg_make_image(&desc);
  font->material = binocle_material_new();
  font->material->albedo_texture = font->image;
  font->material->shader = shader;
  return font;
}

void binocle_ttfont_draw_string(binocle_ttfont *font, const char *str, struct binocle_gd *gd,
                                    float x, float y, kmAABB2 viewport, sg_color color, struct binocle_camera *camera) {
  memset(font->vertexes, BINOCLE_MAX_TTF_VERTICES, sizeof(binocle_vpct));
  int index = 0;
  while (*str) {
    if (*str >= 32 && *str < 128) {
      stbtt_aligned_quad q;
      stbtt_GetBakedQuad(font->cdata, font->texture_width,font->texture_height, *str-32, &x,&y,&q,1);//1=opengl & d3d10+,0=d3d9
//      glTexCoord2f(q.s0,q.t0); glVertex2f(q.x0,q.y0);
//      glTexCoord2f(q.s1,q.t0); glVertex2f(q.x1,q.y0);
//      glTexCoord2f(q.s1,q.t1); glVertex2f(q.x1,q.y1);
//      glTexCoord2f(q.s0,q.t1); glVertex2f(q.x0,q.y1);

      // Add a quad for the current character
      kmVec2 pos;
      kmVec2 tex;

      // TL
      pos.x = q.x0;
      pos.y = y-q.y0+y;
      tex.x = q.s0;
      tex.y = q.t0;
      font->vertexes[index] = binocle_vpct_new(pos, color, tex);

      // TR
      pos.x = q.x1;
      pos.y = y-q.y0+y;
      tex.x = q.s1;
      tex.y = q.t0;
      font->vertexes[index + 1] = binocle_vpct_new(pos, color, tex);

      // BL
      pos.x = q.x0;
      pos.y = y-q.y1+y;
      tex.x = q.s0;
      tex.y = q.t1;
      font->vertexes[index + 2] = binocle_vpct_new(pos, color, tex);

      // BL
      pos.x = q.x0;
      pos.y = y-q.y1+y;
      tex.x = q.s0;
      tex.y = q.t1;
      font->vertexes[index + 3] = binocle_vpct_new(pos, color, tex);

      // TR
      pos.x = q.x1;
      pos.y = y-q.y0+y;
      tex.x = q.s1;
      tex.y = q.t0;
      font->vertexes[index + 4] = binocle_vpct_new(pos, color, tex);

      // BR
      pos.x = q.x1;
      pos.y = y-q.y1+y;
      tex.x = q.s1;
      tex.y = q.t1;
      font->vertexes[index + 5] = binocle_vpct_new(pos, color, tex);

      index += 6;
    }
    ++str;
  }
  font->vertexes_count = index;
  binocle_gd_draw(gd, font->vertexes, font->vertexes_count, *font->material, viewport, camera);
}

float binocle_ttfont_get_string_width(binocle_ttfont *font, const char *str) {
  float x = 0;
  float y = 0;
  while (*str) {
    if (*str >= 32 && *str < 128) {
      stbtt_aligned_quad q;
      stbtt_GetBakedQuad(font->cdata, font->texture_width,font->texture_height, *str-32, &x,&y,&q,1);//1=opengl & d3d10+,0=d3d9
    }
    ++str;
  }
  return x;
}