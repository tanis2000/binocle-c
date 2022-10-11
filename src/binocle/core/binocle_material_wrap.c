//
// Created by Valerio Santinelli on 02/02/2020.
//

#include "binocle_material_wrap.h"
#include "binocle_lua.h"
#include "backend/binocle_material.h"
#include "binocle_sdl.h"
#include "binocle_texture_wrap.h"

int l_binocle_material_new(lua_State *L) {
  l_binocle_material_t *material = lua_newuserdata(L, sizeof(l_binocle_material_t));
  lua_getfield(L, LUA_REGISTRYINDEX, "binocle_material");
  lua_setmetatable(L, -2);
  SDL_memset(material, 0, sizeof(*material));
  binocle_material *mtl = binocle_material_new();
  material->material = mtl;
  return 1;
}

int l_binocle_material_set_texture(lua_State *L) {
  l_binocle_material_t *material = luaL_checkudata(L, 1, "binocle_material");
  l_binocle_texture_t *texture = luaL_checkudata(L, 2, "binocle_texture");
  material->material->albedo_texture = texture->texture;
  return 0;
}

int l_binocle_material_set_shader(lua_State *L) {
  l_binocle_material_t *material = luaL_checkudata(L, 1, "binocle_material");
  sg_shader *shader = lua_touserdata(L, 2);
  if (shader == NULL) {
    return -1;
  }
  material->material->shader = *shader;
  return 0;
}

int l_binocle_material_destroy(lua_State *L) {
  l_binocle_material_t *material = luaL_checkudata(L, 1, "binocle_material");
  binocle_material_destroy(material->material);
  return 0;
}

int l_binocle_material_gc(lua_State *L) {
  l_binocle_material_t *material = luaL_checkudata(L, 1, "binocle_material");
  binocle_material_destroy(material->material);
  return 0;
}

static const struct luaL_Reg image [] = {
  {"__gc", l_binocle_material_gc},
  {"new", l_binocle_material_new},
  {"set_texture", l_binocle_material_set_texture},
  {"set_shader", l_binocle_material_set_shader},
  {"destroy", l_binocle_material_destroy},
  {NULL, NULL}
};

int luaopen_material(lua_State *L) {
  luaL_newlib(L, image);
  lua_setglobal(L, "material");
  luaL_newmetatable(L, "binocle_material");
  return 1;
}