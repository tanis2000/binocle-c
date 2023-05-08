//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#if defined(BINOCLE_HTTP)

#include "binocle_http_wrap.h"
#include "binocle_http.h"
#include "binocle_sdl.h"

int l_binocle_http_get(lua_State *L) {
  const char *url = luaL_checkstring(L, 1);
  l_binocle_http_body_t *wrapper = lua_newuserdata(L, sizeof(l_binocle_http_body_t));
  lua_getfield(L, LUA_REGISTRYINDEX, "binocle_http_body");
  lua_setmetatable(L, -2);
  SDL_memset(wrapper, 0, sizeof(*wrapper));
  binocle_http_body_t *body = SDL_malloc(sizeof(binocle_http_body_t));
  binocle_http_get(url, body);
  wrapper->body = body;
  return 1;
}

int l_binocle_http_post(lua_State *L) {
  const char *url = luaL_checkstring(L, 1);
  const char *req = luaL_checkstring(L, 2);
  l_binocle_http_body_t *wrapper = lua_newuserdata(L, sizeof(l_binocle_http_body_t));
  lua_getfield(L, LUA_REGISTRYINDEX, "binocle_http_body");
  lua_setmetatable(L, -2);
  SDL_memset(wrapper, 0, sizeof(*wrapper));
  binocle_http_body_t *body = SDL_malloc(sizeof(binocle_http_body_t));
  binocle_http_post(url, req, body);
  wrapper->body = body;
  return 1;
}

int l_binocle_http_put(lua_State *L) {
  const char *url = luaL_checkstring(L, 1);
  const char *req = luaL_checkstring(L, 2);
  l_binocle_http_body_t *wrapper = lua_newuserdata(L, sizeof(l_binocle_http_body_t));
  lua_getfield(L, LUA_REGISTRYINDEX, "binocle_http_body");
  lua_setmetatable(L, -2);
  SDL_memset(wrapper, 0, sizeof(*wrapper));
  binocle_http_body_t *body = SDL_malloc(sizeof(binocle_http_body_t));
  binocle_http_put(url, req, body);
  wrapper->body = body;
  return 1;
}

int l_binocle_http_decode(lua_State *L) {
  l_binocle_http_body_t *wrapper = luaL_checkudata(L, 1, "binocle_http_body");
  lua_pushstring(L, wrapper->body->memory);
  return 1;
}

int l_binocle_http_gc(lua_State *L) {
  l_binocle_http_body_t *body = luaL_checkudata(L, 1, "binocle_http_body");
  SDL_free(body->body->memory);
  SDL_free(body->body);
  return 0;
}

static const struct luaL_Reg http [] = {
  {"__gc", l_binocle_http_gc},
  {"get", l_binocle_http_get},
  {"post", l_binocle_http_post},
  {"put", l_binocle_http_put},
  {"decode", l_binocle_http_decode},
  {NULL, NULL}
};

int luaopen_http(lua_State *L) {
  luaL_newlib(L, http);
  lua_setglobal(L, "http");
  luaL_newmetatable(L, "binocle_http_body");
  lua_pop(L, 1);
  return 1;
}

#endif // BINOCLE_HTTP