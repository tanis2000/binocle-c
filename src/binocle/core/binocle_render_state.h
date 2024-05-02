//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_RENDER_STATE_H
#define BINOCLE_RENDER_STATE_H

#include "backend/binocle_blend.h"
#include <kazmath/kazmath.h>
#include "sokol_gfx.h"

/**
 * \brief A snapshot of the current render state
 * At the moment it's being used only by the sprite batcher to optimize the number of draw calls.
 */
typedef struct binocle_render_state {
  sg_blend_state blend_mode;
  kmMat4 transform;
  sg_shader *shader;
  kmAABB2 viewport;
  struct binocle_material *material;
} binocle_render_state;

/**
 * \brief Creates a new render state
 * Blending is set to default values
 * @param texture The current texture
 * @param shader  The current shader
 * @return the render state
 */
binocle_render_state binocle_render_state_new(struct binocle_material *material, struct sg_shader *shader);

#endif //BINOCLE_RENDER_STATE_H
