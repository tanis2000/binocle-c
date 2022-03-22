//
// Created by Valerio Santinelli on 02/02/2020.
//

#ifndef LUA_BINOCLE_TEXTURE_WRAP_H
#define LUA_BINOCLE_TEXTURE_WRAP_H

#include "binocle_lua.h"
#include "sokol_gfx.h"

typedef struct l_binocle_texture_t {
  sg_image texture;
} l_binocle_texture_t;

int luaopen_texture(lua_State *L);

#endif //LUA_BINOCLE_TEXTURE_WRAP_H
