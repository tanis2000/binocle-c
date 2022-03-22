//
// Created by Valerio Santinelli on 02/02/2020.
//

#ifndef LUA_BINOCLE_COLOR_WRAP_H
#define LUA_BINOCLE_COLOR_WRAP_H

#include "binocle_lua.h"
#include "backend/binocle_color.h"

typedef struct l_binocle_color_t {
  struct sg_color color;
} l_binocle_color_t;

int luaopen_color(lua_State *L);

#endif //LUA_BINOCLE_COLOR_WRAP_H
