//
// Created by Valerio Santinelli on 02/02/2020.
//

#include "binocle_gd_wrap.h"
#include "binocle_lua.h"
#include "binocle_gd.h"
#include "backend/binocle_shader.h"
#include "binocle_color.h"

int l_binocle_gd_new(lua_State *L) {
  l_binocle_gd_t *gd = lua_newuserdata(L, sizeof(l_binocle_gd_t));
  lua_getfield(L, LUA_REGISTRYINDEX, "binocle_gd");
  lua_setmetatable(L, -2);
  SDL_memset(gd, 0, sizeof(*gd));
  binocle_gd gdi = binocle_gd_new();
  gd->gd = SDL_malloc(sizeof(binocle_gd));
  SDL_memcpy(gd->gd, &gdi, sizeof(binocle_gd));
  return 1;
}

int l_binocle_gd_init(lua_State *L) {
  l_binocle_gd_t *gd = lua_touserdata(L, 1);
  binocle_gd_init(gd->gd);
  return 0;
}

int l_binocle_gd_create_render_target(lua_State *L) {
  int width = luaL_checkint(L, 1);
  int height = luaL_checkint(L, 2);
  bool use_depth = lua_toboolean(L, 3);
  int format = luaL_checkint(L, 4);
  l_binocle_render_target_t *rt = lua_newuserdata(L, sizeof(l_binocle_render_target_t));
  lua_getfield(L, LUA_REGISTRYINDEX, "binocle_render_target");
  lua_setmetatable(L, -2);
  SDL_memset(rt, 0, sizeof(*rt));
  binocle_render_target render_target = binocle_gd_create_render_target(width, height, use_depth, format);
  rt->rt = render_target;
  return 1;
}

int l_binocle_gd_set_render_target(lua_State *L) {
  if (lua_isnil(L, 1)) {
    binocle_gd_set_render_target(NULL);
  } else {
    l_binocle_render_target_t *render_target = luaL_checkudata(L, 1, "binocle_render_target");
    binocle_gd_set_render_target(&render_target->rt);
  }
  return 0;
}

int l_binocle_gd_apply_viewport(lua_State *L) {
  //kmAABB2 **viewport = luaL_checkudata(L, 1, "KAZMATH{kmAABB2}");
  kmAABB2 **viewport = lua_touserdata(L, 1);
  binocle_gd_apply_viewport(**viewport);
  return 0;
}

int l_binocle_gd_apply_shader(lua_State *L) {
  binocle_gd *gd = lua_touserdata(L, 1);
  binocle_shader **shader = luaL_checkudata(L, 2, "binocle_shader");
  binocle_gd_apply_shader(gd, *shader);
  return 0;
}

int l_binocle_gd_set_uniform_float2(lua_State *L) {
  binocle_shader **shader = luaL_checkudata(L, 1, "binocle_shader");
  const char *name = luaL_checkstring(L, 2);
  float f1 = luaL_checknumber(L, 3);
  float f2 = luaL_checknumber(L, 4);
  binocle_gd_set_uniform_float2(*shader, name, f1, f2);
  return 0;
}

int l_binocle_gd_set_uniform_mat4(lua_State *L) {
  binocle_shader **shader = luaL_checkudata(L, 1, "binocle_shader");
  const char *name = luaL_checkstring(L, 2);
  kmMat4 **m = luaL_checkudata(L, 3, "KAZMATH{kmMat4}");
  binocle_gd_set_uniform_mat4(*shader, name, **m);
  return 0;
}

int l_binocle_gd_draw_quad_to_screen(lua_State *L) {
  binocle_shader **shader = luaL_checkudata(L, 1, "binocle_shader");
  binocle_render_target **rt = luaL_checkudata(L, 2, "binocle_render_target");
  binocle_gd_draw_quad_to_screen(*shader, *rt);
  return 0;
}

int l_binocle_gd_clear(lua_State *L) {
  binocle_color *color = luaL_checkudata(L, 1, "binocle_color");
  binocle_gd_clear(*color);
  return 0;
}

static const struct luaL_Reg gd [] = {
  {"new", l_binocle_gd_new},
  {"init", l_binocle_gd_init},
  {"create_render_target", l_binocle_gd_create_render_target},
  {"set_render_target", l_binocle_gd_set_render_target},
  {"apply_viewport", l_binocle_gd_apply_viewport},
  {"apply_shader", l_binocle_gd_apply_shader},
  {"set_uniform_float2", l_binocle_gd_set_uniform_float2},
  {"set_uniform_mat4", l_binocle_gd_set_uniform_mat4},
  {"draw_quad_to_screen", l_binocle_gd_draw_quad_to_screen},
  {"clear", l_binocle_gd_clear},
  {NULL, NULL}
};

int luaopen_gd(lua_State *L) {
  luaL_newlib(L, gd);
  lua_setglobal(L, "gd");
  luaL_newmetatable(L, "binocle_gd");
  luaL_newmetatable(L, "binocle_render_target");
  return 1;
}