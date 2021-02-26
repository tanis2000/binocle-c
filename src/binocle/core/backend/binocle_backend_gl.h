//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_BACKEND_GL_H
#define BINOCLE_BACKEND_GL_H

#include <stdbool.h>
#include "binocle_backend_types.h"
#include "../binocle_sdl.h"
#include "../binocle_pool.h"

struct binocle_blend;
struct binocle_color;
struct binocle_material;
struct binocle_shader;
struct binocle_texture;
struct binocle_vpct;

struct kmAABB2;
struct kmMat4;

/**
 * \brief a render target
 */
typedef struct binocle_gl_render_target {
  binocle_slot_t slot;
  GLuint frame_buffer;
  GLuint render_buffer; // used for depth
  GLuint texture;
} binocle_gl_render_target;
typedef binocle_gl_render_target binocle_render_target_t;

typedef struct binocle_gl_image {
  binocle_slot_t slot;
  binocle_image_common_t cmn;
  struct {
    GLenum target;
    GLuint depth_render_buffer;
    GLuint msaa_render_buffer;
    GLuint tex[BINOCLE_NUM_INFLIGHT_FRAMES];
    bool ext_textures;  /* if true, external textures were injected with sg_image_desc.gl_textures */
    GLuint frame_buffer; // TODO: remove this if we implement render passes as it would belong there
  } gl;
} binocle_gl_image;
typedef binocle_gl_image binocle_image_t;

typedef struct binocle_gl_uniform_t {
  GLint gl_loc;
  binocle_uniform_type type;
  uint8_t count;
  uint16_t offset;
} binocle_gl_uniform_t;

typedef struct binocle_gl_uniform_block_t {
  int num_uniforms;
  binocle_gl_uniform_t uniforms[BINOCLE_MAX_UB_MEMBERS];
} binocle_gl_uniform_block_t;

typedef struct binocle_gl_shader_image_t {
  int gl_tex_slot;
} binocle_gl_shader_image_t;

typedef struct binocle_gl_shader_attr_t {
  binocle_str_t name;
} binocle_gl_shader_attr_t;

typedef struct binocle_gl_shader_stage_t {
  binocle_gl_uniform_block_t uniform_blocks[BINOCLE_MAX_SHADERSTAGE_UBS];
  binocle_gl_shader_image_t images[BINOCLE_MAX_SHADERSTAGE_IMAGES];
} binocle_gl_shader_stage_t;

typedef struct binocle_gl_shader {
  binocle_slot_t slot;
  binocle_shader_common_t cmn;
  struct {
    GLuint prog;
    binocle_gl_shader_attr_t attrs[BINOCLE_MAX_VERTEX_ATTRIBUTES];
    binocle_gl_shader_stage_t stage[BINOCLE_NUM_SHADER_STAGES];
  } gl;
} binocle_gl_shader;
typedef binocle_gl_shader binocle_shader_t;

typedef struct binocle_gl_attr_t {
  int8_t vb_index;        /* -1 if attr is not enabled */
  int8_t divisor;         /* -1 if not initialized */
  uint8_t stride;
  uint8_t size;
  uint8_t normalized;
  int offset;
  GLenum type;
} binocle_gl_attr_t;

typedef struct binocle_gl_cache_attr_t {
  binocle_gl_attr_t gl_attr;
  GLuint gl_vbuf;
} binocle_gl_cache_attr_t;

typedef struct binocle_gl_texture_bind_slot {
  GLenum target;
  GLuint texture;
} binocle_gl_texture_bind_slot;

typedef struct binocle_gl_state_cache_t {
//  binocle_depth_stencil_state ds;
//  sg_blend_state blend;
//  sg_rasterizer_state rast;
  bool polygon_offset_enabled;
  binocle_gl_cache_attr_t attrs[BINOCLE_MAX_VERTEX_ATTRIBUTES];
  GLuint vertex_buffer;
  GLuint index_buffer;
  GLuint stored_vertex_buffer;
  GLuint stored_index_buffer;
  GLuint prog;
  binocle_gl_texture_bind_slot textures[BINOCLE_MAX_SHADERSTAGE_IMAGES];
  binocle_gl_texture_bind_slot stored_texture;
  int cur_ib_offset;
  GLenum cur_primitive_type;
  GLenum cur_index_type;
  GLenum cur_active_texture;
//  binocle_pipeline_t* cur_pipeline;
//  binocle_pipeline cur_pipeline_id;
} binocle_gl_state_cache_t;

typedef struct binocle_gl_pipeline_t {
  binocle_slot_t slot;
  binocle_pipeline_common_t cmn;
  binocle_shader_t* shader;
  struct {
    binocle_gl_attr_t attrs[BINOCLE_MAX_VERTEX_ATTRIBUTES];
    binocle_depth_state depth;
    binocle_stencil_state stencil;
    binocle_primitive_type primitive_type;
    binocle_blend_state blend;
    binocle_color_mask color_write_mask[BINOCLE_MAX_COLOR_ATTACHMENTS];
    binocle_cull_mode cull_mode;
    binocle_face_winding face_winding;
    int sample_count;
    bool alpha_to_coverage_enabled;
  } gl;
} binocle_gl_pipeline_t;
typedef binocle_gl_pipeline_t binocle_pipeline_t;

typedef struct {
  binocle_image_t* image;
  GLuint gl_msaa_resolve_buffer;
} binocle_gl_attachment_t;

typedef struct binocle_gl_pass_t {
  binocle_slot_t slot;
  binocle_pass_common_t cmn;
  struct {
    GLuint fb;
    binocle_gl_attachment_t color_atts[BINOCLE_MAX_COLOR_ATTACHMENTS];
    binocle_gl_attachment_t ds_att;
  } gl;
} binocle_gl_pass_t;
typedef binocle_gl_pass_t binocle_pass_t;
typedef binocle_pass_attachment_common_t binocle_pass_attachment_t;

typedef struct binocle_gl_backend_t {
  bool valid;
  bool gles2;
  binocle_gl_state_cache_t cache;
  bool ext_anisotropic;
  GLint max_anisotropy;
  GLint max_combined_texture_image_units;

  GLuint vbo;
  GLuint vertex_attribute;
  GLuint color_attribute;
  GLuint tex_coord_attribute;
  GLuint normal_attribute;
  GLint image_uniform;
  GLint projection_matrix_uniform;
  GLint view_matrix_uniform;
  GLint model_matrix_uniform;

  binocle_limits limits;
} binocle_gl_backend_t;

#ifdef DEBUG
// In debug mode, perform a test on every OpenGL call
// The do-while loop is needed so that glCheck can be used as a single statement
// in if/else branches
#define glCheck(expr)                                                          \
  do {                                                                         \
    expr;                                                                      \
    binocle_backend_gl_check_error(__FILE__, __LINE__, #expr);                      \
  } while (false)
#else
// Else, we don't add any overhead
#define glCheck(expr) (expr)
#endif

void binocle_backend_gl_check_error(const char *file, unsigned int line,
                               const char *expression);

/**
 * Initializes the GL backend
 * @param gl the pointer to the backend struct
 */
void binocle_backend_gl_init(binocle_gl_backend_t *gl);

/**
 * \brief Applies the default OpenGL states for basic 2D drawing
 */
void binocle_backend_gl_apply_default_state();

/**
 * \brief Applies the given viewport
 * @param x the bottom-left corner (horizontal)
 * @param y the bottom-left corner (vertical)
 * @param w the width
 * @param h the height
 */
void binocle_backend_gl_apply_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h);

/**
 * \brief Applies the given blend mode
 * @param blend_mode the requested blend mode
 */
void binocle_backend_gl_apply_blend_mode(struct binocle_blend blend_mode);

/**
 * \brief Applies the given shader
 * @param gd the graphics device instance
 * @param shader the shader
 */
void binocle_backend_gl_apply_shader(binocle_gl_backend_t *gl,
                                     binocle_gl_shader *shader);

void binocle_backend_gl_apply_texture(binocle_image_t *texture);

void binocle_backend_gl_apply_3d_texture(binocle_image_t *albedo, binocle_image_t *normal);

void binocle_backend_gl_draw(binocle_gl_backend_t *gl, const struct binocle_vpct *vertices, size_t vertex_count, struct binocle_blend blend,
                             binocle_gl_shader *shader, binocle_image_t *albedo,
                             struct kmAABB2 viewport, struct kmMat4 *cameraTransformMatrix);

binocle_resource_state binocle_backend_gl_create_render_target(binocle_render_target_t *rt, uint32_t width, uint32_t height, bool use_depth, binocle_pixel_format format);
void binocle_backend_gl_destroy_render_target(binocle_render_target_t *render_target);
void binocle_backend_gl_set_render_target(binocle_image_t *render_target);
void binocle_backend_gl_clear(struct binocle_color color);
void binocle_backend_gl_set_uniform_float2(binocle_gl_shader *shader, const char *name, float value1, float value2);
void binocle_backend_gl_set_uniform_mat4(binocle_gl_shader *shader, const char *name, struct kmMat4 mat);
void binocle_backend_gl_draw_quad_to_screen(
  binocle_gl_shader *shader, binocle_image_t *render_target);
binocle_resource_state
binocle_backend_gl_create_image(binocle_gl_backend_t *gl, binocle_image_t *img,
                                const binocle_image_desc *desc);
void binocle_backend_gl_destroy_image(binocle_gl_backend_t *gl, binocle_image_t* img);
void binocle_backend_gl_reset_state_cache(binocle_gl_backend_t *gl);

binocle_resource_state
binocle_backend_gl_create_shader(binocle_gl_backend_t *gl, binocle_shader_t *sha,
                                const binocle_shader_desc *desc);
void binocle_backend_gl_destroy_shader(binocle_gl_backend_t *gl, binocle_shader_t* sha);
binocle_resource_state binocle_backend_gl_create_pipeline(
  binocle_gl_backend_t *gl, binocle_pipeline_t *pip, binocle_shader_t *shd,
  const binocle_pipeline_desc *desc);

#endif // BINOCLE_BACKEND_GL_H
