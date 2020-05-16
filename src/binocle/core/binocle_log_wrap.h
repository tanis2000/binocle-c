//
// Created by Valerio Santinelli on 02/02/2020.
//

#ifndef LUA_BINOCLE_LOG_WRAP_H
#define LUA_BINOCLE_LOG_WRAP_H

#include "binocle_lua.h"

typedef struct l_binocle_log_t {
} l_binocle_log_t;

int luaopen_log(lua_State *L);

#endif //LUA_BINOCLE_LOG_WRAP_H
