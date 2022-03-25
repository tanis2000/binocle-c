//
// Created by Valerio Santinelli on 25/03/22.
//

#ifndef LUA_BINOCLE_BITMAPFONT_WRAP_H
#define LUA_BINOCLE_BITMAPFONT_WRAP_H

#include "binocle_lua.h"

typedef struct l_binocle_bitmapfont_t {
  struct binocle_bitmapfont *bitmapfont;
} l_binocle_bitmapfont_t;

int luaopen_bitmapfont(lua_State *L);

#endif //LUA_BINOCLE_BITMAPFONT_WRAP_H
