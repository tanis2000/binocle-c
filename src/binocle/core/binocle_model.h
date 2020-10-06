//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_MODEL_H
#define BINOCLE_MODEL_H

#include <stdint.h>
#include <kazmath/kazmath.h>
#include <khash/khash.h>
#include <tiniobj_loader_c/tinyobj_loader_c.h>

struct binocle_material;

typedef struct binocle_mesh {
  kmMat4 transform;
  uint64_t vertex_count; // number of vertices
  uint64_t triangle_count; // number of triangles
  struct binocle_vpctn *vertices; // vertex position (XYZ), color (RGBA), texture (UV)
  //float *vertices; // vertex position (XYZ)
  //float *texture_coords; // vertex texture coordinates (UV)
  //float *normals; // vertex normals (XYZ)
  float *tangents; // vertex tangents (XYZW)
  //float *colors; // vertex colors (RGBA)
  uint32_t *indices; // vertex indices
  struct binocle_material *material; // mesh material
} binocle_mesh;

typedef struct binocle_model {
  uint64_t mesh_count; // number of meshes
  binocle_mesh *meshes; // meshes
  uint64_t material_count; // number of materials
  struct binocle_material **materials; // materials
  uint64_t *mesh_materials; // mesh materials
} binocle_model;

typedef uint64_t binocle_model_smooth_vertex_key_t;
#define binocle_model_smooth_vertex_hash_func(key) (binocle_model_smooth_vertex_key_t)(key)
#define binocle_model_smooth_vertex_equal(a, b) ((a) == (b))
KHASH_INIT(spatial_binocle_smooth_vertex_t, binocle_model_smooth_vertex_key_t, kmVec3, 1, binocle_model_smooth_vertex_hash_func, binocle_model_smooth_vertex_equal)

binocle_model binocle_model_load_obj(char *filename, char *mtl_filename);
void binocle_model_compute_normal(float N[3], float v0[3], float v1[3], float v2[3]);
void binocle_model_compute_smoothing_normals(tinyobj_attrib_t *attrib, tinyobj_shape_t *shape, khash_t(spatial_binocle_smooth_vertex_t) *smooth_vertex_normals);

/**
 * Calculates the bounding box of the given mesh (in model space)
 * @param mesh the mesh
 * @param aabb_min minimum vertex of the bounding box
 * @param aabb_max maximum vertex of the bounding box
 */
void binocle_model_calculate_mesh_bounding_box(const struct binocle_mesh *mesh, kmVec3 *aabb_min, kmVec3 *aabb_max);

#endif //BINOCLE_MODEL_H
