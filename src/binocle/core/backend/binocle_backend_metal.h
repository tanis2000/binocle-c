//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_BACKEND_METAL_H
#define BINOCLE_BACKEND_METAL_H

#include <TargetConditionals.h>
#import <Metal/Metal.h>
#include "binocle_backend_types.h"
#include "../binocle_pool.h"

typedef struct binocle_mtl_render_target {
  binocle_slot_t slot;
  // TODO: add more Metal properties here
} binocle_mtl_render_target;
typedef binocle_mtl_render_target binocle_render_target_t;

typedef struct binocle_mtl_backend_t {
  bool valid;
  id<MTLDevice> device;
  id<MTLCommandQueue> cmd_queue;
  id<MTLCommandBuffer> cmd_buffer;
  id<MTLRenderCommandEncoder> cmd_encoder;
  id<MTLBuffer> uniform_buffer;
} binocle_mtl_backend_t;

void binocle_backend_mtl_init(binocle_mtl_backend_t *mtl);

#endif // BINOCLE_BACKEND_METAL_H
