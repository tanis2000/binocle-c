//
// Created by Valerio Santinelli on 02/02/2020.
//

#include <string.h>
#include "binocle_log_wrap.h"
#include "binocle_lua.h"
#include "binocle_log.h"


int l_binocle_log_info(lua_State *L) {
  const char *s = luaL_checkstring(L, 1);
  binocle_log_info("Lua: %s", s);
  return 1;
}

int l_binocle_log_debug(lua_State *L) {
  const char *s = luaL_checkstring(L, 1);
  binocle_log_debug("Lua: %s", s);
  return 1;
}

int l_binocle_log_warning(lua_State *L) {
  const char *s = luaL_checkstring(L, 1);
  binocle_log_warning("Lua: %s", s);
  return 1;
}

int l_binocle_log_error(lua_State *L) {
  const char *s = luaL_checkstring(L, 1);
  binocle_log_error("Lua: %s", s);
  return 1;
}

static const struct luaL_Reg log [] = {
  {"info", l_binocle_log_info},
  {"debug", l_binocle_log_debug},
  {"warning", l_binocle_log_warning},
  {"error", l_binocle_log_error},
  {NULL, NULL}
};

int luaopen_log(lua_State *L) {
  luaL_newlib(L, log);
  lua_setglobal(L, "log");
  luaL_newmetatable(L, "binocle_log");
  return 1;
}