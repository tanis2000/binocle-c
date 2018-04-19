//
// Created by Valerio Santinelli on 17/04/18.
//

#include "binocle_texture.h"
#include "binocle_image.h"
#include "binocle_gd.h"

binocle_texture binocle_texture_new() {
  binocle_texture res = {
    .tex_id = 0,
    .width = 0,
    .height = 0
  };
  return res;
}

binocle_texture binocle_texture_from_image_data(unsigned char *image, uint64_t w, uint64_t h) {
  binocle_texture texture = binocle_texture_new();
  texture.width = w;
  texture.height = h;
  glCheck(glGenTextures(1, &texture.tex_id));
  glCheck(glBindTexture(GL_TEXTURE_2D, texture.tex_id));
  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
  glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image));
  return texture;
}

binocle_texture binocle_texture_from_image(binocle_image image) {
  binocle_texture texture = binocle_texture_new();

  if (image.data != NULL) {
    texture = binocle_texture_from_image_data(image.data, image.width, image.height);
  } else {
    SDL_Log("Cannot create texture");
  }
  return texture;
}