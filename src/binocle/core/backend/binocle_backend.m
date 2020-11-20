//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_backend.h"
#include "../binocle_pool.h"

#if defined(BINOCLE_GL)
#include "binocle_backend_gl.h"
#elif defined(BINOCLE_METAL)
#include "binocle_backend_metal.h"
#endif

typedef struct binocle_pools_t {
  binocle_pool_t render_target_pool;
  binocle_render_target_t* render_targets;
} binocle_pools_t;

typedef struct binocle_backend_t {
  binocle_pools_t pools;
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
  size_t buffer_pool_byte_size = sizeof(binocle_render_target_t) * pools->render_target_pool.size;
  pools->render_targets = (binocle_render_target_t*) malloc(buffer_pool_byte_size);
  assert(pools->render_targets);
  memset(pools->render_targets, 0, buffer_pool_byte_size);
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

void binocle_backend_reset_render_target(binocle_render_target_t* rt) {
  assert(rt);
  memset(rt, 0, sizeof(binocle_render_target_t));
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

void binocle_backend_init() {
#if defined(BINOCLE_GL)
  binocle_backend_setup_pools(&backend.pools);
  binocle_backend_gl_init(&backend.gl);
#elif defined(BINOCLE_METAL)
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

void binocle_backend_apply_texture(binocle_texture texture) {
#if defined(BINOCLE_GL)
  binocle_backend_gl_apply_texture(texture);
#elif defined(BINOCLE_METAL)
  #else
#error("no backend defined")
#endif
}

void binocle_backend_apply_3d_texture(struct binocle_material *material) {
#if defined(BINOCLE_GL)
  binocle_backend_gl_apply_3d_texture(material);
#elif defined(BINOCLE_METAL)
  #else
#error("no backend defined")
#endif
}

void binocle_backend_draw(const struct binocle_vpct *vertices, size_t vertex_count, struct binocle_material material,
                             struct kmAABB2 viewport, struct kmMat4 *cameraTransformMatrix) {
#if defined(BINOCLE_GL)
  binocle_backend_gl_draw(&backend.gl, vertices, vertex_count, material, viewport, cameraTransformMatrix);
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