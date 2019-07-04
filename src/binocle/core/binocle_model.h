//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_MODEL_H
#define BINOCLE_MODEL_H

#include <stdint.h>

struct binocle_material;

typedef struct binocle_mesh {
  uint64_t vertex_count; // number of vertices
  uint64_t triangle_count; // number of triangles
  float *vertices; // vertex position (XYZ)
  float *texture_coords; // vertex texture coordinates (UV)
  float *normals; // vertex normals (XYZ)
  float *tangents; // vertex tangents (XYZW)
  float *colors; // vertex colors (RGBA)
  uint64_t *indices; // vertex indices
} binocle_mesh;

typedef struct binocle_model {
  uint64_t mesh_count; // number of meshes
  binocle_mesh *meshes; // meshes
  uint64_t material_count; // number of materials
  struct binocle_material *materials; // materials
  uint64_t *mesh_materials; // mesh materials
} binocle_model;

binocle_model binocle_model_load_obj(char *filename);

#endif //BINOCLE_MODEL_H
