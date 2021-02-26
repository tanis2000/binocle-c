//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_BACKEND_METAL_H
#define BINOCLE_BACKEND_METAL_H

#include <TargetConditionals.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>
#import <Cocoa/Cocoa.h>
#include "binocle_backend_types.h"
#include "../binocle_pool.h"
#include "binocle_sampler_cache.h"

#define BINOCLE_MTL_INVALID_SLOT_INDEX (0)

struct binocle_color;

typedef struct binocle_mtl_render_target {
  binocle_slot_t slot;
  // TODO: add more Metal properties here
} binocle_mtl_render_target;
typedef binocle_mtl_render_target binocle_render_target_t;

typedef struct binocle_mtl_image {
  binocle_slot_t slot;
  binocle_image_common_t cmn;
  struct {
    uint32_t tex[BINOCLE_NUM_INFLIGHT_FRAMES];
    uint32_t depth_tex;
    uint32_t msaa_tex;
    uint32_t sampler_state;
  } mtl;
} binocle_mtl_image;
typedef binocle_mtl_image binocle_image_t;

typedef struct binocle_mtl_shader_stage_t {
  uint32_t mtl_lib;
  uint32_t mtl_func;
} binocle_mtl_shader_stage_t;

typedef struct binocle_mtl_shader {
  binocle_slot_t slot;
  binocle_shader_common_t cmn;
  struct {
    binocle_mtl_shader_stage_t stage[BINOCLE_NUM_SHADER_STAGES];
  } mtl;
} binocle_mtl_shader;
typedef binocle_mtl_shader binocle_shader_t;

typedef struct binocle_mtl_pipeline_t {
  binocle_slot_t slot;
  binocle_pipeline_common_t cmn;
  binocle_shader_t* shader;
  struct {
    MTLPrimitiveType prim_type;
    int index_size;
    MTLIndexType index_type;
    MTLCullMode cull_mode;
    MTLWinding winding;
    uint32_t stencil_ref;
    int rps;
    int dss;
  } mtl;
} binocle_mtl_pipeline_t;
typedef binocle_mtl_pipeline_t binocle_pipeline_t;

typedef struct binocle_mtl_attachment_t {
  binocle_image_t* image;
} binocle_mtl_attachment_t;

typedef struct binocle_mtl_pass_t {
  binocle_slot_t slot;
  binocle_pass_common_t cmn;
  struct {
    binocle_mtl_attachment_t color_atts[BINOCLE_MAX_COLOR_ATTACHMENTS];
    binocle_mtl_attachment_t ds_att;
  } mtl;
} binocle_mtl_pass_t;
typedef binocle_mtl_pass_t binocle_pass_t;
typedef binocle_pass_attachment_common_t binocle_pass_attachment_t;

typedef struct binocle_mtl_release_item_t {
  uint32_t frame_index;   /* frame index at which it is safe to release this resource */
  uint32_t slot_index;
} binocle_mtl_release_item_t;

typedef struct binocle_mtl_idpool_t {
  NSMutableArray* pool;
  uint32_t num_slots;
  uint32_t free_queue_top;
  uint32_t* free_queue;
  uint32_t release_queue_front;
  uint32_t release_queue_back;
  binocle_mtl_release_item_t* release_queue;
} binocle_mtl_idpool_t;

typedef struct binocle_mtl_state_cache_t {
//  const _sg_pipeline_t* cur_pipeline;
//  sg_pipeline cur_pipeline_id;
//  const _sg_buffer_t* cur_indexbuffer;
//  int cur_indexbuffer_offset;
//  sg_buffer cur_indexbuffer_id;
//  const _sg_buffer_t* cur_vertexbuffers[SG_MAX_SHADERSTAGE_BUFFERS];
//  int cur_vertexbuffer_offsets[SG_MAX_SHADERSTAGE_BUFFERS];
//  sg_buffer cur_vertexbuffer_ids[SG_MAX_SHADERSTAGE_BUFFERS];
//  const _sg_image_t* cur_vs_images[SG_MAX_SHADERSTAGE_IMAGES];
//  sg_image cur_vs_image_ids[SG_MAX_SHADERSTAGE_IMAGES];
//  const _sg_image_t* cur_fs_images[SG_MAX_SHADERSTAGE_IMAGES];
//  sg_image cur_fs_image_ids[SG_MAX_SHADERSTAGE_IMAGES];
} binocle_mtl_state_cache_t;

typedef struct binocle_mtl_backend_t {
  bool valid;
  const void*(*renderpass_descriptor_cb)(void);
  const void*(*renderpass_descriptor_userdata_cb)(void*);
  const void*(*drawable_cb)(void);
  const void*(*drawable_userdata_cb)(void*);
  void* user_data;
  uint32_t frame_index;
  uint32_t cur_frame_rotate_index;
  int ub_size;
  int cur_ub_offset;
  uint8_t* cur_ub_base_ptr;
  bool in_pass;
  bool pass_valid;
  int cur_width;
  int cur_height;
  binocle_mtl_state_cache_t state_cache;
  binocle_sampler_cache_t sampler_cache;
  binocle_mtl_idpool_t idpool;
  dispatch_semaphore_t sem;
  id<MTLDevice> device;
  id<MTLCommandQueue> cmd_queue;
  id<MTLCommandBuffer> cmd_buffer;
  id<MTLRenderCommandEncoder> cmd_encoder;
  id<MTLBuffer> uniform_buffers[BINOCLE_NUM_INFLIGHT_FRAMES];
  CAMetalLayer *layer;
} binocle_mtl_backend_t;

void binocle_backend_mtl_setup_backend(binocle_mtl_backend_t *mtl, binocle_backend_desc *desc);
binocle_resource_state
binocle_backend_mtl_create_image(binocle_mtl_backend_t *mtl, binocle_image_t *img,
                                 const binocle_image_desc *desc);
void binocle_backend_mtl_destroy_image(binocle_mtl_backend_t *mtl, binocle_image_t *img);
binocle_resource_state
binocle_backend_mtl_create_shader(binocle_mtl_backend_t *mtl, binocle_shader_t *shd,
                                  const binocle_shader_desc *desc);
void binocle_backend_mtl_destroy_shader(binocle_mtl_backend_t *mtl,
                                        binocle_shader_t *shd);
void binocle_backend_mtl_clear(binocle_mtl_backend_t *mtl, struct binocle_color color);
binocle_resource_state binocle_backend_mtl_create_pipeline(
  binocle_mtl_backend_t *mtl, binocle_pipeline_t *pip, binocle_shader_t *shd,
  const binocle_pipeline_desc *desc);
#endif // BINOCLE_BACKEND_METAL_H
