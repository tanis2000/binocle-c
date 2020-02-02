//
// Created by Valerio Santinelli on 02/02/2020.
//

#ifndef LUA_BINOCLE_SHADER_WRAP_H
#define LUA_BINOCLE_SHADER_WRAP_H

#include "binocle_lua.h"

typedef struct l_binocle_shader_t {
  struct binocle_shader *shader;
} l_binocle_shader_t;

int luaopen_shader(lua_State *L);

#endif //LUA_BINOCLE_SHADER_WRAP_H
