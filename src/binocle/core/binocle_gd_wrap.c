//
// Created by Valerio Santinelli on 02/02/2020.
//

#include "binocle_gd_wrap.h"
#include "binocle_lua.h"
#include "binocle_gd.h"
#include "backend/binocle_color.h"
#include "binocle_window_wrap.h"
#include "binocle_camera_wrap.h"
#include "binocle_camera.h"
#include "binocle_viewport_adapter.h"

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
  l_binocle_window_t *win = lua_touserdata(L, 2);
  binocle_gd_init(gd->gd, win->window);
  return 0;
}

int l_binocle_gd_create_render_target(lua_State *L) {
  // TODO: redo this
//  int width = luaL_checkint(L, 1);
//  int height = luaL_checkint(L, 2);
//  bool use_depth = lua_toboolean(L, 3);
//  int format = luaL_checkint(L, 4);
//  l_binocle_render_target_t *rt = lua_newuserdata(L, sizeof(l_binocle_render_target_t));
//  lua_getfield(L, LUA_REGISTRYINDEX, "binocle_render_target");
//  lua_setmetatable(L, -2);
//  SDL_memset(rt, 0, sizeof(*rt));
//  binocle_image render_target = binocle_backend_create_render_target(width, height, use_depth, format);
//  rt->rt = render_target;
  return 1;
}

int l_binocle_gd_set_render_target(lua_State *L) {
  if (lua_isnil(L, 1)) {
//    binocle_backend_unset_render_target();
  } else {
    l_binocle_render_target_t *render_target = luaL_checkudata(L, 1, "binocle_render_target");
    binocle_gd_set_render_target(render_target->rt);
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
  sg_shader **shader = luaL_checkudata(L, 2, "binocle_shader");
  binocle_gd_apply_shader(gd, **shader);
  return 0;
}

int l_binocle_gd_set_uniform_float2(lua_State *L) {
  sg_shader **shader = luaL_checkudata(L, 1, "binocle_shader");
  const char *name = luaL_checkstring(L, 2);
  float f1 = luaL_checknumber(L, 3);
  float f2 = luaL_checknumber(L, 4);
  binocle_gd_set_uniform_float2(**shader, name, f1, f2);
  return 0;
}

int l_binocle_gd_set_uniform_mat4(lua_State *L) {
  sg_shader **shader = luaL_checkudata(L, 1, "binocle_shader");
  const char *name = luaL_checkstring(L, 2);
  kmMat4 **m = luaL_checkudata(L, 3, "KAZMATH{kmMat4}");
  binocle_gd_set_uniform_mat4(**shader, name, **m);
  return 0;
}

int l_binocle_gd_draw_quad_to_screen(lua_State *L) {
  l_binocle_gd_t *gd = lua_touserdata(L, 1);
  sg_shader **shader = luaL_checkudata(L, 2, "binocle_shader");
  sg_image **rt = luaL_checkudata(L, 3, "binocle_render_target");
  binocle_gd_draw_quad_to_screen(gd->gd, **shader, **rt);
  return 0;
}

int l_binocle_gd_draw_rect(lua_State *L) {
  l_binocle_gd_t *gd = lua_touserdata(L, 1);
  float center_x = lua_tonumber(L, 2);
  float center_y = lua_tonumber(L, 3);
  float width = lua_tonumber(L, 4);
  float height = lua_tonumber(L, 5);
  sg_color *color = luaL_checkudata(L, 6, "binocle_color");
  kmAABB2 **viewport = lua_touserdata(L, 7);
  l_binocle_camera_t *camera = luaL_checkudata(L, 8, "binocle_camera");
  kmAABB2 rect;
  kmVec2 center;
  center.x = center_x;
  center.y = center_y;
  kmAABB2Initialize(&rect, &center, width, height, 0);
  binocle_gd_draw_rect(gd->gd, rect, *color, **viewport, camera->camera, NULL);
  return 0;
}

int l_binocle_gd_set_offscreen_clear_color(lua_State *L) {
  l_binocle_gd_t *gd = lua_touserdata(L, 1);
  float r = (float)luaL_checknumber(L, 2);
  float g = (float)luaL_checknumber(L, 3);
  float b = (float)luaL_checknumber(L, 4);
  float a = (float)luaL_checknumber(L, 5);
  sg_color color = {
    .r = r,
    .g = g,
    .b = b,
    .a = a
  };
  binocle_gd_set_offscreen_clear_color(gd->gd, color);
  return 0;
}

int l_binocle_gd_render_screen(lua_State *L) {
  l_binocle_gd_t *gd = luaL_checkudata(L, 1, "binocle_gd");
  l_binocle_window_t *window = luaL_checkudata(L, 2, "binocle_window");
  float width = lua_tonumber(L, 3);
  float height = lua_tonumber(L, 4);
  kmAABB2 **vp = lua_touserdata(L, 5);
  l_binocle_camera_t *camera = luaL_checkudata(L, 6, "binocle_camera");
  binocle_gd_render_screen(gd->gd, window->window, width, height, **vp, camera->camera->viewport_adapter->scale_matrix, camera->camera->viewport_adapter->inverse_multiplier);

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
  {"set_offscreen_clear_color", l_binocle_gd_set_offscreen_clear_color},
  {"draw_rect", l_binocle_gd_draw_rect},
  {"render_screen", l_binocle_gd_render_screen},
  {NULL, NULL}
};

int luaopen_gd(lua_State *L) {
  luaL_newlib(L, gd);
  lua_setglobal(L, "gd");
  luaL_newmetatable(L, "binocle_gd");
  luaL_newmetatable(L, "binocle_render_target");
  return 1;
}