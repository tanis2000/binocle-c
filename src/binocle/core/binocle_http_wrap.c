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
  binocle_http_body_t *body = SDL_malloc(sizeof(binocle_http_body_t));
  binocle_http_get(url, body);
  lua_pushstring(L, body->memory);
  return 1;
}

int l_binocle_http_post(lua_State *L) {
  const char *url = luaL_checkstring(L, 1);
  const char *req = luaL_checkstring(L, 2);
  binocle_http_body_t *body = SDL_malloc(sizeof(binocle_http_body_t));
  binocle_http_post(url, req, body);
  lua_pushstring(L, body->memory);
  return 1;
}

int l_binocle_http_put(lua_State *L) {
  const char *url = luaL_checkstring(L, 1);
  const char *req = luaL_checkstring(L, 2);
  binocle_http_body_t *body = SDL_malloc(sizeof(binocle_http_body_t));
  binocle_http_put(url, req, body);
  lua_pushstring(L, body->memory);
  return 1;
}

static const struct luaL_Reg http [] = {
  {"get", l_binocle_http_get},
  {"post", l_binocle_http_post},
  {"put", l_binocle_http_put},
  {NULL, NULL}
};

int luaopen_http(lua_State *L) {
  luaL_newlib(L, http);
  lua_setglobal(L, "http");
  return 1;
}

#endif // BINOCLE_HTTP