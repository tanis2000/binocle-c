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

  model.mesh_count = mesh_count;
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
    mesh.vertex_count = attrib.num_faces * 3; // they are triangles
    mesh.triangle_count = attrib.num_faces;
    mesh.vertices = malloc(mesh.vertex_count * 3 * sizeof(float)); // XYZ
    memset(mesh.vertices, 0, mesh.vertex_count * 3 * sizeof(float));
    mesh.texture_coords = malloc(mesh.vertex_count * 2 * sizeof(float)); // UV
    memset(mesh.texture_coords, 0, mesh.vertex_count * 2 * sizeof(float));
    mesh.normals = malloc(mesh.vertex_count * 3 * sizeof(float)); // XYZ
    memset(mesh.normals, 0, mesh.vertex_count * 3 * sizeof(float));

    int v_count = 0;
    int vt_count = 0;
    int vn_count = 0;
    size_t face_offset = 0;

    for (int j = 0 ; j < attrib.num_face_num_verts ; j++) {
      assert(attrib.face_num_verts[i] % 3 == 0); // assume it's all triangle faces
      for (int f = 0 ; f < attrib.face_num_verts[i] / 3 ; f++) {
        tinyobj_vertex_index_t idx0 = attrib.faces[face_offset + f * 3 + 0];
        tinyobj_vertex_index_t idx1 = attrib.faces[face_offset + f * 3 + 1];
        tinyobj_vertex_index_t idx2 = attrib.faces[face_offset + f * 3 + 2];

        assert(idx0.v_idx >= 0);
        assert(idx1.v_idx >= 0);
        assert(idx2.v_idx >= 0);


        // vertices
        for (int k = 0 ; k < 3 ; k++) {
          mesh.vertices[v_count + k] = attrib.vertices[idx0.v_idx * 3 + k];
        }
        v_count += 3;

        for (int k = 0 ; k < 3 ; k++) {
          mesh.vertices[v_count + k] = attrib.vertices[idx1.v_idx * 3 + k];
        }
        v_count += 3;

        for (int k = 0 ; k < 3 ; k++) {
          mesh.vertices[v_count + k] = attrib.vertices[idx2.v_idx * 3 + k];
        }
        v_count += 3;



        // texture coords
        // we flip Y
        if (attrib.num_texcoords > 0) {
          mesh.texture_coords[vt_count + 0] = attrib.texcoords[idx0.vt_idx*2 + 0];
          mesh.texture_coords[vt_count + 1] = 1.0f - attrib.texcoords[idx0.vt_idx*2 + 1];
          vt_count += 2;

          mesh.texture_coords[vt_count + 0] = attrib.texcoords[idx1.vt_idx*2 + 0];
          mesh.texture_coords[vt_count + 1] = 1.0f - attrib.texcoords[idx1.vt_idx*2 + 1];
          vt_count += 2;

          mesh.texture_coords[vt_count + 0] = attrib.texcoords[idx2.vt_idx*2 + 0];
          mesh.texture_coords[vt_count + 1] = 1.0f - attrib.texcoords[idx2.vt_idx*2 + 1];
          vt_count += 2;
        }

        // normals
        if (attrib.num_normals > 0) {
          if (idx0.vn_idx >= 0 && idx1.vn_idx >= 0 && idx2.vn_idx >= 0) {
            assert(idx0.vn_idx < attrib.num_normals);
            assert(idx1.vn_idx < attrib.num_normals);
            assert(idx2.vn_idx < attrib.num_normals);
            for (int k = 0; k < 3; k++) {
              mesh.normals[vn_count + k] = attrib.normals[idx0.vn_idx*3 + k];
            }
            vn_count +=3;
            for (int k = 0; k < 3; k++) {
              mesh.normals[vn_count + k] = attrib.normals[idx1.vn_idx*3 + k];
            }
            vn_count +=3;
            for (int k = 0; k < 3; k++) {
              mesh.normals[vn_count + k] = attrib.normals[idx2.vn_idx*3 + k];
            }
            vn_count +=3;
          } else {
            // TODO: compute the normals for this face by hand
            vn_count +=3;
          }
        } else {
          // TODO: compute normals by hand
        }

      }
      face_offset += (size_t)attrib.face_num_verts[i];

    }
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