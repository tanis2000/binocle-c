//
// Created by Valerio Santinelli on 02/02/2020.
//

#include "binocle_color_wrap.h"
#include "binocle_lua.h"
#include "binocle_sdl.h"
#include "backend/binocle_color.h"

int l_binocle_color_new(lua_State *L) {
  float r = luaL_checknumber(L, 1);
  float g = luaL_checknumber(L, 2);
  float b = luaL_checknumber(L, 3);
  float a = luaL_checknumber(L, 4);
  l_binocle_color_t *color = lua_newuserdata(L, sizeof(l_binocle_color_t));
  lua_getfield(L, LUA_REGISTRYINDEX, "binocle_color");
  lua_setmetatable(L, -2);
  SDL_memset(color, 0, sizeof(*color));
  binocle_color c = binocle_color_new(r, g, b, a);
  color->color = c;
  return 1;
}


static const struct luaL_Reg window [] = {
  {"new", l_binocle_color_new},
  {NULL, NULL}
};

int luaopen_color(lua_State *L) {
  luaL_newlib(L, window);
  lua_setglobal(L, "color");
  luaL_newmetatable(L, "binocle_color");
  return 1;
}