//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_LUA_H
#define BINOCLE_LUA_H

#if defined(__IPHONEOS__) || defined(__EMSCRIPTEN__) || defined(_WIN32)
#include <lua/src/lua.h>
#include <lua/src/lualib.h>
#include <lua/src/lauxlib.h>
#else
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#endif

#include <inttypes.h>
#include <stdbool.h>
#include <time.h>

#if (LUA_VERSION_NUM < 502 && !defined(luaL_newlib))
#  define luaL_newlib(L,l) (lua_newtable(L), luaL_register(L,NULL,l))
#endif

struct binocle_window;

typedef struct binocle_lua {
  lua_State *L;
  time_t last_check_time;
  char *last_script_run;
} binocle_lua;



binocle_lua binocle_lua_new();
bool binocle_lua_init(binocle_lua *lua);
void binocle_lua_destroy(binocle_lua *lua);
bool binocle_lua_run_script(binocle_lua *lua, char *filename);
void binocle_lua_check_scripts_modification_time(binocle_lua *lua, char *path);

int lua_test(const char *arg);
int lua_test2(const char *arg);
int lua_testffi(const char *arg, struct binocle_window *window);

#endif //BINOCLE_LUA_H
