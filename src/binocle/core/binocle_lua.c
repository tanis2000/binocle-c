//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include <stdlib.h>
#include "binocle_lua.h"
#include "binocle_window.h"
#include "binocle_log.h"
#include "binocle_fs.h"
#include <sokol_time.h>
#include "binocle_sdl_wrap.h"
#include "binocle_image_wrap.h"
#include "binocle_texture_wrap.h"
#include "binocle_shader_wrap.h"
#include "binocle_material_wrap.h"
#include "binocle_sprite_wrap.h"
#include "binocle_input_wrap.h"
#include "lkazmath.h"

binocle_lua binocle_lua_new() {
  binocle_lua res = {0};
  return res;
}

bool binocle_lua_init(binocle_lua *lua) {
  lua->L = luaL_newstate();
  if (lua->L == NULL) {
    binocle_log_error("Cannot initialize Lua environment");
    return false;
  }

  // Load the Lua libraries
  luaL_openlibs(lua->L);

  // Kazmath
  luaopen_lkazmath(lua->L);

  luaopen_sdl(lua->L);
  luaopen_image(lua->L);
  luaopen_texture(lua->L);
  luaopen_shader(lua->L);
  luaopen_material(lua->L);
  luaopen_sprite(lua->L);
  luaopen_input(lua->L);

  //lua->last_check_time = stm_now();
  time_t t = time(NULL);
  //struct tm tm = *localtime(&t);
  lua->last_check_time = t;

  return true;
}

void binocle_lua_destroy(binocle_lua *lua) {
  if (lua->L != NULL) {
    lua_close(lua->L);
  }
}

bool binocle_lua_run_script(binocle_lua *lua, char *filename) {
  int status = luaL_loadfile(lua->L, filename);
  if (status) {
    binocle_log_error("Couldn't load file: %s\n", lua_tostring(lua->L, -1));
    return false;
  }

  // We call the script with 0 arguments and expect 0 results
  int result = lua_pcall(lua->L, 0, 0, 0);
  if (result) {
    binocle_log_error("Failed to run script: %s\n", lua_tostring(lua->L, -1));
    return false;
  }

  lua->last_script_run = filename;
  return true;
}

int binocle_lua_enumerate_callback(void *user_data, const char *path, const char *filename) {
  char *search_path;
  uint64_t mod_time;
  binocle_lua *lua = (binocle_lua *)user_data;
  int needed = SDL_snprintf(NULL, 0, "%s/%s", path, filename);
  search_path = malloc(needed);
  sprintf(search_path, "%s/%s", path, filename);
  binocle_fs_get_last_modification_time(search_path, &mod_time);
  if (mod_time > lua->last_check_time) {
    binocle_lua_destroy(lua);
    binocle_lua_init(lua);
    if (lua->last_script_run != NULL){
      binocle_lua_run_script(lua, lua->last_script_run);
    }
    return 0; // No more files please
  }
  return 1; // 1 means get more files
}

void binocle_lua_check_scripts_modification_time(binocle_lua *lua, char *path) {
  char *search_path;
  int needed = SDL_snprintf(NULL, 0, "%s/*.lua", path);
  search_path = malloc(needed);
  sprintf(search_path, "%s/*.lua", path);
  binocle_fs_enumerate(path, binocle_lua_enumerate_callback, lua);
  free(search_path);
}

static void close_state(lua_State **L) { lua_close(*L); }
#define cleanup(x) __attribute__((cleanup(x)))
#define auto_lclose cleanup(close_state)

int lua_test(const char *arg) {
  /* Create VM state */
  lua_State *L = luaL_newstate();
  if (!L)
    return 1;
  luaL_openlibs(L); /* Open standard libraries */
  /* Load config file */
  if (arg != NULL) {
    luaL_loadfile(L, arg); /* (1) */
    int ret = lua_pcall(L, 0, 0, 0);
    if (ret != 0) {
      fprintf(stderr, "%s\n", lua_tostring(L, -1));
      return 1;
    }
  }
  /* Read out config */
  lua_getglobal(L, "address"); /* (2) */
  lua_getglobal(L, "port");
  printf("address: %s, port: %ld\n", /* (3) */
         lua_tostring(L, -2), lua_tointeger(L, -1));
  lua_settop(L, 0); /* (4) */
  //return 0;
  lua_close(L);
}

int lua_test2(const char *arg) {
  int status, result, i;
  double sum;
  lua_State *L;

  /*
   * All Lua contexts are held in this structure. We work with it almost
   * all the time.
   */
  L = luaL_newstate();

  if (L == NULL) {
    return 1;
  }

  luaL_openlibs(L); /* Load Lua libraries */

  /* Load the file containing the script we are going to run */
  status = luaL_loadfile(L, arg);
  if (status) {
    /* If something went wrong, error message is at the top of */
    /* the stack */
    fprintf(stderr, "Couldn't load file: %s\n", lua_tostring(L, -1));
    return 1;
  }

  /*
   * Ok, now here we go: We pass data to the lua script on the stack.
   * That is, we first have to prepare Lua's virtual stack the way we
   * want the script to receive it, then ask Lua to run it.
   */
  lua_newtable(L);    /* We will pass a table */

  /*
   * To put values into the table, we first push the index, then the
   * value, and then call lua_rawset() with the index of the table in the
   * stack. Let's see why it's -3: In Lua, the value -1 always refers to
   * the top of the stack. When you create the table with lua_newtable(),
   * the table gets pushed into the top of the stack. When you push the
   * index and then the cell value, the stack looks like:
   *
   * <- [stack bottom] -- table, index, value [top]
   *
   * So the -1 will refer to the cell value, thus -3 is used to refer to
   * the table itself. Note that lua_rawset() pops the two last elements
   * of the stack, so that after it has been called, the table is at the
   * top of the stack.
   */
  for (i = 1; i <= 5; i++) {
    lua_pushnumber(L, i);   /* Push the table index */
    lua_pushnumber(L, i*2); /* Push the cell value */
    lua_rawset(L, -3);      /* Stores the pair in the table */
  }

  /* By what name is the script going to reference our table? */
  lua_setglobal(L, "foo");

  /* Ask Lua to run our little script */
  result = lua_pcall(L, 0, LUA_MULTRET, 0);
  if (result) {
    fprintf(stderr, "Failed to run script: %s\n", lua_tostring(L, -1));
    exit(1);
  }

  /* Get the returned value at the top of the stack (index -1) */
  sum = lua_tonumber(L, -1);

  printf("Script returned: %.0f\n", sum);

  lua_pop(L, 1);  /* Take the returned value out of the stack */
  lua_close(L);   /* Cya, Lua */

  return 0;
}

void binocle_lua_reg(lua_State *L, const void* key) {
  if(key==NULL) {
    lua_pushstring(L, "Trying to register with a NULL key!");
    lua_error(L);
  }
  lua_pushlightuserdata(L, (void*)key); // check if table exists
  lua_gettable(L, LUA_REGISTRYINDEX);   //
  if ( lua_isnil(L, -1) ) {
    lua_pop(L,1);
    lua_pushlightuserdata(L, (void*)key); // check if table exists
    lua_newtable(L);
    lua_settable(L, LUA_REGISTRYINDEX);
  }
  else lua_pop(L,1);  // already there
}

int binocle_lua_store(lua_State* L, const void* key, const char* name, void *data) {
  int res = 0;
  lua_pushlightuserdata(L, (void*)key);
  lua_gettable(L, LUA_REGISTRYINDEX);
  if ( lua_istable(L, -1) ) {
    lua_pushlightuserdata(L, data);
    lua_setfield(L, -2, name);
    lua_pop(L,1); // pop registry
    res = 1;
  }
  else {
    printf("** set: couldn't get the associated registry.\n");
    lua_pushfstring(L, "Can't get associated registry.");
    lua_error(L);
    //lua_pop(L,1); // nil
  }
  return res;
}

int lua_testffi(const char *arg, binocle_window *window) {
  int status, result, i;
  double sum;
  lua_State *L;

  /*
   * All Lua contexts are held in this structure. We work with it almost
   * all the time.
   */
  L = luaL_newstate();

  luaL_openlibs(L); /* Load Lua libraries */

  /* Load the file containing the script we are going to run */
  status = luaL_loadfile(L, arg);
  if (status) {
    /* If something went wrong, error message is at the top of */
    /* the stack */
    fprintf(stderr, "Couldn't load file: %s\n", lua_tostring(L, -1));
    return 1;
  }

  lua_pushlightuserdata(L, window);
  lua_setglobal(L, "window");
  //binocle_lua_store(L, window, "window", window);

  /* Ask Lua to run our little script */
  result = lua_pcall(L, 0, 0, 0);
  if (result) {
    fprintf(stderr, "Failed to run script: %s\n", lua_tostring(L, -1));
    exit(1);
  }

  lua_close(L);   /* Cya, Lua */

  return 0;
}