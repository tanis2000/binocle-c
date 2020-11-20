//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_BACKEND_GL_H
#define BINOCLE_BACKEND_GL_H

#include <stdbool.h>
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

typedef struct binocle_gl_backend_t {
  bool valid;
  GLuint vbo;
  GLuint vertex_attribute;
  GLuint color_attribute;
  GLuint tex_coord_attribute;
  GLuint normal_attribute;
  GLint image_uniform;
  GLint projection_matrix_uniform;
  GLint view_matrix_uniform;
  GLint model_matrix_uniform;
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
void binocle_backend_gl_apply_shader(binocle_gl_backend_t *gl, struct binocle_shader *shader);

void binocle_backend_gl_apply_texture(struct binocle_texture texture);

void binocle_backend_gl_apply_3d_texture(struct binocle_material *material);

void binocle_backend_gl_draw(binocle_gl_backend_t *gl, const struct binocle_vpct *vertices, size_t vertex_count, struct binocle_material material,
                             struct kmAABB2 viewport, struct kmMat4 *cameraTransformMatrix);

binocle_resource_state binocle_backend_gl_create_render_target(binocle_render_target_t *rt, uint32_t width, uint32_t height, bool use_depth, GLenum format);
void binocle_backend_gl_destroy_render_target(binocle_render_target_t *render_target);
void binocle_backend_gl_set_render_target(binocle_render_target_t *render_target);
void binocle_backend_gl_clear(struct binocle_color color);
void binocle_backend_gl_set_uniform_float2(struct binocle_shader *shader, const char *name, float value1, float value2);
void binocle_backend_gl_set_uniform_mat4(struct binocle_shader *shader, const char *name, struct kmMat4 mat);
void binocle_backend_gl_draw_quad_to_screen(struct binocle_shader *shader, binocle_render_target_t *render_target);

#endif // BINOCLE_BACKEND_GL_H
