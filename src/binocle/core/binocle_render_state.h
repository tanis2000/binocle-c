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

typedef struct binocle_render_state {
  binocle_blend blend_mode;
  kmMat4 transform;
  struct binocle_texture *texture;
  struct binocle_shader *shader;
  kmAABB2 viewport;
} binocle_render_state;

binocle_render_state binocle_render_state_new(struct binocle_texture *texture, struct binocle_shader *shader);

#endif //BINOCLE_RENDER_STATE_H
