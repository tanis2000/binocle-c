//
// Created by Valerio Santinelli on 02/02/2020.
//

#include "binocle_sprite_wrap.h"
#include "binocle_material_wrap.h"
#include "binocle_subtexture_wrap.h"
#include "binocle_lua.h"
#include "binocle_sprite.h"
#include "binocle_sdl.h"
#include "binocle_gd.h"
#include "binocle_camera.h"
#include "binocle_camera_wrap.h"
#include "binocle_gd_wrap.h"

int l_binocle_sprite_from_material(lua_State *L) {
  l_binocle_material_t *material = luaL_checkudata(L, 1, "binocle_material");
  l_binocle_sprite_t *sprite = lua_newuserdata(L, sizeof(l_binocle_sprite_t));
  lua_getfield(L, LUA_REGISTRYINDEX, "binocle_sprite");
  lua_setmetatable(L, -2);
  SDL_memset(sprite, 0, sizeof(*sprite));
  binocle_sprite *s = binocle_sprite_from_material(material->material);
  sprite->sprite = s;
  return 1;
}

int l_binocle_sprite_draw(lua_State *L) {
  l_binocle_sprite_t *sprite = luaL_checkudata(L, 1, "binocle_sprite");
  l_binocle_gd_t *gd = luaL_checkudata(L, 2, "binocle_gd");
  float player_x = luaL_checknumber(L, 3);
  float player_y = luaL_checknumber(L, 4);
  kmAABB2 **viewport = lua_touserdata(L, 5);
  float rotation = luaL_checknumber(L, 6);
  float scale_x = luaL_checknumber(L, 7);
  float scale_y = luaL_checknumber(L, 8);
  l_binocle_camera_t *camera = luaL_checkudata(L, 9, "binocle_camera");
  kmVec2 scale;
  scale.x = scale_x;
  scale.y = scale_y;
  binocle_sprite_draw(sprite->sprite, gd->gd, player_x, player_y, *viewport, rotation, &scale, camera->camera);
  return 1;
}

int l_binocle_sprite_set_subtexture(lua_State *L) {
  l_binocle_sprite_t *sprite = luaL_checkudata(L, 1, "binocle_sprite");
  l_binocle_subtexture_t *subtexture = luaL_checkudata(L, 2, "binocle_subtexture");
  SDL_memcpy(&sprite->sprite->subtexture, &subtexture->subtexture, sizeof(binocle_subtexture));
  return 1;
}

int l_binocle_sprite_set_origin(lua_State *L) {
  l_binocle_sprite_t *sprite = luaL_checkudata(L, 1, "binocle_sprite");
  float x = luaL_checknumber(L, 2);
  float y = luaL_checknumber(L, 3);
  sprite->sprite->origin.x = x;
  sprite->sprite->origin.y = y;
  return 0;
}

int l_binocle_sprite_destroy(lua_State *L) {
  l_binocle_sprite_t *sprite = luaL_checkudata(L, 1, "binocle_sprite");
  binocle_sprite_destroy(sprite->sprite);
  return 0;
}

static const struct luaL_Reg sprite [] = {
  {"from_material", l_binocle_sprite_from_material},
  {"draw", l_binocle_sprite_draw},
  {"set_subtexture", l_binocle_sprite_set_subtexture},
  {"set_origin", l_binocle_sprite_set_origin},
  {"destroy", l_binocle_sprite_destroy},
  {NULL, NULL}
};

int luaopen_sprite(lua_State *L) {
  luaL_newlib(L, sprite);
  lua_setglobal(L, "sprite");
  luaL_newmetatable(L, "binocle_sprite");
  return 1;
}

int l_binocle_sprite_batch_new(lua_State *L) {
  l_binocle_sprite_batch_t *sprite_batch = lua_newuserdata(L, sizeof(l_binocle_sprite_batch_t));
  lua_getfield(L, LUA_REGISTRYINDEX, "binocle_sprite_batch");
  lua_setmetatable(L, -2);
  SDL_memset(sprite_batch, 0, sizeof(*sprite_batch));
  binocle_sprite_batch sb = binocle_sprite_batch_new();
  sprite_batch->sprite_batch = SDL_malloc(sizeof(binocle_sprite_batch));
  SDL_memcpy(sprite_batch->sprite_batch, &sb, sizeof(binocle_sprite_batch));
  return 1;
}

int l_binocle_sprite_batch_set_gd(lua_State *L) {
  l_binocle_sprite_batch_t *sprite_batch = luaL_checkudata(L, 1, "binocle_sprite_batch");
  l_binocle_gd_t *gd = luaL_checkudata(L, 2, "binocle_gd");
  sprite_batch->sprite_batch->gd = gd->gd;
  return 1;
}

static const struct luaL_Reg sprite_batch [] = {
  {"new", l_binocle_sprite_batch_new},
  {"set_gd", l_binocle_sprite_batch_set_gd},
  {NULL, NULL}
};

int luaopen_sprite_batch(lua_State *L) {
  luaL_newlib(L, sprite_batch);
  lua_setglobal(L, "sprite_batch");
  luaL_newmetatable(L, "binocle_sprite_batch");
  return 1;
}