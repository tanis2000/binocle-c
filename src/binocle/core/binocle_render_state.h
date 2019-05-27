//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_RENDER_STATE_H
#define BINOCLE_RENDER_STATE_H

#include "binocle_blend.h"
#include <kazmath/kazmath.h>

struct binocle_texture;
struct binocle_shader;

/**
 * \brief A snapshot of the current render state
 * At the moment it's being used only by the sprite batcher to optimize the number of draw calls.
 */
typedef struct binocle_render_state {
  binocle_blend blend_mode;
  kmMat4 transform;
  struct binocle_texture *texture;
  struct binocle_shader *shader;
  kmAABB2 viewport;
} binocle_render_state;

/**
 * \brief Creates a new render state
 * Blending is set to default values
 * @param texture The current texture
 * @param shader  The current shader
 * @return the render state
 */
binocle_render_state binocle_render_state_new(struct binocle_texture *texture, struct binocle_shader *shader);

#endif //BINOCLE_RENDER_STATE_H
