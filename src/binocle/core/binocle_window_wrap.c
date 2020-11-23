//
// Created by Valerio Santinelli on 02/02/2020.
//

#include "binocle_window_wrap.h"
#include "binocle_lua.h"
#include "binocle_window.h"
#include "binocle_color_wrap.h"

int l_binocle_window_set_background_color(lua_State *L) {
  l_binocle_window_t *window = luaL_checkudata(L, 1, "binocle_window");
  l_binocle_color_t *color = luaL_checkudata(L, 2, "binocle_color");
  binocle_window_set_background_color(window->window, color->color);
  return 0;
}

int l_binocle_window_set_minimum_size(lua_State *L) {
  l_binocle_window_t *window = luaL_checkudata(L, 1, "binocle_window");
  int width = luaL_checkint(L, 2);
  int height = luaL_checkint(L, 3);
  binocle_window_set_minimum_size(window->window, width, height);
  return 0;
}

int l_binocle_window_new(lua_State *L) {
  int width = luaL_checkint(L, 1);
  int height = luaL_checkint(L, 2);
  const char *title = luaL_checkstring(L, 3);
  l_binocle_window_t *window = lua_newuserdata(L, sizeof(l_binocle_window_t));
  lua_getfield(L, LUA_REGISTRYINDEX, "binocle_window");
  lua_setmetatable(L, -2);
  SDL_memset(window, 0, sizeof(*window));
  binocle_window *win = binocle_window_new(width, height, title);
  window->window = win;
  return 1;
}


static const struct luaL_Reg window [] = {
  {"new", l_binocle_window_new},
  {"set_background_color", l_binocle_window_set_background_color},
  {"set_minimum_size", l_binocle_window_set_minimum_size},
  {NULL, NULL}
};

int luaopen_window(lua_State *L) {
  luaL_newlib(L, window);
  lua_setglobal(L, "window");
  luaL_newmetatable(L, "binocle_window");
  return 1;
}