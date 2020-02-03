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
  binocle_image *b_img = binocle_image_load(filename);
  img->img = b_img;
  return 1;
}

static const struct luaL_Reg image [] = {
  {"load", l_binocle_image_load},
  {NULL, NULL}
};

int luaopen_image(lua_State *L) {
  luaL_newlib(L, image);
  lua_setglobal(L, "image");
  luaL_newmetatable(L, "binocle_image");
  return 1;
}