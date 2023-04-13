//
// Created by Valerio Santinelli on 02/02/2020.
//

#include "binocle_image_wrap.h"
#include "binocle_texture_wrap.h"
#include "binocle_lua.h"
#include "binocle_image.h"
#include "binocle_sdl.h"

int l_binocle_texture_from_image(lua_State *L) {
  l_binocle_image_t *image = luaL_checkudata(L, 1, "binocle_image");
  l_binocle_texture_t *texture = lua_newuserdata(L, sizeof(l_binocle_texture_t));
  lua_getfield(L, LUA_REGISTRYINDEX, "binocle_texture");
  lua_setmetatable(L, -2);
  SDL_memset(texture, 0, sizeof(*texture));
  texture->texture = image->img;
  return 1;
}

int l_binocle_texture_destroy(lua_State *L) {
  l_binocle_texture_t *texture = luaL_checkudata(L, 1, "binocle_texture");
  sg_destroy_image(texture->texture);
  return 1;
}


static const struct luaL_Reg texture [] = {
  {"from_image", l_binocle_texture_from_image},
  {"destroy", l_binocle_texture_destroy},
  {NULL, NULL}
};

int luaopen_texture(lua_State *L) {
  luaL_newlib(L, texture);
  lua_setglobal(L, "texture");
  luaL_newmetatable(L, "binocle_texture");
  lua_pop(L, 1);
  return 1;
}