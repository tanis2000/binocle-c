//
// Created by Valerio Santinelli on 24/04/18.
//

#include "binocle_bitmapfont.h"
#include "binocle_sdl.h"
#include "binocle_log.h"
#include "binocle_vpct.h"
#include "binocle_gd.h"
#include "binocle_color.h"

binocle_bitmapfont binocle_bitmapfont_new() {
  binocle_bitmapfont res = {0};
}

int binocle_bitmapfont_split (const char *str, char c, char ***arr)
{
  int count = 1;
  int token_len = 1;
  int i = 0;
  const char *p;
  char *t;

  p = str;
  while (*p != '\0')
  {
    if (*p == c)
      count++;
    p++;
  }

  *arr = (char**) malloc(sizeof(char*) * count);
  if (*arr == NULL)
    exit(1);

  p = str;
  while (*p != '\0')
  {
    if (*p == c)
    {
      (*arr)[i] = (char*) malloc( sizeof(char) * token_len );
      if ((*arr)[i] == NULL)
        exit(1);

      token_len = 0;
      i++;
    }
    p++;
    token_len++;
  }
  (*arr)[i] = (char*) malloc( sizeof(char) * token_len );
  if ((*arr)[i] == NULL)
    exit(1);

  i = 0;
  p = str;
  t = ((*arr)[i]);
  while (*p != '\0')
  {
    if (*p != c && *p != '\0')
    {
      *t = *p;
      t++;
    }
    else
    {
      *t = '\0';
      i++;
      t = ((*arr)[i]);
    }
    p++;
  }

  return count;
}

bool binocle_bitmapfont_string_starts_with(const char *prefix, const char *str) {
  return strncmp(prefix, str, strlen(prefix)) == 0;
}

void binocle_bitmapfont_parse_character(binocle_bitmapfont *font, const char *line) {
  int count = 0;
  char **components = NULL;
  count = binocle_bitmapfont_split(line, '=', &components);
  int charId = atoi(components[1]);
  font->characters[charId].x = atoi(components[2]);
  font->characters[charId].y = atoi(components[3]);
  font->characters[charId].width = atoi(components[4]);
  font->characters[charId].height = atoi(components[5]);
  font->characters[charId].x_offset = atoi(components[6]);
  font->characters[charId].y_offset = atoi(components[7]);
  font->characters[charId].x_advance = atoi(components[8]);
}

void binocle_bitmapfont_parse_kerning_entry(binocle_bitmapfont *font, const char *line) {
  int count = 0;
  char **components = NULL;
  count = binocle_bitmapfont_split(line, '=', &components);

  int firstChar = atoi(components[1]);
  int secondChar = atoi(components[2]);
  font->kerning[firstChar][secondChar] = atoi(components[3]);
}

void binocle_bitmapfont_parse_common_line(binocle_bitmapfont *font, const char *line, bool flip) {
  int count = 0;
  char **components = NULL;
  count = binocle_bitmapfont_split(line, '=', &components);

  font->line_height = atoi(components[1]);
  font->scale_w = atoi(components[3]);
  font->scale_h = atoi(components[4]);
  font->flip = flip;
}

binocle_bitmapfont binocle_bitmapfont_from_file(const char *filename, bool flip) {
  binocle_bitmapfont font = binocle_bitmapfont_new();

  SDL_RWops *file = SDL_RWFromFile(filename, "rb");
  if (file == NULL) {
    binocle_log_error("Cannot open BitmapFont file %s", filename);
    return font;
  }

  Sint64 res_size = SDL_RWsize(file);
  char *res = (char *) malloc(res_size + 1);

  Sint64 nb_read_total = 0, nb_read = 1;
  char *buf = res;
  while (nb_read_total < res_size && nb_read != 0) {
    nb_read = SDL_RWread(file, buf, 1, (res_size - nb_read_total));
    nb_read_total += nb_read;
    buf += nb_read;
  }
  SDL_RWclose(file);
  if (nb_read_total != res_size) {
    binocle_log_error("Size mismatch");
    free(res);
    return font;
  }

  res[nb_read_total] = '\0';

  char **lines = NULL;
  int count = 0;
  count = binocle_bitmapfont_split(res, '\n', &lines);
  char *line;
  for (size_t i = 0; i < count; i++) {
    line = lines[i];
    // ignore these lines
    if (binocle_bitmapfont_string_starts_with("chars c", line)) continue;
    // ignore this - we don't need it
    if (binocle_bitmapfont_string_starts_with("kernings count", line)) continue;
    // this is a character definition
    if (binocle_bitmapfont_string_starts_with("char", line)) {
      binocle_bitmapfont_parse_character(&font, line);
      continue;
    }
    // kerning info
    if (binocle_bitmapfont_string_starts_with("kerning first", line)) {
      binocle_bitmapfont_parse_kerning_entry(&font, line);
    }
    if (binocle_bitmapfont_string_starts_with("common", line)) {
      binocle_bitmapfont_parse_common_line(&font, line, flip);
    }
  }

  free(res);
  return font;
}

binocle_bitmapfont_square_t binocle_bitmapfont_make_square_t(float x1, float y1, float x2, float y2, float tx1, float ty1, float tx2, float ty2) {
  binocle_bitmapfont_square_t res = BINOCLE_MAKE_SQUARE_T(x1, y1, x2, y2, tx1, ty1, tx2, ty2);
  return res;
}

void binocle_bitmapfont_create_vertice_and_tex_coords_for_string(binocle_bitmapfont *font, const char *str, float height,
    kmMat4 transformation_matrix, binocle_color color) {
  memset(font->vertexes, BINOCLE_MAX_FONT_VERTICES, sizeof(binocle_vpct));
  float scale = height / (float) font->line_height;
  binocle_bitmapfont_square_t *char_squares = (binocle_bitmapfont_square_t *) malloc(sizeof(binocle_bitmapfont_square_t) * strlen(str));
  int index = 0;
  float x = 0;
  int y = 0;
  font->vertexes_count = 0;
  for (size_t i = 0; i < strlen(str); i++) {
    int c = str[i];
    if (i > 0) {
      x += scale * ((float) font->kerning[c][str[i - 1]]);
    }
    const binocle_bitmapfont_character *cdef = &font->characters[c];

    binocle_bitmapfont_square_t tmp;

    if (font->flip) {
      // Flipped version
      tmp = binocle_bitmapfont_make_square_t(
          x + cdef->x_offset * scale,
          y + scale * (cdef->y_offset),
          x + scale * (cdef->x_offset + cdef->width),
          y + scale * (cdef->y_offset + cdef->height),
          (float) cdef->x / (float) font->scale_w,
          (float) (font->scale_h - cdef->y - cdef->height) / (float) font->scale_h,
          (float) (cdef->x + cdef->width) / (float) font->scale_w,
          (float) (font->scale_h - cdef->y) / (float) font->scale_h
      );
    } else {
      // Not flipped version (good if we're loading without setting stbi to flip the image)
      tmp = binocle_bitmapfont_make_square_t(
          x + cdef->x_offset * scale,
          y + scale * (cdef->y_offset),
          x + scale * (cdef->x_offset + cdef->width),
          y + scale * (cdef->y_offset + cdef->height),
          (float) cdef->x / (float) font->scale_w,
          (float) cdef->y / (float) font->scale_h,
          (float) (cdef->x + cdef->width) / (float) font->scale_w,
          (float) (cdef->y + cdef->height) / (float) font->scale_h);
    }

    char_squares[i] = tmp;
    x += scale * (float) cdef->x_advance;


    // TODO: do this for each vertex!
    //glm::vec2.Transform( ref p, ref _transformationMatrix, out p );
    kmVec4 p = { tmp.vtlx, tmp.vtly, 0, 1};
    kmVec4 tl;
    kmVec4MultiplyMat4(&tl, &p, &transformation_matrix);
    p.x = tmp.vtrx;
    p.y = tmp.vtry;
    kmVec4 tr;
    kmVec4MultiplyMat4(&tr, &p, &transformation_matrix);
    p.x = tmp.vblx;
    p.y = tmp.vbly;
    kmVec4 bl;
    kmVec4MultiplyMat4(&bl, &p, &transformation_matrix);
    p.x = tmp.vbrx;
    p.y = tmp.vbry;
    kmVec4 br;
    kmVec4MultiplyMat4(&br, &p, &transformation_matrix);

    // Add a quad for the current character
    kmVec2 pos;
    kmVec2 tex;

    pos.x = tl.x;
    pos.y = tl.y;
    tex.x = tmp.ttlx;
    tex.y = tmp.ttly;
    font->vertexes[index] = binocle_vpct_new(pos, color, tex);

    pos.x = tr.x;
    pos.y = tr.y;
    tex.x = tmp.ttrx;
    tex.y = tmp.ttry;
    font->vertexes[index+1] = binocle_vpct_new(pos, color, tex);

    pos.x = bl.x;
    pos.y = bl.y;
    tex.x = tmp.tblx;
    tex.y = tmp.tbly;
    font->vertexes[index+2] = binocle_vpct_new(pos, color, tex);

    pos.x = bl.x;
    pos.y = bl.y;
    tex.x = tmp.tblx;
    tex.y = tmp.tbly;
    font->vertexes[index+3] = binocle_vpct_new(pos, color, tex);

    pos.x = tr.x;
    pos.y = tr.y;
    tex.x = tmp.ttrx;
    tex.y = tmp.ttry;
    font->vertexes[index+4] = binocle_vpct_new(pos, color, tex);

    pos.x = br.x;
    pos.y = br.y;
    tex.x = tmp.tbrx;
    tex.y = tmp.tbry;
    font->vertexes[index+5] = binocle_vpct_new(pos, color, tex);

    index += 6;
  }
  font->vertexes_count = index;
  free(char_squares);
}

float binocle_bitmapfont_get_width_of_string(binocle_bitmapfont font, const char *str, float height) {
  int x = 0;
  for (size_t i = 0; i < strlen(str); i++) {
    int c = str[i];
    if (i > 0) {
      x += font.kerning[c][str[i - 1]];
    }
    const binocle_bitmapfont_character *cdef = &font.characters[c];
    // special handling for last character
    if (i < strlen(str) - 1) {
      x += cdef->x_advance;
    } else {
      x += cdef->x_offset + cdef->width;
    }
  }
  float scale = height / (float) font.line_height;
  return scale * x;
}

void binocle_bitmapfont_draw_string(binocle_bitmapfont font, const char *str, float height, binocle_gd *gd, uint64_t x, uint64_t y, kmAABB2 viewport, binocle_color color) {
  kmMat4 transformation_matrix;
  kmMat4Identity(&transformation_matrix);
  kmMat4Translation(&transformation_matrix, x, y, 0);
  binocle_bitmapfont_create_vertice_and_tex_coords_for_string(&font, str, height, transformation_matrix, color);
  binocle_gd_draw(gd, font.vertexes, font.vertexes_count, *font.material, viewport);

}