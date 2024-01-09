//
// Created by Valerio Santinelli on 25/03/22.
//

#include "binocle_ttfont_wrap.h"
#include "binocle_sdl.h"
#include "binocle_ttfont.h"
#include "binocle_material_wrap.h"
#include "binocle_viewport_adapter_wrap.h"
#include "binocle_gd_wrap.h"
#include "binocle_camera_wrap.h"

int l_binocle_ttfont_from_file(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  float size = luaL_checknumber(L, 2);
  sg_shader *shd = lua_touserdata(L, 3);
  l_binocle_ttfont_t *ttfont = lua_newuserdata(L, sizeof(l_binocle_ttfont_t));
  lua_getfield(L, LUA_REGISTRYINDEX, "binocle_ttfont");
  lua_setmetatable(L, -2);
  SDL_memset(ttfont, 0, sizeof(*ttfont));
  binocle_ttfont *ttf = binocle_ttfont_from_file(filename, size, 1024, 1024, *shd);
  ttfont->ttfont = ttf;
//  ttfont->ttfont = SDL_malloc(sizeof(binocle_ttfont));
//  SDL_memcpy(ttfont->ttfont, ttf, sizeof(binocle_ttfont));
  return 1;
}

int l_binocle_ttfont_from_assets(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  float size = luaL_checknumber(L, 2);
  sg_shader *shd = lua_touserdata(L, 3);
  l_binocle_ttfont_t *ttfont = lua_newuserdata(L, sizeof(l_binocle_ttfont_t));
  lua_getfield(L, LUA_REGISTRYINDEX, "binocle_ttfont");
  lua_setmetatable(L, -2);
  SDL_memset(ttfont, 0, sizeof(*ttfont));
  binocle_ttfont_load_desc desc = {
    .filename = filename,
    .filter = SG_FILTER_LINEAR,
    .wrap = SG_WRAP_CLAMP_TO_EDGE,
    .fs = BINOCLE_FS_PHYSFS,
    .size = size,
    .texture_width = 1024,
    .texture_height = 1024,
    .shader = *shd,
  };
  binocle_ttfont *ttf = binocle_ttfont_load_with_desc(&desc);
  ttfont->ttfont = ttf;
//  ttfont->ttfont = SDL_malloc(sizeof(binocle_ttfont));
//  SDL_memcpy(ttfont->ttfont, ttf, sizeof(binocle_ttfont));
  return 1;
}

int l_binocle_ttfont_draw_string(lua_State *L) {
  l_binocle_ttfont_t *ttfont = luaL_checkudata(L, 1, "binocle_ttfont");
  const char *s = luaL_checkstring(L, 2);
  l_binocle_gd_t *gd = luaL_checkudata(L, 3, "binocle_gd");
  float x = luaL_checknumber(L, 4);
  float y = luaL_checknumber(L, 5);
  kmAABB2 **viewport = lua_touserdata(L, 6);
  sg_color *color = luaL_checkudata(L, 7, "binocle_color");
  l_binocle_camera_t *camera = luaL_checkudata(L, 8, "binocle_camera");
  float depth = luaL_checknumber(L, 9);
  kmMat4 identity;
  kmMat4Identity(&identity);
  binocle_ttfont_draw_string(ttfont->ttfont, s, gd->gd, x, y, **viewport, *color, camera->camera, depth);
  return 0;
}

int l_binocle_ttfont_get_string_width(lua_State *L) {
  l_binocle_ttfont_t *ttfont = luaL_checkudata(L, 1, "binocle_ttfont");
  const char *s = luaL_checkstring(L, 2);
  float width = binocle_ttfont_get_string_width(ttfont->ttfont, s);
  lua_pushnumber(L, width);
  return 1;
}

int l_binocle_ttfont_destroy(lua_State *L) {
  l_binocle_ttfont_t *ttfont = luaL_checkudata(L, 1, "binocle_ttfont");
  binocle_ttfont_destroy(ttfont->ttfont);
  ttfont->ttfont = NULL;
  return 0;
}

static const struct luaL_Reg ttfont [] = {
  {"from_file", l_binocle_ttfont_from_file},
  {"from_assets", l_binocle_ttfont_from_assets},
  {"draw_string", l_binocle_ttfont_draw_string},
  {"get_string_width", l_binocle_ttfont_get_string_width},
  {"destroy", l_binocle_ttfont_destroy},
  {NULL, NULL}
};

int luaopen_ttfont(lua_State *L) {
  luaL_newlib(L, ttfont);
  lua_setglobal(L, "ttfont");
  luaL_newmetatable(L, "binocle_ttfont");
  lua_pop(L, 1);
  return 1;
}