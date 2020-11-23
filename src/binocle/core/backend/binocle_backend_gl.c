//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_backend_gl.h"
#include "binocle_blend.h"
#include "../binocle_log.h"
#include "binocle_shader.h"
#include "binocle_texture.h"
#include "binocle_material.h"
#include "binocle_vpct.h"

void binocle_backend_gl_check_error(const char *file, unsigned int line, const char *expression) {
  // Get the last error
  GLenum errorCode = glGetError();

  if (errorCode != GL_NO_ERROR) {
    char error[1024] = "Unknown error";
    char description[1024] = "No description";

    // Decode the error code
    switch (errorCode) {
    case GL_INVALID_ENUM: {
      strcpy(error, "GL_INVALID_ENUM");
      strcpy(description, "An unacceptable value has been specified for an enumerated argument.");
      break;
    }

    case GL_INVALID_VALUE: {
      strcpy(error, "GL_INVALID_VALUE");
      strcpy(description, "A numeric argument is out of range.");
      break;
    }

    case GL_INVALID_OPERATION: {
      strcpy(error, "GL_INVALID_OPERATION");
      strcpy(description, "The specified operation is not allowed in the current state.");
      break;
    }

    case GL_STACK_OVERFLOW: {
      strcpy(error, "GL_STACK_OVERFLOW");
      strcpy(description, "This command would cause a stack overflow.");
      break;
    }

    case GL_STACK_UNDERFLOW: {
      strcpy(error, "GL_STACK_UNDERFLOW");
      strcpy(description, "This command would cause a stack underflow.");
      break;
    }

    case GL_OUT_OF_MEMORY: {
      strcpy(error, "GL_OUT_OF_MEMORY");
      strcpy(description, "There is not enough memory left to execute the command.");
      break;
    }
    default: {
      strcpy(error, "UNKNOWN");
      strcpy(description, "An unknown error has occurred.");
    }

      /*case GLEXT_GL_INVALID_FRAMEBUFFER_OPERATION:
      {
        error = "GL_INVALID_FRAMEBUFFER_OPERATION";
        description = "The object bound to FRAMEBUFFER_BINDING is not \"framebuffer complete\".";
        break;
      }*/
    }

    // Log the error
    SDL_Log("An internal OpenGL call failed in %s (%d).\nExpression:\n%s\nError description:\n %s\n%s", file, line,
            expression, error, description);
    SDL_Quit();
  }
}

GLuint binocle_backend_gl_factor_to_gl_constant(binocle_blend_factor blend_factor) {
  switch (blend_factor) {
  case BINOCLE_BLEND_ZERO:
    return GL_ZERO;
  case BINOCLE_BLEND_ONE:
    return GL_ONE;
  case BINOCLE_BLEND_SRCCOLOR:
    return GL_SRC_COLOR;
  case BINOCLE_BLEND_ONEMINUSSRCCOLOR:
    return GL_ONE_MINUS_SRC_COLOR;
  case BINOCLE_BLEND_DSTCOLOR:
    return GL_DST_COLOR;
  case BINOCLE_BLEND_ONEMINUSDSTCOLOR:
    return GL_ONE_MINUS_DST_COLOR;
  case BINOCLE_BLEND_SRCALPHA:
    return GL_SRC_ALPHA;
  case BINOCLE_BLEND_ONEMINUSSRCALPHA:
    return GL_ONE_MINUS_SRC_ALPHA;
  case BINOCLE_BLEND_DSTALPHA:
    return GL_DST_ALPHA;
  case BINOCLE_BLEND_ONEMINUSDSTALPHA:
    return GL_ONE_MINUS_DST_ALPHA;
  }

  SDL_Log("Invalid value for Binocle::BlendMode::Factor! Fallback to Binocle::BlendMode::Zero.");
  return GL_ZERO;
}

GLuint binocle_backend_gl_equation_to_gl_constant(binocle_blend_equation blend_equation) {
  switch (blend_equation) {
  case BINOCLE_BLEND_ADD:
    return GL_FUNC_ADD;
  case BINOCLE_BLEND_SUBTRACT:
    return GL_FUNC_SUBTRACT;
  }

  SDL_Log("Invalid value for Binocle::BlendMode::Equation! Fallback to Binocle::BlendMode::Add.");
  return GL_FUNC_ADD;
}

GLenum binocle_backend_gl_pixel_format_to_gl_texture_format(binocle_pixel_format fmt) {
  switch (fmt) {
  case BINOCLE_PIXEL_FORMAT_RGB:
    return GL_RGB;
  case BINOCLE_PIXEL_FORMAT_RGBA:
    return GL_RGBA;
  default:
    return 0;
  }
}

void binocle_backend_gl_init(binocle_gl_backend_t *gl) {
  // Create a new vertex buffer object
  glCheck(glGenBuffers(1, &gl->vbo));
}

void binocle_backend_gl_apply_default_state() {
  glCheck(glDisable(GL_CULL_FACE));
  glCheck(glDisable(GL_DEPTH_TEST));

  glCheck(glEnable(GL_BLEND));

  // This is for better 2D textures placement
  //glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
  //glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

  /**
   * GL_NEAREST	Returns the value of the texture element that is nearest (in Manhattan distance) to the center of
   * the pixel being textured.
   *
   * GL_LINEAR	Returns the weighted average of the four texture elements that are closest to the center of the pixel being textured.
   * These can include border texture elements, depending on the values of GL_TEXTURE_WRAP_S and GL_TEXTURE_WRAP_T,
   * and on the exact mapping.
   */
  // Enable the following for better 2D pixel rendering
  //glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
  //glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

  glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
}

void binocle_backend_gl_apply_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
  glCheck(glViewport(x, y, w, h));
}

void binocle_backend_gl_apply_blend_mode(const struct binocle_blend blend_mode) {
  glCheck(glBlendFunc(
    binocle_backend_gl_factor_to_gl_constant(blend_mode.color_src_factor),
    binocle_backend_gl_factor_to_gl_constant(blend_mode.color_dst_factor)));
  glCheck(glBlendEquation(binocle_backend_gl_equation_to_gl_constant(blend_mode.color_equation)));
}

void binocle_backend_gl_apply_shader(binocle_gl_backend_t *gl, binocle_shader *shader) {
  GLint id;
  glCheck(id = glGetAttribLocation(shader->program_id, "vertexPosition"));
  gl->vertex_attribute = id;
  gl->tex_coord_attribute = glGetAttribLocation(shader->program_id, "vertexTCoord");
  gl->color_attribute = glGetAttribLocation(shader->program_id, "vertexColor");
  gl->normal_attribute = glGetAttribLocation(shader->program_id, "vertexNormal");
  gl->projection_matrix_uniform = glGetUniformLocation(shader->program_id, "projectionMatrix");
  gl->view_matrix_uniform = glGetUniformLocation(shader->program_id, "viewMatrix");
  gl->model_matrix_uniform = glGetUniformLocation(shader->program_id, "modelMatrix");
  gl->image_uniform = glGetUniformLocation(shader->program_id, "tex0");
  glCheck(glUseProgram(shader->program_id));
}

void binocle_backend_gl_apply_texture(binocle_texture texture) {
  glCheck(glActiveTexture(GL_TEXTURE0));
  glCheck(glBindTexture(GL_TEXTURE_2D, texture.tex_id));
  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
}

void binocle_backend_gl_apply_3d_texture(binocle_material *material) {
  // Diffuse texture
  glCheck(glActiveTexture(GL_TEXTURE0));
  glCheck(glBindTexture(GL_TEXTURE_2D, material->albedo_texture->tex_id));
  // Specular texture
  glCheck(glActiveTexture(GL_TEXTURE1));
  glCheck(glBindTexture(GL_TEXTURE_2D, material->normal_texture->tex_id));
  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
}

void binocle_backend_gl_draw(binocle_gl_backend_t *gl, const struct binocle_vpct *vertices, size_t vertex_count, struct binocle_material material,
                     struct kmAABB2 viewport, struct kmMat4 *cameraTransformMatrix) {
  binocle_backend_gl_apply_default_state();
  binocle_backend_gl_apply_viewport(viewport.min.x, viewport.min.y, viewport.max.x, viewport.max.y);
  binocle_backend_gl_apply_blend_mode(material.blend_mode);
  binocle_backend_gl_apply_shader(gl, material.shader);
  binocle_backend_gl_apply_texture(*material.albedo_texture);

  kmMat4 projectionMatrix = binocle_math_create_orthographic_matrix_off_center(viewport.min.x, viewport.max.x,
                                                                               viewport.min.y, viewport.max.y, -1000.0f,
                                                                               1000.0f);
  //kmMat4 modelViewMatrix = binocle_gd_create_model_view_matrix(0.0f, 0.0f, 1.0f, 0.0f);
  kmMat4 viewMatrix;
  kmMat4Identity(&viewMatrix);
  // TODO: apply the viewport_adapter scale_matrix here
  //kmMat4Scaling(&viewMatrix, 2.0f, 2.0f, 2.0f);

  if (cameraTransformMatrix != NULL) {
    kmMat4Multiply(&viewMatrix, &viewMatrix, cameraTransformMatrix);
  }

  kmMat4 modelMatrix;
  kmMat4Identity(&modelMatrix);

  glCheck(glEnableVertexAttribArray(gl->vertex_attribute));
  glCheck(glEnableVertexAttribArray(gl->color_attribute));
  glCheck(glEnableVertexAttribArray(gl->tex_coord_attribute));

  glCheck(glBindBuffer(GL_ARRAY_BUFFER, gl->vbo));
  glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(binocle_vpct) * vertex_count, vertices, GL_STATIC_DRAW));

  glCheck(glVertexAttribPointer(gl->vertex_attribute, 2, GL_FLOAT, GL_FALSE, sizeof(binocle_vpct), 0));
  glCheck(glVertexAttribPointer(gl->color_attribute, 4, GL_FLOAT, GL_FALSE, sizeof(binocle_vpct),
                                (void *) (2 * sizeof(GLfloat))));
  glCheck(glVertexAttribPointer(
    gl->tex_coord_attribute, 2, GL_FLOAT, GL_FALSE, sizeof(binocle_vpct),
                                (void *) (4 * sizeof(GLfloat) + 2 * sizeof(GLfloat))));

  //kmMat4 finalMatrix = Matrix4::mul(state.transform,projectionMatrix);
  //kmMat4 inverseMatrix: Matrix4<f32> = Matrix4::from_nonuniform_scale(1.0, 1.0, 1.0);

  glCheck(glUniformMatrix4fv(gl->projection_matrix_uniform, 1, GL_FALSE, projectionMatrix.mat));
  //glCheck(glUniformMatrix4fv(gd->model_view_matrix_uniform, 1, GL_FALSE, modelViewMatrix.mat));
  glCheck(glUniformMatrix4fv(gl->view_matrix_uniform, 1, GL_FALSE, viewMatrix.mat));
  glCheck(glUniformMatrix4fv(gl->model_matrix_uniform, 1, GL_FALSE, modelMatrix.mat));

  glCheck(glUniform1i(gl->image_uniform, 0));

  glCheck(glDrawArrays(GL_TRIANGLES, 0, vertex_count));

  glCheck(glDisableVertexAttribArray(gl->vertex_attribute));
  glCheck(glDisableVertexAttribArray(gl->color_attribute));
  glCheck(glDisableVertexAttribArray(gl->tex_coord_attribute));
  glCheck(glUseProgram(GL_ZERO));

  glCheck(glBindTexture(GL_TEXTURE_2D, 0));
  glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

binocle_resource_state binocle_backend_gl_create_render_target(binocle_render_target_t *rt, uint32_t width, uint32_t height, bool use_depth, binocle_pixel_format format) {
  /*
   * glGenBuffers creates regular buffers for vertex data, etc.
   * glGenFrameBuffers creates a framebuffer object primarily used as render targets for offscreen rendering.
   * glGenRenderBuffers creates a renderbuffer object that are specifically used with framebuffer objects for any depth-testing required.
   */

//  binocle_render_target_t *res = SDL_malloc(sizeof(binocle_render_target_t));

  GLenum fmt = binocle_backend_gl_pixel_format_to_gl_texture_format(format);

  GLuint fb[1];
  glCheck(glGenFramebuffers(1, fb));
  rt->frame_buffer = fb[0];

  GLuint rb[1];
  glCheck(glGenRenderbuffers(1, rb));
  rt->render_buffer = rb[0];

  GLuint t[1];
  glCheck(glGenTextures(1, t));
  rt->texture = t[0];

  // set up framebuffer

  // bind the framebuffer
  glCheck(glBindFramebuffer(GL_FRAMEBUFFER, rt->frame_buffer));

  // bind the newly created texture: all future texture functions will modify this texture
  glCheck(glBindTexture(GL_TEXTURE_2D, rt->texture));
  // Give an empty image to OpenGL ( the last "0" )
  glCheck(glTexImage2D(GL_TEXTURE_2D, 0, fmt, width, height, 0, fmt, GL_UNSIGNED_BYTE, 0));
  // filtering
  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
  // attach the texture to the bound framebuffer object
  glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rt->texture, 0));

  // set up renderbuffer (depth buffer)
  glCheck(glBindRenderbuffer(GL_RENDERBUFFER, rt->render_buffer));
  glCheck(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height));
  glCheck(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rt->render_buffer));

  // clean up
  glCheck(glBindTexture(GL_TEXTURE_2D, 0));
  glCheck(glBindRenderbuffer(GL_RENDERBUFFER, 0));
  glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));

  //if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
  //  binocle_log_error("Framebuffer isn't complete");
  //}
  return BINOCLE_RESOURCESTATE_VALID;
}

void binocle_backend_gl_destroy_render_target(binocle_render_target_t *render_target) {
  glCheck(glDeleteTextures(1, &render_target->texture));
  render_target->texture = GL_NONE;
  glCheck(glDeleteRenderbuffers(1, &render_target->render_buffer));
  render_target->render_buffer = GL_NONE;
  glCheck(glDeleteFramebuffers(1, &render_target->frame_buffer));
  render_target->frame_buffer = GL_NONE;
//  SDL_free(render_target);
}

void binocle_backend_gl_set_render_target(binocle_render_target_t *render_target) {
  if (render_target == NULL) {
    glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    glCheck(glBindRenderbuffer(GL_RENDERBUFFER, 0));
  } else {
    glCheck(glBindFramebuffer(GL_FRAMEBUFFER, render_target->frame_buffer));
    glCheck(glBindRenderbuffer(GL_RENDERBUFFER, render_target->render_buffer));
  }
}

void binocle_backend_gl_clear(struct binocle_color color) {
  glCheck(glClearColor(color.r, color.g, color.b, color.a));
  glCheck(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void binocle_backend_gl_set_uniform_float2(struct binocle_shader *shader, const char *name, float value1, float value2) {
  GLint id = 0;
  glCheck(id = glGetUniformLocation(shader->program_id, name));
  if (id == -1) {
    binocle_log_error("Cannot find uniform called %s", name);
    return;
  }
  glCheck(glUniform2f(id, value1, value2));
}

void binocle_backend_gl_set_uniform_mat4(struct binocle_shader *shader, const char *name, struct kmMat4 mat) {
  GLint id = 0;
  glCheck(id = glGetUniformLocation(shader->program_id, name));
  if (id == -1) {
    binocle_log_error("Cannot find uniform called %s", name);
    return;
  }
  glCheck(glUniformMatrix4fv(id, 1, GL_FALSE, mat.mat));
}

void binocle_backend_gl_draw_quad_to_screen(struct binocle_shader *shader, binocle_render_target_t *render_target) {
  static const GLfloat g_quad_vertex_buffer_data[] = {
      -1.0f, -1.0f,
      1.0f, -1.0f,
      -1.0f, 1.0f,
      -1.0f, 1.0f,
      1.0f, -1.0f,
      1.0f, 1.0f,
  };

  GLuint quad_vertexbuffer;
  glCheck(glGenBuffers(1, &quad_vertexbuffer));
  glCheck(glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer));
  glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW));
  GLint pos_id;
  glCheck(pos_id = glGetAttribLocation(shader->program_id, "position"));
  glCheck(glVertexAttribPointer(pos_id, 2, GL_FLOAT, false, 0, 0));
  glCheck(glEnableVertexAttribArray(pos_id));
  GLint tex_id;
  glCheck(tex_id = glGetUniformLocation(shader->program_id, "texture"));
  glCheck(glUniform1i(tex_id, 0));
  glCheck(glActiveTexture(GL_TEXTURE0));
  glCheck(glBindTexture(GL_TEXTURE_2D, render_target->texture));
  // Sets the frame buffer to use as the screen
#if defined(__IPHONEOS__)
  SDL_SysWMinfo info;
  SDL_VERSION(&info.version);
  SDL_GetWindowWMInfo(SDL_GL_GetCurrentWindow(), &info);
  GLuint id = info.info.uikit.framebuffer;
  glCheck(glBindFramebuffer(GL_FRAMEBUFFER, id));
#else
  glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));
#endif
  glCheck(glDrawArrays(GL_TRIANGLES, 0, 6));

  glCheck(glDisableVertexAttribArray(pos_id));
  glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
  glCheck(glDeleteBuffers(1, &quad_vertexbuffer));
  glCheck(glBindTexture(GL_TEXTURE_2D, 0));
}