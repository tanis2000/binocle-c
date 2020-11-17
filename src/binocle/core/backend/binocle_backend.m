//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_backend.h"
#if defined(BINOCLE_GL)
#include "binocle_backend_gl.h"
#elif defined(BINOCLE_METAL)
#include "binocle_backend_metal.h"
#endif

typedef struct binocle_backend_t {
#if defined(BINOCLE_GL)
  binocle_gl_backend_t gl;
#elif defined(BINOCLE_METAL)
  binocle_mtl_backend_t mtl;
#else
#error("no backend defined")
#endif
} binocle_backend_t;

void binocle_backend_init() {

}