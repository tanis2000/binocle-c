//
// Created by Valerio Santinelli on 02/02/2020.
//

#ifndef LUA_BINOCLE_VIEWPORT_ADAPTER_WRAP_H
#define LUA_BINOCLE_VIEWPORT_ADAPTER_WRAP_H

#include "binocle_lua.h"

typedef struct l_binocle_viewport_adapter_t {
  struct binocle_viewport_adapter *viewport_adapter;
} l_binocle_viewport_adapter_t;

int luaopen_viewport_adapter(lua_State *L);

#endif //LUA_BINOCLE_VIEWPORT_ADAPTER_WRAP_H
