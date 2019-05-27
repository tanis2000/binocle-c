//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_MATERIAL_H
#define BINOCLE_MATERIAL_H

#include "binocle_blend.h"

struct binocle_texture;
struct binocle_shader;

/**
 * \brief A material as used internally by the engine
 */
typedef struct binocle_material {
  /// The blending mode
  binocle_blend blend_mode;
  /// The texture
  struct binocle_texture *texture;
  /// The shader (couple of VS/FS)
  struct binocle_shader *shader;
} binocle_material;

/**
 * \brief Instantiates a new material
 * @return The new material
 */
binocle_material binocle_material_new();

#endif //BINOCLE_MATERIAL_H
