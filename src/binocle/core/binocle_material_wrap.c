//
// Created by Valerio Santinelli on 02/02/2020.
//

#include "binocle_material_wrap.h"
#include "binocle_lua.h"
#include "backend/binocle_material.h"
#include "binocle_sdl.h"
#include "binocle_texture_wrap.h"
#include "binocle_gd_wrap.h"

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

int l_binocle_material_set_pipeline(lua_State *L) {
  l_binocle_material_t *material = luaL_checkudata(L, 1, "binocle_material");
  l_binocle_shader_t *shader = luaL_checkudata(L, 2, "binocle_shader");
  material->material->pip = shader->pip;
  material->material->shader = shader->shader;
  material->material->shader_desc = shader->shader_desc;
  return 0;
}

int l_binocle_material_find_uniform_index(sg_shader_desc desc, const char *name) {
  for (int i = 0 ; i < SG_MAX_UB_MEMBERS ; i++) {
    if (desc.fs.uniform_blocks[0].uniforms[i].name != NULL && SDL_strcmp(desc.fs.uniform_blocks[0].uniforms[i].name, name) == 0) {
      return i;
    }
  }
  return -1;
}

int l_binocle_material_uniform_size(sg_uniform_type type) {
  switch (type) {
    case SG_UNIFORMTYPE_FLOAT:
      return sizeof(float);
      break;
    case SG_UNIFORMTYPE_FLOAT2:
      return sizeof(float) * 2;
      break;
    case SG_UNIFORMTYPE_FLOAT3:
      return sizeof(float) * 3;
      break;
    case SG_UNIFORMTYPE_FLOAT4:
      return sizeof(float) * 4;
      break;
    case SG_UNIFORMTYPE_INT:
      return sizeof(int);
      break;
    case SG_UNIFORMTYPE_INT2:
      return sizeof(int) * 2;
      break;
    case SG_UNIFORMTYPE_INT3:
      return sizeof(int) * 3;
      break;
    case SG_UNIFORMTYPE_INT4:
      return sizeof(int) * 4;
      break;
    case SG_UNIFORMTYPE_MAT4:
      return sizeof(float) * 16;
      break;
    default:
      return 0;
  }
  return 0;
}

int l_binocle_material_find_uniform_offset(sg_shader_desc desc, const char *stage, const char *name) {
  int idx = l_binocle_material_find_uniform_index(desc, name);
  if (idx == -1) {
    return -1;
  }
  int offset = 0;
  for (int i = 0 ; i < idx ; i++) {
    offset += l_binocle_material_uniform_size(desc.fs.uniform_blocks[0].uniforms[i].type);
  }
  return offset;
}

int l_binocle_material_set_uniform_float(lua_State *L) {
  l_binocle_material_t *material = luaL_checkudata(L, 1, "binocle_material");
  const char *stage = luaL_checkstring(L, 2);
  const char *name = luaL_checkstring(L, 3);
  float f1 = (float)luaL_checknumber(L, 4);

  int offset = l_binocle_material_find_uniform_offset(material->material->shader_desc, stage, name);
  memcpy(&material->material->custom_fs_uniforms[offset], &f1, sizeof(float));
  return 0;
}

int l_binocle_material_set_uniform_float4(lua_State *L) {
  l_binocle_material_t *material = luaL_checkudata(L, 1, "binocle_material");
  const char *stage = luaL_checkstring(L, 2);
  const char *name = luaL_checkstring(L, 3);
  float f1 = (float)luaL_checknumber(L, 4);
  float f2 = (float)luaL_checknumber(L, 5);
  float f3 = (float)luaL_checknumber(L, 6);
  float f4 = (float)luaL_checknumber(L, 7);

  int offset = l_binocle_material_find_uniform_offset(material->material->shader_desc, stage, name);
  memcpy(&material->material->custom_fs_uniforms[offset], &f1, sizeof(float));
  memcpy(&material->material->custom_fs_uniforms[offset + sizeof(float)], &f2, sizeof(float));
  memcpy(&material->material->custom_fs_uniforms[offset + sizeof(float)*2], &f3, sizeof(float));
  memcpy(&material->material->custom_fs_uniforms[offset + sizeof(float)*3], &f4, sizeof(float));
  return 0;
}


static const struct luaL_Reg image [] = {
  {"__gc", l_binocle_material_gc},
  {"new", l_binocle_material_new},
  {"set_texture", l_binocle_material_set_texture},
  {"set_shader", l_binocle_material_set_shader},
  {"destroy", l_binocle_material_destroy},
  {"set_pipeline", l_binocle_material_set_pipeline},
  {"set_uniform_float4", l_binocle_material_set_uniform_float4},
  {"set_uniform_float", l_binocle_material_set_uniform_float},
  {NULL, NULL}
};

int luaopen_material(lua_State *L) {
  luaL_newlib(L, image);
  lua_setglobal(L, "material");
  luaL_newmetatable(L, "binocle_material");
  lua_pop(L, 1);
  return 1;
}