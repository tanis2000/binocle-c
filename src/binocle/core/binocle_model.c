//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include <tinyobj_loader_c/tinyobj_loader_c.h>

#include "binocle_model.h"
#include "binocle_sdl.h"
#include "binocle_log.h"
#include "backend/binocle_material.h"
#include "binocle_image.h"
#include "backend/binocle_vpct.h"
#include "cute_path/cute_path.h"


void binocle_model_file_reader_callback(void *ctx, const char *filename, int is_mtl, const char *obj_filename, char **buf, size_t *len) {
  binocle_model_buffers *buffers = (binocle_model_buffers *)ctx;
  if (is_mtl) {
    *buf = buffers->mtl.buffer;
    *len = buffers->mtl.buffer_length;
  } else {
    *buf = buffers->obj.buffer;
    *len = buffers->obj.buffer_length;
  }
  if (!binocle_sdl_load_binary_file(filename, buf, len)) {
    *len = 0;
    binocle_log_error("OBJ loader: cannot open file %s", filename);
    return;
  }
}

void binocle_model_get_mtl_filename(const char *filename, char *mtl_filename, int *length) {
  char name[CUTE_PATH_MAX_PATH];

  binocle_fs_path_without_extension(filename, '.', '/', name);
  sprintf(mtl_filename, "%s.mtl", name);
}

binocle_model binocle_model_load_obj(char *filename) {
  binocle_model model = {0};
  binocle_model_buffers buffers = {0};
  tinyobj_attrib_t attrib;
  tinyobj_shape_t *meshes = NULL;
  size_t mesh_count = 0;
  tinyobj_material_t *materials = NULL;
  size_t material_count = 0;
  uint64_t flags = TINYOBJ_FLAG_TRIANGULATE;
  khash_t(spatial_binocle_smooth_vertex_t) *smooth_vertex_normals;

  smooth_vertex_normals = kh_init(spatial_binocle_smooth_vertex_t);

  int res = tinyobj_parse_obj(&attrib, &meshes, &mesh_count, &materials, &material_count, filename, binocle_model_file_reader_callback, &buffers, flags);

  if (res != TINYOBJ_SUCCESS) {
    binocle_log_warning("Cannot load model data for %s", filename);
  } else {
    binocle_log_info("Loaded model data for %s: %i meshes and %i materials", filename, mesh_count, material_count);
  }

  char mtl_filename[CUTE_PATH_MAX_PATH];
  int mtl_filename_length = 0;

  binocle_model_get_mtl_filename(filename, (char *) &mtl_filename, &mtl_filename_length);

  res = tinyobj_parse_mtl_file(&materials, &material_count, mtl_filename, filename, binocle_model_file_reader_callback, &buffers);
  if (res != TINYOBJ_SUCCESS) {
    binocle_log_warning("Cannot load material data for %s", mtl_filename);
  } else {
    binocle_log_info("Loaded material data for %s: %i materials", mtl_filename, material_count);
  }

  binocle_log_info("# of vertices = %d", attrib.num_vertices);
  binocle_log_info("# of normals = %d", attrib.num_normals);
  binocle_log_info("# of texcoords = %d", attrib.num_texcoords);

  model.mesh_count = mesh_count;
  model.meshes = malloc(model.mesh_count * sizeof(binocle_mesh));
  memset(model.meshes, 0, model.mesh_count * sizeof(binocle_model));
  model.material_count = material_count;
  //model.materials = malloc(model.material_count * sizeof(binocle_material));
  //memset(model.materials, 0, model.material_count * sizeof(binocle_material));
  model.materials = malloc(model.material_count * sizeof(binocle_material *));
  memset(model.materials, 0, model.material_count * sizeof(binocle_material *));
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

    binocle_model_compute_smoothing_normals(&attrib, &meshes[i], smooth_vertex_normals);


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

//#define FAKE_NORMALS
#define USE_NORMALS

#if defined(FAKE_NORMALS)
      mesh.vertices[j * 3 + 0].normal.x = mesh.vertices[j * 3 + 0].pos.x;
      mesh.vertices[j * 3 + 0].normal.y = mesh.vertices[j * 3 + 0].pos.y;
      mesh.vertices[j * 3 + 0].normal.z = mesh.vertices[j * 3 + 0].pos.z;
      mesh.vertices[j * 3 + 1].normal.x = mesh.vertices[j * 3 + 1].pos.x;
      mesh.vertices[j * 3 + 1].normal.y = mesh.vertices[j * 3 + 1].pos.y;
      mesh.vertices[j * 3 + 1].normal.z = mesh.vertices[j * 3 + 1].pos.z;
      mesh.vertices[j * 3 + 2].normal.x = mesh.vertices[j * 3 + 2].pos.x;
      mesh.vertices[j * 3 + 2].normal.y = mesh.vertices[j * 3 + 2].pos.y;
      mesh.vertices[j * 3 + 2].normal.z = mesh.vertices[j * 3 + 2].pos.z;
#else
#if defined(USE_NORMALS)
      if (attrib.num_normals > 0) {
        if (idx_0.vn_idx >= 0 && idx_1.vn_idx >= 0 && idx_2.vn_idx >= 0) {
          mesh.vertices[j * 3 + 0].normal.x = attrib.normals[idx_0.vn_idx * 3 + 0];
          mesh.vertices[j * 3 + 0].normal.y = attrib.normals[idx_0.vn_idx * 3 + 1];
          mesh.vertices[j * 3 + 0].normal.z = attrib.normals[idx_0.vn_idx * 3 + 2];

          mesh.vertices[j * 3 + 1].normal.x = attrib.normals[idx_1.vn_idx * 3 + 0];
          mesh.vertices[j * 3 + 1].normal.y = attrib.normals[idx_1.vn_idx * 3 + 1];
          mesh.vertices[j * 3 + 1].normal.z = attrib.normals[idx_1.vn_idx * 3 + 2];

          mesh.vertices[j * 3 + 2].normal.x = attrib.normals[idx_2.vn_idx * 3 + 0];
          mesh.vertices[j * 3 + 2].normal.y = attrib.normals[idx_2.vn_idx * 3 + 1];
          mesh.vertices[j * 3 + 2].normal.z = attrib.normals[idx_2.vn_idx * 3 + 2];
        } else {
          float v[3][3];
          float n[3][3];
          v[0][0] = mesh.vertices[j * 3 + 0].pos.x;
          v[0][1] = mesh.vertices[j * 3 + 0].pos.y;
          v[0][2] = mesh.vertices[j * 3 + 0].pos.z;
          v[1][0] = mesh.vertices[j * 3 + 1].pos.x;
          v[1][1] = mesh.vertices[j * 3 + 1].pos.y;
          v[1][2] = mesh.vertices[j * 3 + 1].pos.z;
          v[2][0] = mesh.vertices[j * 3 + 2].pos.x;
          v[2][1] = mesh.vertices[j * 3 + 2].pos.y;
          v[2][2] = mesh.vertices[j * 3 + 2].pos.z;
          binocle_model_compute_normal(n[0], v[0], v[1], v[2]);
          mesh.vertices[j * 3 + 0].normal.x = n[0][0];
          mesh.vertices[j * 3 + 0].normal.y = n[0][1];
          mesh.vertices[j * 3 + 0].normal.z = n[0][2];
          mesh.vertices[j * 3 + 1].normal.x = n[0][0];
          mesh.vertices[j * 3 + 1].normal.y = n[0][1];
          mesh.vertices[j * 3 + 1].normal.z = n[0][2];
          mesh.vertices[j * 3 + 2].normal.x = n[0][0];
          mesh.vertices[j * 3 + 2].normal.y = n[0][1];
          mesh.vertices[j * 3 + 2].normal.z = n[0][2];
        }
      } else {
        float v[3][3];
        float n[3][3];
        v[0][0] = mesh.vertices[j * 3 + 0].pos.x;
        v[0][1] = mesh.vertices[j * 3 + 0].pos.y;
        v[0][2] = mesh.vertices[j * 3 + 0].pos.z;
        v[1][0] = mesh.vertices[j * 3 + 1].pos.x;
        v[1][1] = mesh.vertices[j * 3 + 1].pos.y;
        v[1][2] = mesh.vertices[j * 3 + 1].pos.z;
        v[2][0] = mesh.vertices[j * 3 + 2].pos.x;
        v[2][1] = mesh.vertices[j * 3 + 2].pos.y;
        v[2][2] = mesh.vertices[j * 3 + 2].pos.z;
        binocle_model_compute_normal(n[0], v[0], v[1], v[2]);
        mesh.vertices[j * 3 + 0].normal.x = n[0][0];
        mesh.vertices[j * 3 + 0].normal.y = n[0][1];
        mesh.vertices[j * 3 + 0].normal.z = n[0][2];
        mesh.vertices[j * 3 + 1].normal.x = n[0][0];
        mesh.vertices[j * 3 + 1].normal.y = n[0][1];
        mesh.vertices[j * 3 + 1].normal.z = n[0][2];
        mesh.vertices[j * 3 + 2].normal.x = n[0][0];
        mesh.vertices[j * 3 + 2].normal.y = n[0][1];
        mesh.vertices[j * 3 + 2].normal.z = n[0][2];
      }
#else
      // Use smoothing normals
      int f0 = idx_0.v_idx;
      int f1 = idx_1.v_idx;
      int f2 = idx_2.v_idx;

      if (f0 >= 0 && f1 >= 0 && f2 >= 0) {
        khiter_t iter = kh_get(spatial_binocle_smooth_vertex_t, smooth_vertex_normals, f0);
        if (iter != kh_end(smooth_vertex_normals)) {
          // found
          kmVec3 item = kh_val(smooth_vertex_normals, iter);
          mesh.vertices[j * 3 + 0].normal.x = item.x;
          mesh.vertices[j * 3 + 0].normal.x = item.y;
          mesh.vertices[j * 3 + 0].normal.x = item.z;
        } else {
          binocle_log_error("Missing smoothed normal for index %d and face %d", j, f0);
        }

        iter = kh_get(spatial_binocle_smooth_vertex_t, smooth_vertex_normals, f1);
        if (iter != kh_end(smooth_vertex_normals)) {
          // found
          kmVec3 item = kh_val(smooth_vertex_normals, iter);
          mesh.vertices[j * 3 + 1].normal.x = item.x;
          mesh.vertices[j * 3 + 1].normal.x = item.y;
          mesh.vertices[j * 3 + 1].normal.x = item.z;
        } else {
          binocle_log_error("Missing smoothed normal for index %d and face %d", j, f1);
        }

        iter = kh_get(spatial_binocle_smooth_vertex_t, smooth_vertex_normals, f2);
        if (iter != kh_end(smooth_vertex_normals)) {
          // found
          kmVec3 item = kh_val(smooth_vertex_normals, iter);
          mesh.vertices[j * 3 + 2].normal.x = item.x;
          mesh.vertices[j * 3 + 2].normal.x = item.y;
          mesh.vertices[j * 3 + 2].normal.x = item.z;
        } else {
          binocle_log_error("Missing smoothed normal for index %d and face %d", j, f2);
        }
      } else {
        binocle_log_error("Missing face index for smoothed normal for index %d", j);
      }
#endif //defined(USE_NORMALS)
#endif //defined(FAKE_NORMALS)




    }

    for (int j = 0 ; j < mesh.vertex_count ; j++) {
      mesh.vertices[j].color.a = 1.0f;
      mesh.vertices[j].color.r = 1.0f;
      mesh.vertices[j].color.g = 1.0f;
      mesh.vertices[j].color.b = 1.0f;
    }

    binocle_material *mesh_default_material = binocle_material_new();
    // TODO: fix this now that we no longer have default shaders
    //mesh_default_material->shader = &binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_FLAT];
    mesh.material = malloc(sizeof(binocle_material));
    memcpy(mesh.material, mesh_default_material, sizeof(binocle_material));
    model.meshes[i] = mesh;
    model.mesh_materials[i] = attrib.material_ids[i];
  }


  for (int i = 0 ; i < material_count ; i++) {
    binocle_material *mat = binocle_material_new();
    model.materials[i] = mat;
    if (materials[i].diffuse_texname != NULL) {
      sg_image image = binocle_image_load(materials[i].diffuse_texname);
      model.materials[i]->albedo_texture = image;
    }

    if (materials[i].bump_texname != NULL) {
      sg_image image = binocle_image_load(materials[i].bump_texname);
      model.materials[i]->normal_texture = image;
    }

    if (materials[i].ambient_texname != NULL) {
      sg_image image = binocle_image_load(materials[i].ambient_texname);
      model.materials[i]->ao_texture = image;
    }

    if (materials[i].metallic_texname != NULL) {
      sg_image image = binocle_image_load(materials[i].metallic_texname);
      model.materials[i]->metallic_texture = image;
    }

    if (materials[i].specular_highlight_texname != NULL) {
      sg_image image = binocle_image_load(materials[i].specular_highlight_texname);
      model.materials[i]->roughness_texture = image;
    }
  }

  // Fix up the materials of each mesh.
  // TODO: check that we are assigning the right material and not just the first one.
  for (int i = 0 ; i < mesh_count ; i++) {
    model.meshes[i].material = model.materials[model.mesh_materials[i]];
  }

  tinyobj_attrib_free(&attrib);
  tinyobj_shapes_free(meshes, mesh_count);
  tinyobj_materials_free(materials, material_count);
  SDL_free(buffers.obj.buffer);
  SDL_free(buffers.mtl.buffer);

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
    float len = sqrtf(len2);

    N[0] /= len;
    N[1] /= len;
    N[2] /= len;
  }
}

void binocle_model_compute_smoothing_normals(tinyobj_attrib_t *attrib, tinyobj_shape_t *shape, khash_t(spatial_binocle_smooth_vertex_t) *smooth_vertex_normals) {
  kh_clear(spatial_binocle_smooth_vertex_t, smooth_vertex_normals);

  for (int j = 0 ; j < attrib->num_face_num_verts ; j++) {
    tinyobj_vertex_index_t idx_0 = attrib->faces[j * 3 + 0];
    tinyobj_vertex_index_t idx_1 = attrib->faces[j * 3 + 1];
    tinyobj_vertex_index_t idx_2 = attrib->faces[j * 3 + 2];

    int vi[3]; // indexes
    float v[3][3]; // coordinates

    for (int k = 0; k < 3; k++) {
      vi[0] = idx_0.v_idx;
      vi[1] = idx_1.v_idx;
      vi[2] = idx_2.v_idx;
      assert(vi[0] >= 0);
      assert(vi[1] >= 0);
      assert(vi[2] >= 0);

      v[0][k] = attrib->vertices[3 * vi[0] + k];
      v[1][k] = attrib->vertices[3 * vi[1] + k];
      v[2][k] = attrib->vertices[3 * vi[2] + k];
    }

    // Compute the normal of the face
    float normal[3];
    binocle_model_compute_normal(normal, v[0], v[1], v[2]);

    // Add the normal to the three vertexes
    for (size_t i = 0; i < 3; ++i) {
      khiter_t iter = kh_get(spatial_binocle_smooth_vertex_t, smooth_vertex_normals, vi[i]);
      if (iter != kh_end(smooth_vertex_normals)) {
        // found
        kmVec3 item = kh_val(smooth_vertex_normals, iter);
        item.x += normal[0];
        item.y += normal[1];
        item.z += normal[2];
        kh_value(smooth_vertex_normals, iter).x = item.x;
        kh_value(smooth_vertex_normals, iter).y = item.y;
        kh_value(smooth_vertex_normals, iter).z = item.z;
      } else {
        // not found
        int ret;
        khiter_t k = kh_put(spatial_binocle_smooth_vertex_t, smooth_vertex_normals, vi[i], &ret);
        kh_value(smooth_vertex_normals, k).x = normal[0];
        kh_value(smooth_vertex_normals, k).y = normal[1];
        kh_value(smooth_vertex_normals, k).z = normal[2];
      }
    }
  }

  // Normalize the normals, that is, make them unit vectors
  khiter_t iter;
  for (iter = 0 ; iter < kh_end(smooth_vertex_normals); ++iter) {
    kmVec3 item = kh_val(smooth_vertex_normals, iter);
    kmVec3Normalize(&item, &item);
    kh_value(smooth_vertex_normals, iter).x = item.x;
    kh_value(smooth_vertex_normals, iter).y = item.y;
    kh_value(smooth_vertex_normals, iter).z = item.z;
  }
}

void binocle_model_calculate_mesh_bounding_box(const struct binocle_mesh *mesh, kmVec3 *aabb_min, kmVec3 *aabb_max) {
  float min_x, min_y, min_z, max_x, max_y, max_z;

  min_x = mesh->vertices[0].pos.x;
  min_y = mesh->vertices[0].pos.y;
  min_z = mesh->vertices[0].pos.z;

  for(int i = 0; i < mesh->vertex_count; i++) {
    // x-axis
    if(mesh->vertices[i].pos.x < min_x)
      min_x = mesh->vertices[i].pos.x;
    if(mesh->vertices[i].pos.x > max_x)
      max_x = mesh->vertices[i].pos.x;
    // y-axis
    if(mesh->vertices[i].pos.y < min_y)
      min_y = mesh->vertices[i].pos.y;
    if(mesh->vertices[i].pos.y > max_y)
      max_y = mesh->vertices[i].pos.y;
    // z-axis
    if(mesh->vertices[i].pos.z < min_z)
      min_z = mesh->vertices[i].pos.z;
    if(mesh->vertices[i].pos.z > max_z)
      max_z = mesh->vertices[i].pos.z;
  }

  kmVec3Fill(aabb_min, min_x, min_y, min_z);
  kmVec3Fill(aabb_max, max_x, max_y, max_z);
}