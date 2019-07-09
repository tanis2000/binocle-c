//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_model.h"
#include "binocle_sdl.h"
#include "binocle_log.h"
#include "binocle_material.h"
#include "binocle_texture.h"
#include "binocle_image.h"
#include "binocle_shader.h"
#include "binocle_vpct.h"

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include <tiniobj_loader_c/tinyobj_loader_c.h>

binocle_model binocle_model_load_obj(char *filename) {
  binocle_model model = {0};
  char *buffer = NULL;
  size_t buffer_length = 0;
  tinyobj_attrib_t attrib;
  tinyobj_shape_t *meshes = NULL;
  size_t mesh_count = 0;
  tinyobj_material_t *materials = NULL;
  size_t material_count = 0;
  uint64_t flags = TINYOBJ_FLAG_TRIANGULATE;

  if (!binocle_sdl_load_binary_file(filename, &buffer, &buffer_length)) {
    binocle_log_error("Cannot open OBJ file %s", filename);
    return model;
  }

  int res = tinyobj_parse_obj(&attrib, &meshes, &mesh_count, &materials, &material_count, buffer, buffer_length, flags);

  if (res != TINYOBJ_SUCCESS) {
    binocle_log_warning("Cannot load model data for %s", filename);
  } else {
    binocle_log_info("Loaded model data for %s: %i meshes and %i materials", filename, mesh_count, material_count);
  }

  model.mesh_count = 1; //mesh_count;
  model.meshes = malloc(model.mesh_count * sizeof(binocle_mesh));
  memset(model.meshes, 0, model.mesh_count * sizeof(binocle_model));
  model.material_count = material_count;
  model.materials = malloc(model.material_count * sizeof(binocle_material));
  memset(model.materials, 0, model.material_count * sizeof(binocle_material));
  model.mesh_materials = malloc(model.mesh_count * sizeof(uint64_t));
  memset(model.mesh_materials, 0, model.mesh_count * sizeof(uint64_t));

  for (int i = 0 ; i < model.mesh_count; i++) {
    binocle_mesh mesh = {0};
    memset(&mesh, 0, sizeof(binocle_mesh));
    kmMat4Identity(&mesh.transform);
    mesh.triangle_count = attrib.num_face_num_verts;
    mesh.vertex_count = attrib.num_face_num_verts * 3;
    mesh.vertices = malloc(mesh.vertex_count * sizeof(binocle_vpctn));
    memset(mesh.vertices, 0, mesh.vertex_count * sizeof(binocle_vpctn));
    mesh.indices = malloc(mesh.triangle_count * 3 * sizeof(uint32_t));
    memset(mesh.indices, 0, mesh.triangle_count * 3 * sizeof(uint32_t));

    /*
    for (int j = 0 ; j < mesh.vertex_count ; j++) {
      mesh.vertices[j].pos.x = attrib.vertices[j * 3 + 0];
      mesh.vertices[j].pos.y = attrib.vertices[j * 3 + 1];
      mesh.vertices[j].pos.z = attrib.vertices[j * 3 + 2];
    }
     */

    /*
    for (int j = 0 ; j < attrib.num_normals ; j++) {
      mesh.vertices[j].normal.x = attrib.normals[j * 3 + 0];
      mesh.vertices[j].normal.y = attrib.normals[j * 3 + 1];
      mesh.vertices[j].normal.z = attrib.normals[j * 3 + 2];
    }

    for (int j = 0 ; j < attrib.num_texcoords ; j++) {
      mesh.vertices[j].tex.x = attrib.texcoords[j * 2 + 0];
      mesh.vertices[j].tex.y = attrib.texcoords[j * 2 + 1];
    }
     */



    for (int j = 0 ; j < attrib.num_face_num_verts ; j++) {
      tinyobj_vertex_index_t idx_0 = attrib.faces[j * 3 + 0];
      tinyobj_vertex_index_t idx_1 = attrib.faces[j * 3 + 1];
      tinyobj_vertex_index_t idx_2 = attrib.faces[j * 3 + 2];

      mesh.vertices[j * 3 + 0].pos.x = attrib.vertices[idx_0.v_idx * 3 + 0];
      mesh.vertices[j * 3 + 0].pos.y = attrib.vertices[idx_0.v_idx * 3 + 1];
      mesh.vertices[j * 3 + 0].pos.z = attrib.vertices[idx_0.v_idx * 3 + 2];

      mesh.vertices[j * 3 + 1].pos.x = attrib.vertices[idx_1.v_idx * 3 + 0];
      mesh.vertices[j * 3 + 1].pos.y = attrib.vertices[idx_1.v_idx * 3 + 1];
      mesh.vertices[j * 3 + 1].pos.z = attrib.vertices[idx_1.v_idx * 3 + 2];

      mesh.vertices[j * 3 + 2].pos.x = attrib.vertices[idx_2.v_idx * 3 + 0];
      mesh.vertices[j * 3 + 2].pos.y = attrib.vertices[idx_2.v_idx * 3 + 1];
      mesh.vertices[j * 3 + 2].pos.z = attrib.vertices[idx_2.v_idx * 3 + 2];

      //mesh.indices[j * 3 + 0] = idx_0.v_idx;
      //mesh.indices[j * 3 + 1] = idx_1.v_idx;
      //mesh.indices[j * 3 + 2] = idx_2.v_idx;

      //binocle_log_info("Face %i index: v %i/%i/%i . vt %i/%i/%i . vn %i/%i/%i\n", j, idx_0.v_idx, idx_1.v_idx, idx_2.v_idx, idx_0.vt_idx, idx_1.vt_idx, idx_2.vt_idx, idx_0.vn_idx, idx_1.vn_idx, idx_2.vn_idx);

      if (idx_0.vt_idx >= 0) {
        mesh.vertices[j * 3 + 0].tex.x = attrib.texcoords[idx_0.vt_idx * 2 + 0];
        mesh.vertices[j * 3 + 0].tex.y = attrib.texcoords[idx_0.vt_idx * 2 + 1];
        //binocle_log_info("Face %i vert %i tex coords: %f %f", j, j * 3 + 0, mesh.vertices[j * 3 + 0].tex.x, mesh.vertices[j * 3 + 0].tex.y);
      }
      if (idx_1.vt_idx >= 0) {
        mesh.vertices[j * 3 + 1].tex.x = attrib.texcoords[idx_1.vt_idx * 2 + 0];
        mesh.vertices[j * 3 + 1].tex.y = attrib.texcoords[idx_1.vt_idx * 2 + 1];
        //binocle_log_info("Face %i vert %i tex coords: %f %f", j, j * 3 + 1, mesh.vertices[j * 3 + 1].tex.x, mesh.vertices[j * 3 + 1].tex.y);
      }
      if (idx_2.vt_idx >= 0) {
        mesh.vertices[j * 3 + 2].tex.x = attrib.texcoords[idx_2.vt_idx * 2 + 0];
        mesh.vertices[j * 3 + 2].tex.y = attrib.texcoords[idx_2.vt_idx * 2 + 1];
        //binocle_log_info("Face %i vert %i tex coords: %f %f", j, j * 3 + 2, mesh.vertices[j * 3 + 2].tex.x, mesh.vertices[j * 3 + 2].tex.y);
      }

      if (idx_0.vn_idx >= 0) {
        mesh.vertices[j * 3 + 0].normal.x = attrib.normals[idx_0.vn_idx * 3 + 0];
        mesh.vertices[j * 3 + 0].normal.y = attrib.normals[idx_0.vn_idx * 3 + 1];
        mesh.vertices[j * 3 + 0].normal.z = attrib.normals[idx_0.vn_idx * 3 + 2];
      }
      if (idx_1.vn_idx >= 0) {
        mesh.vertices[j * 3 + 1].normal.x = attrib.normals[idx_1.vn_idx * 3 + 0];
        mesh.vertices[j * 3 + 1].normal.y = attrib.normals[idx_1.vn_idx * 3 + 1];
        mesh.vertices[j * 3 + 1].normal.z = attrib.normals[idx_1.vn_idx * 3 + 2];
      }
      if (idx_2.vn_idx >= 0) {
        mesh.vertices[j * 3 + 2].normal.x = attrib.normals[idx_2.vn_idx * 3 + 0];
        mesh.vertices[j * 3 + 2].normal.y = attrib.normals[idx_2.vn_idx * 3 + 1];
        mesh.vertices[j * 3 + 2].normal.z = attrib.normals[idx_2.vn_idx * 3 + 2];
      }

    }

    for (int j = 0 ; j < mesh.vertex_count ; j++) {
      mesh.vertices[j].color.a = 1.0f;
      mesh.vertices[j].color.r = 1.0f;
      mesh.vertices[j].color.g = 1.0f;
      mesh.vertices[j].color.b = 1.0f;
    }


    binocle_material mesh_default_material = binocle_material_new();
    mesh_default_material.shader = &binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_FLAT];
    //mesh.material = &mesh_default_material;
    mesh.material = malloc(sizeof(binocle_material));
    memcpy(mesh.material, &mesh_default_material, sizeof(binocle_material));
    model.meshes[i] = mesh;
    model.mesh_materials[i] = attrib.material_ids[i];
  }


  for (int i = 0 ; i < material_count ; i++) {
    model.materials[i] = binocle_material_new();
    // We use diffuse only atm
    if (materials[i].diffuse_texname != NULL) {
      binocle_image image = binocle_image_load(materials[i].diffuse_texname);
      binocle_texture texture = binocle_texture_from_image(image);
      model.materials[i].texture = malloc(sizeof(binocle_texture));
      model.materials[i].texture = &texture;
      //memcpy(model.materials[i].texture, &texture, sizeof(binocle_texture));
      model.materials[i].shader = &binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_FLAT];
    }
  }

  tinyobj_attrib_free(&attrib);
  tinyobj_shapes_free(meshes, mesh_count);
  tinyobj_materials_free(materials, material_count);

  binocle_log_info("Model %s loaded successfully", filename);

  return model;
}

void binocle_model_compute_normal(float N[3], float v0[3], float v1[3], float v2[3]) {
  float v10[3];
  float v20[3];
  float len2;

  v10[0] = v1[0] - v0[0];
  v10[1] = v1[1] - v0[1];
  v10[2] = v1[2] - v0[2];

  v20[0] = v2[0] - v0[0];
  v20[1] = v2[1] - v0[1];
  v20[2] = v2[2] - v0[2];

  N[0] = v20[1] * v10[2] - v20[2] * v10[1];
  N[1] = v20[2] * v10[0] - v20[0] * v10[2];
  N[2] = v20[0] * v10[1] - v20[1] * v10[0];

  len2 = N[0] * N[0] + N[1] * N[1] + N[2] * N[2];
  if (len2 > 0.0f) {
    float len = (float)sqrt((double)len2);

    N[0] /= len;
    N[1] /= len;
  }
}