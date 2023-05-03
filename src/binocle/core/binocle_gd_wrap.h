//
// Created by Valerio Santinelli on 02/02/2020.
//

#ifndef LUA_BINOCLE_GD_WRAP_H
#define LUA_BINOCLE_GD_WRAP_H

#include "binocle_lua.h"
#include "sokol_gfx.h"
#include "kazmath/kazmath.h"

typedef struct l_binocle_gd_t {
  struct binocle_gd *gd;
  struct binocle_window *win;
} l_binocle_gd_t;

typedef struct l_binocle_render_target_t {
  sg_image rt;
} l_binocle_render_target_t;

typedef struct l_binocle_shader_t {
  sg_shader_desc shader_desc;
  sg_shader shader;
  sg_pipeline pip;
} l_binocle_shader_t;

int luaopen_gd(lua_State *L);

#endif //LUA_BINOCLE_GD_WRAP_H
