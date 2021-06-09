//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_BACKEND_H
#define BINOCLE_BACKEND_H

#include "../binocle_pool.h"
#include "binocle_backend_types.h"
#include "binocle_texture.h"
#include "binocle_vpct.h"

#include <memory.h>
#include <stdbool.h>
#include <stdint.h>

struct binocle_material;

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
void LEGACY_binocle_backend_draw(const struct binocle_vpct *vertices, size_t vertex_count, struct binocle_material material,
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
bool binocle_backend_is_valid_rendertarget_color_format(binocle_pixel_format fmt);
bool binocle_backend_is_valid_rendertarget_depth_format(binocle_pixel_format fmt);
bool binocle_backend_is_depth_stencil_format(binocle_pixel_format fmt);
bool binocle_backend_is_compressed_pixel_format(binocle_pixel_format fmt);
void binocle_backend_shader_common_init(binocle_shader_common_t* cmn, const binocle_shader_desc* desc);
bool binocle_backend_strempty(const binocle_str_t* str);
const char* binocle_backend_strptr(const binocle_str_t* str);
void binocle_backend_strcpy(binocle_str_t* dst, const char* src);
int binocle_backend_uniform_size(binocle_uniform_type type, int count);
binocle_shader binocle_backend_make_shader(const binocle_shader_desc* desc);
uint32_t binocle_backend_surface_pitch(binocle_pixel_format fmt, uint32_t width, uint32_t height, uint32_t row_align);
uint32_t binocle_backend_row_pitch(binocle_pixel_format fmt, uint32_t width,
                                   uint32_t row_align);
void binocle_backend_pixelformat_all(binocle_pixelformat_info* pfi);
void binocle_backend_pixelformat_s(binocle_pixelformat_info* pfi);
void binocle_backend_pixelformat_sf(binocle_pixelformat_info* pfi);
void binocle_backend_pixelformat_sr(binocle_pixelformat_info* pfi);
void binocle_backend_pixelformat_srmd(binocle_pixelformat_info* pfi);
void binocle_backend_pixelformat_srm(binocle_pixelformat_info* pfi);
void binocle_backend_pixelformat_sfrm(binocle_pixelformat_info* pfi);
void binocle_backend_pixelformat_sbrm(binocle_pixelformat_info* pfi);
void binocle_backend_pixelformat_sbr(binocle_pixelformat_info* pfi);
void binocle_backend_pixelformat_sfbr(binocle_pixelformat_info* pfi);
void binocle_backend_mtl_init_caps(void);
void binocle_backend_pipeline_common_init(binocle_pipeline_common_t* cmn, const binocle_pipeline_desc* desc);
void binocle_pass_common_init(binocle_pass_common_t *cmn,
                              const binocle_pass_desc *desc);
binocle_pipeline binocle_backend_make_pipeline(const binocle_pipeline_desc* desc);
binocle_pass binocle_backend_make_pass(const binocle_pass_desc* desc);
void binocle_backend_begin_pass(binocle_pass pass_id, const binocle_pass_action* pass_action);
void binocle_backend_end_pass(void);
void binocle_backend_draw(int base_element, int num_elements, int num_instances);
void binocle_backend_apply_pipeline(binocle_pipeline pip_id);
void binocle_backend_apply_bindings(const binocle_bindings* bindings);
void binocle_backend_apply_uniforms(binocle_shader_stage stage, int ub_index, const binocle_range* data);
void binocle_backend_begin_default_pass(const binocle_pass_action* pass_action, int width, int height);
void binocle_backend_commit(void);
void binocle_backend_buffer_common_init(binocle_buffer_common_t* cmn, const binocle_buffer_desc* desc);
binocle_buffer binocle_backend_make_buffer(const binocle_buffer_desc* desc);
void binocle_backend_update_buffer(binocle_buffer buf_id, const binocle_range* data);
#endif // BINOCLE_BACKEND_H
