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
#if defined(BINOCLE_LUAJIT)
#include "luajit/src/luajit.h"
#include "luajit/src/lua.h"
#include "luajit/src/lualib.h"
#include "luajit/src/lauxlib.h"
#else
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#endif // defined(BINOCLE_LUAJIT)
#endif

#include <inttypes.h>
#include <stdbool.h>
#include <time.h>

#if (LUA_VERSION_NUM < 502 && !defined(luaL_newlib))
#  define luaL_newlib(L,l) (lua_newtable(L), luaL_register(L,NULL,l))
#endif

struct binocle_window;
struct binocle_memory_arena;

typedef struct binocle_lua {
  lua_State *L;
  time_t last_check_time;
  char *last_script_run;
  struct binocle_memory_arena *arena;
} binocle_lua;

typedef int (binocle_lua_fs_enumerate_pre_run_callback)();
typedef int (binocle_lua_fs_enumerate_post_run_callback)();

typedef struct binocle_lua_fs_enumerate_user_data_t {
  binocle_lua *lua;
  bool reloaded;
  binocle_lua_fs_enumerate_pre_run_callback *pre_run_callback;
  binocle_lua_fs_enumerate_post_run_callback *post_run_callback;
} binocle_lua_fs_enumerate_user_data_t;

binocle_lua binocle_lua_new();
bool binocle_lua_init(binocle_lua *lua);
void binocle_lua_destroy(binocle_lua *lua);
bool binocle_lua_run_script(binocle_lua *lua, char *filename);
bool binocle_lua_check_scripts_modification_time(binocle_lua *lua, char *path, binocle_lua_fs_enumerate_pre_run_callback *pre_run_callback, binocle_lua_fs_enumerate_post_run_callback *post_run_callback);
int binocle_lua_fs_loader(lua_State *L);

int lua_test(const char *arg);
int lua_test2(const char *arg);
int lua_testffi(const char *arg, struct binocle_window *window);
int lua_test_profiler(const char *arg);

#endif //BINOCLE_LUA_H
