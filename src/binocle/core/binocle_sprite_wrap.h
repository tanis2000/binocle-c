//
// Created by Valerio Santinelli on 02/02/2020.
//

#ifndef LUA_BINOCLE_SPRITE_WRAP_H
#define LUA_BINOCLE_SPRITE_WRAP_H

#include "binocle_lua.h"

typedef struct l_binocle_sprite_t {
  struct binocle_sprite *sprite;
} l_binocle_sprite_t;

int luaopen_sprite(lua_State *L);

typedef struct l_binocle_sprite_batch_t {
  struct binocle_sprite_batch *sprite_batch;
} l_binocle_sprite_batch_t;

int luaopen_sprite_batch(lua_State *L);

#endif //LUA_BINOCLE_SPRITE_WRAP_H
