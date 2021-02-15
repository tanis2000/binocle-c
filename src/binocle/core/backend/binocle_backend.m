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
  binocle_pool_t image_pool;
  binocle_image_t* images;

  binocle_pool_t shader_pool;
  binocle_shader_t* shaders;
} binocle_pools_t;

typedef struct binocle_backend_t {
  bool valid;
  binocle_backend_desc desc;       /* original desc with default values patched in */
  uint32_t frame_index;
  binocle_pools_t pools;
  binocle_pixelformat_info formats[BINOCLE_PIXEL_FORMAT_NUM];
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

void binocle_backend_setup_pools(binocle_pools_t *pools, const binocle_backend_desc* desc) {
  assert(pools);
  assert(desc);
  /* note: the pools here will have an additional item, since slot 0 is reserved */
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
}

void binocle_backend_discard_pools(binocle_pools_t *p) {
  assert(p);

  free(p->images);
  p->images = 0;
  binocle_pool_discard(&p->image_pool);

  free(p->shaders);
  p->shaders = 0;
  binocle_pool_discard(&p->shader_pool);
}


/* returns pointer to resource by id without matching id check */
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

/* returns pointer to resource with matching id check, may return 0 */
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


void binocle_backend_reset_image(binocle_image_t* img) {
  assert(img);
  memset(img, 0, sizeof(binocle_image_t));
}

void binocle_backend_reset_shader(binocle_shader_t* sha) {
  assert(sha);
  memset(sha, 0, sizeof(binocle_shader_t));
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
    def.pixel_format = BINOCLE_DEF(def.pixel_format, BINOCLE_PIXEL_FORMAT_RGBA8);
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
  backend.formats[BINOCLE_PIXEL_FORMAT_RGB].render = true;
  backend.formats[BINOCLE_PIXEL_FORMAT_RGBA].render = true;
  backend.formats[BINOCLE_PIXEL_FORMAT_RGBA8].render = true;
  backend.formats[BINOCLE_PIXEL_FORMAT_DEPTH].render = true;
  backend.formats[BINOCLE_PIXEL_FORMAT_DEPTH].depth = true;
  backend.formats[BINOCLE_PIXEL_FORMAT_DEPTH_STENCIL].render = true;
  backend.formats[BINOCLE_PIXEL_FORMAT_DEPTH_STENCIL].depth = true;
  binocle_backend_gl_init(&backend.gl);
#elif defined(BINOCLE_METAL)
  binocle_backend_mtl_init(&backend.mtl, desc);
#else
#error("no backend defined")
#endif
}

void binocle_backend_setup(const binocle_backend_desc* desc) {
  assert(desc);
  backend = (binocle_backend_t){ 0 };
  backend.desc = *desc;

#if defined(BINOCLE_METAL)
  backend.desc.context.color_format = BINOCLE_DEF(backend.desc.context.color_format, BINOCLE_PIXEL_FORMAT_BGRA8);
#else
  backend.desc.context.color_format = BINOCLE_DEF(backend.desc.context.color_format, BINOCLE_PIXEL_FORMAT_RGBA8);
#endif
  backend.desc.context.depth_format = BINOCLE_DEF(backend.desc.context.depth_format, BINOCLE_PIXEL_FORMAT_DEPTH_STENCIL);
  backend.desc.context.sample_count = BINOCLE_DEF(backend.desc.context.sample_count, 1);
  backend.desc.buffer_pool_size = BINOCLE_DEF(backend.desc.buffer_pool_size, BINOCLE_DEFAULT_BUFFER_POOL_SIZE);
  backend.desc.image_pool_size = BINOCLE_DEF(backend.desc.image_pool_size, BINOCLE_DEFAULT_IMAGE_POOL_SIZE);
  backend.desc.shader_pool_size = BINOCLE_DEF(backend.desc.shader_pool_size, BINOCLE_DEFAULT_SHADER_POOL_SIZE);
  //backend.desc.pipeline_pool_size = BINOCLE_DEF(backend.desc.pipeline_pool_size, BINOCLE_DEFAULT_PIPELINE_POOL_SIZE);
  //backend.desc.pass_pool_size = BINOCLE_DEF(backend.desc.pass_pool_size, BINOCLE_DEFAULT_PASS_POOL_SIZE);
  //backend.desc.context_pool_size = BINOCLE_DEF(backend.desc.context_pool_size, BINOCLE_DEFAULT_CONTEXT_POOL_SIZE);
  backend.desc.uniform_buffer_size = BINOCLE_DEF(backend.desc.uniform_buffer_size, BINOCLE_DEFAULT_UB_SIZE);
  //backend.desc.staging_buffer_size = BINOCLE_DEF(backend.desc.staging_buffer_size, BINOCLE_DEFAULT_STAGING_SIZE);
  backend.desc.sampler_cache_size = BINOCLE_DEF(backend.desc.sampler_cache_size, BINOCLE_DEFAULT_SAMPLER_CACHE_CAPACITY);

  binocle_backend_setup_pools(&backend.pools, &backend.desc);
  backend.frame_index = 1;
  binocle_backend_setup_backend(&backend.desc);
  backend.valid = true;
  // Not yet using this as we work with just one context for the time being
  //binocle_backend_setup_context();
}

void binocle_backend_apply_default_state() {
#if defined(BINOCLE_GL)
  binocle_backend_gl_apply_default_state();
#elif defined(BINOCLE_METAL)
  #else
#error("no backend defined")
#endif
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

void binocle_backend_draw(const struct binocle_vpct *vertices, size_t vertex_count, struct binocle_material material,
                             struct kmAABB2 viewport, struct kmMat4 *cameraTransformMatrix) {
#if defined(BINOCLE_GL)
  binocle_image_t *albedo = binocle_backend_lookup_image(&backend.pools, material.albedo_texture.id);
  binocle_shader_t *shader = binocle_backend_lookup_shader(&backend.pools, material.shader.id);
  binocle_backend_gl_draw(&backend.gl, vertices, vertex_count, material.blend_mode, shader, albedo, viewport, cameraTransformMatrix);
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

/* return true if pixel format is a valid depth format */
bool binocle_backend_is_valid_rendertarget_depth_format(binocle_pixel_format fmt) {
  const int fmt_index = (int) fmt;
  assert((fmt_index >= 0) && (fmt_index < BINOCLE_PIXEL_FORMAT_NUM));
  return backend.formats[fmt_index].render && backend.formats[fmt_index].depth;
}

bool binocle_backend_is_compressed_pixel_format(binocle_pixel_format fmt) {
  return false;
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
