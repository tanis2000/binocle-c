//
// Created by Valerio Santinelli on 02/02/2020.
//

#include <string.h>
#include "binocle_input_wrap.h"
#include "binocle_lua.h"
#include "binocle_input.h"
#include "binocle_sdl.h"

#define L_BINOCLE_INPUT_MAX_KEY 5

typedef struct l_key_t {
  char *key_str;
  binocle_input_keyboard_key key;
} l_key_t;

static l_key_t key_map[L_BINOCLE_INPUT_MAX_KEY];

void l_binocle_input_build_key_map() {
  key_map[0].key_str = "KEY_RIGHT";
  key_map[0].key = KEY_RIGHT;
  key_map[1].key_str = "KEY_LEFT";
  key_map[1].key = KEY_LEFT;
  key_map[2].key_str = "KEY_UP";
  key_map[2].key = KEY_UP;
  key_map[3].key_str = "KEY_DOWN";
  key_map[3].key = KEY_DOWN;
  key_map[4].key_str = "KEY_SPACE";
  key_map[4].key = KEY_SPACE;
}

int l_binocle_input_is_key_pressed(lua_State *L) {
  binocle_input *input = lua_touserdata(L, 1);
  const char *key_str = luaL_checkstring(L, 2);
  binocle_input_keyboard_key key = KEY_UNKNOWN;
  for (int i = 0 ; i < L_BINOCLE_INPUT_MAX_KEY ; i++) {
    if (strcmp(key_str, key_map[i].key_str) == 0) {
      key = key_map[i].key;
    }
  }
  bool res = binocle_input_is_key_pressed(input, key);
  lua_pushboolean(L, res);
  return 1;
}

static const struct luaL_Reg input [] = {
  {"is_key_pressed", l_binocle_input_is_key_pressed},
  {NULL, NULL}
};

int luaopen_input(lua_State *L) {
  luaL_newlib(L, input);
  lua_setglobal(L, "input");
  luaL_newmetatable(L, "binocle_input");

  l_binocle_input_build_key_map();

  lua_newtable(L);
  lua_pushstring(L, "KEY_RIGHT");
  lua_pushstring(L, "KEY_RIGHT");
  lua_settable(L, -3);
  lua_pushstring(L, "KEY_LEFT");
  lua_pushstring(L, "KEY_LEFT");
  lua_settable(L, -3);
  lua_pushstring(L, "KEY_UP");
  lua_pushstring(L, "KEY_UP");
  lua_settable(L, -3);
  lua_pushstring(L, "KEY_DOWN");
  lua_pushstring(L, "KEY_DOWN");
  lua_settable(L, -3);
  lua_pushstring(L, "KEY_SPACE");
  lua_pushstring(L, "KEY_SPACE");
  lua_settable(L, -3);
  lua_setglobal(L, "key");
  return 1;
}