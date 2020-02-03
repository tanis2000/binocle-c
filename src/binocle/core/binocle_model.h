//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_MODEL_H
#define BINOCLE_MODEL_H

#include <stdint.h>
#include <kazmath/kazmath.h>

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

binocle_model binocle_model_load_obj(char *filename, char *mtl_filename);
void binocle_model_compute_normal(float N[3], float v0[3], float v1[3], float v2[3]);

#endif //BINOCLE_MODEL_H
