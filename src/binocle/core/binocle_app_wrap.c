//
// Created by Valerio Santinelli on 02/02/2020.
//

#include "binocle_lua.h"
#include "binocle_app.h"

int l_binocle_app_assets_dir(lua_State *L) {
  char *assets_dir = "/assets/";
  lua_pushstring(L, assets_dir);
  return 1;
}

static const struct luaL_Reg app [] = {
  {"assets_dir", l_binocle_app_assets_dir},
  {NULL, NULL}
};

int luaopen_app(lua_State *L) {
  luaL_newlib(L, app);
  lua_setglobal(L, "app");
//  lua_pop(L, 1);
  return 1;
}