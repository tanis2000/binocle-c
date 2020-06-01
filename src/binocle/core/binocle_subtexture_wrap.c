//
// Created by Valerio Santinelli on 02/02/2020.
//

#include "binocle_subtexture_wrap.h"
#include "binocle_texture_wrap.h"
#include "binocle_material_wrap.h"
#include "binocle_lua.h"
#include "binocle_subtexture.h"
#include "binocle_sdl.h"
#include "binocle_gd.h"
#include "binocle_camera.h"

int l_binocle_subtexture_with_texture(lua_State *L) {
  l_binocle_texture_t *texture = luaL_checkudata(L, 1, "binocle_texture");
  float x = luaL_checknumber(L, 2);
  float y = luaL_checknumber(L, 3);
  float width = luaL_checknumber(L, 4);
  float height = luaL_checknumber(L, 5);
  l_binocle_subtexture_t *subtexture = lua_newuserdata(L, sizeof(l_binocle_subtexture_t));
  lua_getfield(L, LUA_REGISTRYINDEX, "binocle_subtexture");
  lua_setmetatable(L, -2);
  SDL_memset(subtexture, 0, sizeof(*subtexture));
  binocle_subtexture s = binocle_subtexture_with_texture(texture->texture, x, y, width, height);
  subtexture->subtexture = SDL_malloc(sizeof(binocle_subtexture));
  SDL_memcpy(subtexture->subtexture, &s, sizeof(binocle_subtexture));
  return 1;
}

static const struct luaL_Reg subtexture [] = {
  {"subtexture_with_texture", l_binocle_subtexture_with_texture},
  {NULL, NULL}
};

int luaopen_subtexture(lua_State *L) {
  luaL_newlib(L, subtexture);
  lua_setglobal(L, "subtexture");
  luaL_newmetatable(L, "binocle_subtexture");
  return 1;
}