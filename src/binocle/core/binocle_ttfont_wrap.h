//
// Created by Valerio Santinelli on 25/03/22.
//

#ifndef LUA_BINOCLE_TTFONT_WRAP_H
#define LUA_BINOCLE_TTFONT_WRAP_H

#include "binocle_lua.h"

typedef struct l_binocle_ttfont_t {
  struct binocle_ttfont *ttfont;
} l_binocle_ttfont_t;

int luaopen_ttfont(lua_State *L);

#endif //LUA_BINOCLE_TTFONT_WRAP_H
