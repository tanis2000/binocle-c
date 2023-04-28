//
// Created by Valerio Santinelli on 02/02/2020.
//

#include "binocle_image_wrap.h"
#include "binocle_lua.h"
#include "binocle_image.h"
#include "binocle_sdl.h"

int l_binocle_image_load(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  l_binocle_image_t *img = lua_newuserdata(L, sizeof(l_binocle_image_t));
  lua_getfield(L, LUA_REGISTRYINDEX, "binocle_image");
  lua_setmetatable(L, -2);
  SDL_memset(img, 0, sizeof(*img));
  sg_image b_img = binocle_image_load(filename);
  img->img = b_img;
  return 1;
}

int l_binocle_image_load_from_assets(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  l_binocle_image_t *img = lua_newuserdata(L, sizeof(l_binocle_image_t));
  lua_getfield(L, LUA_REGISTRYINDEX, "binocle_image");
  lua_setmetatable(L, -2);
  SDL_memset(img, 0, sizeof(*img));
  binocle_image_load_desc desc = {
    .filename = filename,
    .filter = SG_FILTER_LINEAR,
    .wrap = SG_WRAP_CLAMP_TO_EDGE,
    .fs = BINOCLE_FS_PHYSFS,
  };
  sg_image b_img = binocle_image_load_with_desc(&desc);
  img->img = b_img;
  return 1;
}

int l_binocle_image_get_info(lua_State *L) {
  l_binocle_image_t *img = luaL_checkudata(L, 1, "binocle_image");
  sg_image_info info = sg_query_image_info(img->img);
  lua_pushnumber(L, info.width);
  lua_pushnumber(L, info.height);
  return 2;
}

static const struct luaL_Reg image [] = {
  {"load", l_binocle_image_load},
  {"load_from_assets", l_binocle_image_load_from_assets},
  {"get_info", l_binocle_image_get_info},
  {NULL, NULL}
};

int luaopen_image(lua_State *L) {
  luaL_newlib(L, image);
  lua_setglobal(L, "image");
  luaL_newmetatable(L, "binocle_image");
  lua_pop(L, 1);
  return 1;
}