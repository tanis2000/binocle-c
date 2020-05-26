//
// Created by Valerio Santinelli on 02/02/2020.
//

#include "binocle_viewport_adapter_wrap.h"
#include "binocle_lua.h"
#include "binocle_viewport_adapter.h"
#include "binocle_window_wrap.h"
#include "binocle_sdl.h"

static const char* binocle_viewport_adapter_kind_str[] = {
  "basic", "scaling", NULL
};

static enum binocle_viewport_adapter_kind binocle_viewport_adapter_kind_vals[] = {
  BINOCLE_VIEWPORT_ADAPTER_KIND_BASIC, BINOCLE_VIEWPORT_ADAPTER_KIND_SCALING, 0,
};

static const char* binocle_viewport_adapter_scaling_type_str[] = {
  "none", "free", "pixel_perfect", "boxing", NULL
};

static enum binocle_viewport_adapter_scaling_type binocle_viewport_adapter_scaling_type_vals[] = {
  BINOCLE_VIEWPORT_ADAPTER_SCALING_TYPE_NONE,
  BINOCLE_VIEWPORT_ADAPTER_SCALING_TYPE_FREE,
  BINOCLE_VIEWPORT_ADAPTER_SCALING_TYPE_PIXEL_PERFECT,
  BINOCLE_VIEWPORT_ADAPTER_SCALING_TYPE_BOXING,
  0
};

int l_binocle_viewport_adapter_new(lua_State *L) {
  l_binocle_window_t *window = luaL_checkudata(L, 1, "binocle_window");
  int kind_int = luaL_checkoption(L, 2, "basic", binocle_viewport_adapter_kind_str);
  binocle_viewport_adapter_kind kind = binocle_viewport_adapter_kind_vals[kind_int];
  int scaling_type_int = luaL_checkoption(L, 3, "none", binocle_viewport_adapter_scaling_type_str);
  binocle_viewport_adapter_scaling_type scaling_type = binocle_viewport_adapter_scaling_type_vals[scaling_type_int];
  uint32_t width = luaL_checkinteger(L, 4);
  uint32_t height = luaL_checkinteger(L, 5);
  uint32_t virtual_width = luaL_checkinteger(L, 6);
  uint32_t virtual_height = luaL_checkinteger(L, 7);
  l_binocle_viewport_adapter_t *va = lua_newuserdata(L, sizeof(l_binocle_viewport_adapter_t));
  lua_getfield(L, LUA_REGISTRYINDEX, "binocle_viewport_adapter");
  lua_setmetatable(L, -2);
  SDL_memset(va, 0, sizeof(*va));
  binocle_viewport_adapter *adapter = binocle_viewport_adapter_new(window->window, kind, scaling_type, width, height, virtual_width, virtual_height);
  va->viewport_adapter = adapter;
  return 1;
}

int l_binocle_viewport_adapter_get_viewport(lua_State *L) {
  l_binocle_viewport_adapter_t *adapter = luaL_checkudata(L, 1, "binocle_viewport_adapter");
  kmAABB2 *p = &adapter->viewport_adapter->viewport;
  kmAABB2 **my__p = lua_newuserdata(L, sizeof(void *));
  *my__p = p;
  //(lua_getfield(L, LUA_REGISTRYINDEX, ("KAZMATH{kmAABB2}")));
  //lua_setmetatable(L, -2);
  return 1;
}

int l_binocle_viewport_adapter_get_viewport_min_x(lua_State *L) {
  l_binocle_viewport_adapter_t *adapter = luaL_checkudata(L, 1, "binocle_viewport_adapter");
  lua_pushnumber(L, adapter->viewport_adapter->viewport.min.x);
  return 1;
}

int l_binocle_viewport_adapter_get_viewport_min_y(lua_State *L) {
  l_binocle_viewport_adapter_t *adapter = luaL_checkudata(L, 1, "binocle_viewport_adapter");
  lua_pushnumber(L, adapter->viewport_adapter->viewport.min.y);
  return 1;
}

static const struct luaL_Reg viewport_adapter [] = {
  {"new", l_binocle_viewport_adapter_new},
  {"get_viewport", l_binocle_viewport_adapter_get_viewport},
  {"get_viewport_min_x", l_binocle_viewport_adapter_get_viewport_min_x},
  {"get_viewport_min_y", l_binocle_viewport_adapter_get_viewport_min_y},
  {NULL, NULL}
};

int luaopen_viewport_adapter(lua_State *L) {
  luaL_newlib(L, viewport_adapter);
  lua_setglobal(L, "viewport_adapter");
  luaL_newmetatable(L, "binocle_viewport_adapter");
  return 1;
}