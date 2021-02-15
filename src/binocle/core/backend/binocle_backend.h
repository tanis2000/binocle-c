//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_BACKEND_H
#define BINOCLE_BACKEND_H

#include "../binocle_pool.h"
#include "binocle_backend_types.h"
#include "binocle_blend.h"
#include "binocle_texture.h"
#include "binocle_vpct.h"

#include <memory.h>
#include <stdbool.h>
#include <stdint.h>

struct binocle_material;

typedef struct binocle_image { uint32_t id; } binocle_image;
typedef struct binocle_shader { uint32_t id; } binocle_shader;

typedef struct binocle_slot_info {
  binocle_resource_state state;    /* the current state of this resource slot */
  uint32_t res_id;        /* type-neutral resource if (e.g. sg_buffer.id) */
  uint32_t ctx_id;        /* the context this resource belongs to */
} binocle_slot_info;

typedef struct binocle_image_info {
  binocle_slot_info slot;              /* resource pool slot info */
  uint32_t upd_frame_index;       /* frame index of last sg_update_image() */
  int num_slots;                  /* number of renaming-slots for dynamically updated images */
  int active_slot;                /* currently active write-slot for dynamically updated images */
  int width;                      /* image width */
  int height;                     /* image height */
} binocle_image_info;

void binocle_backend_setup(const binocle_backend_desc* desc);
void binocle_backend_apply_default_state();
void binocle_backend_apply_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
void binocle_backend_apply_blend_mode(struct binocle_blend blend_mode);
void binocle_backend_apply_shader(binocle_shader shader);
void binocle_backend_apply_texture(binocle_image texture);
void binocle_backend_apply_3d_texture(struct binocle_material *material);
void binocle_backend_draw(const struct binocle_vpct *vertices, size_t vertex_count, struct binocle_material material,
                          struct kmAABB2 viewport, struct kmMat4 *cameraTransformMatrix);
void binocle_backend_set_render_target(binocle_image rt);
void binocle_backend_unset_render_target();
void binocle_backend_clear(struct binocle_color color);
void binocle_backend_set_uniform_float2(binocle_shader shader, const char *name, float value1, float value2);
void binocle_backend_set_uniform_mat4(binocle_shader shader, const char *name, struct kmMat4 mat);
void binocle_backend_draw_quad_to_screen(binocle_shader shader, binocle_image rt);
binocle_image binocle_backend_make_image(const binocle_image_desc* desc);
void binocle_backend_destroy_image(binocle_image img);
binocle_image_info binocle_backend_query_image_info(binocle_image img_id);
bool binocle_backend_is_valid_rendertarget_depth_format(binocle_pixel_format fmt);
bool binocle_backend_is_compressed_pixel_format(binocle_pixel_format fmt);
void binocle_backend_shader_common_init(binocle_shader_common_t* cmn, const binocle_shader_desc* desc);
void binocle_backend_strcpy(binocle_str_t* dst, const char* src);
int binocle_backend_uniform_size(binocle_uniform_type type, int count);
binocle_shader binocle_backend_make_shader(const binocle_shader_desc* desc);

#endif // BINOCLE_BACKEND_H
