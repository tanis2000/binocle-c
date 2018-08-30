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
  res.origin.x = 0;
  res.origin.y = 0;
  res.material = material;
  res.subtexture = binocle_subtexture_with_texture(material->texture, 0, 0, 1, 1);
  return res;
}

void binocle_sprite_draw(binocle_sprite sprite, binocle_gd *gd, uint64_t x, uint64_t y, kmAABB2 viewport) {
  binocle_vpct vertices[BINOCLE_SPRITE_VERTEX_COUNT];
  vertices[0].pos.x = sprite.origin.x + x;
  vertices[0].pos.y = sprite.origin.y + sprite.material->texture->height + y;
  vertices[0].color = binocle_color_white();
  vertices[0].tex.x = sprite.subtexture.rect.min.x;
  vertices[0].tex.y = sprite.subtexture.rect.max.y;

  vertices[1].pos.x = sprite.origin.x + sprite.material->texture->width + x;
  vertices[1].pos.y = sprite.origin.y + sprite.material->texture->height + y;
  vertices[1].color = binocle_color_white();
  vertices[1].tex.x = sprite.subtexture.rect.max.x;
  vertices[1].tex.y = sprite.subtexture.rect.max.y;

  vertices[2].pos.x = sprite.origin.x + x;
  vertices[2].pos.y = sprite.origin.y + y;
  vertices[2].color = binocle_color_white();
  vertices[2].tex.x = sprite.subtexture.rect.min.x;
  vertices[2].tex.y = sprite.subtexture.rect.min.y;

  vertices[3].pos.x = sprite.origin.x + sprite.material->texture->width + x;
  vertices[3].pos.y = sprite.origin.y + sprite.material->texture->height + y;
  vertices[3].color = binocle_color_white();
  vertices[3].tex.x = sprite.subtexture.rect.max.x;
  vertices[3].tex.y = sprite.subtexture.rect.max.y;

  vertices[4].pos.x = sprite.origin.x + sprite.material->texture->width + x;
  vertices[4].pos.y = sprite.origin.y + y;
  vertices[4].color = binocle_color_white();
  vertices[4].tex.x = sprite.subtexture.rect.max.x;
  vertices[4].tex.y = sprite.subtexture.rect.min.y;

  vertices[5].pos.x = sprite.origin.x + x;
  vertices[5].pos.y = sprite.origin.y + y;
  vertices[5].color = binocle_color_white();
  vertices[5].tex.x = sprite.subtexture.rect.min.x;
  vertices[5].tex.y = sprite.subtexture.rect.min.y;

  binocle_gd_draw(gd, vertices, BINOCLE_SPRITE_VERTEX_COUNT, *sprite.material, viewport);
}