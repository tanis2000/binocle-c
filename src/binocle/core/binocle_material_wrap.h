//
// Created by Valerio Santinelli on 02/02/2020.
//

#ifndef LUA_BINOCLE_MATERIAL_WRAP_H
#define LUA_BINOCLE_MATERIAL_WRAP_H

#include "binocle_lua.h"

typedef struct l_binocle_material_t {
  struct binocle_material *material;
} l_binocle_material_t;

int luaopen_material(lua_State *L);

#endif //LUA_BINOCLE_MATERIAL_WRAP_H
