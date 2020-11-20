//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_BACKEND_H
#define BINOCLE_BACKEND_H

#include <stdint.h>
#include <stdbool.h>
#include "binocle_blend.h"
#include "binocle_material.h"
#include "binocle_shader.h"
#include "binocle_texture.h"
#include "binocle_vpct.h"
#include "../binocle_pool.h"

typedef struct binocle_render_target { uint32_t id; } binocle_render_target;

typedef enum binocle_pixel_format {
  BINOCLE_PIXEL_FORMAT_DEFAULT,
  BINOCLE_PIXEL_FORMAT_NONE,
  BINOCLE_PIXEL_FORMAT_RGB,
  BINOCLE_PIXEL_FORMAT_RGBA,
} binocle_pixel_format;

typedef struct binocle_render_target_desc {
  uint32_t width;
  uint32_t height;
  bool use_depth;
  binocle_pixel_format format;
} binocle_render_target_desc;

void binocle_backend_init();
void binocle_backend_apply_default_state();
void binocle_backend_apply_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
void binocle_backend_apply_blend_mode(struct binocle_blend blend_mode);
void binocle_backend_apply_shader(struct binocle_shader *shader);
void binocle_backend_apply_texture(binocle_texture texture);
void binocle_backend_apply_3d_texture(struct binocle_material *material);
void binocle_backend_draw(const struct binocle_vpct *vertices, size_t vertex_count, struct binocle_material material,
                          struct kmAABB2 viewport, struct kmMat4 *cameraTransformMatrix);
binocle_render_target binocle_backend_create_render_target(binocle_render_target_desc *desc);
void binocle_backend_destroy_render_target(binocle_render_target rt);
void binocle_backend_set_render_target(binocle_render_target *rt);
void binocle_backend_clear(struct binocle_color color);
void binocle_backend_set_uniform_float2(struct binocle_shader *shader, const char *name, float value1, float value2);
void binocle_backend_set_uniform_mat4(struct binocle_shader *shader, const char *name, struct kmMat4 mat);
void binocle_backend_draw_quad_to_screen(struct binocle_shader *shader, binocle_render_target *rt);

#endif // BINOCLE_BACKEND_H
