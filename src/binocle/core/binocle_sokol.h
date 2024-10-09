#ifndef BINOCLE_SOKOL_H
#define BINOCLE_SOKOL_H
#include "sokol_gfx.h"
#include "sokol_log.h"
void binocle_metal_init(void *mtl_view);
// sg_context_desc binocle_metal_get_context(void);
const void* binocle_metal_get_device(void);
const void* binocle_sokol_mtk_get_drawable(void);
const void* binocle_sokol_mtk_get_depth_stencil_texture(void);
const void* binocle_sokol_mtk_get_msaa_color_texture(void);
// GLuint binocle_sokol_tex_id(sg_image img_id);
#endif