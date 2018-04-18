//
// Created by Valerio Santinelli on 17/04/18.
//

#include "binocle_texture.h"

binocle_texture binocle_texture_new() {
  binocle_texture res = {
    .tex_id = 0,
    .width = 0,
    .height = 0
  };
}

void binocle_texture_from_image(binocle_texture texture, unsigned char *image, uint64_t w, uint64_t h) {
  texture.width = w;
  texture.height = h;
  glGenTextures(1, &texture.tex_id);
  glBindTexture(GL_TEXTURE_2D, texture.tex_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
}

