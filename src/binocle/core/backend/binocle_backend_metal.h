//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_BACKEND_METAL_H
#define BINOCLE_BACKEND_METAL_H

#include <TargetConditionals.h>
#import <Metal/Metal.h>

typedef struct binocle_mtl_backend_t {
  bool valid;
  id<MTLDevice> device;
  id<MTLCommandQueue> cmd_queue;
  id<MTLCommandBuffer> cmd_buffer;
  id<MTLRenderCommandEncoder> cmd_encoder;
  id<MTLBuffer> uniform_buffer;
} binocle_mtl_backend_t;

#endif // BINOCLE_BACKEND_METAL_H
