//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_backend.h"
#include "../binocle_log.h"
#include "../binocle_pool.h"
#include "binocle_material.h"
#import <SDL.h>

#if defined(BINOCLE_GL)
#include "binocle_backend_gl.h"
#elif defined(BINOCLE_METAL)
#include "binocle_backend_metal.h"
#endif

typedef struct binocle_pools_t {
  binocle_pool_t buffer_pool;
  binocle_buffer_t* buffers;

  binocle_pool_t image_pool;
  binocle_image_t* images;

  binocle_pool_t shader_pool;
  binocle_shader_t* shaders;

  binocle_pool_t pipeline_pool;
  binocle_pipeline_t* pipelines;

  binocle_pool_t pass_pool;
  binocle_pass_t* passes;

  binocle_pool_t context_pool;
  binocle_context_t* contexts;
} binocle_pools_t;

typedef struct binocle_backend_t {
  bool valid;
  binocle_backend_desc desc;       /* original desc with default values patched in */
  uint32_t frame_index;
  binocle_context active_context;
  binocle_pass cur_pass;
  binocle_pipeline cur_pipeline;
  bool pass_valid;
  bool bindings_valid;
  bool next_draw_valid;
  binocle_pools_t pools;
  binocle_pixelformat_info formats[BINOCLE_PIXELFORMAT_NUM];
#if defined(BINOCLE_GL)
  binocle_gl_backend_t gl;
#elif defined(BINOCLE_METAL)
  binocle_mtl_backend_t mtl;
#else
#error("no backend defined")
#endif
} binocle_backend_t;

static binocle_backend_t backend;

/// Helper functions

bool binocle_backend_strempty(const binocle_str_t* str) {
  return 0 == str->buf[0];
}

const char* binocle_backend_strptr(const binocle_str_t* str) {
  return &str->buf[0];
}

void binocle_backend_strcpy(binocle_str_t* dst, const char* src) {
  assert(dst);
  if (src) {
#if defined(_MSC_VER)
    strncpy_s(dst->buf, BINOCLE_STRING_SIZE, src, (BINOCLE_STRING_SIZE-1));
#else
    strncpy(dst->buf, src, BINOCLE_STRING_SIZE);
#endif
    dst->buf[BINOCLE_STRING_SIZE-1] = 0;
  }
  else {
    memset(dst->buf, 0, BINOCLE_STRING_SIZE);
  }
}

/* return the byte size of a shader uniform */
int binocle_backend_uniform_size(binocle_uniform_type type, int count) {
  switch (type) {
  case BINOCLE_UNIFORMTYPE_INVALID:
    return 0;
  case BINOCLE_UNIFORMTYPE_FLOAT:
    return 4 * count;
  case BINOCLE_UNIFORMTYPE_FLOAT2:
    return 8 * count;
  case BINOCLE_UNIFORMTYPE_FLOAT3:
    return 12 * count; /* FIXME: std140??? */
  case BINOCLE_UNIFORMTYPE_FLOAT4:
    return 16 * count;
  case BINOCLE_UNIFORMTYPE_MAT4:
    return 64 * count;
  default:
    assert(false);
    return -1;
  }
}

/* return the bytes-per-pixel for a pixel format */
int binocle_pixelformat_bytesize(binocle_pixel_format fmt) {
  switch (fmt) {
  case BINOCLE_PIXELFORMAT_R8:
  case BINOCLE_PIXELFORMAT_R8SN:
  case BINOCLE_PIXELFORMAT_R8UI:
  case BINOCLE_PIXELFORMAT_R8SI:
    return 1;

  case BINOCLE_PIXELFORMAT_R16:
  case BINOCLE_PIXELFORMAT_R16SN:
  case BINOCLE_PIXELFORMAT_R16UI:
  case BINOCLE_PIXELFORMAT_R16SI:
  case BINOCLE_PIXELFORMAT_R16F:
  case BINOCLE_PIXELFORMAT_RG8:
  case BINOCLE_PIXELFORMAT_RG8SN:
  case BINOCLE_PIXELFORMAT_RG8UI:
  case BINOCLE_PIXELFORMAT_RG8SI:
    return 2;

  case BINOCLE_PIXELFORMAT_R32UI:
  case BINOCLE_PIXELFORMAT_R32SI:
  case BINOCLE_PIXELFORMAT_R32F:
  case BINOCLE_PIXELFORMAT_RG16:
  case BINOCLE_PIXELFORMAT_RG16SN:
  case BINOCLE_PIXELFORMAT_RG16UI:
  case BINOCLE_PIXELFORMAT_RG16SI:
  case BINOCLE_PIXELFORMAT_RG16F:
  case BINOCLE_PIXELFORMAT_RGBA8:
  case BINOCLE_PIXELFORMAT_RGBA8SN:
  case BINOCLE_PIXELFORMAT_RGBA8UI:
  case BINOCLE_PIXELFORMAT_RGBA8SI:
  case BINOCLE_PIXELFORMAT_BGRA8:
  case BINOCLE_PIXELFORMAT_RGB10A2:
  case BINOCLE_PIXELFORMAT_RG11B10F:
    return 4;

  case BINOCLE_PIXELFORMAT_RG32UI:
  case BINOCLE_PIXELFORMAT_RG32SI:
  case BINOCLE_PIXELFORMAT_RG32F:
  case BINOCLE_PIXELFORMAT_RGBA16:
  case BINOCLE_PIXELFORMAT_RGBA16SN:
  case BINOCLE_PIXELFORMAT_RGBA16UI:
  case BINOCLE_PIXELFORMAT_RGBA16SI:
  case BINOCLE_PIXELFORMAT_RGBA16F:
    return 8;

  case BINOCLE_PIXELFORMAT_RGBA32UI:
  case BINOCLE_PIXELFORMAT_RGBA32SI:
  case BINOCLE_PIXELFORMAT_RGBA32F:
    return 16;

  default:
    assert(false);
    return 0;
  }
}

/* return row pitch for an image
    see ComputePitch in https://github.com/microsoft/DirectXTex/blob/master/DirectXTex/DirectXTexUtil.cpp
*/
uint32_t binocle_backend_row_pitch(binocle_pixel_format fmt, uint32_t width,
                                   uint32_t row_align) {
  uint32_t pitch;
  switch (fmt) {
  case BINOCLE_PIXELFORMAT_BC1_RGBA:
  case BINOCLE_PIXELFORMAT_BC4_R:
  case BINOCLE_PIXELFORMAT_BC4_RSN:
  case BINOCLE_PIXELFORMAT_ETC2_RGB8:
  case BINOCLE_PIXELFORMAT_ETC2_RGB8A1:
    pitch = ((width + 3) / 4) * 8;
    pitch = pitch < 8 ? 8 : pitch;
    break;
  case BINOCLE_PIXELFORMAT_BC2_RGBA:
  case BINOCLE_PIXELFORMAT_BC3_RGBA:
  case BINOCLE_PIXELFORMAT_BC5_RG:
  case BINOCLE_PIXELFORMAT_BC5_RGSN:
  case BINOCLE_PIXELFORMAT_BC6H_RGBF:
  case BINOCLE_PIXELFORMAT_BC6H_RGBUF:
  case BINOCLE_PIXELFORMAT_BC7_RGBA:
  case BINOCLE_PIXELFORMAT_ETC2_RGBA8:
  case BINOCLE_PIXELFORMAT_ETC2_RG11:
  case BINOCLE_PIXELFORMAT_ETC2_RG11SN:
    pitch = ((width + 3) / 4) * 16;
    pitch = pitch < 16 ? 16 : pitch;
    break;
  case BINOCLE_PIXELFORMAT_PVRTC_RGB_4BPP:
  case BINOCLE_PIXELFORMAT_PVRTC_RGBA_4BPP: {
    const int block_size = 4 * 4;
    const int bpp = 4;
    int width_blocks = width / 4;
    width_blocks = width_blocks < 2 ? 2 : width_blocks;
    pitch = width_blocks * ((block_size * bpp) / 8);
  } break;
  case BINOCLE_PIXELFORMAT_PVRTC_RGB_2BPP:
  case BINOCLE_PIXELFORMAT_PVRTC_RGBA_2BPP: {
    const int block_size = 8 * 4;
    const int bpp = 2;
    int width_blocks = width / 4;
    width_blocks = width_blocks < 2 ? 2 : width_blocks;
    pitch = width_blocks * ((block_size * bpp) / 8);
  } break;
  default:
    pitch = width * binocle_pixelformat_bytesize(fmt);
    break;
  }
  pitch = BINOCLE_ROUNDUP(pitch, row_align);
  return pitch;
}

/* compute the number of rows in a surface depending on pixel format */
int binocle_backend_num_rows(binocle_pixel_format fmt, int height) {
  int num_rows;
  switch (fmt) {
  case BINOCLE_PIXELFORMAT_BC1_RGBA:
  case BINOCLE_PIXELFORMAT_BC4_R:
  case BINOCLE_PIXELFORMAT_BC4_RSN:
  case BINOCLE_PIXELFORMAT_ETC2_RGB8:
  case BINOCLE_PIXELFORMAT_ETC2_RGB8A1:
  case BINOCLE_PIXELFORMAT_ETC2_RGBA8:
  case BINOCLE_PIXELFORMAT_ETC2_RG11:
  case BINOCLE_PIXELFORMAT_ETC2_RG11SN:
  case BINOCLE_PIXELFORMAT_BC2_RGBA:
  case BINOCLE_PIXELFORMAT_BC3_RGBA:
  case BINOCLE_PIXELFORMAT_BC5_RG:
  case BINOCLE_PIXELFORMAT_BC5_RGSN:
  case BINOCLE_PIXELFORMAT_BC6H_RGBF:
  case BINOCLE_PIXELFORMAT_BC6H_RGBUF:
  case BINOCLE_PIXELFORMAT_BC7_RGBA:
  case BINOCLE_PIXELFORMAT_PVRTC_RGB_4BPP:
  case BINOCLE_PIXELFORMAT_PVRTC_RGBA_4BPP:
  case BINOCLE_PIXELFORMAT_PVRTC_RGB_2BPP:
  case BINOCLE_PIXELFORMAT_PVRTC_RGBA_2BPP:
    num_rows = ((height + 3) / 4);
    break;
  default:
    num_rows = height;
    break;
  }
  if (num_rows < 1) {
    num_rows = 1;
  }
  return num_rows;
}

/* return byte size of a vertex format */
int binocle_backend_vertexformat_bytesize(binocle_vertex_format fmt) {
  switch (fmt) {
  case BINOCLE_VERTEXFORMAT_FLOAT:     return 4;
  case BINOCLE_VERTEXFORMAT_FLOAT2:    return 8;
  case BINOCLE_VERTEXFORMAT_FLOAT3:    return 12;
  case BINOCLE_VERTEXFORMAT_FLOAT4:    return 16;
  case BINOCLE_VERTEXFORMAT_BYTE4:     return 4;
  case BINOCLE_VERTEXFORMAT_BYTE4N:    return 4;
  case BINOCLE_VERTEXFORMAT_UBYTE4:    return 4;
  case BINOCLE_VERTEXFORMAT_UBYTE4N:   return 4;
  case BINOCLE_VERTEXFORMAT_SHORT2:    return 4;
  case BINOCLE_VERTEXFORMAT_SHORT2N:   return 4;
  case BINOCLE_VERTEXFORMAT_USHORT2N:  return 4;
  case BINOCLE_VERTEXFORMAT_SHORT4:    return 8;
  case BINOCLE_VERTEXFORMAT_SHORT4N:   return 8;
  case BINOCLE_VERTEXFORMAT_USHORT4N:  return 8;
  case BINOCLE_VERTEXFORMAT_UINT10_N2: return 4;
  case BINOCLE_VERTEXFORMAT_INVALID:   return 0;
  default:
    assert(false);
    return -1;
  }
}

/* return pitch of a 2D subimage / texture slice
    see ComputePitch in https://github.com/microsoft/DirectXTex/blob/master/DirectXTex/DirectXTexUtil.cpp
*/
uint32_t binocle_backend_surface_pitch(binocle_pixel_format fmt, uint32_t width, uint32_t height, uint32_t row_align) {
  int num_rows = binocle_backend_num_rows(fmt, height);
  return num_rows * binocle_backend_row_pitch(fmt, width, row_align);
}

/* capability table pixel format helper functions */
void binocle_backend_pixelformat_all(binocle_pixelformat_info* pfi) {
  pfi->sample = true;
  pfi->filter = true;
  pfi->blend = true;
  pfi->render = true;
  pfi->msaa = true;
}

void binocle_backend_pixelformat_s(binocle_pixelformat_info* pfi) {
  pfi->sample = true;
}

void binocle_backend_pixelformat_sf(binocle_pixelformat_info* pfi) {
  pfi->sample = true;
  pfi->filter = true;
}

void binocle_backend_pixelformat_sr(binocle_pixelformat_info* pfi) {
  pfi->sample = true;
  pfi->render = true;
}

void binocle_backend_pixelformat_srmd(binocle_pixelformat_info* pfi) {
  pfi->sample = true;
  pfi->render = true;
  pfi->msaa = true;
  pfi->depth = true;
}

void binocle_backend_pixelformat_srm(binocle_pixelformat_info* pfi) {
  pfi->sample = true;
  pfi->render = true;
  pfi->msaa = true;
}

void binocle_backend_pixelformat_sfrm(binocle_pixelformat_info* pfi) {
  pfi->sample = true;
  pfi->filter = true;
  pfi->render = true;
  pfi->msaa = true;
}

void binocle_backend_pixelformat_sbrm(binocle_pixelformat_info* pfi) {
  pfi->sample = true;
  pfi->blend = true;
  pfi->render = true;
  pfi->msaa = true;
}

void binocle_backend_pixelformat_sbr(binocle_pixelformat_info* pfi) {
  pfi->sample = true;
  pfi->blend = true;
  pfi->render = true;
}

void binocle_backend_pixelformat_sfbr(binocle_pixelformat_info* pfi) {
  pfi->sample = true;
  pfi->filter = true;
  pfi->blend = true;
  pfi->render = true;
}

void binocle_backend_setup_pools(binocle_pools_t *pools, const binocle_backend_desc* desc) {
  assert(pools);
  assert(desc);
  /* note: the pools here will have an additional item, since slot 0 is reserved */
  assert((desc->buffer_pool_size > 0) && (desc->buffer_pool_size < BINOCLE_MAX_POOL_SIZE));
  binocle_pool_init(&pools->buffer_pool, desc->buffer_pool_size);
  size_t buffer_pool_byte_size = sizeof(binocle_buffer_t) * pools->buffer_pool.size;
  pools->buffers = (binocle_buffer_t*) malloc(buffer_pool_byte_size);
  assert(pools->buffers);
  memset(pools->buffers, 0, buffer_pool_byte_size);

  assert((desc->image_pool_size > 0) && (desc->image_pool_size < BINOCLE_MAX_POOL_SIZE));
  binocle_pool_init(&pools->image_pool, desc->image_pool_size);
  size_t image_pool_byte_size = sizeof(binocle_image_t) * pools->image_pool.size;
  pools->images = (binocle_image_t*) malloc(image_pool_byte_size);
  assert(pools->images);
  memset(pools->images, 0, image_pool_byte_size);

  assert((desc->shader_pool_size > 0) && (desc->shader_pool_size < BINOCLE_MAX_POOL_SIZE));
  binocle_pool_init(&pools->shader_pool, desc->shader_pool_size);
  size_t shader_pool_byte_size = sizeof(binocle_shader_t) * pools->shader_pool.size;
  pools->shaders = (binocle_shader_t*) malloc(shader_pool_byte_size);
  assert(pools->shaders);
  memset(pools->shaders, 0, shader_pool_byte_size);

  assert((desc->pipeline_pool_size > 0) && (desc->pipeline_pool_size < BINOCLE_MAX_POOL_SIZE));
  binocle_pool_init(&pools->pipeline_pool, desc->pipeline_pool_size);
  size_t pipeline_pool_byte_size = sizeof(binocle_pipeline_t) * pools->pipeline_pool.size;
  pools->pipelines = (binocle_pipeline_t*) malloc(pipeline_pool_byte_size);
  assert(pools->pipelines);
  memset(pools->pipelines, 0, pipeline_pool_byte_size);

  assert((desc->pass_pool_size > 0) && (desc->pass_pool_size < BINOCLE_MAX_POOL_SIZE));
  binocle_pool_init(&pools->pass_pool, desc->pass_pool_size);
  size_t pass_pool_byte_size = sizeof(binocle_pass_t) * pools->pass_pool.size;
  pools->passes = (binocle_pass_t*) malloc(pass_pool_byte_size);
  assert(pools->passes);
  memset(pools->passes, 0, pass_pool_byte_size);

  assert((desc->context_pool_size > 0) && (desc->context_pool_size < BINOCLE_MAX_POOL_SIZE));
  binocle_pool_init(&pools->context_pool, desc->context_pool_size);
  size_t context_pool_byte_size = sizeof(binocle_context_t) * pools->context_pool.size;
  pools->contexts = (binocle_context_t*) malloc(context_pool_byte_size);
  assert(pools->contexts);
  memset(pools->contexts, 0, context_pool_byte_size);
}

void binocle_backend_discard_pools(binocle_pools_t *p) {
  assert(p);

  free(p->buffers);
  p->buffers = 0;
  binocle_pool_discard(&p->buffer_pool);

  free(p->images);
  p->images = 0;
  binocle_pool_discard(&p->image_pool);

  free(p->shaders);
  p->shaders = 0;
  binocle_pool_discard(&p->shader_pool);

  free(p->pipelines);
  p->pipelines = 0;
  binocle_pool_discard(&p->pipeline_pool);

  free(p->passes);
  p->passes = 0;
  binocle_pool_discard(&p->pass_pool);
}


/* returns pointer to resource by id without matching id check */
binocle_buffer_t* binocle_buffer_at(const binocle_pools_t* p, uint32_t rt_id) {
  assert(p && (BINOCLE_INVALID_ID != rt_id));
  int slot_index = binocle_pool_slot_index(rt_id);
  assert((slot_index > BINOCLE_POOL_INVALID_SLOT_INDEX) && (slot_index < p->buffer_pool.size));
  return &p->buffers[slot_index];
}

binocle_image_t* binocle_image_at(const binocle_pools_t* p, uint32_t rt_id) {
  assert(p && (BINOCLE_INVALID_ID != rt_id));
  int slot_index = binocle_pool_slot_index(rt_id);
  assert((slot_index > BINOCLE_POOL_INVALID_SLOT_INDEX) && (slot_index < p->image_pool.size));
  return &p->images[slot_index];
}

binocle_shader_t* binocle_shader_at(const binocle_pools_t* p, uint32_t rt_id) {
  assert(p && (BINOCLE_INVALID_ID != rt_id));
  int slot_index = binocle_pool_slot_index(rt_id);
  assert((slot_index > BINOCLE_POOL_INVALID_SLOT_INDEX) && (slot_index < p->shader_pool.size));
  return &p->shaders[slot_index];
}

binocle_pipeline_t* binocle_pipeline_at(const binocle_pools_t* p, uint32_t rt_id) {
  assert(p && (BINOCLE_INVALID_ID != rt_id));
  int slot_index = binocle_pool_slot_index(rt_id);
  assert((slot_index > BINOCLE_POOL_INVALID_SLOT_INDEX) && (slot_index < p->pipeline_pool.size));
  return &p->pipelines[slot_index];
}

binocle_pass_t* binocle_pass_at(const binocle_pools_t* p, uint32_t rt_id) {
  assert(p && (BINOCLE_INVALID_ID != rt_id));
  int slot_index = binocle_pool_slot_index(rt_id);
  assert((slot_index > BINOCLE_POOL_INVALID_SLOT_INDEX) && (slot_index < p->pass_pool.size));
  return &p->passes[slot_index];
}

binocle_context_t* binocle_backend_context_at(const binocle_pools_t* p, uint32_t rt_id) {
  assert(p && (BINOCLE_INVALID_ID != rt_id));
  int slot_index = binocle_pool_slot_index(rt_id);
  assert((slot_index > BINOCLE_POOL_INVALID_SLOT_INDEX) && (slot_index < p->context_pool.size));
  return &p->contexts[slot_index];
}

/* returns pointer to resource with matching id check, may return 0 */
binocle_buffer_t* binocle_backend_lookup_buffer(const binocle_pools_t* p, uint32_t buf_id) {
  if (BINOCLE_INVALID_ID != buf_id) {
    binocle_buffer_t* buf = binocle_buffer_at(p, buf_id);
    if (buf->slot.id == buf_id) {
      return buf;
    }
  }
  return 0;
}

binocle_image_t* binocle_backend_lookup_image(const binocle_pools_t* p, uint32_t img_id) {
  if (BINOCLE_INVALID_ID != img_id) {
    binocle_image_t* img = binocle_image_at(p, img_id);
    if (img->slot.id == img_id) {
      return img;
    }
  }
  return 0;
}

binocle_shader_t* binocle_backend_lookup_shader(const binocle_pools_t* p, uint32_t sha_id) {
  if (BINOCLE_INVALID_ID != sha_id) {
    binocle_shader_t* sha = binocle_shader_at(p, sha_id);
    if (sha->slot.id == sha_id) {
      return sha;
    }
  }
  return 0;
}

binocle_pipeline_t* binocle_backend_lookup_pipeline(const binocle_pools_t* p, uint32_t pip_id) {
  if (BINOCLE_INVALID_ID != pip_id) {
    binocle_pipeline_t* pip = binocle_pipeline_at(p, pip_id);
    if (pip->slot.id == pip_id) {
      return pip;
    }
  }
  return 0;
}

binocle_pass_t* binocle_backend_lookup_pass(const binocle_pools_t* p, uint32_t pass_id) {
  if (BINOCLE_INVALID_ID != pass_id) {
    binocle_pass_t* pass = binocle_pass_at(p, pass_id);
    if (pass->slot.id == pass_id) {
      return pass;
    }
  }
  return 0;
}

binocle_context_t* binocle_backend_lookup_context(const binocle_pools_t* p, uint32_t ctx_id) {
  if (BINOCLE_INVALID_ID != ctx_id) {
    binocle_context_t* ctx = binocle_backend_context_at(p, ctx_id);
    if (ctx->slot.id == ctx_id) {
      return ctx;
    }
  }
  return 0;
}

void binocle_backend_reset_slot(binocle_slot_t* slot) {
  assert(slot);
  memset(slot, 0, sizeof(binocle_slot_t));
}

void binocle_backend_reset_buffer(binocle_buffer_t* buf) {
  assert(buf);
  binocle_slot_t slot = buf->slot;
  memset(buf, 0, sizeof(binocle_buffer_t));
  buf->slot = slot;
  buf->slot.state = BINOCLE_RESOURCESTATE_ALLOC;
}

void binocle_backend_reset_image(binocle_image_t* img) {
  assert(img);
  binocle_slot_t slot = img->slot;
  memset(img, 0, sizeof(binocle_image_t));
  img->slot = slot;
  img->slot.state = BINOCLE_RESOURCESTATE_ALLOC;
}

void binocle_backend_reset_shader(binocle_shader_t* sha) {
  assert(sha);
  binocle_slot_t slot = sha->slot;
  memset(sha, 0, sizeof(binocle_shader_t));
  sha->slot = slot;
  sha->slot.state = BINOCLE_RESOURCESTATE_ALLOC;
}

void binocle_backend_reset_context(binocle_context_t* ctx) {
  assert(ctx);
  binocle_slot_t slot = ctx->slot;
  memset(ctx, 0, sizeof(binocle_context_t));
  ctx->slot = slot;
  ctx->slot.state = BINOCLE_RESOURCESTATE_ALLOC;
}

binocle_buffer_desc binocle_backend_buffer_desc_defaults(const binocle_buffer_desc* desc) {
  binocle_buffer_desc def = *desc;
  def.type = BINOCLE_DEF(def.type, BINOCLE_BUFFERTYPE_VERTEXBUFFER);
  def.usage = BINOCLE_DEF(def.usage, BINOCLE_USAGE_IMMUTABLE);
  if (def.size == 0) {
    def.size = def.data.size;
  }
  else if (def.data.size == 0) {
    def.data.size = def.size;
  }
  return def;
}

binocle_buffer binocle_backend_alloc_buffer(void) {
  binocle_buffer res;
  int slot_index = binocle_pool_alloc_index(&backend.pools.buffer_pool);
  if (BINOCLE_POOL_INVALID_SLOT_INDEX != slot_index) {
    res.id = binocle_pool_slot_alloc(&backend.pools.buffer_pool, &backend.pools.buffers[slot_index].slot, slot_index);
  }
  else {
    /* pool is exhausted */
    res.id = BINOCLE_INVALID_ID;
  }
  return res;
}

bool binocle_backend_validate_buffer_desc(const binocle_buffer_desc* desc) {
#if !defined(BINOCLE_DEBUG)
  return true;
#else
  SOKOL_ASSERT(desc);
        SOKOL_VALIDATE_BEGIN();
        SOKOL_VALIDATE(desc->_start_canary == 0, _SG_VALIDATE_BUFFERDESC_CANARY);
        SOKOL_VALIDATE(desc->_end_canary == 0, _SG_VALIDATE_BUFFERDESC_CANARY);
        SOKOL_VALIDATE(desc->size > 0, _SG_VALIDATE_BUFFERDESC_SIZE);
        bool injected = (0 != desc->gl_buffers[0]) ||
                        (0 != desc->mtl_buffers[0]) ||
                        (0 != desc->d3d11_buffer) ||
                        (0 != desc->wgpu_buffer);
        if (!injected && (desc->usage == SG_USAGE_IMMUTABLE)) {
            SOKOL_VALIDATE((0 != desc->data.ptr) && (desc->data.size > 0), _SG_VALIDATE_BUFFERDESC_DATA);
            SOKOL_VALIDATE(desc->size == desc->data.size, _SG_VALIDATE_BUFFERDESC_DATA_SIZE);
        }
        else {
            SOKOL_VALIDATE(0 == desc->data.ptr, _SG_VALIDATE_BUFFERDESC_NO_DATA);
        }
        return SOKOL_VALIDATE_END();
#endif
}

binocle_resource_state binocle_backend_create_buffer(binocle_buffer_t* buf, const binocle_buffer_desc* desc) {
#if defined(BINOCLE_GL)
  return binocle_backend_gl_create_buffer(&backend.gl, buf, desc);
#elif defined(BINOCLE_METAL)
  return binocle_backend_mtl_create_buffer(&backend.mtl, buf, desc);
#else
#error("INVALID BACKEND");
#endif
}

void binocle_backend_init_buffer(binocle_buffer buf_id, const binocle_buffer_desc* desc) {
  assert(buf_id.id != BINOCLE_INVALID_ID && desc);
  binocle_buffer_t* buf = binocle_backend_lookup_buffer(&backend.pools, buf_id.id);
  assert(buf && buf->slot.state == BINOCLE_RESOURCESTATE_ALLOC);
  buf->slot.ctx_id = backend.active_context.id;
  if (binocle_backend_validate_buffer_desc(desc)) {
    buf->slot.state = binocle_backend_create_buffer(buf, desc);
  }
  else {
    buf->slot.state = BINOCLE_RESOURCESTATE_FAILED;
  }
  assert((buf->slot.state == BINOCLE_RESOURCESTATE_VALID)||(buf->slot.state == BINOCLE_RESOURCESTATE_FAILED));
}

binocle_buffer binocle_backend_make_buffer(const binocle_buffer_desc* desc) {
  assert(backend.valid);
  assert(desc);
  binocle_buffer_desc desc_def = binocle_backend_buffer_desc_defaults(desc);
  binocle_buffer buf_id = binocle_backend_alloc_buffer();
  if (buf_id.id != BINOCLE_INVALID_ID) {
    binocle_backend_init_buffer(buf_id, &desc_def);
  }
  else {
    binocle_log_error("buffer pool exhausted!");
//    BINOCLE_TRACE_NOARGS(err_buffer_pool_exhausted);
  }
//  BINOCLE_TRACE_ARGS(make_buffer, &desc_def, buf_id);
  return buf_id;
}

void binocle_backend_destroy_buffer_t(binocle_buffer_t* buf) {
#if defined(BINOCLE_GL)
  binocle_backend_gl_destroy_buffer(&backend.gl, buf);
#elif defined(BINOCLE_METAL)
  binocle_backend_mtl_destroy_buffer(&backend.mtl, buf);
#else
#error("no backend defined")
#endif
}

void binocle_backend_destroy_buffer(binocle_buffer buf) {
  binocle_buffer_t* buffer = binocle_backend_lookup_buffer(&backend.pools, buf.id);
  if (buffer) {
    binocle_backend_destroy_buffer_t(buffer);
    binocle_backend_reset_buffer(buffer);
    binocle_pool_free_index(&backend.pools.buffer_pool, binocle_pool_slot_index(buf.id));
  }
}


binocle_image_desc binocle_backend_image_desc_defaults(const binocle_image_desc* desc) {
  binocle_image_desc def = *desc;
  def.type = BINOCLE_DEF(def.type, BINOCLE_IMAGETYPE_2D);
  def.depth = BINOCLE_DEF(def.depth, 1);
  def.num_mipmaps = BINOCLE_DEF(def.num_mipmaps, 1);
  def.usage = BINOCLE_DEF(def.usage, BINOCLE_USAGE_IMMUTABLE);
  if (desc->render_target) {
    def.pixel_format = BINOCLE_DEF(def.pixel_format, backend.desc.context.color_format);
    def.sample_count = BINOCLE_DEF(def.sample_count, backend.desc.context.sample_count);
  } else {
    def.pixel_format = BINOCLE_DEF(def.pixel_format, BINOCLE_PIXELFORMAT_RGBA8);
    def.sample_count = BINOCLE_DEF(def.sample_count, 1);
  }
  def.min_filter = BINOCLE_DEF(def.min_filter, BINOCLE_FILTER_NEAREST);
  def.mag_filter = BINOCLE_DEF(def.mag_filter, BINOCLE_FILTER_NEAREST);
  def.wrap_u = BINOCLE_DEF(def.wrap_u, BINOCLE_WRAP_REPEAT);
  def.wrap_v = BINOCLE_DEF(def.wrap_v, BINOCLE_WRAP_REPEAT);
  def.wrap_w = BINOCLE_DEF(def.wrap_w, BINOCLE_WRAP_REPEAT);
  def.border_color = BINOCLE_DEF(def.border_color, BINOCLE_BORDERCOLOR_OPAQUE_BLACK);
  def.max_anisotropy = BINOCLE_DEF(def.max_anisotropy, 1);
  def.max_lod = BINOCLE_DEF_FLT(def.max_lod, FLT_MAX);
  return def;
}

binocle_image binocle_backend_alloc_image(void) {
  binocle_image res;
  int slot_index = binocle_pool_alloc_index(&backend.pools.image_pool);
  if (BINOCLE_POOL_INVALID_SLOT_INDEX != slot_index) {
    res.id = binocle_pool_slot_alloc(&backend.pools.image_pool, &backend.pools.images[slot_index].slot, slot_index);
  }
  else {
    /* pool is exhausted */
    res.id = BINOCLE_INVALID_ID;
  }
  return res;
}


bool binocle_backend_validate_image_desc(const binocle_image_desc* desc) {
#if !defined(XXX)
  return true;
#else
  SOKOL_ASSERT(desc);
        SOKOL_VALIDATE_BEGIN();
        SOKOL_VALIDATE(desc->_start_canary == 0, _SG_VALIDATE_IMAGEDESC_CANARY);
        SOKOL_VALIDATE(desc->_end_canary == 0, _SG_VALIDATE_IMAGEDESC_CANARY);
        SOKOL_VALIDATE(desc->width > 0, _SG_VALIDATE_IMAGEDESC_WIDTH);
        SOKOL_VALIDATE(desc->height > 0, _SG_VALIDATE_IMAGEDESC_HEIGHT);
        const sg_pixel_format fmt = desc->pixel_format;
        const sg_usage usage = desc->usage;
        const bool injected = (0 != desc->gl_textures[0]) ||
                              (0 != desc->mtl_textures[0]) ||
                              (0 != desc->d3d11_texture) ||
                              (0 != desc->wgpu_texture);
        if (desc->render_target) {
            SOKOL_ASSERT(((int)fmt >= 0) && ((int)fmt < _SG_PIXELFORMAT_NUM));
            SOKOL_VALIDATE(_sg.formats[fmt].render, _SG_VALIDATE_IMAGEDESC_RT_PIXELFORMAT);
            /* on GLES2, sample count for render targets is completely ignored */
            #if defined(SOKOL_GLES2) || defined(SOKOL_GLES3)
            if (!_sg.gl.gles2) {
            #endif
                if (desc->sample_count > 1) {
                    SOKOL_VALIDATE(_sg.features.msaa_render_targets && _sg.formats[fmt].msaa, _SG_VALIDATE_IMAGEDESC_NO_MSAA_RT_SUPPORT);
                }
            #if defined(SOKOL_GLES2) || defined(SOKOL_GLES3)
            }
            #endif
            SOKOL_VALIDATE(usage == SG_USAGE_IMMUTABLE, _SG_VALIDATE_IMAGEDESC_RT_IMMUTABLE);
            SOKOL_VALIDATE(desc->content.subimage[0][0].ptr==0, _SG_VALIDATE_IMAGEDESC_RT_NO_CONTENT);
        }
        else {
            SOKOL_VALIDATE(desc->sample_count <= 1, _SG_VALIDATE_IMAGEDESC_MSAA_BUT_NO_RT);
            const bool valid_nonrt_fmt = !_sg_is_valid_rendertarget_depth_format(fmt);
            SOKOL_VALIDATE(valid_nonrt_fmt, _SG_VALIDATE_IMAGEDESC_NONRT_PIXELFORMAT);
            /* FIXME: should use the same "expected size" computation as in _sg_validate_update_image() here */
            if (!injected && (usage == SG_USAGE_IMMUTABLE)) {
                const int num_faces = desc->type == SG_IMAGETYPE_CUBE ? 6:1;
                const int num_mips = desc->num_mipmaps;
                for (int face_index = 0; face_index < num_faces; face_index++) {
                    for (int mip_index = 0; mip_index < num_mips; mip_index++) {
                        const bool has_data = desc->content.subimage[face_index][mip_index].ptr != 0;
                        const bool has_size = desc->content.subimage[face_index][mip_index].size > 0;
                        SOKOL_VALIDATE(has_data && has_size, _SG_VALIDATE_IMAGEDESC_CONTENT);
                    }
                }
            }
            else {
                for (int face_index = 0; face_index < SG_CUBEFACE_NUM; face_index++) {
                    for (int mip_index = 0; mip_index < SG_MAX_MIPMAPS; mip_index++) {
                        const bool no_data = 0 == desc->content.subimage[face_index][mip_index].ptr;
                        const bool no_size = 0 == desc->content.subimage[face_index][mip_index].size;
                        SOKOL_VALIDATE(no_data && no_size, _SG_VALIDATE_IMAGEDESC_NO_CONTENT);
                    }
                }
            }
        }
        return SOKOL_VALIDATE_END();
#endif
}

static inline binocle_resource_state binocle_backend_create_image(binocle_image_t* img, const binocle_image_desc* desc) {
#if defined(BINOCLE_GL)
  return binocle_backend_gl_create_image(&backend.gl, img, desc);
#elif defined(BINOCLE_METAL)
  return binocle_backend_mtl_create_image(&backend.mtl, img, desc);
#else
#error("INVALID BACKEND");
#endif
}

void binocle_backend_init_image(binocle_image img_id, const binocle_image_desc* desc) {
  assert(img_id.id != BINOCLE_INVALID_ID && desc);
  binocle_image_t* img = binocle_backend_lookup_image(&backend.pools, img_id.id);
  assert(img && img->slot.state == BINOCLE_RESOURCESTATE_ALLOC);
  if (binocle_backend_validate_image_desc(desc)) {
    img->slot.state = binocle_backend_create_image(img, desc);
  }
  else {
    img->slot.state = BINOCLE_RESOURCESTATE_FAILED;
  }
  assert((img->slot.state == BINOCLE_RESOURCESTATE_VALID)||(img->slot.state == BINOCLE_RESOURCESTATE_FAILED));
}

binocle_image binocle_backend_make_image(const binocle_image_desc* desc) {
  assert(desc);
  binocle_image_desc desc_def = binocle_backend_image_desc_defaults(desc);
  binocle_image img_id = binocle_backend_alloc_image();
  if (img_id.id != BINOCLE_INVALID_ID) {
    binocle_backend_init_image(img_id, &desc_def);
  }
  else {
    binocle_log_error("image pool exhausted!");
  }
  return img_id;
}

void binocle_backend_destroy_image_t(binocle_image_t* img) {
#if defined(BINOCLE_GL)
  binocle_backend_gl_destroy_image(&backend.gl, img);
#elif defined(BINOCLE_METAL)
  binocle_backend_mtl_destroy_image(&backend.mtl, img);
#else
#error("no backend defined")
#endif
}

void binocle_backend_destroy_image(binocle_image img) {
  binocle_image_t* image = binocle_backend_lookup_image(&backend.pools, img.id);
  if (image) {
    binocle_backend_destroy_image_t(image);
    binocle_backend_reset_image(image);
    binocle_pool_free_index(&backend.pools.image_pool, binocle_pool_slot_index(img.id));
  }
}

bool binocle_backend_validate_shader_desc(const binocle_shader_desc* desc) {
#if !defined(XXX)
  return true;
#else
  assert(desc);
        SOKOL_VALIDATE_BEGIN();
        SOKOL_VALIDATE(desc->_start_canary == 0, _SG_VALIDATE_SHADERDESC_CANARY);
        SOKOL_VALIDATE(desc->_end_canary == 0, _SG_VALIDATE_SHADERDESC_CANARY);
        #if defined(SOKOL_GLES2)
            SOKOL_VALIDATE(0 != desc->attrs[0].name, _SG_VALIDATE_SHADERDESC_ATTR_NAMES);
        #elif defined(SOKOL_D3D11)
            SOKOL_VALIDATE(0 != desc->attrs[0].sem_name, _SG_VALIDATE_SHADERDESC_ATTR_SEMANTICS);
        #endif
        #if defined(SOKOL_GLCORE33) || defined(SOKOL_GLES2) || defined(SOKOL_GLES3)
            /* on GL, must provide shader source code */
            SOKOL_VALIDATE(0 != desc->vs.source, _SG_VALIDATE_SHADERDESC_SOURCE);
            SOKOL_VALIDATE(0 != desc->fs.source, _SG_VALIDATE_SHADERDESC_SOURCE);
        #elif defined(SOKOL_METAL) || defined(SOKOL_D3D11)
            /* on Metal or D3D11, must provide shader source code or byte code */
            SOKOL_VALIDATE((0 != desc->vs.source)||(0 != desc->vs.byte_code), _SG_VALIDATE_SHADERDESC_SOURCE_OR_BYTECODE);
            SOKOL_VALIDATE((0 != desc->fs.source)||(0 != desc->fs.byte_code), _SG_VALIDATE_SHADERDESC_SOURCE_OR_BYTECODE);
        #elif defined(SOKOL_WGPU)
            /* on WGPU byte code must be provided */
            SOKOL_VALIDATE((0 != desc->vs.byte_code), _SG_VALIDATE_SHADERDESC_BYTECODE);
            SOKOL_VALIDATE((0 != desc->fs.byte_code), _SG_VALIDATE_SHADERDESC_BYTECODE);
        #else
            /* Dummy Backend, don't require source or bytecode */
        #endif
        for (int i = 0; i < SG_MAX_VERTEX_ATTRIBUTES; i++) {
            if (desc->attrs[i].name) {
                SOKOL_VALIDATE(strlen(desc->attrs[i].name) < _SG_STRING_SIZE, _SG_VALIDATE_SHADERDESC_ATTR_STRING_TOO_LONG);
            }
            if (desc->attrs[i].sem_name) {
                SOKOL_VALIDATE(strlen(desc->attrs[i].sem_name) < _SG_STRING_SIZE, _SG_VALIDATE_SHADERDESC_ATTR_STRING_TOO_LONG);
            }
        }
        /* if shader byte code, the size must also be provided */
        if (0 != desc->vs.byte_code) {
            SOKOL_VALIDATE(desc->vs.byte_code_size > 0, _SG_VALIDATE_SHADERDESC_NO_BYTECODE_SIZE);
        }
        if (0 != desc->fs.byte_code) {
            SOKOL_VALIDATE(desc->fs.byte_code_size > 0, _SG_VALIDATE_SHADERDESC_NO_BYTECODE_SIZE);
        }
        for (int stage_index = 0; stage_index < SG_NUM_SHADER_STAGES; stage_index++) {
            const sg_shader_stage_desc* stage_desc = (stage_index == 0)? &desc->vs : &desc->fs;
            bool uniform_blocks_continuous = true;
            for (int ub_index = 0; ub_index < SG_MAX_SHADERSTAGE_UBS; ub_index++) {
                const sg_shader_uniform_block_desc* ub_desc = &stage_desc->uniform_blocks[ub_index];
                if (ub_desc->size > 0) {
                    SOKOL_VALIDATE(uniform_blocks_continuous, _SG_VALIDATE_SHADERDESC_NO_CONT_UBS);
                    bool uniforms_continuous = true;
                    int uniform_offset = 0;
                    int num_uniforms = 0;
                    for (int u_index = 0; u_index < SG_MAX_UB_MEMBERS; u_index++) {
                        const sg_shader_uniform_desc* u_desc = &ub_desc->uniforms[u_index];
                        if (u_desc->type != SG_UNIFORMTYPE_INVALID) {
                            SOKOL_VALIDATE(uniforms_continuous, _SG_VALIDATE_SHADERDESC_NO_CONT_UB_MEMBERS);
                            #if defined(SOKOL_GLES2) || defined(SOKOL_GLES3)
                            SOKOL_VALIDATE(0 != u_desc->name, _SG_VALIDATE_SHADERDESC_UB_MEMBER_NAME);
                            #endif
                            const int array_count = u_desc->array_count;
                            uniform_offset += _sg_uniform_size(u_desc->type, array_count);
                            num_uniforms++;
                        }
                        else {
                            uniforms_continuous = false;
                        }
                    }
                    #if defined(SOKOL_GLCORE33) || defined(SOKOL_GLES2) || defined(SOKOL_GLES3)
                    SOKOL_VALIDATE(uniform_offset == ub_desc->size, _SG_VALIDATE_SHADERDESC_UB_SIZE_MISMATCH);
                    SOKOL_VALIDATE(num_uniforms > 0, _SG_VALIDATE_SHADERDESC_NO_UB_MEMBERS);
                    #endif
                }
                else {
                    uniform_blocks_continuous = false;
                }
            }
            bool images_continuous = true;
            for (int img_index = 0; img_index < SG_MAX_SHADERSTAGE_IMAGES; img_index++) {
                const sg_shader_image_desc* img_desc = &stage_desc->images[img_index];
                if (img_desc->type != _SG_IMAGETYPE_DEFAULT) {
                    SOKOL_VALIDATE(images_continuous, _SG_VALIDATE_SHADERDESC_NO_CONT_IMGS);
                    #if defined(SOKOL_GLES2)
                    SOKOL_VALIDATE(0 != img_desc->name, _SG_VALIDATE_SHADERDESC_IMG_NAME);
                    #endif
                }
                else {
                    images_continuous = false;
                }
            }
        }
        return SOKOL_VALIDATE_END();
#endif
}

binocle_shader_desc binocle_backend_shader_desc_defaults(const binocle_shader_desc* desc) {
  binocle_shader_desc def = *desc;
#if defined(BINOCLE_METAL)
  def.vs.entry = BINOCLE_DEF(def.vs.entry, "_main");
        def.fs.entry = BINOCLE_DEF(def.fs.entry, "_main");
#else
  def.vs.entry = BINOCLE_DEF(def.vs.entry, "main");
  def.fs.entry = BINOCLE_DEF(def.fs.entry, "main");
#endif
  for (int stage_index = 0; stage_index < BINOCLE_NUM_SHADER_STAGES; stage_index++) {
    binocle_shader_stage_desc* stage_desc = (stage_index == BINOCLE_SHADERSTAGE_VS)? &def.vs : &def.fs;
    for (int ub_index = 0; ub_index < BINOCLE_MAX_SHADERSTAGE_UBS; ub_index++) {
      binocle_shader_uniform_block_desc* ub_desc = &stage_desc->uniform_blocks[ub_index];
      if (0 == ub_desc->size) {
        break;
      }
      for (int u_index = 0; u_index < BINOCLE_MAX_UB_MEMBERS; u_index++) {
        binocle_shader_uniform_desc* u_desc = &ub_desc->uniforms[u_index];
        if (u_desc->type == BINOCLE_UNIFORMTYPE_INVALID) {
          break;
        }
        u_desc->array_count = BINOCLE_DEF(u_desc->array_count, 1);
      }
    }
    for (int img_index = 0; img_index < BINOCLE_MAX_SHADERSTAGE_IMAGES; img_index++) {
      binocle_shader_image_desc* img_desc = &stage_desc->images[img_index];
      if (img_desc->type == BINOCLE_IMAGETYPE_DEFAULT) {
        break;
      }
      img_desc->sampler_type = BINOCLE_DEF(img_desc->sampler_type, BINOCLE_SAMPLERTYPE_FLOAT);
    }
  }
  return def;
}

binocle_shader binocle_backend_alloc_shader(void) {
  binocle_shader res;
  int slot_index = binocle_pool_alloc_index(&backend.pools.shader_pool);
  if (BINOCLE_POOL_INVALID_SLOT_INDEX != slot_index) {
    res.id = binocle_pool_slot_alloc(&backend.pools.shader_pool, &backend.pools.shaders[slot_index].slot, slot_index);
  }
  else {
    /* pool is exhausted */
    res.id = BINOCLE_INVALID_ID;
  }
  return res;
}

static inline binocle_resource_state binocle_backend_create_shader(binocle_shader_t* sha, const binocle_shader_desc* desc) {
#if defined(BINOCLE_GL)
  return binocle_backend_gl_create_shader(&backend.gl, sha, desc);
#elif defined(BINOCLE_METAL)
  return binocle_backend_mtl_create_shader(&backend.mtl, sha, desc);
#else
#error("INVALID BACKEND");
#endif
}

void binocle_backend_init_shader(binocle_shader sha_id, const binocle_shader_desc* desc) {
  assert(sha_id.id != BINOCLE_INVALID_ID && desc);
  binocle_shader_t* sha = binocle_backend_lookup_shader(&backend.pools, sha_id.id);
  assert(sha && sha->slot.state == BINOCLE_RESOURCESTATE_ALLOC);
  if (binocle_backend_validate_shader_desc(desc)) {
    sha->slot.state = binocle_backend_create_shader(sha, desc);
  }
  else {
    sha->slot.state = BINOCLE_RESOURCESTATE_FAILED;
  }
  assert((sha->slot.state == BINOCLE_RESOURCESTATE_VALID)||(sha->slot.state == BINOCLE_RESOURCESTATE_FAILED));
}

binocle_shader binocle_backend_make_shader(const binocle_shader_desc* desc) {
  assert(desc);
  binocle_shader_desc desc_def = binocle_backend_shader_desc_defaults(desc);
  binocle_shader sha_id = binocle_backend_alloc_shader();
  if (sha_id.id != BINOCLE_INVALID_ID) {
    binocle_backend_init_shader(sha_id, &desc_def);
  }
  else {
    binocle_log_error("shader pool exhausted!");
  }
  return sha_id;
}

void binocle_backend_destroy_shader_t(binocle_shader_t* sha) {
#if defined(BINOCLE_GL)
  binocle_backend_gl_destroy_shader(&backend.gl, sha);
#elif defined(BINOCLE_METAL)
  binocle_backend_mtl_destroy_shader(&backend.mtl, sha);
#else
#error("no backend defined")
#endif
}

void binocle_backend_destroy_shader(binocle_shader sha) {
  binocle_shader_t* shader = binocle_backend_lookup_shader(&backend.pools, sha.id);
  if (shader) {
    binocle_backend_destroy_shader_t(shader);
    binocle_backend_reset_shader(shader);
    binocle_pool_free_index(&backend.pools.shader_pool, binocle_pool_slot_index(sha.id));
  }
}

binocle_pipeline_desc binocle_backend_pipeline_desc_defaults(const binocle_pipeline_desc* desc) {
  binocle_pipeline_desc def = *desc;

  def.primitive_type = BINOCLE_DEF(def.primitive_type, BINOCLE_PRIMITIVETYPE_TRIANGLES);
  def.index_type = BINOCLE_DEF(def.index_type, BINOCLE_INDEXTYPE_NONE);
  def.cull_mode = BINOCLE_DEF(def.cull_mode, BINOCLE_CULLMODE_NONE);
  def.face_winding = BINOCLE_DEF(def.face_winding, BINOCLE_FACEWINDING_CW);
  def.sample_count = BINOCLE_DEF(def.sample_count, backend.desc.context.sample_count);

  def.stencil.front.compare = BINOCLE_DEF(def.stencil.front.compare, BINOCLE_COMPAREFUNC_ALWAYS);
  def.stencil.front.fail_op = BINOCLE_DEF(def.stencil.front.fail_op, BINOCLE_STENCILOP_KEEP);
  def.stencil.front.depth_fail_op = BINOCLE_DEF(def.stencil.front.depth_fail_op, BINOCLE_STENCILOP_KEEP);
  def.stencil.front.pass_op = BINOCLE_DEF(def.stencil.front.pass_op, BINOCLE_STENCILOP_KEEP);
  def.stencil.back.compare = BINOCLE_DEF(def.stencil.back.compare, BINOCLE_COMPAREFUNC_ALWAYS);
  def.stencil.back.fail_op = BINOCLE_DEF(def.stencil.back.fail_op, BINOCLE_STENCILOP_KEEP);
  def.stencil.back.depth_fail_op = BINOCLE_DEF(def.stencil.back.depth_fail_op, BINOCLE_STENCILOP_KEEP);
  def.stencil.back.pass_op = BINOCLE_DEF(def.stencil.back.pass_op, BINOCLE_STENCILOP_KEEP);

  def.depth.compare = BINOCLE_DEF(def.depth.compare, BINOCLE_COMPAREFUNC_ALWAYS);
  def.depth.pixel_format = BINOCLE_DEF(def.depth.pixel_format, backend.desc.context.depth_format);
  def.color_count = BINOCLE_DEF(def.color_count, 1);
  if (def.color_count > BINOCLE_MAX_COLOR_ATTACHMENTS) {
    def.color_count = BINOCLE_MAX_COLOR_ATTACHMENTS;
  }
  for (int i = 0; i < def.color_count; i++) {
    binocle_color_state* cs = &def.colors[i];
    cs->pixel_format = BINOCLE_DEF(cs->pixel_format, backend.desc.context.color_format);
    cs->write_mask = BINOCLE_DEF(cs->write_mask, BINOCLE_COLORMASK_RGBA);
    binocle_blend_state* bs = &def.colors[i].blend;
    bs->src_factor_rgb = BINOCLE_DEF(bs->src_factor_rgb, BINOCLE_BLENDFACTOR_ONE);
    bs->dst_factor_rgb = BINOCLE_DEF(bs->dst_factor_rgb, BINOCLE_BLENDFACTOR_ZERO);
    bs->op_rgb = BINOCLE_DEF(bs->op_rgb, BINOCLE_BLENDOP_ADD);
    bs->src_factor_alpha = BINOCLE_DEF(bs->src_factor_alpha, BINOCLE_BLENDFACTOR_ONE);
    bs->dst_factor_alpha = BINOCLE_DEF(bs->dst_factor_alpha, BINOCLE_BLENDFACTOR_ZERO);
    bs->op_alpha = BINOCLE_DEF(bs->op_alpha, BINOCLE_BLENDOP_ADD);
  }

  for (int attr_index = 0; attr_index < BINOCLE_MAX_VERTEX_ATTRIBUTES; attr_index++) {
    binocle_vertex_attr_desc* a_desc = &def.layout.attrs[attr_index];
    if (a_desc->format == BINOCLE_VERTEXFORMAT_INVALID) {
      break;
    }
    assert(a_desc->buffer_index < BINOCLE_MAX_SHADERSTAGE_BUFFERS);
    binocle_buffer_layout_desc* b_desc = &def.layout.buffers[a_desc->buffer_index];
    b_desc->step_func = BINOCLE_DEF(b_desc->step_func, BINOCLE_VERTEXSTEP_PER_VERTEX);
    b_desc->step_rate = BINOCLE_DEF(b_desc->step_rate, 1);
  }

  /* resolve vertex layout strides and offsets */
  int auto_offset[BINOCLE_MAX_SHADERSTAGE_BUFFERS];
  memset(auto_offset, 0, sizeof(auto_offset));
  bool use_auto_offset = true;
  for (int attr_index = 0; attr_index < BINOCLE_MAX_VERTEX_ATTRIBUTES; attr_index++) {
    /* to use computed offsets, *all* attr offsets must be 0 */
    if (def.layout.attrs[attr_index].offset != 0) {
      use_auto_offset = false;
    }
  }
  for (int attr_index = 0; attr_index < BINOCLE_MAX_VERTEX_ATTRIBUTES; attr_index++) {
    binocle_vertex_attr_desc* a_desc = &def.layout.attrs[attr_index];
    if (a_desc->format == BINOCLE_VERTEXFORMAT_INVALID) {
      break;
    }
    assert(a_desc->buffer_index < BINOCLE_MAX_SHADERSTAGE_BUFFERS);
    if (use_auto_offset) {
      a_desc->offset = auto_offset[a_desc->buffer_index];
    }
    auto_offset[a_desc->buffer_index] += binocle_backend_vertexformat_bytesize(a_desc->format);
  }
  /* compute vertex strides if needed */
  for (int buf_index = 0; buf_index < BINOCLE_MAX_SHADERSTAGE_BUFFERS; buf_index++) {
    binocle_buffer_layout_desc* l_desc = &def.layout.buffers[buf_index];
    if (l_desc->stride == 0) {
      l_desc->stride = auto_offset[buf_index];
    }
  }

  return def;
}

binocle_pipeline binocle_backend_alloc_pipeline(void) {
  binocle_pipeline res;
  int slot_index = binocle_pool_alloc_index(&backend.pools.pipeline_pool);
  if (BINOCLE_POOL_INVALID_SLOT_INDEX != slot_index) {
    res.id = binocle_pool_slot_alloc(&backend.pools.pipeline_pool, &backend.pools.pipelines[slot_index].slot, slot_index);
  }
  else {
    /* pool is exhausted */
    res.id = BINOCLE_INVALID_ID;
  }
  return res;
}

binocle_resource_state binocle_backend_create_pipeline(binocle_pipeline_t* pip, binocle_shader_t* shd, const binocle_pipeline_desc* desc) {
#if defined(BINOCLE_GL)
  return binocle_backend_gl_create_pipeline(&backend.gl, pip, shd, desc);
#elif defined(BINOCLE_METAL)
  return binocle_backend_mtl_create_pipeline(&backend.mtl, pip, shd, desc);
#else
#error("INVALID BACKEND");
#endif
}

bool binocle_backend_validate_pipeline_desc(const binocle_pipeline_desc* desc) {
#if !defined(BINOCLE_DEBUG)
  return true;
#else
  SOKOL_ASSERT(desc);
        SOKOL_VALIDATE_BEGIN();
        SOKOL_VALIDATE(desc->_start_canary == 0, _SG_VALIDATE_PIPELINEDESC_CANARY);
        SOKOL_VALIDATE(desc->_end_canary == 0, _SG_VALIDATE_PIPELINEDESC_CANARY);
        SOKOL_VALIDATE(desc->shader.id != SG_INVALID_ID, _SG_VALIDATE_PIPELINEDESC_SHADER);
        for (int buf_index = 0; buf_index < SG_MAX_SHADERSTAGE_BUFFERS; buf_index++) {
            const sg_buffer_layout_desc* l_desc = &desc->layout.buffers[buf_index];
            if (l_desc->stride == 0) {
                continue;
            }
            SOKOL_VALIDATE((l_desc->stride & 3) == 0, _SG_VALIDATE_PIPELINEDESC_LAYOUT_STRIDE4);
        }
        SOKOL_VALIDATE(desc->layout.attrs[0].format != SG_VERTEXFORMAT_INVALID, _SG_VALIDATE_PIPELINEDESC_NO_ATTRS);
        const _sg_shader_t* shd = _sg_lookup_shader(&_sg.pools, desc->shader.id);
        SOKOL_VALIDATE(0 != shd, _SG_VALIDATE_PIPELINEDESC_SHADER);
        if (shd) {
            SOKOL_VALIDATE(shd->slot.state == SG_RESOURCESTATE_VALID, _SG_VALIDATE_PIPELINEDESC_SHADER);
            bool attrs_cont = true;
            for (int attr_index = 0; attr_index < SG_MAX_VERTEX_ATTRIBUTES; attr_index++) {
                const sg_vertex_attr_desc* a_desc = &desc->layout.attrs[attr_index];
                if (a_desc->format == SG_VERTEXFORMAT_INVALID) {
                    attrs_cont = false;
                    continue;
                }
                SOKOL_VALIDATE(attrs_cont, _SG_VALIDATE_PIPELINEDESC_NO_ATTRS);
                SOKOL_ASSERT(a_desc->buffer_index < SG_MAX_SHADERSTAGE_BUFFERS);
                #if defined(SOKOL_GLES2)
                /* on GLES2, vertex attribute names must be provided */
                SOKOL_VALIDATE(!_sg_strempty(&shd->gl.attrs[attr_index].name), _SG_VALIDATE_PIPELINEDESC_ATTR_NAME);
                #elif defined(SOKOL_D3D11)
                /* on D3D11, semantic names (and semantic indices) must be provided */
                SOKOL_VALIDATE(!_sg_strempty(&shd->d3d11.attrs[attr_index].sem_name), _SG_VALIDATE_PIPELINEDESC_ATTR_SEMANTICS);
                #endif
            }
        }
        return SOKOL_VALIDATE_END();
#endif
}

void binocle_backend_init_pipeline(binocle_pipeline pip_id, const binocle_pipeline_desc* desc) {
  assert(pip_id.id != BINOCLE_INVALID_ID && desc);
  binocle_pipeline_t* pip = binocle_backend_lookup_pipeline(&backend.pools, pip_id.id);
  assert(pip && pip->slot.state == BINOCLE_RESOURCESTATE_ALLOC);
//  pip->slot.ctx_id = backend.active_context.id;
  if (binocle_backend_validate_pipeline_desc(desc)) {
    binocle_shader_t* shd = binocle_backend_lookup_shader(&backend.pools, desc->shader.id);
    if (shd && (shd->slot.state == BINOCLE_RESOURCESTATE_VALID)) {
      pip->slot.state = binocle_backend_create_pipeline(pip, shd, desc);
    }
    else {
      pip->slot.state = BINOCLE_RESOURCESTATE_FAILED;
    }
  }
  else {
    pip->slot.state = BINOCLE_RESOURCESTATE_FAILED;
  }
  assert((pip->slot.state == BINOCLE_RESOURCESTATE_VALID)||(pip->slot.state == BINOCLE_RESOURCESTATE_FAILED));
}


binocle_pipeline binocle_backend_make_pipeline(const binocle_pipeline_desc* desc) {
  assert(desc);
  binocle_pipeline_desc desc_def = binocle_backend_pipeline_desc_defaults(desc);
  binocle_pipeline pip_id = binocle_backend_alloc_pipeline();
  if (pip_id.id != BINOCLE_INVALID_ID) {
    binocle_backend_init_pipeline(pip_id, &desc_def);
  }
  else {
    binocle_log_error("pipeline pool exhausted!");
  }
  return pip_id;
}

binocle_pass_desc binocle_backend_pass_desc_defaults(const binocle_pass_desc* desc) {
  /* FIXME: no values to replace in binocle_pass_desc? */
  binocle_pass_desc def = *desc;
  return def;
}

binocle_pass binocle_backend_alloc_pass(void) {
  binocle_pass res;
  int slot_index = binocle_pool_alloc_index(&backend.pools.pass_pool);
  if (BINOCLE_POOL_INVALID_SLOT_INDEX != slot_index) {
    res.id = binocle_pool_slot_alloc(&backend.pools.pass_pool, &backend.pools.passes[slot_index].slot, slot_index);
  }
  else {
    /* pool is exhausted */
    res.id = BINOCLE_INVALID_ID;
  }
  return res;
}

bool binocle_backend_validate_pass_desc(const binocle_pass_desc* desc) {
#if !defined(BINOCLE_DEBUG)
  return true;
#else
  SOKOL_ASSERT(desc);
        SOKOL_VALIDATE_BEGIN();
        SOKOL_VALIDATE(desc->_start_canary == 0, _SG_VALIDATE_PASSDESC_CANARY);
        SOKOL_VALIDATE(desc->_end_canary == 0, _SG_VALIDATE_PASSDESC_CANARY);
        bool atts_cont = true;
        int width = -1, height = -1, sample_count = -1;
        for (int att_index = 0; att_index < SG_MAX_COLOR_ATTACHMENTS; att_index++) {
            const sg_pass_attachment_desc* att = &desc->color_attachments[att_index];
            if (att->image.id == SG_INVALID_ID) {
                SOKOL_VALIDATE(att_index > 0, _SG_VALIDATE_PASSDESC_NO_COLOR_ATTS);
                atts_cont = false;
                continue;
            }
            SOKOL_VALIDATE(atts_cont, _SG_VALIDATE_PASSDESC_NO_CONT_COLOR_ATTS);
            const _sg_image_t* img = _sg_lookup_image(&_sg.pools, att->image.id);
            SOKOL_ASSERT(img);
            SOKOL_VALIDATE(img->slot.state == SG_RESOURCESTATE_VALID, _SG_VALIDATE_PASSDESC_IMAGE);
            SOKOL_VALIDATE(att->mip_level < img->cmn.num_mipmaps, _SG_VALIDATE_PASSDESC_MIPLEVEL);
            if (img->cmn.type == SG_IMAGETYPE_CUBE) {
                SOKOL_VALIDATE(att->slice < 6, _SG_VALIDATE_PASSDESC_FACE);
            }
            else if (img->cmn.type == SG_IMAGETYPE_ARRAY) {
                SOKOL_VALIDATE(att->slice < img->cmn.num_slices, _SG_VALIDATE_PASSDESC_LAYER);
            }
            else if (img->cmn.type == SG_IMAGETYPE_3D) {
                SOKOL_VALIDATE(att->slice < img->cmn.num_slices, _SG_VALIDATE_PASSDESC_SLICE);
            }
            SOKOL_VALIDATE(img->cmn.render_target, _SG_VALIDATE_PASSDESC_IMAGE_NO_RT);
            if (att_index == 0) {
                width = img->cmn.width >> att->mip_level;
                height = img->cmn.height >> att->mip_level;
                sample_count = img->cmn.sample_count;
            }
            else {
                SOKOL_VALIDATE(width == img->cmn.width >> att->mip_level, _SG_VALIDATE_PASSDESC_IMAGE_SIZES);
                SOKOL_VALIDATE(height == img->cmn.height >> att->mip_level, _SG_VALIDATE_PASSDESC_IMAGE_SIZES);
                SOKOL_VALIDATE(sample_count == img->cmn.sample_count, _SG_VALIDATE_PASSDESC_IMAGE_SAMPLE_COUNTS);
            }
            SOKOL_VALIDATE(_sg_is_valid_rendertarget_color_format(img->cmn.pixel_format), _SG_VALIDATE_PASSDESC_COLOR_INV_PIXELFORMAT);
        }
        if (desc->depth_stencil_attachment.image.id != SG_INVALID_ID) {
            const sg_pass_attachment_desc* att = &desc->depth_stencil_attachment;
            const _sg_image_t* img = _sg_lookup_image(&_sg.pools, att->image.id);
            SOKOL_ASSERT(img);
            SOKOL_VALIDATE(img->slot.state == SG_RESOURCESTATE_VALID, _SG_VALIDATE_PASSDESC_IMAGE);
            SOKOL_VALIDATE(att->mip_level < img->cmn.num_mipmaps, _SG_VALIDATE_PASSDESC_MIPLEVEL);
            if (img->cmn.type == SG_IMAGETYPE_CUBE) {
                SOKOL_VALIDATE(att->slice < 6, _SG_VALIDATE_PASSDESC_FACE);
            }
            else if (img->cmn.type == SG_IMAGETYPE_ARRAY) {
                SOKOL_VALIDATE(att->slice < img->cmn.num_slices, _SG_VALIDATE_PASSDESC_LAYER);
            }
            else if (img->cmn.type == SG_IMAGETYPE_3D) {
                SOKOL_VALIDATE(att->slice < img->cmn.num_slices, _SG_VALIDATE_PASSDESC_SLICE);
            }
            SOKOL_VALIDATE(img->cmn.render_target, _SG_VALIDATE_PASSDESC_IMAGE_NO_RT);
            SOKOL_VALIDATE(width == img->cmn.width >> att->mip_level, _SG_VALIDATE_PASSDESC_IMAGE_SIZES);
            SOKOL_VALIDATE(height == img->cmn.height >> att->mip_level, _SG_VALIDATE_PASSDESC_IMAGE_SIZES);
            SOKOL_VALIDATE(sample_count == img->cmn.sample_count, _SG_VALIDATE_PASSDESC_IMAGE_SAMPLE_COUNTS);
            SOKOL_VALIDATE(_sg_is_valid_rendertarget_depth_format(img->cmn.pixel_format), _SG_VALIDATE_PASSDESC_DEPTH_INV_PIXELFORMAT);
        }
        return SOKOL_VALIDATE_END();
#endif
}

binocle_resource_state binocle_backend_create_pass(binocle_pass_t* pass, binocle_image_t** att_images, const binocle_pass_desc* desc) {
#if defined(BINOCLE_GL)
  return binocle_backend_gl_create_pass(&backend.gl, pass, att_images, desc);
#elif defined(BINOCLE_METAL)
  return binocle_backend_mtl_create_pass(pass, att_images, desc);
#else
#error("INVALID BACKEND");
#endif
}

void binocle_backend_init_pass(binocle_pass pass_id, const binocle_pass_desc* desc) {
  assert(pass_id.id != BINOCLE_INVALID_ID && desc);
  binocle_pass_t* pass = binocle_backend_lookup_pass(&backend.pools, pass_id.id);
  assert(pass && pass->slot.state == BINOCLE_RESOURCESTATE_ALLOC);
//  pass->slot.ctx_id = backend.active_context.id;
  if (binocle_backend_validate_pass_desc(desc)) {
    /* lookup pass attachment image pointers */
    binocle_image_t* att_imgs[BINOCLE_MAX_COLOR_ATTACHMENTS + 1];
    for (int i = 0; i < BINOCLE_MAX_COLOR_ATTACHMENTS; i++) {
      if (desc->color_attachments[i].image.id) {
        att_imgs[i] = binocle_backend_lookup_image(&backend.pools, desc->color_attachments[i].image.id);
        /* FIXME: this shouldn't be an assertion, but result in a BINOCLE_RESOURCESTATE_FAILED pass */
        assert(att_imgs[i] && att_imgs[i]->slot.state == BINOCLE_RESOURCESTATE_VALID);
      }
      else {
        att_imgs[i] = 0;
      }
    }
    const int ds_att_index = BINOCLE_MAX_COLOR_ATTACHMENTS;
    if (desc->depth_stencil_attachment.image.id) {
      att_imgs[ds_att_index] = binocle_backend_lookup_image(&backend.pools, desc->depth_stencil_attachment.image.id);
      /* FIXME: this shouldn't be an assertion, but result in a BINOCLE_RESOURCESTATE_FAILED pass */
      assert(att_imgs[ds_att_index] && att_imgs[ds_att_index]->slot.state == BINOCLE_RESOURCESTATE_VALID);
    }
    else {
      att_imgs[ds_att_index] = 0;
    }
    pass->slot.state = binocle_backend_create_pass(pass, att_imgs, desc);
  }
  else {
    pass->slot.state = BINOCLE_RESOURCESTATE_FAILED;
  }
  assert((pass->slot.state == BINOCLE_RESOURCESTATE_VALID)||(pass->slot.state == BINOCLE_RESOURCESTATE_FAILED));
}

binocle_pass binocle_backend_make_pass(const binocle_pass_desc* desc) {
  assert(desc);
  binocle_pass_desc desc_def = binocle_backend_pass_desc_defaults(desc);
  binocle_pass pass_id = binocle_backend_alloc_pass();
  if (pass_id.id != BINOCLE_INVALID_ID) {
    binocle_backend_init_pass(pass_id, &desc_def);
  }
  else {
    binocle_log_error("pass pool exhausted!");
  }
  return pass_id;
}

void binocle_backend_destroy_all_resources(binocle_pools_t* p, uint32_t ctx_id) {
  /*  this is a bit dumb since it loops over all pool slots to
      find the occupied slots, on the other hand it is only ever
      executed at shutdown
      NOTE: ONLY EXECUTE THIS AT SHUTDOWN
            ...because the free queues will not be reset
            and the resource slots not be cleared!
  */
  for (int i = 1; i < p->image_pool.size; i++) {
    if (p->images[i].slot.ctx_id == ctx_id) {
      binocle_resource_state state = p->images[i].slot.state;
      if ((state == BINOCLE_RESOURCESTATE_VALID) || (state == BINOCLE_RESOURCESTATE_FAILED)) {
        binocle_backend_destroy_image_t(&p->images[i]);
      }
    }
  }

  for (int i = 1; i < p->shader_pool.size; i++) {
    if (p->shaders[i].slot.ctx_id == ctx_id) {
      binocle_resource_state state = p->shaders[i].slot.state;
      if ((state == BINOCLE_RESOURCESTATE_VALID) || (state == BINOCLE_RESOURCESTATE_FAILED)) {
        binocle_backend_destroy_shader_t(&p->shaders[i]);
      }
    }
  }
}

void binocle_backend_setup_backend(binocle_backend_desc *desc) {
#if defined(BINOCLE_GL)
  // TODO: move this somewhere else
  backend.formats[BINOCLE_PIXELFORMAT_RGBA8].render = true;
  backend.formats[BINOCLE_PIXELFORMAT_RGBA8].render = true;
  backend.formats[BINOCLE_PIXELFORMAT_DEPTH].render = true;
  backend.formats[BINOCLE_PIXELFORMAT_DEPTH].depth = true;
  backend.formats[BINOCLE_PIXELFORMAT_DEPTH_STENCIL].render = true;
  backend.formats[BINOCLE_PIXELFORMAT_DEPTH_STENCIL].depth = true;
  binocle_backend_gl_setup_backend(&backend.gl, desc);
#elif defined(BINOCLE_METAL)
  binocle_backend_mtl_setup_backend(&backend.mtl, desc);
#else
#error("no backend defined")
#endif
}

void _binocle_backend_activate_context(binocle_context_t* ctx) {
#if defined(BINOCLE_GL)
  binocle_backend_gl_activate_context(&backend.gl, ctx);
#elif defined(BINOCLE_METAL)
  binocle_backend_mtl_activate_context(&backend.mtl, ctx);
#else
#error("INVALID BACKEND");
#endif
}

binocle_resource_state binocle_backend_create_context(binocle_context_t* ctx) {
#if defined(BINOCLE_GL)
  return binocle_backend_gl_create_context(&backend.gl, ctx);
#elif defined(BINOCLE_METAL)
  return binocle_backend_mtl_create_context(ctx);
#else
#error("INVALID BACKEND");
#endif
}

void binocle_backend_destroy_context(binocle_context_t* ctx) {
#if defined(BINOCLE_GL)
  binocle_backend_gl_destroy_context(&backend.gl, ctx);
#elif defined(BINOCLE_METAL)
  binocle_backend_mtl_destroy_context(ctx);
#else
#error("INVALID BACKEND");
#endif
}

binocle_context binocle_backend_setup_context(void) {
  assert(backend.valid);
  binocle_context res;
  int slot_index = binocle_pool_alloc_index(&backend.pools.context_pool);
  if (BINOCLE_POOL_INVALID_SLOT_INDEX != slot_index) {
    res.id = binocle_pool_slot_alloc(&backend.pools.context_pool, &backend.pools.contexts[slot_index].slot, slot_index);
    binocle_context_t* ctx = binocle_backend_context_at(&backend.pools, res.id);
    ctx->slot.state = binocle_backend_create_context(ctx);
    assert(ctx->slot.state == BINOCLE_RESOURCESTATE_VALID);
    _binocle_backend_activate_context(ctx);
  }
  else {
    /* pool is exhausted */
    res.id = BINOCLE_INVALID_ID;
  }
  backend.active_context = res;
  return res;
}

void binocle_backend_discard_context(binocle_context ctx_id) {
  assert(backend.valid);
  binocle_backend_destroy_all_resources(&backend.pools, ctx_id.id);
  binocle_context_t* ctx = binocle_backend_lookup_context(&backend.pools, ctx_id.id);
  if (ctx) {
    binocle_backend_destroy_context(ctx);
    binocle_backend_reset_context(ctx);
    binocle_backend_reset_slot(&ctx->slot);
    binocle_pool_free_index(&backend.pools.context_pool, binocle_pool_slot_index(ctx_id.id));
  }
  backend.active_context.id = BINOCLE_INVALID_ID;
  _binocle_backend_activate_context(0);
}

void binocle_backend_activate_context(binocle_context ctx_id) {
  assert(backend.valid);
  backend.active_context = ctx_id;
  binocle_context_t* ctx = binocle_backend_lookup_context(&backend.pools, ctx_id.id);
  /* NOTE: ctx can be 0 here if the context is no longer valid */
  _binocle_backend_activate_context(ctx);
}

void binocle_backend_setup(const binocle_backend_desc* desc) {
  assert(desc);
  backend = (binocle_backend_t){ 0 };
  backend.desc = *desc;

#if defined(BINOCLE_METAL)
  backend.desc.context.color_format = BINOCLE_DEF(backend.desc.context.color_format, BINOCLE_PIXELFORMAT_BGRA8);
#else
  backend.desc.context.color_format = BINOCLE_DEF(backend.desc.context.color_format, BINOCLE_PIXELFORMAT_RGBA8);
#endif
  backend.desc.context.depth_format = BINOCLE_DEF(backend.desc.context.depth_format, BINOCLE_PIXELFORMAT_DEPTH_STENCIL);
  backend.desc.context.sample_count = BINOCLE_DEF(backend.desc.context.sample_count, 1);
  backend.desc.buffer_pool_size = BINOCLE_DEF(backend.desc.buffer_pool_size, BINOCLE_DEFAULT_BUFFER_POOL_SIZE);
  backend.desc.image_pool_size = BINOCLE_DEF(backend.desc.image_pool_size, BINOCLE_DEFAULT_IMAGE_POOL_SIZE);
  backend.desc.shader_pool_size = BINOCLE_DEF(backend.desc.shader_pool_size, BINOCLE_DEFAULT_SHADER_POOL_SIZE);
  backend.desc.pipeline_pool_size = BINOCLE_DEF(backend.desc.pipeline_pool_size, BINOCLE_DEFAULT_PIPELINE_POOL_SIZE);
  backend.desc.pass_pool_size = BINOCLE_DEF(backend.desc.pass_pool_size, BINOCLE_DEFAULT_PASS_POOL_SIZE);
  backend.desc.context_pool_size = BINOCLE_DEF(backend.desc.context_pool_size, BINOCLE_DEFAULT_CONTEXT_POOL_SIZE);
  backend.desc.uniform_buffer_size = BINOCLE_DEF(backend.desc.uniform_buffer_size, BINOCLE_DEFAULT_UB_SIZE);
  //backend.desc.staging_buffer_size = BINOCLE_DEF(backend.desc.staging_buffer_size, BINOCLE_DEFAULT_STAGING_SIZE);
  backend.desc.sampler_cache_size = BINOCLE_DEF(backend.desc.sampler_cache_size, BINOCLE_DEFAULT_SAMPLER_CACHE_CAPACITY);

  binocle_backend_setup_pools(&backend.pools, &backend.desc);
  backend.frame_index = 1;
  binocle_backend_setup_backend(&backend.desc);
  backend.valid = true;
  binocle_backend_setup_context();
}

void binocle_backend_apply_default_state() {
#if defined(BINOCLE_GL)
  binocle_backend_gl_apply_default_state();
#elif defined(BINOCLE_METAL)
  #else
#error("no backend defined")
#endif
}

void _binocle_backend_begin_pass(binocle_pass_t* pass, const binocle_pass_action* action, int w, int h) {
#if defined(BINOCLE_GL)
  binocle_backend_gl_begin_pass(&backend.gl, pass, action, w, h);
#elif defined(BINOCLE_METAL)
  binocle_backend_mtl_begin_pass(&backend.mtl, pass, action, w, h);
#else
#error("no backend defined");
#endif
}

bool binocle_backend_validate_begin_pass(binocle_pass_t* pass) {
#if !defined(BINOCLE_DEBUG)
  return true;
#else
  SOKOL_VALIDATE_BEGIN();
        SOKOL_VALIDATE(pass->slot.state == SG_RESOURCESTATE_VALID, _SG_VALIDATE_BEGINPASS_PASS);

        for (int i = 0; i < SG_MAX_COLOR_ATTACHMENTS; i++) {
            const _sg_pass_attachment_t* att = &pass->cmn.color_atts[i];
            const _sg_image_t* img = _sg_pass_color_image(pass, i);
            if (img) {
                SOKOL_VALIDATE(img->slot.state == SG_RESOURCESTATE_VALID, _SG_VALIDATE_BEGINPASS_IMAGE);
                SOKOL_VALIDATE(img->slot.id == att->image_id.id, _SG_VALIDATE_BEGINPASS_IMAGE);
            }
        }
        const _sg_image_t* ds_img = _sg_pass_ds_image(pass);
        if (ds_img) {
            const _sg_pass_attachment_t* att = &pass->cmn.ds_att;
            SOKOL_VALIDATE(ds_img->slot.state == SG_RESOURCESTATE_VALID, _SG_VALIDATE_BEGINPASS_IMAGE);
            SOKOL_VALIDATE(ds_img->slot.id == att->image_id.id, _SG_VALIDATE_BEGINPASS_IMAGE);
        }
        return SOKOL_VALIDATE_END();
#endif
}

void binocle_backend_resolve_default_pass_action(const binocle_pass_action* from, binocle_pass_action* to) {
  assert(from && to);
  *to = *from;
  for (int i = 0; i < BINOCLE_MAX_COLOR_ATTACHMENTS; i++) {
    if (to->colors[i].action  == BINOCLE_ACTION_DEFAULT) {
      to->colors[i].action = BINOCLE_ACTION_CLEAR;
      to->colors[i].value.r = BINOCLE_DEFAULT_CLEAR_RED;
      to->colors[i].value.g = BINOCLE_DEFAULT_CLEAR_GREEN;
      to->colors[i].value.b = BINOCLE_DEFAULT_CLEAR_BLUE;
      to->colors[i].value.a = BINOCLE_DEFAULT_CLEAR_ALPHA;
    }
  }
  if (to->depth.action == BINOCLE_ACTION_DEFAULT) {
    to->depth.action = BINOCLE_ACTION_CLEAR;
    to->depth.value = BINOCLE_DEFAULT_CLEAR_DEPTH;
  }
  if (to->stencil.action == BINOCLE_ACTION_DEFAULT) {
    to->stencil.action = BINOCLE_ACTION_CLEAR;
    to->stencil.value = BINOCLE_DEFAULT_CLEAR_STENCIL;
  }
}

binocle_image_t* binocle_backend_pass_color_image(const binocle_pass_t* pass, int index) {
#if defined(BINOCLE_GL)
  return binocle_backend_gl_pass_color_image(pass, index);
#elif defined(BINOCLE_METAL)
  return binocle_backend_mtl_pass_color_image(pass, index);
#else
#error("INVALID BACKEND");
#endif
}

void binocle_backend_begin_pass(binocle_pass pass_id, const binocle_pass_action* pass_action) {
  assert(backend.valid);
  assert(pass_action);
  assert((pass_action->_start_canary == 0) && (pass_action->_end_canary == 0));
  backend.cur_pass = pass_id;
  binocle_pass_t* pass = binocle_backend_lookup_pass(&backend.pools, pass_id.id);
  if (pass && binocle_backend_validate_begin_pass(pass)) {
    backend.pass_valid = true;
    binocle_pass_action pa;
    binocle_backend_resolve_default_pass_action(pass_action, &pa);
    const binocle_image_t* img = binocle_backend_pass_color_image(pass, 0);
    assert(img);
    const int w = img->cmn.width;
    const int h = img->cmn.height;
    _binocle_backend_begin_pass(pass, &pa, w, h);
//    BINOCLE_TRACE_ARGS(begin_pass, pass_id, pass_action);
  }
  else {
    backend.pass_valid = false;
//    BINOCLE_TRACE_NOARGS(err_pass_invalid);
  }
}

void binocle_backend_end_pass(void) {
#if defined(BINOCLE_GL)
  binocle_backend_gl_end_pass(&backend.gl);
#elif defined(BINOCLE_METAL)
  binocle_backend_mtl_end_pass(&backend.mtl);
#else
#error("no backend defined");
#endif
}

bool binocle_backend_validate_apply_pipeline(binocle_pipeline pip_id) {
#if !defined(BINOCLE_DEBUG)
  return true;
#else
  SOKOL_VALIDATE_BEGIN();
        /* the pipeline object must be alive and valid */
        SOKOL_VALIDATE(pip_id.id != SG_INVALID_ID, _SG_VALIDATE_APIP_PIPELINE_VALID_ID);
        const _sg_pipeline_t* pip = _sg_lookup_pipeline(&_sg.pools, pip_id.id);
        SOKOL_VALIDATE(pip != 0, _SG_VALIDATE_APIP_PIPELINE_EXISTS);
        if (!pip) {
            return SOKOL_VALIDATE_END();
        }
        SOKOL_VALIDATE(pip->slot.state == SG_RESOURCESTATE_VALID, _SG_VALIDATE_APIP_PIPELINE_VALID);
        /* the pipeline's shader must be alive and valid */
        SOKOL_ASSERT(pip->shader);
        SOKOL_VALIDATE(pip->shader->slot.id == pip->cmn.shader_id.id, _SG_VALIDATE_APIP_SHADER_EXISTS);
        SOKOL_VALIDATE(pip->shader->slot.state == SG_RESOURCESTATE_VALID, _SG_VALIDATE_APIP_SHADER_VALID);
        /* check that pipeline attributes match current pass attributes */
        const _sg_pass_t* pass = _sg_lookup_pass(&_sg.pools, _sg.cur_pass.id);
        if (pass) {
            /* an offscreen pass */
            SOKOL_VALIDATE(pip->cmn.color_attachment_count == pass->cmn.num_color_atts, _SG_VALIDATE_APIP_ATT_COUNT);
            for (int i = 0; i < pip->cmn.color_attachment_count; i++) {
                const _sg_image_t* att_img = _sg_pass_color_image(pass, i);
                SOKOL_VALIDATE(pip->cmn.color_formats[i] == att_img->cmn.pixel_format, _SG_VALIDATE_APIP_COLOR_FORMAT);
                SOKOL_VALIDATE(pip->cmn.sample_count == att_img->cmn.sample_count, _SG_VALIDATE_APIP_SAMPLE_COUNT);
            }
            const _sg_image_t* att_dsimg = _sg_pass_ds_image(pass);
            if (att_dsimg) {
                SOKOL_VALIDATE(pip->cmn.depth_format == att_dsimg->cmn.pixel_format, _SG_VALIDATE_APIP_DEPTH_FORMAT);
            }
            else {
                SOKOL_VALIDATE(pip->cmn.depth_format == SG_PIXELFORMAT_NONE, _SG_VALIDATE_APIP_DEPTH_FORMAT);
            }
        }
        else {
            /* default pass */
            SOKOL_VALIDATE(pip->cmn.color_attachment_count == 1, _SG_VALIDATE_APIP_ATT_COUNT);
            SOKOL_VALIDATE(pip->cmn.color_formats[0] == _sg.desc.context.color_format, _SG_VALIDATE_APIP_COLOR_FORMAT);
            SOKOL_VALIDATE(pip->cmn.depth_format == _sg.desc.context.depth_format, _SG_VALIDATE_APIP_DEPTH_FORMAT);
            SOKOL_VALIDATE(pip->cmn.sample_count == _sg.desc.context.sample_count, _SG_VALIDATE_APIP_SAMPLE_COUNT);
        }
        return SOKOL_VALIDATE_END();
#endif
}

bool binocle_backend_validate_apply_bindings(const binocle_bindings* bindings) {
#if !defined(BINOCLE_DEBUG)
  return true;
#else
  SOKOL_VALIDATE_BEGIN();

        /* a pipeline object must have been applied */
        SOKOL_VALIDATE(_sg.cur_pipeline.id != SG_INVALID_ID, _SG_VALIDATE_ABND_PIPELINE);
        const _sg_pipeline_t* pip = _sg_lookup_pipeline(&_sg.pools, _sg.cur_pipeline.id);
        SOKOL_VALIDATE(pip != 0, _SG_VALIDATE_ABND_PIPELINE_EXISTS);
        if (!pip) {
            return SOKOL_VALIDATE_END();
        }
        SOKOL_VALIDATE(pip->slot.state == SG_RESOURCESTATE_VALID, _SG_VALIDATE_ABND_PIPELINE_VALID);
        SOKOL_ASSERT(pip->shader && (pip->cmn.shader_id.id == pip->shader->slot.id));

        /* has expected vertex buffers, and vertex buffers still exist */
        for (int i = 0; i < SG_MAX_SHADERSTAGE_BUFFERS; i++) {
            if (bindings->vertex_buffers[i].id != SG_INVALID_ID) {
                SOKOL_VALIDATE(pip->cmn.vertex_layout_valid[i], _SG_VALIDATE_ABND_VBS);
                /* buffers in vertex-buffer-slots must be of type SG_BUFFERTYPE_VERTEXBUFFER */
                const _sg_buffer_t* buf = _sg_lookup_buffer(&_sg.pools, bindings->vertex_buffers[i].id);
                SOKOL_VALIDATE(buf != 0, _SG_VALIDATE_ABND_VB_EXISTS);
                if (buf && buf->slot.state == SG_RESOURCESTATE_VALID) {
                    SOKOL_VALIDATE(SG_BUFFERTYPE_VERTEXBUFFER == buf->cmn.type, _SG_VALIDATE_ABND_VB_TYPE);
                    SOKOL_VALIDATE(!buf->cmn.append_overflow, _SG_VALIDATE_ABND_VB_OVERFLOW);
                }
            }
            else {
                /* vertex buffer provided in a slot which has no vertex layout in pipeline */
                SOKOL_VALIDATE(!pip->cmn.vertex_layout_valid[i], _SG_VALIDATE_ABND_VBS);
            }
        }

        /* index buffer expected or not, and index buffer still exists */
        if (pip->cmn.index_type == SG_INDEXTYPE_NONE) {
            /* pipeline defines non-indexed rendering, but index buffer provided */
            SOKOL_VALIDATE(bindings->index_buffer.id == SG_INVALID_ID, _SG_VALIDATE_ABND_IB);
        }
        else {
            /* pipeline defines indexed rendering, but no index buffer provided */
            SOKOL_VALIDATE(bindings->index_buffer.id != SG_INVALID_ID, _SG_VALIDATE_ABND_NO_IB);
        }
        if (bindings->index_buffer.id != SG_INVALID_ID) {
            /* buffer in index-buffer-slot must be of type SG_BUFFERTYPE_INDEXBUFFER */
            const _sg_buffer_t* buf = _sg_lookup_buffer(&_sg.pools, bindings->index_buffer.id);
            SOKOL_VALIDATE(buf != 0, _SG_VALIDATE_ABND_IB_EXISTS);
            if (buf && buf->slot.state == SG_RESOURCESTATE_VALID) {
                SOKOL_VALIDATE(SG_BUFFERTYPE_INDEXBUFFER == buf->cmn.type, _SG_VALIDATE_ABND_IB_TYPE);
                SOKOL_VALIDATE(!buf->cmn.append_overflow, _SG_VALIDATE_ABND_IB_OVERFLOW);
            }
        }

        /* has expected vertex shader images */
        for (int i = 0; i < SG_MAX_SHADERSTAGE_IMAGES; i++) {
            _sg_shader_stage_t* stage = &pip->shader->cmn.stage[SG_SHADERSTAGE_VS];
            if (bindings->vs_images[i].id != SG_INVALID_ID) {
                SOKOL_VALIDATE(i < stage->num_images, _SG_VALIDATE_ABND_VS_IMGS);
                const _sg_image_t* img = _sg_lookup_image(&_sg.pools, bindings->vs_images[i].id);
                SOKOL_VALIDATE(img != 0, _SG_VALIDATE_ABND_VS_IMG_EXISTS);
                if (img && img->slot.state == SG_RESOURCESTATE_VALID) {
                    SOKOL_VALIDATE(img->cmn.type == stage->images[i].image_type, _SG_VALIDATE_ABND_VS_IMG_TYPES);
                }
            }
            else {
                SOKOL_VALIDATE(i >= stage->num_images, _SG_VALIDATE_ABND_VS_IMGS);
            }
        }

        /* has expected fragment shader images */
        for (int i = 0; i < SG_MAX_SHADERSTAGE_IMAGES; i++) {
            _sg_shader_stage_t* stage = &pip->shader->cmn.stage[SG_SHADERSTAGE_FS];
            if (bindings->fs_images[i].id != SG_INVALID_ID) {
                SOKOL_VALIDATE(i < stage->num_images, _SG_VALIDATE_ABND_FS_IMGS);
                const _sg_image_t* img = _sg_lookup_image(&_sg.pools, bindings->fs_images[i].id);
                SOKOL_VALIDATE(img != 0, _SG_VALIDATE_ABND_FS_IMG_EXISTS);
                if (img && img->slot.state == SG_RESOURCESTATE_VALID) {
                    SOKOL_VALIDATE(img->cmn.type == stage->images[i].image_type, _SG_VALIDATE_ABND_FS_IMG_TYPES);
                }
            }
            else {
                SOKOL_VALIDATE(i >= stage->num_images, _SG_VALIDATE_ABND_FS_IMGS);
            }
        }
        return SOKOL_VALIDATE_END();
#endif
}

bool binocle_backend_validate_apply_uniforms(binocle_shader_stage stage_index, int ub_index, const binocle_range* data) {
#if !defined(BINOCLE_DEBUG)
  return true;
#else
  SOKOL_ASSERT((stage_index == SG_SHADERSTAGE_VS) || (stage_index == SG_SHADERSTAGE_FS));
        SOKOL_ASSERT((ub_index >= 0) && (ub_index < SG_MAX_SHADERSTAGE_UBS));
        SOKOL_VALIDATE_BEGIN();
        SOKOL_VALIDATE(_sg.cur_pipeline.id != SG_INVALID_ID, _SG_VALIDATE_AUB_NO_PIPELINE);
        const _sg_pipeline_t* pip = _sg_lookup_pipeline(&_sg.pools, _sg.cur_pipeline.id);
        SOKOL_ASSERT(pip && (pip->slot.id == _sg.cur_pipeline.id));
        SOKOL_ASSERT(pip->shader && (pip->shader->slot.id == pip->cmn.shader_id.id));

        /* check that there is a uniform block at 'stage' and 'ub_index' */
        const _sg_shader_stage_t* stage = &pip->shader->cmn.stage[stage_index];
        SOKOL_VALIDATE(ub_index < stage->num_uniform_blocks, _SG_VALIDATE_AUB_NO_UB_AT_SLOT);

        /* check that the provided data size doesn't exceed the uniform block size */
        SOKOL_VALIDATE(data->size <= stage->uniform_blocks[ub_index].size, _SG_VALIDATE_AUB_SIZE);

        return SOKOL_VALIDATE_END();
#endif
}

void _binocle_backend_apply_pipeline(binocle_pipeline_t* pip) {
#if defined(BINOCLE_GL)
  binocle_backend_gl_apply_pipeline(&backend.gl, pip);
#elif defined(BINOCLE_METAL)
  binocle_backend_mtl_apply_pipeline(&backend.mtl, pip);
#else
#error("INVALID BACKEND");
#endif
}

void _binocle_backend_apply_bindings(
  binocle_pipeline_t* pip,
  binocle_buffer_t** vbs, const int* vb_offsets, int num_vbs,
  binocle_buffer_t* ib, int ib_offset,
  binocle_image_t** vs_imgs, int num_vs_imgs,
  binocle_image_t** fs_imgs, int num_fs_imgs)
{
#if defined(BINOCLE_GL)
  binocle_backend_gl_apply_bindings(&backend.gl, pip, vbs, vb_offsets, num_vbs, ib, ib_offset, vs_imgs, num_vs_imgs, fs_imgs, num_fs_imgs);
#elif defined(BINOCLE_METAL)
  binocle_backend_mtl_apply_bindings(&backend.mtl, pip, vbs, vb_offsets, num_vbs, ib, ib_offset, vs_imgs, num_vs_imgs, fs_imgs, num_fs_imgs);
#else
#error("INVALID BACKEND");
#endif
}

void _binocle_backend_apply_uniforms(binocle_shader_stage stage_index, int ub_index, const binocle_range* data) {
#if defined(BINOCLE_GL)
  binocle_backend_gl_apply_uniforms(&backend.gl, stage_index, ub_index, data);
#elif defined(BINOCLE_METAL)
  binocle_backend_mtl_apply_uniforms(&backend.mtl, stage_index, ub_index, data);
#else
#error("INVALID BACKEND");
#endif
}

void _binocle_backend_draw(int base_element, int num_elements, int num_instances) {
#if defined(BINOCLE_GL)
  binocle_backend_gl_draw(&backend.gl, base_element, num_elements, num_instances);
#elif defined(BINOCLE_METAL)
  binocle_backend_mtl_draw(&backend.mtl, base_element, num_elements, num_instances);
#else
#error("INVALID BACKEND");
#endif
}

void binocle_backend_apply_pipeline(binocle_pipeline pip_id) {
  assert(backend.valid);
  backend.bindings_valid = false;
  if (!binocle_backend_validate_apply_pipeline(pip_id)) {
    backend.next_draw_valid = false;
//    BINOCLE_TRACE_NOARGS(err_draw_invalid);
    return;
  }
  if (!backend.pass_valid) {
//    BINOCLE_TRACE_NOARGS(err_pass_invalid);
    return;
  }
  backend.cur_pipeline = pip_id;
  binocle_pipeline_t* pip = binocle_backend_lookup_pipeline(&backend.pools, pip_id.id);
  assert(pip);
  backend.next_draw_valid = (BINOCLE_RESOURCESTATE_VALID == pip->slot.state);
  assert(pip->shader && (pip->shader->slot.id == pip->cmn.shader_id.id));
  _binocle_backend_apply_pipeline(pip);
//  BINOCLE_TRACE_ARGS(apply_pipeline, pip_id);
}

void binocle_backend_apply_bindings(const binocle_bindings* bindings) {
  assert(backend.valid);
  assert(bindings);
  assert((bindings->_start_canary == 0) && (bindings->_end_canary==0));
  if (!binocle_backend_validate_apply_bindings(bindings)) {
    backend.next_draw_valid = false;
//    BINOCLE_TRACE_NOARGS(err_draw_invalid);
    return;
  }
  backend.bindings_valid = true;

  binocle_pipeline_t* pip = binocle_backend_lookup_pipeline(&backend.pools, backend.cur_pipeline.id);
  assert(pip);

  binocle_buffer_t* vbs[BINOCLE_MAX_SHADERSTAGE_BUFFERS] = { 0 };
  int num_vbs = 0;
  for (int i = 0; i < BINOCLE_MAX_SHADERSTAGE_BUFFERS; i++, num_vbs++) {
    if (bindings->vertex_buffers[i].id) {
      vbs[i] = binocle_backend_lookup_buffer(&backend.pools, bindings->vertex_buffers[i].id);
      assert(vbs[i]);
      backend.next_draw_valid &= (BINOCLE_RESOURCESTATE_VALID == vbs[i]->slot.state);
      backend.next_draw_valid &= !vbs[i]->cmn.append_overflow;
    }
    else {
      break;
    }
  }

  binocle_buffer_t* ib = 0;
  if (bindings->index_buffer.id) {
    ib = binocle_backend_lookup_buffer(&backend.pools, bindings->index_buffer.id);
    assert(ib);
    backend.next_draw_valid &= (BINOCLE_RESOURCESTATE_VALID == ib->slot.state);
    backend.next_draw_valid &= !ib->cmn.append_overflow;
  }

  binocle_image_t* vs_imgs[BINOCLE_MAX_SHADERSTAGE_IMAGES] = { 0 };
  int num_vs_imgs = 0;
  for (int i = 0; i < BINOCLE_MAX_SHADERSTAGE_IMAGES; i++, num_vs_imgs++) {
    if (bindings->vs_images[i].id) {
      vs_imgs[i] = binocle_backend_lookup_image(&backend.pools, bindings->vs_images[i].id);
      assert(vs_imgs[i]);
      backend.next_draw_valid &= (BINOCLE_RESOURCESTATE_VALID == vs_imgs[i]->slot.state);
    }
    else {
      break;
    }
  }

  binocle_image_t* fs_imgs[BINOCLE_MAX_SHADERSTAGE_IMAGES] = { 0 };
  int num_fs_imgs = 0;
  for (int i = 0; i < BINOCLE_MAX_SHADERSTAGE_IMAGES; i++, num_fs_imgs++) {
    if (bindings->fs_images[i].id) {
      fs_imgs[i] = binocle_backend_lookup_image(&backend.pools, bindings->fs_images[i].id);
      assert(fs_imgs[i]);
      backend.next_draw_valid &= (BINOCLE_RESOURCESTATE_VALID == fs_imgs[i]->slot.state);
    }
    else {
      break;
    }
  }
  if (backend.next_draw_valid) {
    const int* vb_offsets = bindings->vertex_buffer_offsets;
    int ib_offset = bindings->index_buffer_offset;
    _binocle_backend_apply_bindings(pip, vbs, vb_offsets, num_vbs, ib, ib_offset, vs_imgs, num_vs_imgs, fs_imgs, num_fs_imgs);
//    BINOCLE_TRACE_ARGS(apply_bindings, bindings);
  }
  else {
//    BINOCLE_TRACE_NOARGS(err_draw_invalid);
  }
}

void binocle_backend_apply_uniforms(binocle_shader_stage stage, int ub_index, const binocle_range* data) {
  assert(backend.valid);
  assert((stage == BINOCLE_SHADERSTAGE_VS) || (stage == BINOCLE_SHADERSTAGE_FS));
  assert((ub_index >= 0) && (ub_index < BINOCLE_MAX_SHADERSTAGE_UBS));
  assert(data && data->ptr && (data->size > 0));
  if (!binocle_backend_validate_apply_uniforms(stage, ub_index, data)) {
    backend.next_draw_valid = false;
//    BINOCLE_TRACE_NOARGS(err_draw_invalid);
    return;
  }
  if (!backend.pass_valid) {
//    BINOCLE_TRACE_NOARGS(err_pass_invalid);
    return;
  }
  if (!backend.next_draw_valid) {
//    BINOCLE_TRACE_NOARGS(err_draw_invalid);
  }
  _binocle_backend_apply_uniforms(stage, ub_index, data);
//  BINOCLE_TRACE_ARGS(apply_uniforms, stage, ub_index, data);
}

void binocle_backend_draw(int base_element, int num_elements, int num_instances) {
  assert(backend.valid);
  assert(base_element >= 0);
  assert(num_elements >= 0);
  assert(num_instances >= 0);
#if defined(BINOCLE_DEBUG)
  if (!backend.bindings_valid) {
            binocle_log_warn("attempting to draw without resource bindings");
        }
#endif
  if (!backend.pass_valid) {
//    BINOCLE_TRACE_NOARGS(err_pass_invalid);
    return;
  }
  if (!backend.next_draw_valid) {
//    BINOCLE_TRACE_NOARGS(err_draw_invalid);
    return;
  }
  if (!backend.bindings_valid) {
//    BINOCLE_TRACE_NOARGS(err_bindings_invalid);
    return;
  }
  /* attempting to draw with zero elements or instances is not technically an
     error, but might be handled as an error in the backend API (e.g. on Metal)
  */
  if ((0 == num_elements) || (0 == num_instances)) {
//    BINOCLE_TRACE_NOARGS(err_draw_invalid);
    return;
  }
  _binocle_backend_draw(base_element, num_elements, num_instances);
//  BINOCLE_TRACE_ARGS(draw, base_element, num_elements, num_instances);
}

void binocle_backend_apply_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
#if defined(BINOCLE_GL)
  binocle_backend_gl_apply_viewport(x, y, w, h);
#elif defined(BINOCLE_METAL)
  #else
#error("no backend defined")
#endif
}

void binocle_backend_apply_blend_mode(struct binocle_blend blend_mode) {
#if defined(BINOCLE_GL)
  binocle_backend_gl_apply_blend_mode(blend_mode);
#elif defined(BINOCLE_METAL)
  #else
#error("no backend defined")
#endif
}

void binocle_backend_apply_shader(binocle_shader shader) {
#if defined(BINOCLE_GL)
  binocle_shader_t *sha = binocle_backend_lookup_shader(&backend.pools, shader.id);
  if (sha != NULL) {
    binocle_backend_gl_apply_shader(&backend.gl, sha);
  }
#elif defined(BINOCLE_METAL)
  #else
#error("no backend defined")
#endif
}

void binocle_backend_apply_texture(binocle_image texture) {
#if defined(BINOCLE_GL)
  binocle_image_t *img = binocle_backend_lookup_image(&backend.pools, texture.id);
  if (img != NULL) {
    binocle_backend_gl_apply_texture(img);
  }
#elif defined(BINOCLE_METAL)
  #else
#error("no backend defined")
#endif
}

void binocle_backend_apply_3d_texture(struct binocle_material *material) {
#if defined(BINOCLE_GL)
  binocle_image_t *albedo = binocle_backend_lookup_image(&backend.pools, material->albedo_texture.id);
  binocle_image_t *normal = binocle_backend_lookup_image(&backend.pools, material->normal_texture.id);
  if (albedo != NULL && normal != NULL) {
    binocle_backend_gl_apply_3d_texture(albedo, normal);
  }
#elif defined(BINOCLE_METAL)
  #else
#error("no backend defined")
#endif
}

void LEGACY_binocle_backend_draw(const struct binocle_vpct *vertices, size_t vertex_count, struct binocle_material material,
                             struct kmAABB2 viewport, struct kmMat4 *cameraTransformMatrix) {
#if defined(BINOCLE_GL)
  binocle_image_t *albedo = binocle_backend_lookup_image(&backend.pools, material.albedo_texture.id);
  binocle_shader_t *shader = binocle_backend_lookup_shader(&backend.pools, material.shader.id);
  LEGACY_binocle_backend_gl_draw(&backend.gl, vertices, vertex_count, material.blend_mode, shader, albedo, viewport, cameraTransformMatrix);
#elif defined(BINOCLE_METAL)
  #else
#error("no backend defined")
#endif
}

void binocle_backend_set_render_target(binocle_image rt) {
#if defined(BINOCLE_GL)
  if (rt.id == 0) {
    binocle_backend_gl_set_render_target(NULL);
  } else {
    binocle_image_t *render_target = binocle_backend_lookup_image(&backend.pools, rt.id);
    binocle_backend_gl_set_render_target(render_target);
  }
#elif defined(BINOCLE_METAL)
  #else
#error("no backend defined")
#endif
}

void binocle_backend_unset_render_target() {
#if defined(BINOCLE_GL)
  binocle_backend_gl_set_render_target(NULL);
#elif defined(BINOCLE_METAL)
  #else
#error("no backend defined")
#endif
}

void binocle_backend_clear(struct binocle_color color) {
#if defined(BINOCLE_GL)
  binocle_backend_gl_clear(color);
#elif defined(BINOCLE_METAL)
  binocle_backend_mtl_clear(&backend.mtl, color);
#else
#error("no backend defined")
#endif
}

void binocle_backend_set_uniform_float2(binocle_shader shader, const char *name, float value1, float value2) {
#if defined(BINOCLE_GL)
  binocle_shader_t *sha = binocle_backend_lookup_shader(&backend.pools, shader.id);
  if (sha != NULL) {
    binocle_backend_gl_set_uniform_float2(sha, name, value1, value2);
  }
#elif defined(BINOCLE_METAL)
  #else
#error("no backend defined")
#endif
}

void binocle_backend_set_uniform_mat4(binocle_shader shader, const char *name, struct kmMat4 mat) {
#if defined(BINOCLE_GL)
  binocle_shader_t *sha = binocle_backend_lookup_shader(&backend.pools, shader.id);
  if (sha != NULL) {
    binocle_backend_gl_set_uniform_mat4(sha, name, mat);
  }
#elif defined(BINOCLE_METAL)
  #else
#error("no backend defined")
#endif
}

void binocle_backend_draw_quad_to_screen(binocle_shader shader, binocle_image rt) {
#if defined(BINOCLE_GL)
  binocle_shader_t *sha = binocle_backend_lookup_shader(&backend.pools, shader.id);
  if (sha != NULL) {
    binocle_image_t *render_target = binocle_backend_lookup_image(&backend.pools, rt.id);
    binocle_backend_gl_draw_quad_to_screen(sha, render_target);
  }
#elif defined(BINOCLE_METAL)
  #else
#error("no backend defined")
#endif
}

binocle_image_info binocle_backend_query_image_info(binocle_image img_id) {
  binocle_image_info info;
  memset(&info, 0, sizeof(info));
  const binocle_image_t *img =
    binocle_backend_lookup_image(&backend.pools, img_id.id);
  if (img) {
    info.slot.state = img->slot.state;
    info.slot.res_id = img->slot.id;
    info.slot.ctx_id = img->slot.ctx_id;
    info.num_slots = img->cmn.num_slots;
    info.active_slot = img->cmn.active_slot;
    info.width = img->cmn.width;
    info.height = img->cmn.height;
  }
  return info;
}

/* return true if pixel format is a valid render target format */
bool binocle_backend_is_valid_rendertarget_color_format(binocle_pixel_format fmt) {
  const int fmt_index = (int) fmt;
  assert((fmt_index >= 0) && (fmt_index < BINOCLE_PIXELFORMAT_NUM));
  return backend.formats[fmt_index].render && !backend.formats[fmt_index].depth;
}

/* return true if pixel format is a valid depth format */
bool binocle_backend_is_valid_rendertarget_depth_format(binocle_pixel_format fmt) {
  const int fmt_index = (int) fmt;
  assert((fmt_index >= 0) && (fmt_index < BINOCLE_PIXELFORMAT_NUM));
  return backend.formats[fmt_index].render && backend.formats[fmt_index].depth;
}

/* return true if pixel format is a depth-stencil format */
bool binocle_backend_is_depth_stencil_format(binocle_pixel_format fmt) {
  return (BINOCLE_PIXELFORMAT_DEPTH_STENCIL == fmt);
}

bool binocle_backend_is_compressed_pixel_format(binocle_pixel_format fmt) {
  switch (fmt) {
  case BINOCLE_PIXELFORMAT_BC1_RGBA:
  case BINOCLE_PIXELFORMAT_BC2_RGBA:
  case BINOCLE_PIXELFORMAT_BC3_RGBA:
  case BINOCLE_PIXELFORMAT_BC4_R:
  case BINOCLE_PIXELFORMAT_BC4_RSN:
  case BINOCLE_PIXELFORMAT_BC5_RG:
  case BINOCLE_PIXELFORMAT_BC5_RGSN:
  case BINOCLE_PIXELFORMAT_BC6H_RGBF:
  case BINOCLE_PIXELFORMAT_BC6H_RGBUF:
  case BINOCLE_PIXELFORMAT_BC7_RGBA:
  case BINOCLE_PIXELFORMAT_PVRTC_RGB_2BPP:
  case BINOCLE_PIXELFORMAT_PVRTC_RGB_4BPP:
  case BINOCLE_PIXELFORMAT_PVRTC_RGBA_2BPP:
  case BINOCLE_PIXELFORMAT_PVRTC_RGBA_4BPP:
  case BINOCLE_PIXELFORMAT_ETC2_RGB8:
  case BINOCLE_PIXELFORMAT_ETC2_RGB8A1:
  case BINOCLE_PIXELFORMAT_ETC2_RGBA8:
  case BINOCLE_PIXELFORMAT_ETC2_RG11:
  case BINOCLE_PIXELFORMAT_ETC2_RG11SN:
    return true;
  default:
    return false;
  }
}

void binocle_backend_shader_common_init(binocle_shader_common_t* cmn, const binocle_shader_desc* desc) {
  for (int stage_index = 0; stage_index < BINOCLE_NUM_SHADER_STAGES; stage_index++) {
    const binocle_shader_stage_desc* stage_desc = (stage_index == BINOCLE_SHADERSTAGE_VS) ? &desc->vs : &desc->fs;
    binocle_shader_stage_t* stage = &cmn->stage[stage_index];
    assert(stage->num_uniform_blocks == 0);
    for (int ub_index = 0; ub_index < BINOCLE_MAX_SHADERSTAGE_UBS; ub_index++) {
      const binocle_shader_uniform_block_desc* ub_desc = &stage_desc->uniform_blocks[ub_index];
      if (0 == ub_desc->size) {
        break;
      }
      stage->uniform_blocks[ub_index].size = ub_desc->size;
      stage->num_uniform_blocks++;
    }
    assert(stage->num_images == 0);
    for (int img_index = 0; img_index < BINOCLE_MAX_SHADERSTAGE_IMAGES; img_index++) {
      const binocle_shader_image_desc* img_desc = &stage_desc->images[img_index];
      if (img_desc->type == BINOCLE_IMAGETYPE_DEFAULT) {
        break;
      }
      stage->images[img_index].type = img_desc->type;
      stage->images[img_index].sampler_type = img_desc->sampler_type;
      stage->num_images++;
    }
  }
}

void binocle_backend_pipeline_common_init(binocle_pipeline_common_t *cmn,
                                          const binocle_pipeline_desc *desc) {
  assert(desc->color_count < BINOCLE_MAX_COLOR_ATTACHMENTS);
  cmn->shader_id = desc->shader;
  cmn->index_type = desc->index_type;
  for (int i = 0; i < BINOCLE_MAX_SHADERSTAGE_BUFFERS; i++) {
    cmn->vertex_layout_valid[i] = false;
  }
  cmn->color_attachment_count = desc->color_count;
  for (int i = 0; i < cmn->color_attachment_count; i++) {
    cmn->color_formats[i] = desc->colors[i].pixel_format;
  }
  cmn->depth_format = desc->depth.pixel_format;
  cmn->sample_count = desc->sample_count;
  cmn->depth_bias = desc->depth.bias;
  cmn->depth_bias_slope_scale = desc->depth.bias_slope_scale;
  cmn->depth_bias_clamp = desc->depth.bias_clamp;
  cmn->blend_color = desc->blend_color;
}

void binocle_pass_common_init(binocle_pass_common_t *cmn,
                              const binocle_pass_desc *desc) {
  const binocle_pass_attachment_desc *att_desc;
  binocle_pass_attachment_common_t *att;
  for (int i = 0; i < BINOCLE_MAX_COLOR_ATTACHMENTS; i++) {
    att_desc = &desc->color_attachments[i];
    if (att_desc->image.id != BINOCLE_INVALID_ID) {
      cmn->num_color_atts++;
      att = &cmn->color_atts[i];
      att->image_id = att_desc->image;
      att->mip_level = att_desc->mip_level;
      att->slice = att_desc->slice;
    }
  }
  att_desc = &desc->depth_stencil_attachment;
  if (att_desc->image.id != BINOCLE_INVALID_ID) {
    att = &cmn->ds_att;
    att->image_id = att_desc->image;
    att->mip_level = att_desc->mip_level;
    att->slice = att_desc->slice;
  }
}

/* https://developer.apple.com/metal/Metal-Feature-Set-Tables.pdf */
void binocle_backend_mtl_init_caps(void) {
//#if defined(BINOCLE_MACOS)
//  backend.backend = BINOCLE_BACKEND_METAL_MACOS;
//#elif defined(BINOCLE_IOS)
//  #if defined(BINOCLE_IOS_SIMULATOR)
//            backend.backend = BINOCLE_BACKEND_METAL_SIMULATOR;
//        #else
//            backend.backend = BINOCLE_BACKEND_METAL_IOS;
//        #endif
//#endif
//  backend.features.instancing = true;
//  backend.features.origin_top_left = true;
//  backend.features.multiple_render_targets = true;
//  backend.features.msaa_render_targets = true;
//  backend.features.imagetype_3d = true;
//  backend.features.imagetype_array = true;
//#if defined(_SG_TARGET_MACOS)
//  backend.features.image_clamp_to_border = true;
//#else
//  backend.features.image_clamp_to_border = false;
//#endif
//  backend.features.mrt_independent_blend_state = true;
//  backend.features.mrt_independent_write_mask = true;
//
//#if defined(_SG_TARGET_MACOS)
//  backend.limits.max_image_size_2d = 16 * 1024;
//        backend.limits.max_image_size_cube = 16 * 1024;
//        backend.limits.max_image_size_3d = 2 * 1024;
//        backend.limits.max_image_size_array = 16 * 1024;
//        backend.limits.max_image_array_layers = 2 * 1024;
//#else
//  /* newer iOS devices support 16k textures */
//  backend.limits.max_image_size_2d = 8 * 1024;
//  backend.limits.max_image_size_cube = 8 * 1024;
//  backend.limits.max_image_size_3d = 2 * 1024;
//  backend.limits.max_image_size_array = 8 * 1024;
//  backend.limits.max_image_array_layers = 2 * 1024;
//#endif
//  backend.limits.max_vertex_attrs = SG_MAX_VERTEX_ATTRIBUTES;

  binocle_backend_pixelformat_all(&backend.formats[BINOCLE_PIXELFORMAT_R8]);
  binocle_backend_pixelformat_all(&backend.formats[BINOCLE_PIXELFORMAT_R8SN]);
  binocle_backend_pixelformat_srm(&backend.formats[BINOCLE_PIXELFORMAT_R8UI]);
  binocle_backend_pixelformat_srm(&backend.formats[BINOCLE_PIXELFORMAT_R8SI]);
#if defined(BINOCLE_MACOS)
  binocle_backend_pixelformat_all(&backend.formats[BINOCLE_PIXELFORMAT_R16]);
  binocle_backend_pixelformat_all(&backend.formats[BINOCLE_PIXELFORMAT_R16SN]);
#else
  binocle_backend_pixelformat_sfbr(&backend.formats[BINOCLE_PIXELFORMAT_R16]);
  binocle_backend_pixelformat_sfbr(&backend.formats[BINOCLE_PIXELFORMAT_R16SN]);
#endif
  binocle_backend_pixelformat_srm(&backend.formats[BINOCLE_PIXELFORMAT_R16UI]);
  binocle_backend_pixelformat_srm(&backend.formats[BINOCLE_PIXELFORMAT_R16SI]);
  binocle_backend_pixelformat_all(&backend.formats[BINOCLE_PIXELFORMAT_R16F]);
  binocle_backend_pixelformat_all(&backend.formats[BINOCLE_PIXELFORMAT_RG8]);
  binocle_backend_pixelformat_all(&backend.formats[BINOCLE_PIXELFORMAT_RG8SN]);
  binocle_backend_pixelformat_srm(&backend.formats[BINOCLE_PIXELFORMAT_RG8UI]);
  binocle_backend_pixelformat_srm(&backend.formats[BINOCLE_PIXELFORMAT_RG8SI]);
  binocle_backend_pixelformat_sr(&backend.formats[BINOCLE_PIXELFORMAT_R32UI]);
  binocle_backend_pixelformat_sr(&backend.formats[BINOCLE_PIXELFORMAT_R32SI]);
#if defined(BINOCLE_MACOS)
  binocle_backend_pixelformat_all(&backend.formats[BINOCLE_PIXELFORMAT_R32F]);
#else
  binocle_backend_pixelformat_sbr(&backend.formats[BINOCLE_PIXELFORMAT_R32F]);
#endif
#if defined(BINOCLE_MACOS)
  binocle_backend_pixelformat_all(&backend.formats[BINOCLE_PIXELFORMAT_RG16]);
  binocle_backend_pixelformat_all(&backend.formats[BINOCLE_PIXELFORMAT_RG16SN]);
#else
  binocle_backend_pixelformat_sfbr(&backend.formats[BINOCLE_PIXELFORMAT_RG16]);
  binocle_backend_pixelformat_sfbr(&backend.formats[BINOCLE_PIXELFORMAT_RG16SN]);
#endif
  binocle_backend_pixelformat_srm(&backend.formats[BINOCLE_PIXELFORMAT_RG16UI]);
  binocle_backend_pixelformat_srm(&backend.formats[BINOCLE_PIXELFORMAT_RG16SI]);
  binocle_backend_pixelformat_all(&backend.formats[BINOCLE_PIXELFORMAT_RG16F]);
  binocle_backend_pixelformat_all(&backend.formats[BINOCLE_PIXELFORMAT_RGBA8]);
  binocle_backend_pixelformat_all(&backend.formats[BINOCLE_PIXELFORMAT_RGBA8SN]);
  binocle_backend_pixelformat_srm(&backend.formats[BINOCLE_PIXELFORMAT_RGBA8UI]);
  binocle_backend_pixelformat_srm(&backend.formats[BINOCLE_PIXELFORMAT_RGBA8SI]);
  binocle_backend_pixelformat_all(&backend.formats[BINOCLE_PIXELFORMAT_BGRA8]);
  binocle_backend_pixelformat_all(&backend.formats[BINOCLE_PIXELFORMAT_RGB10A2]);
  binocle_backend_pixelformat_all(&backend.formats[BINOCLE_PIXELFORMAT_RG11B10F]);
#if defined(BINOCLE_MACOS)
  binocle_backend_pixelformat_srm(&backend.formats[BINOCLE_PIXELFORMAT_RG32UI]);
  binocle_backend_pixelformat_srm(&backend.formats[BINOCLE_PIXELFORMAT_RG32SI]);
#else
  binocle_backend_pixelformat_sr(&backend.formats[BINOCLE_PIXELFORMAT_RG32UI]);
  binocle_backend_pixelformat_sr(&backend.formats[BINOCLE_PIXELFORMAT_RG32SI]);
#endif
#if defined(BINOCLE_MACOS)
  binocle_backend_pixelformat_all(&backend.formats[BINOCLE_PIXELFORMAT_RG32F]);
#else
  binocle_backend_pixelformat_sbr(&backend.formats[BINOCLE_PIXELFORMAT_RG32F]);
#endif
#if defined(BINOCLE_MACOS)
  binocle_backend_pixelformat_all(&backend.formats[BINOCLE_PIXELFORMAT_RGBA16]);
  binocle_backend_pixelformat_all(&backend.formats[BINOCLE_PIXELFORMAT_RGBA16SN]);
#else
  binocle_backend_pixelformat_sfbr(&backend.formats[BINOCLE_PIXELFORMAT_RGBA16]);
  binocle_backend_pixelformat_sfbr(&backend.formats[BINOCLE_PIXELFORMAT_RGBA16SN]);
#endif
  binocle_backend_pixelformat_srm(&backend.formats[BINOCLE_PIXELFORMAT_RGBA16UI]);
  binocle_backend_pixelformat_srm(&backend.formats[BINOCLE_PIXELFORMAT_RGBA16SI]);
  binocle_backend_pixelformat_all(&backend.formats[BINOCLE_PIXELFORMAT_RGBA16F]);
#if defined(BINOCLE_MACOS)
  binocle_backend_pixelformat_srm(&backend.formats[BINOCLE_PIXELFORMAT_RGBA32UI]);
  binocle_backend_pixelformat_srm(&backend.formats[BINOCLE_PIXELFORMAT_RGBA32SI]);
  binocle_backend_pixelformat_all(&backend.formats[BINOCLE_PIXELFORMAT_RGBA32F]);
#else
  binocle_backend_pixelformat_sr(&backend.formats[BINOCLE_PIXELFORMAT_RGBA32UI]);
  binocle_backend_pixelformat_sr(&backend.formats[BINOCLE_PIXELFORMAT_RGBA32SI]);
  binocle_backend_pixelformat_sr(&backend.formats[BINOCLE_PIXELFORMAT_RGBA32F]);
#endif
  binocle_backend_pixelformat_srmd(&backend.formats[BINOCLE_PIXELFORMAT_DEPTH]);
  binocle_backend_pixelformat_srmd(&backend.formats[BINOCLE_PIXELFORMAT_DEPTH_STENCIL]);
#if defined(BINOCLE_MACOS)
  binocle_backend_pixelformat_sf(&backend.formats[BINOCLE_PIXELFORMAT_BC1_RGBA]);
  binocle_backend_pixelformat_sf(&backend.formats[BINOCLE_PIXELFORMAT_BC2_RGBA]);
  binocle_backend_pixelformat_sf(&backend.formats[BINOCLE_PIXELFORMAT_BC3_RGBA]);
  binocle_backend_pixelformat_sf(&backend.formats[BINOCLE_PIXELFORMAT_BC4_R]);
  binocle_backend_pixelformat_sf(&backend.formats[BINOCLE_PIXELFORMAT_BC4_RSN]);
  binocle_backend_pixelformat_sf(&backend.formats[BINOCLE_PIXELFORMAT_BC5_RG]);
  binocle_backend_pixelformat_sf(&backend.formats[BINOCLE_PIXELFORMAT_BC5_RGSN]);
  binocle_backend_pixelformat_sf(&backend.formats[BINOCLE_PIXELFORMAT_BC6H_RGBF]);
  binocle_backend_pixelformat_sf(&backend.formats[BINOCLE_PIXELFORMAT_BC6H_RGBUF]);
  binocle_backend_pixelformat_sf(&backend.formats[BINOCLE_PIXELFORMAT_BC7_RGBA]);
#else
  binocle_backend_pixelformat_sf(&backend.formats[BINOCLE_PIXELFORMAT_PVRTC_RGB_2BPP]);
  binocle_backend_pixelformat_sf(&backend.formats[BINOCLE_PIXELFORMAT_PVRTC_RGB_4BPP]);
  binocle_backend_pixelformat_sf(&backend.formats[BINOCLE_PIXELFORMAT_PVRTC_RGBA_2BPP]);
  binocle_backend_pixelformat_sf(&backend.formats[BINOCLE_PIXELFORMAT_PVRTC_RGBA_4BPP]);
  binocle_backend_pixelformat_sf(&backend.formats[BINOCLE_PIXELFORMAT_ETC2_RGB8]);
  binocle_backend_pixelformat_sf(&backend.formats[BINOCLE_PIXELFORMAT_ETC2_RGB8A1]);
  binocle_backend_pixelformat_sf(&backend.formats[BINOCLE_PIXELFORMAT_ETC2_RGBA8]);
  binocle_backend_pixelformat_sf(&backend.formats[BINOCLE_PIXELFORMAT_ETC2_RG11]);
  binocle_backend_pixelformat_sf(&backend.formats[BINOCLE_PIXELFORMAT_ETC2_RG11SN]);
#endif
}

void binocle_backend_begin_default_pass(const binocle_pass_action* pass_action, int width, int height) {
  assert(backend.valid);
  assert(pass_action);
  assert((pass_action->_start_canary == 0) && (pass_action->_end_canary == 0));
  binocle_pass_action pa;
  binocle_backend_resolve_default_pass_action(pass_action, &pa);
  backend.cur_pass.id = BINOCLE_INVALID_ID;
  backend.pass_valid = true;
  _binocle_backend_begin_pass(0, &pa, width, height);
//  BINOCLE_TRACE_ARGS(begin_default_pass, pass_action, width, height);
}

static inline void _binocle_backend_commit(void) {
#if defined(BINOCLE_GL)
  binocle_backend_gl_commit(&backend.gl);
#elif defined(BINOCLE_METAL)
  binocle_backend_mtl_commit(&backend.mtl);
#else
#error("INVALID BACKEND");
#endif
}

void binocle_backend_commit(void) {
  assert(backend.valid);
  _binocle_backend_commit();
//  BINOCLE_TRACE_NOARGS(commit);
  backend.frame_index++;
}

void binocle_backend_buffer_common_init(binocle_buffer_common_t* cmn, const binocle_buffer_desc* desc) {
  cmn->size = (int)desc->size;
  cmn->append_pos = 0;
  cmn->append_overflow = false;
  cmn->type = desc->type;
  cmn->usage = desc->usage;
  cmn->update_frame_index = 0;
  cmn->append_frame_index = 0;
  cmn->num_slots = (cmn->usage == BINOCLE_USAGE_IMMUTABLE) ? 1 : BINOCLE_NUM_INFLIGHT_FRAMES;
  cmn->active_slot = 0;
}

static inline void _binocle_backend_update_buffer(binocle_buffer_t* buf, const binocle_range* data) {
#if defined(BINOCLE_GL)
  binocle_backend_gl_update_buffer(&backend, buf, data);
#elif defined(BINOCLE_METAL)
  binocle_backend_mtl_update_buffer(&backend.mtl, buf, data);
#else
#error("INVALID BACKEND");
#endif
}

static inline int _binocle_backend_append_buffer(binocle_buffer_t* buf, const binocle_range* data, bool new_frame) {
#if defined(BINOCLE_GL)
  binocle_backend_gl_append_buffer(&backend, buf, data, new_frame);
#elif defined(BINOCLE_METAL)
  binocle_backend_mtl_append_buffer(&backend.mtl, buf, data, new_frame);
#else
#error("INVALID BACKEND");
#endif
}

bool binocle_backend_validate_update_buffer(const binocle_buffer_t* buf, const binocle_range* data) {
#if !defined(BINOCLE_DEBUG)
  (void)(buf);
  (void)(data);
  return true;
#else
  SOKOL_ASSERT(buf && data && data->ptr);
        SOKOL_VALIDATE_BEGIN();
        SOKOL_VALIDATE(buf->cmn.usage != SG_USAGE_IMMUTABLE, _SG_VALIDATE_UPDATEBUF_USAGE);
        SOKOL_VALIDATE(buf->cmn.size >= (int)data->size, _SG_VALIDATE_UPDATEBUF_SIZE);
        SOKOL_VALIDATE(buf->cmn.update_frame_index != _sg.frame_index, _SG_VALIDATE_UPDATEBUF_ONCE);
        SOKOL_VALIDATE(buf->cmn.append_frame_index != _sg.frame_index, _SG_VALIDATE_UPDATEBUF_APPEND);
        return SOKOL_VALIDATE_END();
#endif
}

bool binocle_backend_validate_append_buffer(const binocle_buffer_t* buf, const binocle_range* data) {
#if !defined(SOKOL_DEBUG)
  (void)(buf);
  (void)(data);
  return true;
#else
  assert(buf && data && data->ptr);
        SOKOL_VALIDATE_BEGIN();
        SOKOL_VALIDATE(buf->cmn.usage != SG_USAGE_IMMUTABLE, _SG_VALIDATE_APPENDBUF_USAGE);
        SOKOL_VALIDATE(buf->cmn.size >= (buf->cmn.append_pos + (int)data->size), _SG_VALIDATE_APPENDBUF_SIZE);
        SOKOL_VALIDATE(buf->cmn.update_frame_index != _sg.frame_index, _SG_VALIDATE_APPENDBUF_UPDATE);
        return SOKOL_VALIDATE_END();
#endif
}


void binocle_backend_update_buffer(binocle_buffer buf_id, const binocle_range* data) {
  assert(backend.valid);
  assert(data && data->ptr && (data->size > 0));
  binocle_buffer_t* buf = binocle_backend_lookup_buffer(&backend.pools, buf_id.id);
  if ((data->size > 0) && buf && (buf->slot.state == BINOCLE_RESOURCESTATE_VALID)) {
    if (binocle_backend_validate_update_buffer(buf, data)) {
      assert(data->size <= (size_t)buf->cmn.size);
      /* only one update allowed per buffer and frame */
      assert(buf->cmn.update_frame_index != backend.frame_index);
      /* update and append on same buffer in same frame not allowed */
      assert(buf->cmn.append_frame_index != backend.frame_index);
      _binocle_backend_update_buffer(buf, data);
      buf->cmn.update_frame_index = backend.frame_index;
    }
  }
//  _SG_TRACE_ARGS(update_buffer, buf_id, data);
}

int binocle_backend_append_buffer(binocle_buffer buf_id, const binocle_range* data) {
  assert(backend.valid);
  assert(data && data->ptr);
  binocle_buffer_t* buf = binocle_backend_lookup_buffer(&backend.pools, buf_id.id);
  int result;
  if (buf) {
    /* rewind append cursor in a new frame */
    if (buf->cmn.append_frame_index != backend.frame_index) {
      buf->cmn.append_pos = 0;
      buf->cmn.append_overflow = false;
    }
    if ((buf->cmn.append_pos + BINOCLE_ROUNDUP((int)data->size, 4)) > buf->cmn.size) {
      buf->cmn.append_overflow = true;
    }
    const int start_pos = buf->cmn.append_pos;
    if (buf->slot.state == BINOCLE_RESOURCESTATE_VALID) {
      if (binocle_backend_validate_append_buffer(buf, data)) {
        if (!buf->cmn.append_overflow && (data->size > 0)) {
          /* update and append on same buffer in same frame not allowed */
          assert(buf->cmn.update_frame_index != backend.frame_index);
          int copied_num_bytes = _binocle_backend_append_buffer(buf, data, buf->cmn.append_frame_index != backend.frame_index);
          buf->cmn.append_pos += copied_num_bytes;
          buf->cmn.append_frame_index = backend.frame_index;
        }
      }
    }
    result = start_pos;
  }
  else {
    /* FIXME: should we return -1 here? */
    result = 0;
  }
//  _SG_TRACE_ARGS(append_buffer, buf_id, data, result);
  return result;
}
