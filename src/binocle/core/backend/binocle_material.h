//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_MATERIAL_H
#define BINOCLE_MATERIAL_H

#include "binocle_backend.h"
#include "binocle_blend.h"

/**
 * \brief A material as used internally by the engine
 */
typedef struct binocle_material {
  /// The blending mode
  binocle_blend blend_mode;
  /// The albedo/diffuse texture
  binocle_image albedo_texture;
  /// The normal/specular texture
  binocle_image normal_texture;
  /// The metallic texture
  binocle_image metallic_texture;
  /// The roughness texture
  binocle_image roughness_texture;
  /// The ambient occlusion texture
  binocle_image ao_texture;
  /// The shader (couple of VS/FS)
  binocle_shader shader;
} binocle_material;

/**
 * \brief Instantiates a new material
 * @return The new material
 */
binocle_material *binocle_material_new();

/**
 * \brief Frees the memory of the material
 * @param material the material to destroy
 */
void binocle_material_destroy(binocle_material *material);

#endif //BINOCLE_MATERIAL_H
