//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_GD_H
#define BINOCLE_GD_H

#include "binocle_sdl.h"
#include <kazmath/kazmath.h>

struct binocle_blend;
struct binocle_camera;
struct binocle_color;
struct binocle_material;
struct binocle_shader;
struct binocle_texture;
struct binocle_vpct;

enum binocle_blend_factor;
enum binocle_blend_equation;

typedef struct binocle_gd {
  GLuint vbo;
  GLuint vertex_attribute;
  GLuint color_attribute;
  GLuint tex_coord_attribute;
  GLuint normal_attribute;
  GLint image_uniform;
  GLint projection_matrix_uniform;
  // GLint model_view_matrix_uniform;
  GLint view_matrix_uniform;
  GLint model_matrix_uniform;
} binocle_gd;

typedef struct binocle_render_target {
  GLuint frame_buffer;
  GLuint render_buffer; // used for depth
  GLuint texture;
} binocle_render_target;

#ifdef DEBUG
// In debug mode, perform a test on every OpenGL call
// The do-while loop is needed so that glCheck can be used as a single statement
// in if/else branches
#define glCheck(expr)                                                          \
  do {                                                                         \
    expr;                                                                      \
    binocle_gd_gl_check_error(__FILE__, __LINE__, #expr);                      \
  } while (false)
#else
// Else, we don't add any overhead
#define glCheck(expr) (expr)
#endif

void binocle_gd_gl_check_error(const char *file, unsigned int line,
                               const char *expression);

binocle_gd binocle_gd_new();
void binocle_gd_init(binocle_gd *gd);
kmMat4 binocle_gd_create_model_view_matrix(float x, float y, float scale,
                                           float rotation);
void binocle_gd_draw(binocle_gd *gd, const struct binocle_vpct *vertices,
                     size_t vertex_count, struct binocle_material material,
                     kmAABB2 viewport, struct binocle_camera *camera);
void binocle_gd_apply_gl_states();
void binocle_gd_apply_viewport(kmAABB2 viewport);
void binocle_gd_apply_blend_mode(const struct binocle_blend blend_mode);
void binocle_gd_apply_shader(binocle_gd *gd, struct binocle_shader shader);
void binocle_gd_apply_texture(struct binocle_texture texture);
GLuint binocle_gd_factor_to_gl_constant(enum binocle_blend_factor blend_factor);
GLuint
binocle_gd_equation_to_gl_constant(enum binocle_blend_equation blend_equation);
binocle_render_target binocle_gd_create_render_target(uint32_t width,
                                                      uint32_t height,
                                                      bool use_depth,
                                                      GLenum format);
void binocle_gd_destroy_render_target(binocle_render_target *render_target);
void binocle_gd_set_uniform_float(struct binocle_shader shader,
                                  const char *name, float value);
void binocle_gd_set_uniform_float2(struct binocle_shader shader,
                                   const char *name, float value1,
                                   float value2);
void binocle_gd_set_uniform_float3(struct binocle_shader shader,
                                   const char *name, float value1,
                                   float value2,
                                   float value3);
void binocle_gd_set_uniform_float4(struct binocle_shader shader,
                                   const char *name, float value1,
                                   float value2,
                                   float value3,
                                   float value4);
void binocle_gd_clear(struct binocle_color color);
void binocle_gd_set_render_target(binocle_render_target render_target);
void binocle_gd_draw_quad(struct binocle_shader shader);
void binocle_gd_draw_quad_to_screen(struct binocle_shader shader,
                                    binocle_render_target render_target);
void binocle_gd_set_uniform_render_target_as_texture(
  struct binocle_shader shader, const char *name,
  binocle_render_target render_target);
void binocle_gd_set_uniform_mat4(struct binocle_shader shader, const char *name,
                                 kmMat4 mat);
void binocle_gd_draw_rect(binocle_gd *gd, kmAABB2 rect,
                          struct binocle_color col, kmAABB2 viewport,
                          kmMat4 viewMatrix);

#endif // BINOCLE_GD_H
