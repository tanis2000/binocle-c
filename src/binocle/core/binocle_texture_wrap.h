//
// Created by Valerio Santinelli on 02/02/2020.
//

#ifndef LUA_BINOCLE_TEXTURE_WRAP_H
#define LUA_BINOCLE_TEXTURE_WRAP_H

#include "binocle_lua.h"

typedef struct l_binocle_texture_t {
  struct binocle_texture *texture;
} l_binocle_texture_t;

int luaopen_texture(lua_State *L);

#endif //LUA_BINOCLE_TEXTURE_WRAP_H
