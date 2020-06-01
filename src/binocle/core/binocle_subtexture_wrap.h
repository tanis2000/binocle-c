//
// Created by Valerio Santinelli on 02/02/2020.
//

#ifndef LUA_BINOCLE_SUBTEXTURE_WRAP_H
#define LUA_BINOCLE_SUBTEXTURE_WRAP_H

#include "binocle_lua.h"

typedef struct l_binocle_subtexture_t {
  struct binocle_subtexture *subtexture;
} l_binocle_subtexture_t;

int luaopen_subtexture(lua_State *L);

#endif //LUA_BINOCLE_SUBTEXTURE_WRAP_H
