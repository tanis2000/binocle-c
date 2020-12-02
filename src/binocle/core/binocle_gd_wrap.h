//
// Created by Valerio Santinelli on 02/02/2020.
//

#ifndef LUA_BINOCLE_GD_WRAP_H
#define LUA_BINOCLE_GD_WRAP_H

#include "binocle_lua.h"
#include "backend/binocle_backend.h"

typedef struct l_binocle_gd_t {
  struct binocle_gd *gd;
  struct binocle_window *win;
} l_binocle_gd_t;

typedef struct l_binocle_render_target_t {
  binocle_render_target rt;
} l_binocle_render_target_t;

int luaopen_gd(lua_State *L);

#endif //LUA_BINOCLE_GD_WRAP_H
