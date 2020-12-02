//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_backend.h"
#include "../binocle_pool.h"
#include "../binocle_log.h"
#include "binocle_material.h"

#if defined(BINOCLE_GL)
#include "binocle_backend_gl.h"
#elif defined(BINOCLE_METAL)
#include "binocle_backend_metal.h"
#endif

typedef struct binocle_pools_t {
  binocle_pool_t render_target_pool;
  binocle_render_target_t* render_targets;
  binocle_pool_t image_pool;
  binocle_image_t* images;
} binocle_pools_t;

typedef struct binocle_backend_t {
  binocle_backend_desc desc;       /* original desc with default values patched in */
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

void binocle_backend_setup_pools(binocle_pools_t *pools) {
  assert(pools);
  /* note: the pools here will have an additional item, since slot 0 is reserved */
  binocle_pool_init(&pools->render_target_pool, 128);
  size_t render_target_pool_byte_size = sizeof(binocle_render_target_t) * pools->render_target_pool.size;
  pools->render_targets = (binocle_render_target_t*) malloc(render_target_pool_byte_size);
  assert(pools->render_targets);
  memset(pools->render_targets, 0, render_target_pool_byte_size);

  binocle_pool_init(&pools->image_pool, 128);
  size_t image_pool_byte_size = sizeof(binocle_image_t) * pools->image_pool.size;
  pools->images = (binocle_image_t*) malloc(image_pool_byte_size);
  assert(pools->images);
  memset(pools->images, 0, image_pool_byte_size);
}

void binocle_backend_discard_pools(binocle_pools_t *p) {
  assert(p);
  free(p->render_targets);
  p->render_targets = 0;
  binocle_pool_discard(&p->render_target_pool);
}


/* returns pointer to resource by id without matching id check */
binocle_render_target_t* binocle_render_target_at(const binocle_pools_t* p, uint32_t rt_id) {
  assert(p && (BINOCLE_INVALID_ID != rt_id));
  int slot_index = binocle_pool_slot_index(rt_id);
  assert((slot_index > BINOCLE_POOL_INVALID_SLOT_INDEX) && (slot_index < p->render_target_pool.size));
  return &p->render_targets[slot_index];
}

binocle_image_t* binocle_image_at(const binocle_pools_t* p, uint32_t rt_id) {
  assert(p && (BINOCLE_INVALID_ID != rt_id));
  int slot_index = binocle_pool_slot_index(rt_id);
  assert((slot_index > BINOCLE_POOL_INVALID_SLOT_INDEX) && (slot_index < p->image_pool.size));
  return &p->images[slot_index];
}

/* returns pointer to resource with matching id check, may return 0 */
binocle_render_target_t* binocle_backend_lookup_render_target(const binocle_pools_t* p, uint32_t rt_id) {
  if (BINOCLE_INVALID_ID != rt_id) {
    binocle_render_target_t* rt = binocle_render_target_at(p, rt_id);
    if (rt->slot.id == rt_id) {
      return rt;
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


void binocle_backend_reset_render_target(binocle_render_target_t* rt) {
  assert(rt);
  memset(rt, 0, sizeof(binocle_render_target_t));
}

void binocle_backend_reset_image(binocle_image_t* img) {
  assert(img);
  memset(img, 0, sizeof(binocle_image_t));
}

binocle_resource_state binocle_backend_create_render_target_t(binocle_render_target_t *rt, binocle_render_target_desc *desc) {
#if defined(BINOCLE_GL)
  return binocle_backend_gl_create_render_target(rt, desc->width, desc->height, desc->use_depth, desc->format);
#elif defined(BINOCLE_METAL)
  #else
#error("no backend defined")
#endif
}

void binocle_backend_destroy_render_target_t(binocle_render_target_t* rt) {
#if defined(BINOCLE_GL)
  binocle_backend_gl_destroy_render_target(rt);
#elif defined(BINOCLE_METAL)
  #else
#error("no backend defined")
#endif
}

void binocle_backend_destroy_all_resources(binocle_pools_t* p, uint32_t ctx_id) {
  /*  this is a bit dumb since it loops over all pool slots to
      find the occupied slots, on the other hand it is only ever
      executed at shutdown
      NOTE: ONLY EXECUTE THIS AT SHUTDOWN
            ...because the free queues will not be reset
            and the resource slots not be cleared!
  */
  for (int i = 1; i < p->render_target_pool.size; i++) {
    if (p->render_targets[i].slot.ctx_id == ctx_id) {
      binocle_resource_state state = p->render_targets[i].slot.state;
      if ((state == BINOCLE_RESOURCESTATE_VALID) || (state == BINOCLE_RESOURCESTATE_FAILED)) {
        binocle_backend_destroy_render_target_t(&p->render_targets[i]);
      }
    }
  }
}

binocle_render_target binocle_backend_alloc_render_target(void) {
  binocle_render_target res;
  int slot_index = binocle_pool_alloc_index(&backend.pools.render_target_pool);
  if (BINOCLE_POOL_INVALID_SLOT_INDEX != slot_index) {
    res.id = binocle_pool_slot_alloc(&backend.pools.render_target_pool, &backend.pools.render_targets[slot_index].slot, slot_index);
  }
  else {
    /* pool is exhausted */
    res.id = BINOCLE_INVALID_ID;
  }
  return res;
}

void binocle_backend_init_render_target(binocle_render_target rt_id, binocle_render_target_desc *desc) {
  assert(rt_id.id != BINOCLE_INVALID_ID);
  binocle_render_target_t* img = binocle_backend_lookup_render_target(&backend.pools, rt_id.id);
  assert(img && img->slot.state == BINOCLE_RESOURCESTATE_ALLOC);
  img->slot.state = binocle_backend_create_render_target_t(img, desc);
  assert((img->slot.state == BINOCLE_RESOURCESTATE_VALID)||(img->slot.state == BINOCLE_RESOURCESTATE_FAILED));
}

binocle_image_desc binocle_backend_image_desc_defaults(const binocle_image_desc* desc) {
  binocle_image_desc def = *desc;
  def.type = BINOCLE_DEF(def.type, BINOCLE_IMAGETYPE_2D);
  def.depth = BINOCLE_DEF(def.depth, 1);
  def.num_mipmaps = BINOCLE_DEF(def.num_mipmaps, 1);
  def.usage = BINOCLE_DEF(def.usage, BINOCLE_USAGE_IMMUTABLE);
  if (desc->render_target) {
    def.pixel_format = BINOCLE_DEF(def.pixel_format, backend.desc.ctx.color_format);
    def.sample_count = BINOCLE_DEF(def.sample_count, backend.desc.ctx.sample_count);
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

void binocle_backend_init(binocle_backend_desc *desc) {
  binocle_backend_setup_pools(&backend.pools);
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

void binocle_backend_apply_shader(struct binocle_shader *shader) {
#if defined(BINOCLE_GL)
  binocle_backend_gl_apply_shader(&backend.gl, shader);
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
  binocle_backend_gl_draw(&backend.gl, vertices, vertex_count, material.blend_mode, material.shader, albedo, viewport, cameraTransformMatrix);
#elif defined(BINOCLE_METAL)
  #else
#error("no backend defined")
#endif
}

binocle_render_target binocle_backend_create_render_target(binocle_render_target_desc *desc) {
  assert(desc);
  binocle_render_target rt_id = binocle_backend_alloc_render_target();
  if (rt_id.id != BINOCLE_INVALID_ID) {
    binocle_backend_init_render_target(rt_id, desc);
  }
  else {
    SDL_Log("image pool exhausted!");
  }
  return rt_id;
}

void binocle_backend_destroy_render_target(binocle_render_target rt) {
  binocle_render_target_t* img = binocle_backend_lookup_render_target(&backend.pools, rt.id);
  if (img) {
    binocle_backend_destroy_render_target_t(img);
    binocle_backend_reset_render_target(img);
    binocle_pool_free_index(&backend.pools.render_target_pool, binocle_pool_slot_index(rt.id));
  }
}

void binocle_backend_set_render_target(binocle_render_target *rt) {
#if defined(BINOCLE_GL)
  // TODO this is ugly
  if (rt == NULL) {
    binocle_backend_gl_set_render_target(NULL);
  } else {
    binocle_render_target_t *render_target = binocle_backend_lookup_render_target(&backend.pools, rt->id);
    binocle_backend_gl_set_render_target(render_target);
  }
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

void binocle_backend_set_uniform_float2(struct binocle_shader *shader, const char *name, float value1, float value2) {
#if defined(BINOCLE_GL)
  binocle_backend_gl_set_uniform_float2(shader, name, value1, value2);
#elif defined(BINOCLE_METAL)
  #else
#error("no backend defined")
#endif
}

void binocle_backend_set_uniform_mat4(struct binocle_shader *shader, const char *name, struct kmMat4 mat) {
#if defined(BINOCLE_GL)
  binocle_backend_gl_set_uniform_mat4(shader, name, mat);
#elif defined(BINOCLE_METAL)
  #else
#error("no backend defined")
#endif
}

void binocle_backend_draw_quad_to_screen(struct binocle_shader *shader, binocle_render_target *rt) {
#if defined(BINOCLE_GL)
  // TODO this is ugly
  if (rt == NULL) {
    binocle_backend_gl_set_render_target(NULL);
  } else {
    binocle_render_target_t *render_target = binocle_backend_lookup_render_target(&backend.pools, rt->id);
    binocle_backend_gl_draw_quad_to_screen(shader, render_target);
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
