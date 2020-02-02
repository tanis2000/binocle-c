//
// Created by Valerio Santinelli on 02/02/2020.
//

#include "binocle_sprite_wrap.h"
#include "binocle_material_wrap.h"
#include "binocle_lua.h"
#include "binocle_sprite.h"
#include "binocle_sdl.h"
#include "binocle_gd.h"
#include "binocle_camera.h"

int l_binocle_sprite_from_material(lua_State *L) {
  l_binocle_material_t *material = luaL_checkudata(L, 1, "binocle_material");
  l_binocle_sprite_t *sprite = lua_newuserdata(L, sizeof(l_binocle_sprite_t));
  luaL_setmetatable(L, "binocle_sprite");
  SDL_memset(sprite, 0, sizeof(*sprite));
  binocle_sprite *s = binocle_sprite_from_material(material->material);
  sprite->sprite = s;
  return 1;
}

int l_binocle_sprite_draw(lua_State *L) {
  l_binocle_sprite_t *sprite = luaL_checkudata(L, 1, "binocle_sprite");
  binocle_gd *gd = lua_touserdata(L, 2);
  float player_x = luaL_checknumber(L, 3);
  float player_y = luaL_checknumber(L, 4);
  kmAABB2 *viewport = lua_touserdata(L, 5);
  float rotation = luaL_checknumber(L, 6);
  kmVec2 *scale = lua_touserdata(L, 7);
  binocle_camera *camera = lua_touserdata(L, 8);
  kmVec2 sc;
  sc.x = 1;
  sc.y = 1;
  binocle_sprite_draw(sprite->sprite, gd, player_x, player_y, viewport, rotation, &sc, camera);
  return 1;
}

static const struct luaL_Reg texture [] = {
  {"from_material", l_binocle_sprite_from_material},
  {"draw", l_binocle_sprite_draw},
  {NULL, NULL}
};

int luaopen_sprite(lua_State *L) {
  luaL_newlib(L, texture);
  lua_setglobal(L, "sprite");
  luaL_newmetatable(L, "binocle_sprite");
  return 1;
}