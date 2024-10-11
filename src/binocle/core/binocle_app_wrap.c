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

int l_binocle_app_shader_prefix(lua_State *L) {
#if defined(BINOCLE_MACOS) && defined(BINOCLE_METAL)
  char *s = "dst/metal-macos";
#elif defined(__IPHONEOS__) || defined(__ANDROID__) || defined(__EMSCRIPTEN__)
  char *s = "dst/gles";
#else
  char *s = "dst/gl33";
#endif
  lua_pushstring(L, s);
  return 1;
}

int l_binocle_app_shader_vs_suffix(lua_State *L) {
#if defined(BINOCLE_MACOS) && defined(BINOCLE_METAL)
  char *s = "_metal_macos_vs.metal";
#elif defined(__IPHONEOS__) || defined(__ANDROID__) || defined(__EMSCRIPTEN__)
  char *s = "_glsl300es_vs.glsl";
#else
  char *s = "_glsl410_vs.glsl";
#endif
  lua_pushstring(L, s);
  return 1;
}

int l_binocle_app_shader_fs_suffix(lua_State *L) {
#if defined(BINOCLE_MACOS) && defined(BINOCLE_METAL)
  char *s = "_metal_macos_fs.metal";
#elif defined(__IPHONEOS__) || defined(__ANDROID__) || defined(__EMSCRIPTEN__)
  char *s = "_glsl300es_fs.glsl";
#else
  char *s = "_glsl410_fs.glsl";
#endif
  lua_pushstring(L, s);
  return 1;
}

static const struct luaL_Reg app [] = {
  {"assets_dir", l_binocle_app_assets_dir},
  {"shader_prefix", l_binocle_app_shader_prefix},
  {"shader_vs_suffix", l_binocle_app_shader_vs_suffix},
  {"shader_fs_suffix", l_binocle_app_shader_fs_suffix},
  {NULL, NULL}
};

int luaopen_app(lua_State *L) {
  luaL_newlib(L, app);
  lua_setglobal(L, "app");
//  lua_pop(L, 1);
  return 1;
}