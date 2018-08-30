//
// Created by Valerio Santinelli on 19/04/18.
//

#include "binocle_sprite.h"
#include "binocle_texture.h"
#include "binocle_subtexture.h"
#include "binocle_material.h"
#include "binocle_gd.h"
#include "binocle_vpct.h"

#define BINOCLE_SPRITE_VERTEX_COUNT 6

binocle_sprite binocle_sprite_from_material(binocle_material *material) {
  binocle_sprite res = {0};
  // Default origin to bottom-left
  res.origin.x = 0;
  res.origin.y = 0;
  res.material = material;
  // Default to use the whole texture
  res.subtexture = binocle_subtexture_with_texture(material->texture, 0, 0, 1, 1);
  return res;
}

void binocle_sprite_draw(binocle_sprite sprite, binocle_gd *gd, int64_t x, int64_t y, kmAABB2 viewport, float rotation, float scale) {
  binocle_vpct vertices[BINOCLE_SPRITE_VERTEX_COUNT];
  // TL
  vertices[0].pos.x = -sprite.origin.x * scale * cosf(rotation) - (-sprite.origin.y * scale + sprite.material->texture->height * scale) * sinf(rotation) + x;
  vertices[0].pos.y = (-sprite.origin.y * scale + sprite.material->texture->height * scale) * cosf(rotation) - sprite.origin.x * scale * sinf(rotation) + y;
  vertices[0].color = binocle_color_white();
  vertices[0].tex.x = sprite.subtexture.rect.min.x;
  vertices[0].tex.y = sprite.subtexture.rect.max.y;
  // TR
  vertices[1].pos.x = (-sprite.origin.x * scale + sprite.material->texture->width * scale) * cosf(rotation) - (-sprite.origin.y * scale + sprite.material->texture->height * scale) * sinf(rotation) + x;
  vertices[1].pos.y = (-sprite.origin.y * scale + sprite.material->texture->height * scale) * cosf(rotation) + (-sprite.origin.x * scale + sprite.material->texture->width * scale) * sinf(rotation) + y;
  vertices[1].color = binocle_color_white();
  vertices[1].tex.x = sprite.subtexture.rect.max.x;
  vertices[1].tex.y = sprite.subtexture.rect.max.y;
  // BL
  vertices[2].pos.x = -sprite.origin.x * scale * cosf(rotation) + sprite.origin.y * scale * sinf(rotation) + x;
  vertices[2].pos.y = -sprite.origin.y * scale * cosf(rotation) - sprite.origin.x * scale * sinf(rotation) + y;
  vertices[2].color = binocle_color_white();
  vertices[2].tex.x = sprite.subtexture.rect.min.x;
  vertices[2].tex.y = sprite.subtexture.rect.min.y;
  // TR
  vertices[3].pos.x = (-sprite.origin.x * scale + sprite.material->texture->width * scale) * cosf(rotation) - (-sprite.origin.y * scale + sprite.material->texture->height * scale) * sinf(rotation) + x;
  vertices[3].pos.y = (-sprite.origin.y * scale + sprite.material->texture->height * scale) * cosf(rotation) + (-sprite.origin.x * scale + sprite.material->texture->width * scale) * sinf(rotation) + y;
  vertices[3].color = binocle_color_white();
  vertices[3].tex.x = sprite.subtexture.rect.max.x;
  vertices[3].tex.y = sprite.subtexture.rect.max.y;
  // BR
  vertices[4].pos.x = (-sprite.origin.x * scale + sprite.material->texture->width * scale) * cosf(rotation) + sprite.origin.y * scale * sinf(rotation) + x;
  vertices[4].pos.y = -sprite.origin.y * scale * cosf(rotation) + (-sprite.origin.x * scale + sprite.material->texture->width * scale) * sinf(rotation) + y;
  vertices[4].color = binocle_color_white();
  vertices[4].tex.x = sprite.subtexture.rect.max.x;
  vertices[4].tex.y = sprite.subtexture.rect.min.y;
  // BL
  vertices[5].pos.x = -sprite.origin.x * scale * cosf(rotation) + sprite.origin.y * scale * sinf(rotation) + x;
  vertices[5].pos.y = -sprite.origin.y * scale * cosf(rotation) - sprite.origin.x * scale * sinf(rotation) + y;
  vertices[5].color = binocle_color_white();
  vertices[5].tex.x = sprite.subtexture.rect.min.x;
  vertices[5].tex.y = sprite.subtexture.rect.min.y;

  binocle_gd_draw(gd, vertices, BINOCLE_SPRITE_VERTEX_COUNT, *sprite.material, viewport);
}