//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_GD_H
#define BINOCLE_GD_H

#include "binocle_sdl.h"
#include <kazmath/kazmath.h>
#include "backend/binocle_backend.h"

//#if defined(BINOCLE_GL)
//#include "backend/binocle_backend_gl.h"
//#elif defined(BINOCLE_METAL)
//#include "backend/binocle_backend_metal.h"
//#endif

#define BINOCLE_GD_MAX_VERTICES (16535 * 6)
#define BINOCLE_GD_MAX_COMMANDS (16535)

struct binocle_blend;
struct binocle_camera;
struct binocle_color;
struct binocle_material;
struct binocle_render_state;
struct binocle_shader;
struct binocle_texture;
struct binocle_vpct;
struct binocle_mesh;
struct binocle_camera_3d;
struct binocle_window;
struct binocle_viewport_adapter;

enum binocle_blend_factor;
enum binocle_blend_equation;

typedef struct binocle_gd_gfx_t {
  binocle_pass pass;
  binocle_pipeline pip;
  binocle_bindings bind;
  binocle_pass_action action;
  binocle_image render_target;
  binocle_buffer vbuf;
  binocle_buffer ibuf;
} binocle_gd_gfx_t;

typedef struct binocle_gd_uniform_t {
  kmMat4 projectionMatrix;
  kmMat4 viewMatrix;
  kmMat4 modelMatrix;
} binocle_gd_uniform_t;

typedef struct binocle_gd_command_t {
  binocle_image img;
  uint32_t base_vertex;
  uint32_t num_vertices;
  binocle_gd_uniform_t uniforms;
} binocle_gd_command_t;

/**
 * \brief a graphic device used to perform OpenGL calls and store the needed state
 */
typedef struct binocle_gd {
  binocle_gd_gfx_t offscreen;
  binocle_gd_gfx_t display;
  binocle_gd_gfx_t flat;
  binocle_vpct *vertices;
  uint32_t num_vertices;
  binocle_gd_command_t *commands;
  uint32_t num_commands;
} binocle_gd;

/**
 * \brief Creates a new graphic device
 * @return the instance of the graphic device
 */
binocle_gd binocle_gd_new();

void binocle_gd_destroy(binocle_gd *gd);

/**
 * Initializes a graphic device
 * @param gd the pointer to the graphics device
 * @param wind the pointer to the window
 */
void binocle_gd_init(binocle_gd *gd, struct binocle_window *win);

void binocle_gd_setup_default_pipeline(binocle_gd *gd, uint32_t offscreen_width, uint32_t offscreen_height, binocle_shader offscreen_shader, binocle_shader display_shader);

void binocle_gd_render_offscreen(binocle_gd *gd);
void binocle_gd_render_screen(binocle_gd *gd, struct binocle_window *window, float design_width, float design_height, kmAABB2 viewport);
void binocle_gd_render(binocle_gd *gd, struct binocle_window *window, float design_width, float design_height, kmAABB2 viewport);

/**
 * Creates a 2D model view matrix
 * @param x the horizontal position
 * @param y the vertical position
 * @param scale the scale
 * @param rotation the rotation
 * @return the model-view matrix
 */
kmMat4 binocle_gd_create_model_view_matrix(float x, float y, float scale,
                                           float rotation);

/**
 * \brief draws a vertex buffer to the screen given a material, viewport and camera
 * @param gd the graphics device instance
 * @param vertices the vertex buffer
 * @param vertex_count the numer of vertices
 * @param material the material to use
 * @param viewport the viewport to apply
 * @param camera the camera
 */
void binocle_gd_draw(binocle_gd *gd, const struct binocle_vpct *vertices,
                     size_t vertex_count, struct binocle_material material,
                     kmAABB2 viewport, struct binocle_camera *camera);

/**
 * \brief Applies the default OpenGL states for basic 2D drawing
 */
void binocle_gd_apply_gl_states();

/**
 * \brief Applies the given viewport
 * @param viewport the viewport to apply
 */
void binocle_gd_apply_viewport(kmAABB2 viewport);

/**
 * \brief Applies the given blend mode
 * @param blend_mode the requested blend mode
 */
void binocle_gd_apply_blend_mode(const struct binocle_blend blend_mode);

/**
 * \brief Applies the given shader
 * @param gd the graphics device instance
 * @param shader the shader
 */
void binocle_gd_apply_shader(binocle_gd *gd, binocle_shader shader);

/**
 * \brief Applies the given texture
 * @param texture the texture to use
 */
void binocle_gd_apply_texture(struct binocle_image texture);

GLuint binocle_gd_factor_to_gl_constant(enum binocle_blend_factor blend_factor);

/**
 * \brief Converts the given blend equation to the corresponding OpenGL one
 * @param blend_equation the blend equation
 * @return the OpenGL blend equation constant
 */
GLuint
binocle_gd_equation_to_gl_constant(enum binocle_blend_equation blend_equation);

/**
 * \brief Sets a uniform float value for the given shader
 * @param shader the shader
 * @param name the name of the uniform
 * @param value the value to set
 */
void binocle_gd_set_uniform_float(struct binocle_shader *shader,
                                  const char *name, float value);

/**
 * \brief Sets a uniform float2 value for the given shader
 * @param shader the shader
 * @param name the name of the uniform
 * @param value1 the first float value
 * @param value2 the second float value
 */
void binocle_gd_set_uniform_float2(binocle_shader shader,
                                   const char *name, float value1,
                                   float value2);

/**
 * \brief Sets a uniform float3 value for the given shader
 * @param shader the shader
 * @param name the name of the uniform
 * @param value1 the first float value
 * @param value2 the second float value
 * @param value3 the third float value
 */
void binocle_gd_set_uniform_float3(struct binocle_shader *shader,
                                   const char *name, float value1,
                                   float value2,
                                   float value3);

/**
 * \brief Sets a uniform float4 value for the given shader
 * @param shader the shader
 * @param name the name of the uniform
 * @param value1 the first float value
 * @param value2 the second float value
 * @param value3 the third float value
 * @param value4 the fourth float value
 */
void binocle_gd_set_uniform_float4(struct binocle_shader *shader,
                                   const char *name, float value1,
                                   float value2,
                                   float value3,
                                   float value4);

/**
 * \brief Clears the current buffer with the given color
 * @param color the color
 */
void binocle_gd_clear(struct binocle_color color);

/**
 * \brief Binds the frame buffer and the render buffer of a render target
 * @param render_target the render target. If NULL, it sets both the frame buffer and render buffer to 0.
 */
void binocle_gd_set_render_target(binocle_image render_target);

/**
 * \brief Draws a quad to the current buffer using the given shader
 * @param shader the shader
 */
void binocle_gd_draw_quad(binocle_gd *gd, binocle_image image);

/**
 * \brief Draws a quad to the screen buffer using the given shader and render target
 * @param shader the shader
 * @param render_target the render target to use as source
 */
void binocle_gd_draw_quad_to_screen(binocle_shader shader,
                                    binocle_image render_target);

/**
 * \brief Sets a render target as the texture for a given uniform
 * @param shader the shader
 * @param name the name of the uniform
 * @param render_target the render target whose texture will be set as uniform
 */
void binocle_gd_set_uniform_render_target_as_texture(
    struct binocle_shader *shader, const char *name,
    binocle_image render_target);

/**
 * \brief Sets a uniform vec3 value
 * @param shader the shader
 * @param name the name of the uniform
 * @param vec the vec3 value
 */
void binocle_gd_set_uniform_vec3(struct binocle_shader *shader, const char *name, kmVec3 vec);

/**
 * \brief Sets a uniform mat4 value
 * @param shader the shader
 * @param name the name of the uniform
 * @param mat the mat4 value
 */
void binocle_gd_set_uniform_mat4(binocle_shader shader, const char *name,
                                 kmMat4 mat);
/**
 * \brief Draws a rectangle to the current buffer
 * @param gd the graphics device instance
 * @param rect the rectangle to draw
 * @param col the color to fill the rectangle with
 * @param viewport the viewport
 * @param viewMatrix the view matrix
 */
void binocle_gd_draw_rect(binocle_gd *gd, kmAABB2 rect,
                          struct binocle_color col, kmAABB2 viewport,
                          kmMat4 viewMatrix);

/**
 * \brief Draws the outline of a rectangle to the current buffer
 * @param gd the graphics device instance
 * @param rect the rectangle to draw
 * @param col the color to fill the rectangle with
 * @param viewport the viewport
 * @param viewMatrix the view matrix
 */
void binocle_gd_draw_rect_outline(binocle_gd *gd, kmAABB2 rect, struct binocle_color col, kmAABB2 viewport,
                                  kmMat4 viewMatrix);

/**
 * \brief Draws a line to the current buffer
 * @param gd the graphics device
 * @param start the starting point
 * @param end the ending point
 * @param col the color
 * @param viewport the viewport
 * @param viewMatrix the view matrix
 */
void binocle_gd_draw_line(binocle_gd *gd, kmVec2 start, kmVec2 end, struct binocle_color col, kmAABB2 viewport,
                          kmMat4 viewMatrix);

/**
 * \brief Draws a circle to the current buffer
 * @param gd the graphics device
 * @param center the coordinates of the center of the circle
 * @param radius the radius of the circle
 * @param col the color
 * @param viewport the viewport
 * @param viewMatrix the view matrix
 */
void binocle_gd_draw_circle(binocle_gd *gd, kmVec2 center, float radius, struct binocle_color col, kmAABB2 viewport,
                            kmMat4 viewMatrix);

/**
 * \brief Draws the graphic device's vertex buffer using the given render state
 * @param gd the graphics device instance
 * @param vertices the buffer with the vertices to draw
 * @param vertex_count the number of vertices
 * @param render_state the render state
 */
void binocle_gd_draw_with_state(binocle_gd *gd, const struct binocle_vpct *vertices, size_t vertex_count,
                                struct binocle_render_state *render_state);

void binocle_gd_draw_mesh(binocle_gd *gd, const struct binocle_mesh *mesh, kmAABB2 viewport, struct binocle_camera_3d *camera);
void binocle_gd_draw_test_triangle(struct binocle_shader *shader);
void binocle_gd_draw_test_cube(struct binocle_shader *shader);
void binocle_gd_setup_flat_pipeline(binocle_gd *gd);

#endif // BINOCLE_GD_H
