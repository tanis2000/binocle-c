//
// Created by Valerio Santinelli on 02/02/2020.
//

#include "binocle_lua.h"
#include "binocle_sdl.h"

int l_binocle_sdl_assets_dir(lua_State *L) {
  char *assets_dir = binocle_sdl_assets_dir();
  lua_pushstring(L, assets_dir);
  return 1;
}

int l_binocle_sdl_get_last_modification_time(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  time_t time = binocle_sdl_get_last_modification_time(filename);
  lua_pushnumber(L, time);
  return 1;
}

int l_binocle_sdl_load_text_file(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  char *buffer = NULL;
  size_t buffer_size;
  bool res = binocle_sdl_load_text_file(filename, &buffer, &buffer_size);
  lua_pushstring(L, buffer);
  // TODO: keep track of the buffer to free it when done
  return 1;
}
static const struct luaL_Reg sdl [] = {
  {"assets_dir", l_binocle_sdl_assets_dir},
  {"get_last_modification_time", l_binocle_sdl_get_last_modification_time},
  {"load_text_file", l_binocle_sdl_load_text_file},
  {NULL, NULL}
};

int luaopen_sdl(lua_State *L) {
  luaL_newlib(L, sdl);
  lua_setglobal(L, "sdl");
  return 1;
}