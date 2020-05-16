//
// Created by Valerio Santinelli on 02/02/2020.
//

#ifndef LUA_BINOCLE_INPUT_WRAP_H
#define LUA_BINOCLE_INPUT_WRAP_H

#include "binocle_lua.h"

typedef struct l_binocle_input_t {
  struct binocle_input *input;
} l_binocle_input_t;

int luaopen_input(lua_State *L);

#endif //LUA_BINOCLE_INPUT_WRAP_H
