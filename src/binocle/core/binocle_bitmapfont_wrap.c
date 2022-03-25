//
// Created by Valerio Santinelli on 25/03/22.
//

#include "binocle_bitmapfont_wrap.h"
#include "binocle_sdl.h"
#include "binocle_bitmapfont.h"
#include "binocle_material_wrap.h"
#include "binocle_viewport_adapter_wrap.h"
#include "binocle_gd_wrap.h"

int l_binocle_bitmapfont_from_file(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  l_binocle_bitmapfont_t *bitmapfont = lua_newuserdata(L, sizeof(l_binocle_bitmapfont_t));
  lua_getfield(L, LUA_REGISTRYINDEX, "binocle_bitmapfont");
  lua_setmetatable(L, -2);
  SDL_memset(bitmapfont, 0, sizeof(*bitmapfont));
  binocle_bitmapfont *b = binocle_bitmapfont_from_file(filename, true);
  bitmapfont->bitmapfont = SDL_malloc(sizeof(binocle_bitmapfont));
  SDL_memcpy(bitmapfont->bitmapfont, b, sizeof(binocle_bitmapfont));
  return 1;
}

int l_binocle_bitmapfont_set_material(lua_State *L) {
  l_binocle_bitmapfont_t *bitmapfont = luaL_checkudata(L, 1, "binocle_bitmapfont");
  l_binocle_material_t *mat = luaL_checkudata(L, 2, "binocle_material");
  bitmapfont->bitmapfont->material = mat->material;
  return 0;
}

int l_binocle_bitmapfont_draw_string(lua_State *L) {
  l_binocle_bitmapfont_t *bitmapfont = luaL_checkudata(L, 1, "binocle_bitmapfont");
  const char *s = luaL_checkstring(L, 2);
  float height = luaL_checknumber(L, 3);
  l_binocle_gd_t *gd = luaL_checkudata(L, 4, "binocle_gd");
  float x = luaL_checknumber(L, 5);
  float y = luaL_checknumber(L, 6);
  kmAABB2 **viewport = lua_touserdata(L, 7);
  sg_color *color = luaL_checkudata(L, 8, "binocle_color");
  kmMat4 identity;
  kmMat4Identity(&identity);
  binocle_bitmapfont_draw_string(bitmapfont->bitmapfont, s, height, gd->gd, x, y, **viewport, *color, identity);
  return 0;
}

static const struct luaL_Reg bitmapfont [] = {
  {"from_file", l_binocle_bitmapfont_from_file},
  {"set_material", l_binocle_bitmapfont_set_material},
  {"draw_string", l_binocle_bitmapfont_draw_string},
  {NULL, NULL}
};

int luaopen_bitmapfont(lua_State *L) {
  luaL_newlib(L, bitmapfont);
  lua_setglobal(L, "bitmapfont");
  luaL_newmetatable(L, "binocle_bitmapfont");
  return 1;
}