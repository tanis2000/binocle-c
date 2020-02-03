//
// Created by Valerio Santinelli on 02/02/2020.
//

#include "binocle_shader_wrap.h"
#include "binocle_lua.h"
#include "binocle_shader.h"
#include "binocle_sdl.h"

int l_binocle_shader_load_from_file(lua_State *L) {
  const char *vert_filename = luaL_checkstring(L, 1);
  const char *frag_filename = luaL_checkstring(L, 2);
  l_binocle_shader_t *shader = lua_newuserdata(L, sizeof(l_binocle_shader_t));
  lua_getfield(L, LUA_REGISTRYINDEX, "binocle_shader");
  lua_setmetatable(L, -2);
  SDL_memset(shader, 0, sizeof(*shader));
  binocle_shader *sh =  binocle_shader_load_from_file(vert_filename, frag_filename);
  shader->shader = sh;
  return 1;
}

static const struct luaL_Reg image [] = {
  {"load_from_file", l_binocle_shader_load_from_file},
  {NULL, NULL}
};

int luaopen_shader(lua_State *L) {
  luaL_newlib(L, image);
  lua_setglobal(L, "shader");
  luaL_newmetatable(L, "binocle_shader");
  return 1;
}