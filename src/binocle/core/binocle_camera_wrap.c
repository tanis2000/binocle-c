//
// Created by Valerio Santinelli on 02/02/2020.
//

#include "binocle_camera_wrap.h"
#include "binocle_texture_wrap.h"
#include "binocle_viewport_adapter_wrap.h"
#include "binocle_lua.h"
#include "binocle_subtexture.h"
#include "binocle_sdl.h"
#include "binocle_gd.h"
#include "binocle_camera.h"

int l_binocle_camera_new(lua_State *L) {
  l_binocle_viewport_adapter_t *va = luaL_checkudata(L, 1, "binocle_viewport_adapter");
  l_binocle_camera_t *camera = lua_newuserdata(L, sizeof(l_binocle_camera_t));
  lua_getfield(L, LUA_REGISTRYINDEX, "binocle_camera");
  lua_setmetatable(L, -2);
  SDL_memset(camera, 0, sizeof(*camera));
  binocle_camera c = binocle_camera_new(va->viewport_adapter);
  camera->camera = SDL_malloc(sizeof(binocle_camera));
  SDL_memcpy(camera->camera, &c, sizeof(binocle_camera));
  return 1;
}

int l_binocle_camera_set_position(lua_State *L) {
  l_binocle_camera_t *camera = luaL_checkudata(L, 1, "binocle_camera");
  float x = luaL_checknumber(L, 2);
  float y = luaL_checknumber(L, 3);
  binocle_camera_set_position(camera->camera, x, y);
  return 0;
}

int l_binocle_camera_get_x(lua_State *L) {
  l_binocle_camera_t *camera = luaL_checkudata(L, 1, "binocle_camera");
  lua_pushnumber(L, camera->camera->position.x);
  return 1;
}

int l_binocle_camera_get_y(lua_State *L) {
  l_binocle_camera_t *camera = luaL_checkudata(L, 1, "binocle_camera");
  lua_pushnumber(L, camera->camera->position.y);
  return 1;
}

static const struct luaL_Reg camera [] = {
  {"new", l_binocle_camera_new},
  {"set_position", l_binocle_camera_set_position},
  {"x", l_binocle_camera_get_x},
  {"y", l_binocle_camera_get_y},
  {NULL, NULL}
};

int luaopen_camera(lua_State *L) {
  luaL_newlib(L, camera);
  lua_setglobal(L, "camera");
  luaL_newmetatable(L, "binocle_camera");
  return 1;
}