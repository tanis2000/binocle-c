//
// Created by Valerio Santinelli on 02/02/2020.
//

#include "binocle_lua.h"
#include "binocle_fs.h"

int l_binocle_fs_get_last_modification_time(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  uint64_t time = 0;
  bool res = binocle_fs_get_last_modification_time(filename, &time);
  lua_pushnumber(L, (double)time);
  return 1;
}

int l_binocle_fs_load_text_file(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  char *buffer = NULL;
  size_t buffer_size;
  bool res = binocle_fs_load_text_file(filename, &buffer, &buffer_size);
  lua_pushstring(L, buffer);
  // TODO: keep track of the buffer to free it when done
  return 1;
}
static const struct luaL_Reg fs [] = {
  {"get_last_modification_time", l_binocle_fs_get_last_modification_time},
  {"load_text_file", l_binocle_fs_load_text_file},
  {NULL, NULL}
};

int luaopen_fs(lua_State *L) {
  luaL_newlib(L, fs);
  lua_setglobal(L, "fs");
//  lua_pop(L, 1);
  return 1;
}