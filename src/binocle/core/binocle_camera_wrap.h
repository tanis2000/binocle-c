//
// Created by Valerio Santinelli on 02/02/2020.
//

#ifndef LUA_BINOCLE_CAMERA_WRAP_H
#define LUA_BINOCLE_CAMERA_WRAP_H

#include "binocle_lua.h"

typedef struct l_binocle_camera_t {
  struct binocle_camera *camera;
} l_binocle_camera_t;

int luaopen_camera(lua_State *L);

#endif //LUA_BINOCLE_CAMERA_WRAP_H
