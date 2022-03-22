#ifndef BINOCLE_SOKOL_H
#define BINOCLE_SOKOL_H
#include "sokol_gfx.h"
void binocle_metal_init(void *mtl_view);
sg_context_desc binocle_metal_get_context(void);
#endif