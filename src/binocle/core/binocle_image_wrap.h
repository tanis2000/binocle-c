//
// Created by Valerio Santinelli on 02/02/2020.
//

#ifndef LUA_BINOCLE_IMAGE_WRAP_H
#define LUA_BINOCLE_IMAGE_WRAP_H

#include "binocle_lua.h"
#include "sokol_gfx.h"

typedef struct l_binocle_image_t {
  sg_image img;
} l_binocle_image_t;

int luaopen_image(lua_State *L);

#endif //LUA_BINOCLE_IMAGE_WRAP_H
