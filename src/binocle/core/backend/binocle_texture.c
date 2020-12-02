//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include <stdlib.h>
#include <string.h>
#include "binocle_texture.h"
//#include "../binocle_image.h"
//#include "binocle_backend_gl.h"
//
//binocle_texture *binocle_texture_new() {
//  binocle_texture *res = malloc(sizeof(binocle_texture));
//  memset(res, 0, sizeof(*res));
//  return res;
//}
//
//binocle_texture *binocle_texture_from_image_data(unsigned char *image, uint64_t w, uint64_t h) {
//  binocle_texture *texture = binocle_texture_new();
//  texture->width = w;
//  texture->height = h;
//#if defined(__IPHONEOS__) || defined(__EMSCRIPTEN__) || defined(__ANDROID__)
//  glCheck(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
//  glCheck(glPixelStorei(GL_PACK_ALIGNMENT, 1) );
//#else
//  glCheck(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
//  glCheck(glPixelStorei(GL_PACK_LSB_FIRST, 1));
//#endif
//  glCheck(glGenTextures(1, &texture->tex_id));
//  glCheck(glBindTexture(GL_TEXTURE_2D, texture->tex_id));
//  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
//  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
//  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
//  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
//  glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image));
//  return texture;
//}
//
//binocle_texture *binocle_texture_from_image(binocle_image *image) {
//  binocle_texture *texture = binocle_texture_new();
//
//  if (image->data != NULL) {
//    texture = binocle_texture_from_image_data(image->data, image->width, image->height);
//  } else {
//    SDL_Log("Cannot create texture, image.data is NULL");
//  }
//  return texture;
//}
//
//void binocle_texture_destroy(binocle_texture *texture) {
//  free(texture);
//  texture = NULL;
//}