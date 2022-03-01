//
// Created by Valerio Santinelli on 02/02/2020.
//

#ifndef LUA_BINOCLE_WINDOW_WRAP_H
#define LUA_BINOCLE_WINDOW_WRAP_H

#include "binocle_lua.h"

typedef struct l_binocle_window_t {
  struct binocle_window *window;
} l_binocle_window_t;

int luaopen_window(lua_State *L);

#endif //LUA_BINOCLE_WINDOW_WRAP_H
