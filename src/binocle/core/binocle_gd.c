//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include <string.h>
#include "binocle_gd.h"
#include "binocle_vpct.h"
#include "binocle_material.h"
#include "binocle_viewport_adapter.h"
#include "binocle_render_state.h"
#include "binocle_shader.h"
#include "binocle_texture.h"
#include "binocle_camera.h"
#include "binocle_log.h"
#include "binocle_model.h"

void binocle_gd_gl_check_error(const char *file, unsigned int line, const char *expression) {
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

binocle_gd binocle_gd_new() {
  binocle_gd res = {0};
  return res;
}

void binocle_gd_init(binocle_gd *gd) {
  // Create a new vertex buffer object
  glCheck(glGenBuffers(1, &gd->vbo));
}

void binocle_gd_draw(binocle_gd *gd, const binocle_vpct *vertices, size_t vertex_count, binocle_material material,
                     kmAABB2 viewport, binocle_camera *camera) {
  binocle_gd_apply_gl_states();
  binocle_gd_apply_viewport(viewport);
  binocle_gd_apply_blend_mode(material.blend_mode);
  binocle_gd_apply_shader(gd, *material.shader);
  binocle_gd_apply_texture(*material.texture);

  kmMat4 projectionMatrix = binocle_math_create_orthographic_matrix_off_center(viewport.min.x, viewport.max.x,
                                                                               viewport.min.y, viewport.max.y, -1000.0f,
                                                                               1000.0f);
  //kmMat4 modelViewMatrix = binocle_gd_create_model_view_matrix(0.0f, 0.0f, 1.0f, 0.0f);
  kmMat4 viewMatrix;
  kmMat4Identity(&viewMatrix);
  // TODO: apply the viewport_adapter scale_matrix here
  //kmMat4Scaling(&viewMatrix, 2.0f, 2.0f, 2.0f);

  if (camera != NULL) {
    kmMat4Multiply(&viewMatrix, &viewMatrix, binocle_camera_get_transform_matrix(camera));
  }

  kmMat4 modelMatrix;
  kmMat4Identity(&modelMatrix);

  glCheck(glEnableVertexAttribArray(gd->vertex_attribute));
  glCheck(glEnableVertexAttribArray(gd->color_attribute));
  glCheck(glEnableVertexAttribArray(gd->tex_coord_attribute));

  glCheck(glBindBuffer(GL_ARRAY_BUFFER, gd->vbo));
  glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(binocle_vpct) * vertex_count, vertices, GL_STATIC_DRAW));

  glCheck(glVertexAttribPointer(gd->vertex_attribute, 2, GL_FLOAT, GL_FALSE, sizeof(binocle_vpct), 0));
  glCheck(glVertexAttribPointer(gd->color_attribute, 4, GL_FLOAT, GL_FALSE, sizeof(binocle_vpct),
                                (void *) (2 * sizeof(GLfloat))));
  glCheck(glVertexAttribPointer(gd->tex_coord_attribute, 2, GL_FLOAT, GL_FALSE, sizeof(binocle_vpct),
                                (void *) (4 * sizeof(GLfloat) + 2 * sizeof(GLfloat))));

  //kmMat4 finalMatrix = Matrix4::mul(state.transform,projectionMatrix);
  //kmMat4 inverseMatrix: Matrix4<f32> = Matrix4::from_nonuniform_scale(1.0, 1.0, 1.0);

  glCheck(glUniformMatrix4fv(gd->projection_matrix_uniform, 1, GL_FALSE, projectionMatrix.mat));
  //glCheck(glUniformMatrix4fv(gd->model_view_matrix_uniform, 1, GL_FALSE, modelViewMatrix.mat));
  glCheck(glUniformMatrix4fv(gd->view_matrix_uniform, 1, GL_FALSE, viewMatrix.mat));
  glCheck(glUniformMatrix4fv(gd->model_matrix_uniform, 1, GL_FALSE, modelMatrix.mat));

  glCheck(glUniform1i(gd->image_uniform, 0));

  glCheck(glDrawArrays(GL_TRIANGLES, 0, vertex_count));

  glCheck(glDisableVertexAttribArray(gd->vertex_attribute));
  glCheck(glDisableVertexAttribArray(gd->color_attribute));
  glCheck(glDisableVertexAttribArray(gd->tex_coord_attribute));
  glCheck(glUseProgram(GL_ZERO));

  glCheck(glBindTexture(GL_TEXTURE_2D, 0));
  glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));

}

kmMat4 binocle_gd_create_model_view_matrix(float x, float y, float scale, float rotation) {
  float theta = rotation * (float) M_PI / 180.0f;
  float c = cosf(theta);
  float s = sinf(theta);

  kmMat4 m;
  m.mat[0] = c * scale;
  m.mat[1] = s * -scale;
  m.mat[2] = 0.0f;
  m.mat[3] = 0.0f;

  m.mat[4] = s * scale;
  m.mat[5] = c * scale;
  m.mat[6] = 0.0f;
  m.mat[7] = 0.0f;

  m.mat[8] = 0.0f;
  m.mat[9] = 0.0f;
  m.mat[10] = 1.0f;
  m.mat[11] = 0.0f;

  m.mat[12] = x;
  m.mat[13] = y;
  m.mat[14] = 0.0f;
  m.mat[15] = 1.0f;

  return m;
}

void binocle_gd_apply_gl_states() {
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


void binocle_gd_apply_3d_gl_states() {
  glCheck(glDisable(GL_CULL_FACE));
  glCheck(glEnable(GL_DEPTH_TEST));
  glCheck(glEnable(GL_BLEND));
  glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
}

void binocle_gd_apply_viewport(kmAABB2 viewport) {
  glCheck(glViewport(viewport.min.x, viewport.min.y, viewport.max.x, viewport.max.y));
}

void binocle_gd_apply_blend_mode(const binocle_blend blend_mode) {
  glCheck(glBlendFunc(
      binocle_gd_factor_to_gl_constant(blend_mode.color_src_factor),
      binocle_gd_factor_to_gl_constant(blend_mode.color_dst_factor)));
  glCheck(glBlendEquation(binocle_gd_equation_to_gl_constant(blend_mode.color_equation)));
}

void binocle_gd_apply_shader(binocle_gd *gd, binocle_shader shader) {
  gd->vertex_attribute = glGetAttribLocation(shader.program_id, "vertexPosition");
  gd->tex_coord_attribute = glGetAttribLocation(shader.program_id, "vertexTCoord");
  gd->color_attribute = glGetAttribLocation(shader.program_id, "vertexColor");
  gd->normal_attribute = glGetAttribLocation(shader.program_id, "vertexNormal");
  gd->projection_matrix_uniform = glGetUniformLocation(shader.program_id, "projectionMatrix");
  //gd->model_view_matrix_uniform = glGetUniformLocation(shader.program_id, "modelViewMatrix");
  gd->view_matrix_uniform = glGetUniformLocation(shader.program_id, "viewMatrix");
  gd->model_matrix_uniform = glGetUniformLocation(shader.program_id, "modelMatrix");
  gd->image_uniform = glGetUniformLocation(shader.program_id, "tex0");
  glCheck(glUseProgram(shader.program_id));
}

void binocle_gd_apply_texture(binocle_texture texture) {
  glCheck(glActiveTexture(GL_TEXTURE0));
  glCheck(glBindTexture(GL_TEXTURE_2D, texture.tex_id));
  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
}

GLuint binocle_gd_factor_to_gl_constant(binocle_blend_factor blend_factor) {
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

GLuint binocle_gd_equation_to_gl_constant(binocle_blend_equation blend_equation) {
  switch (blend_equation) {
    case BINOCLE_BLEND_ADD:
      return GL_FUNC_ADD;
    case BINOCLE_BLEND_SUBTRACT:
      return GL_FUNC_SUBTRACT;
  }

  SDL_Log("Invalid value for Binocle::BlendMode::Equation! Fallback to Binocle::BlendMode::Add.");
  return GL_FUNC_ADD;
}

binocle_render_target binocle_gd_create_render_target(uint32_t width, uint32_t height, bool use_depth, GLenum format) {
  /*
   * glGenBuffers creates regular buffers for vertex data, etc.
   * glGenFrameBuffers creates a framebuffer object primarily used as render targets for offscreen rendering.
   * glGenRenderBuffers creates a renderbuffer object that are specifically used with framebuffer objects for any depth-testing required.
   */

  binocle_render_target res = {0};

  GLuint fb[1];
  glCheck(glGenFramebuffers(1, fb));
  res.frame_buffer = fb[0];

  GLuint rb[1];
  glCheck(glGenRenderbuffers(1, rb));
  res.render_buffer = rb[0];

  GLuint t[1];
  glCheck(glGenTextures(1, t));
  res.texture = t[0];

  // set up framebuffer

  // bind the framebuffer
  glCheck(glBindFramebuffer(GL_FRAMEBUFFER, res.frame_buffer));

  // bind the newly created texture: all future texture functions will modify this texture
  glCheck(glBindTexture(GL_TEXTURE_2D, res.texture));
  // Give an empty image to OpenGL ( the last "0" )
  glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0));
  // filtering
  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
  // attach the texture to the bound framebuffer object
  glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, res.texture, 0));

  // set up renderbuffer (depth buffer)
  glCheck(glBindRenderbuffer(GL_RENDERBUFFER, res.render_buffer));
  glCheck(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height));
  glCheck(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, res.render_buffer));

  // clean up
  glCheck(glBindTexture(GL_TEXTURE_2D, 0));
  glCheck(glBindRenderbuffer(GL_RENDERBUFFER, 0));
  glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));

  //if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
  //  binocle_log_error("Framebuffer isn't complete");
  //}
  return res;
}

void binocle_gd_destroy_render_target(binocle_render_target *render_target) {
  glCheck(glDeleteTextures(1, &render_target->texture));
  render_target->texture = GL_NONE;
  glCheck(glDeleteRenderbuffers(1, &render_target->render_buffer));
  render_target->render_buffer = GL_NONE;
  glCheck(glDeleteFramebuffers(1, &render_target->frame_buffer));
  render_target->frame_buffer = GL_NONE;
}

void binocle_gd_set_uniform_float(struct binocle_shader shader, const char *name, float value) {
  GLint id = 0;
  glCheck(id = glGetUniformLocation(shader.program_id, name));
  if (id == -1) {
    binocle_log_error("Cannot find uniform called %s", name);
    return;
  }
  glCheck(glUniform1f(id, value));
}

void binocle_gd_set_uniform_float2(struct binocle_shader shader, const char *name, float value1, float value2) {
  GLint id = 0;
  glCheck(id = glGetUniformLocation(shader.program_id, name));
  if (id == -1) {
    binocle_log_error("Cannot find uniform called %s", name);
    return;
  }
  glCheck(glUniform2f(id, value1, value2));
}

void binocle_gd_set_uniform_float3(struct binocle_shader shader, const char *name, float value1, float value2,
                                   float value3) {
  GLint id = 0;
  glCheck(id = glGetUniformLocation(shader.program_id, name));
  if (id == -1) {
    binocle_log_error("Cannot find uniform called %s", name);
    return;
  }
  glCheck(glUniform3f(id, value1, value2, value3));
}

void
binocle_gd_set_uniform_float4(struct binocle_shader shader, const char *name, float value1, float value2, float value3,
                              float value4) {
  GLint id = 0;
  glCheck(id = glGetUniformLocation(shader.program_id, name));
  if (id == -1) {
    binocle_log_error("Cannot find uniform called %s", name);
    return;
  }
  glCheck(glUniform4f(id, value1, value2, value3, value4));
}

void binocle_gd_set_uniform_render_target_as_texture(struct binocle_shader shader, const char *name,
                                                     binocle_render_target render_target) {
  GLint id = 0;
  glCheck(id = glGetUniformLocation(shader.program_id, name));
  if (id == -1) {
    binocle_log_error("Cannot find uniform called %s", name);
    return;
  }
  glCheck(glUniform1i(id, 0));
  glCheck(glActiveTexture(GL_TEXTURE0));
  glCheck(glBindTexture(GL_TEXTURE_2D, render_target.texture));
}

void binocle_gd_set_uniform_mat4(struct binocle_shader shader, const char *name, kmMat4 mat) {
  GLint id = 0;
  glCheck(id = glGetUniformLocation(shader.program_id, name));
  if (id == -1) {
    binocle_log_error("Cannot find uniform called %s", name);
    return;
  }
  glCheck(glUniformMatrix4fv(id, 1, GL_FALSE, mat.mat));
}

void binocle_gd_clear(struct binocle_color color) {
  glCheck(glClearColor(color.r, color.g, color.b, color.a));
  glCheck(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void binocle_gd_set_render_target(binocle_render_target render_target) {
  glCheck(glBindFramebuffer(GL_FRAMEBUFFER, render_target.frame_buffer));
  glCheck(glBindRenderbuffer(GL_RENDERBUFFER, render_target.render_buffer));
}

void binocle_gd_draw_quad(struct binocle_shader shader) {
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
  glCheck(pos_id = glGetAttribLocation(shader.program_id, "position"));
  glCheck(glVertexAttribPointer(pos_id, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, 0));
  glCheck(glEnableVertexAttribArray(pos_id));
  glCheck(glDrawArrays(GL_TRIANGLES, 0, 6));

  glCheck(glDisableVertexAttribArray(pos_id));
  glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
  glCheck(glDeleteBuffers(1, &quad_vertexbuffer));
}

void binocle_gd_draw_quad_to_screen(struct binocle_shader shader, binocle_render_target render_target) {
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
  glCheck(pos_id = glGetAttribLocation(shader.program_id, "position"));
  glCheck(glVertexAttribPointer(pos_id, 2, GL_FLOAT, false, 0, 0));
  glCheck(glEnableVertexAttribArray(pos_id));
  GLint tex_id;
  glCheck(tex_id = glGetUniformLocation(shader.program_id, "texture"));
  glCheck(glUniform1i(tex_id, 0));
  glCheck(glActiveTexture(GL_TEXTURE0));
  glCheck(glBindTexture(GL_TEXTURE_2D, render_target.texture));
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

void binocle_gd_draw_rect(binocle_gd *gd, kmAABB2 rect, binocle_color col, kmAABB2 viewport, kmMat4 viewMatrix) {
  static GLfloat g_quad_vertex_buffer_data[6 * 2] = {0};
  g_quad_vertex_buffer_data[0] = rect.min.x;
  g_quad_vertex_buffer_data[1] = rect.min.y;
  g_quad_vertex_buffer_data[2] = rect.max.x;
  g_quad_vertex_buffer_data[3] = rect.min.y;
  g_quad_vertex_buffer_data[4] = rect.min.x;
  g_quad_vertex_buffer_data[5] = rect.max.y;
  g_quad_vertex_buffer_data[6] = rect.min.x;
  g_quad_vertex_buffer_data[7] = rect.max.y;
  g_quad_vertex_buffer_data[8] = rect.max.x;
  g_quad_vertex_buffer_data[9] = rect.min.y;
  g_quad_vertex_buffer_data[10] = rect.max.x;
  g_quad_vertex_buffer_data[11] = rect.max.y;

  kmMat4 projectionMatrix = binocle_math_create_orthographic_matrix_off_center(viewport.min.x, viewport.max.x,
                                                                               viewport.min.y, viewport.max.y, -1000.0f,
                                                                               1000.0f);
  //kmMat4 viewMatrix;
  //kmMat4Identity(&viewMatrix);
  kmMat4 modelMatrix;
  kmMat4Identity(&modelMatrix);

  //GLuint quad_vertexbuffer;
  //glCheck(glGenBuffers(1, &quad_vertexbuffer));
  //glCheck(glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer));
  glCheck(glBindBuffer(GL_ARRAY_BUFFER, gd->vbo));
  glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_DYNAMIC_DRAW));
  glCheck(glUseProgram(binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_FLAT].program_id));
  GLint color_id;
  glCheck(color_id = glGetUniformLocation(binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_FLAT].program_id, "color"));
  GLint pos_id;
  glCheck(
      pos_id = glGetAttribLocation(binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_FLAT].program_id, "vertexPosition"));
  glCheck(glUniform4f(color_id, col.r, col.g, col.b, col.a));
  binocle_gd_apply_viewport(viewport);
  glCheck(glVertexAttribPointer(pos_id, 2, GL_FLOAT, false, sizeof(GLfloat) * 2, 0));
  glCheck(glEnableVertexAttribArray(pos_id));
  GLint projection_matrix_uniform;
  glCheck(
      projection_matrix_uniform = glGetUniformLocation(binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_FLAT].program_id,
                                                       "projectionMatrix"));
  GLint view_matrix_uniform;
  glCheck(view_matrix_uniform = glGetUniformLocation(binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_FLAT].program_id,
                                                     "viewMatrix"));
  GLint model_matrix_uniform;
  glCheck(model_matrix_uniform = glGetUniformLocation(binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_FLAT].program_id,
                                                      "modelMatrix"));
  glCheck(glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, projectionMatrix.mat));
  glCheck(glUniformMatrix4fv(view_matrix_uniform, 1, GL_FALSE, viewMatrix.mat));
  glCheck(glUniformMatrix4fv(model_matrix_uniform, 1, GL_FALSE, modelMatrix.mat));
  //glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));
  glCheck(glDrawArrays(GL_TRIANGLES, 0, 6));

  glCheck(glDisableVertexAttribArray(pos_id));
  glCheck(glUseProgram(GL_ZERO));
  glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void binocle_gd_draw_rect_outline(binocle_gd *gd, kmAABB2 rect, binocle_color col, kmAABB2 viewport, kmMat4 viewMatrix) {
  kmAABB2 rect_bottom;
  kmAABB2 rect_top;
  kmAABB2 rect_left;
  kmAABB2 rect_right;

  rect_bottom.min.x = rect.min.x;
  rect_bottom.min.y = rect.min.y;
  rect_bottom.max.x = rect.max.x;
  rect_bottom.max.y = rect.min.y + 1;

  rect_top.min.x = rect.min.x;
  rect_top.min.y = rect.max.y-1;
  rect_top.max.x = rect.max.x;
  rect_top.max.y = rect.max.y;

  rect_left.min.x = rect.min.x;
  rect_left.min.y = rect.min.y;
  rect_left.max.x = rect.min.x + 1;
  rect_left.max.y = rect.max.y;

  rect_right.min.x = rect.max.x - 1;
  rect_right.min.y = rect.min.y;
  rect_right.max.x = rect.max.x;
  rect_right.max.y = rect.max.y;

  binocle_gd_draw_rect(gd, rect_bottom, col, viewport, viewMatrix);
  binocle_gd_draw_rect(gd, rect_top, col, viewport, viewMatrix);
  binocle_gd_draw_rect(gd, rect_left, col, viewport, viewMatrix);
  binocle_gd_draw_rect(gd, rect_right, col, viewport, viewMatrix);
}

void binocle_gd_draw_line(binocle_gd *gd, kmVec2 start, kmVec2 end, binocle_color col, kmAABB2 viewport, kmMat4 viewMatrix) {

  //float angle = atan2f(end.y - start.y, end.x - start.x);//kmVec2DegreesBetween(&start, &end);
  kmVec2 right;
  right.x = 1;
  right.y = 0;
  kmVec2 norm_end;
  kmVec2Subtract(&norm_end, &end, &start);
  kmVec2Normalize(&norm_end, &norm_end);
  float angle = atan2f(norm_end.y - right.y, norm_end.x - right.x);//kmVec2DegreesBetween(&right, &end);
  float length = kmVec2DistanceBetween(&start, &end);
  kmVec2 zero;
  zero.x = 0;
  zero.y = 0;
  kmAABB2 rect;
  rect.min.x = 0;
  rect.min.y = 0;
  rect.max.x = length;
  rect.max.y = 1;
  kmAABB2Sanitize(&rect, &rect);

  kmMat4 rot;
  kmMat4RotationZ(&rot, angle);
  kmMat4Multiply(&viewMatrix, &rot, &viewMatrix);
  kmMat4 trans;
  kmMat4Translation(&trans, start.x, start.y, 0);
  kmMat4Multiply(&viewMatrix, &trans, &viewMatrix);

  //kmVec2 centre;
  //kmAABB2Centre(&rect, &centre);
  //kmVec2RotateBy(&rect.min, &rect.min, angle, &centre);
  //kmVec2RotateBy(&rect.max, &rect.max, angle, &centre);
  binocle_gd_draw_rect(gd, rect, col, viewport, viewMatrix);
}

void binocle_gd_draw_circle(binocle_gd *gd, kmVec2 center, float radius, binocle_color col, kmAABB2 viewport, kmMat4 viewMatrix) {
  static GLfloat vertex_buffer_data[2 * 3 * 32] = {0};

  int circle_segments = 32;
  float increment = M_PI * 2.0f / circle_segments;
  float theta = 0.0f;

  kmVec2 v0;
  v0.x = cosf(theta) * radius + center.x;
  v0.y = sinf(theta) * radius + center.y;
  theta += increment;

  int count = 0;
  for( int i = 1; i < circle_segments - 1; i++ )
  {
    kmVec2 v1;
    v1.x = cosf(theta) * radius + center.x;
    v1.y = sinf(theta) * radius + center.y;

    kmVec2 v2;
    v2.x = cosf(theta + increment) * radius + center.x;
    v2.y = sinf(theta + increment) * radius + center.y;

    vertex_buffer_data[count] = v0.x;
    count++;
    vertex_buffer_data[count] = v0.y;
    count++;
    vertex_buffer_data[count] = v1.x;
    count++;
    vertex_buffer_data[count] = v1.y;
    count++;
    vertex_buffer_data[count] = v2.x;
    count++;
    vertex_buffer_data[count] = v2.y;
    count++;

    theta += increment;
  }

  kmMat4 projectionMatrix = binocle_math_create_orthographic_matrix_off_center(viewport.min.x, viewport.max.x,
                                                                               viewport.min.y, viewport.max.y, -1000.0f,
                                                                               1000.0f);
  //kmMat4 viewMatrix;
  //kmMat4Identity(&viewMatrix);
  kmMat4 modelMatrix;
  kmMat4Identity(&modelMatrix);

  //GLuint quad_vertexbuffer;
  //glCheck(glGenBuffers(1, &quad_vertexbuffer));
  //glCheck(glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer));
  glCheck(glBindBuffer(GL_ARRAY_BUFFER, gd->vbo));
  glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_DYNAMIC_DRAW));
  glCheck(glUseProgram(binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_FLAT].program_id));
  GLint color_id;
  glCheck(color_id = glGetUniformLocation(binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_FLAT].program_id, "color"));
  GLint pos_id;
  glCheck(
    pos_id = glGetAttribLocation(binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_FLAT].program_id, "vertexPosition"));
  glCheck(glUniform4f(color_id, col.r, col.g, col.b, col.a));
  binocle_gd_apply_viewport(viewport);
  glCheck(glVertexAttribPointer(pos_id, 2, GL_FLOAT, false, sizeof(GLfloat) * 2, 0));
  glCheck(glEnableVertexAttribArray(pos_id));
  GLint projection_matrix_uniform;
  glCheck(
    projection_matrix_uniform = glGetUniformLocation(binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_FLAT].program_id,
                                                     "projectionMatrix"));
  GLint view_matrix_uniform;
  glCheck(view_matrix_uniform = glGetUniformLocation(binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_FLAT].program_id,
                                                     "viewMatrix"));
  GLint model_matrix_uniform;
  glCheck(model_matrix_uniform = glGetUniformLocation(binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_FLAT].program_id,
                                                      "modelMatrix"));
  glCheck(glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, projectionMatrix.mat));
  glCheck(glUniformMatrix4fv(view_matrix_uniform, 1, GL_FALSE, viewMatrix.mat));
  glCheck(glUniformMatrix4fv(model_matrix_uniform, 1, GL_FALSE, modelMatrix.mat));
  //glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));
  glCheck(glDrawArrays(GL_TRIANGLES, 0, 3 * 32));

  glCheck(glDisableVertexAttribArray(pos_id));
  glCheck(glUseProgram(GL_ZERO));
  glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));

}

void binocle_gd_draw_with_state(binocle_gd *gd, const binocle_vpct *vertices, size_t vertex_count, binocle_render_state *render_state) {
  binocle_gd_apply_gl_states();
  binocle_gd_apply_viewport(render_state->viewport);
  binocle_gd_apply_blend_mode(render_state->blend_mode);
  binocle_gd_apply_shader(gd, *render_state->shader);
  binocle_gd_apply_texture(*render_state->texture);

  kmMat4 projectionMatrix = binocle_math_create_orthographic_matrix_off_center(render_state->viewport.min.x, render_state->viewport.max.x,
                                                                               render_state->viewport.min.y, render_state->viewport.max.y, -1000.0f,
                                                                               1000.0f);
  //kmMat4 modelViewMatrix = binocle_gd_create_model_view_matrix(0.0f, 0.0f, 1.0f, 0.0f);
  kmMat4 viewMatrix;
  kmMat4Identity(&viewMatrix);
  // TODO: apply the viewport_adapter scale_matrix here
  //kmMat4Scaling(&viewMatrix, 2.0f, 2.0f, 2.0f);

  kmMat4 modelMatrix;
  kmMat4Identity(&modelMatrix);

  glCheck(glEnableVertexAttribArray(gd->vertex_attribute));
  glCheck(glEnableVertexAttribArray(gd->color_attribute));
  glCheck(glEnableVertexAttribArray(gd->tex_coord_attribute));

  glCheck(glBindBuffer(GL_ARRAY_BUFFER, gd->vbo));
  glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(binocle_vpct) * vertex_count, vertices, GL_STATIC_DRAW));

  glCheck(glVertexAttribPointer(gd->vertex_attribute, 2, GL_FLOAT, GL_FALSE, sizeof(binocle_vpct), 0));
  glCheck(glVertexAttribPointer(gd->color_attribute, 4, GL_FLOAT, GL_FALSE, sizeof(binocle_vpct),
                                (void *) (2 * sizeof(GLfloat))));
  glCheck(glVertexAttribPointer(gd->tex_coord_attribute, 2, GL_FLOAT, GL_FALSE, sizeof(binocle_vpct),
                                (void *) (4 * sizeof(GLfloat) + 2 * sizeof(GLfloat))));

  //kmMat4 finalMatrix = Matrix4::mul(state.transform,projectionMatrix);
  //kmMat4 inverseMatrix: Matrix4<f32> = Matrix4::from_nonuniform_scale(1.0, 1.0, 1.0);

  glCheck(glUniformMatrix4fv(gd->projection_matrix_uniform, 1, GL_FALSE, projectionMatrix.mat));
  //glCheck(glUniformMatrix4fv(gd->model_view_matrix_uniform, 1, GL_FALSE, modelViewMatrix.mat));
  glCheck(glUniformMatrix4fv(gd->view_matrix_uniform, 1, GL_FALSE, viewMatrix.mat));
  glCheck(glUniformMatrix4fv(gd->model_matrix_uniform, 1, GL_FALSE, modelMatrix.mat));

  glCheck(glUniform1i(gd->image_uniform, 0));

  glCheck(glDrawArrays(GL_TRIANGLES, 0, vertex_count));

  glCheck(glDisableVertexAttribArray(gd->vertex_attribute));
  glCheck(glDisableVertexAttribArray(gd->color_attribute));
  glCheck(glDisableVertexAttribArray(gd->tex_coord_attribute));
  glCheck(glUseProgram(GL_ZERO));

  glCheck(glBindTexture(GL_TEXTURE_2D, 0));
  glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));

}

void binocle_gd_draw_mesh(binocle_gd *gd, const struct binocle_mesh *mesh, kmAABB2 viewport, struct binocle_camera_3d *camera) {
  if (camera == NULL) {
    binocle_log_warning("Missing camera for call to binocle_gd_draw_mesh");
    return;
  }
  binocle_gd_apply_3d_gl_states();
  binocle_gd_apply_viewport(viewport);
  binocle_gd_apply_blend_mode(mesh->material->blend_mode);
  binocle_gd_apply_shader(gd, *mesh->material->shader);
  binocle_gd_apply_texture(*mesh->material->texture);

  kmMat4 projectionMatrix;
  kmMat4Identity(&projectionMatrix);
  /*
  kmMat4 projectionMatrix = binocle_math_create_orthographic_matrix_off_center(viewport.min.x, viewport.max.x,
                                                                               viewport.min.y, viewport.max.y, -1000.0f,
                                                                               1000.0f);
                                                                               */

  kmMat4PerspectiveProjection(&projectionMatrix, camera->fov_y, viewport.max.x / viewport.max.y, camera->near, camera->far);

  /*
  kmVec3 eye;
  eye.x = 0;
  eye.y = 0;
  eye.z = 0.0f;
  kmVec3 center;
  center.x = 0;
  center.y = 0;
  center.z = 0;
  kmVec3 up;
  up.x = 0;
  up.y = 1.0f;
  up.z = 0;
  kmMat4LookAt(&projectionMatrix, &eye, &center, &up);
  */

  kmMat4 viewMatrix;
  kmMat4Identity(&viewMatrix);
  /*
  kmMat4 trans;
  kmMat4Translation(&trans, 0, 0, -500.0f);
  kmMat4Multiply(&viewMatrix, &viewMatrix, &trans);
  */
  kmMat4Multiply(&viewMatrix, &viewMatrix, binocle_camera_3d_get_transform_matrix(camera));

  kmMat4 modelMatrix;
  modelMatrix = mesh->transform;
  /*
  kmMat4Identity(&modelMatrix);
  kmMat4 rot;
  kmVec3 axis;
  axis.x = 1.0f;
  axis.y = 0;
  axis.z = 0;
  kmMat4RotationAxisAngle(&rot, &axis, kmDegreesToRadians(-55.0f));
  kmMat4Multiply(&modelMatrix, &modelMatrix, &rot);
   */

  glCheck(glEnableVertexAttribArray(gd->vertex_attribute));
  glCheck(glEnableVertexAttribArray(gd->color_attribute));
  glCheck(glEnableVertexAttribArray(gd->tex_coord_attribute));

  glCheck(glBindBuffer(GL_ARRAY_BUFFER, gd->vbo));
  glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(binocle_vpctn) * mesh->vertex_count, mesh->vertices, GL_STATIC_DRAW));

  glCheck(glVertexAttribPointer(gd->vertex_attribute, 3, GL_FLOAT, GL_FALSE, sizeof(binocle_vpctn), 0));
  glCheck(glVertexAttribPointer(gd->color_attribute, 4, GL_FLOAT, GL_FALSE, sizeof(binocle_vpctn),
                                (void *) (3 * sizeof(GLfloat))));
  glCheck(glVertexAttribPointer(gd->tex_coord_attribute, 2, GL_FLOAT, GL_FALSE, sizeof(binocle_vpctn),
                                (void *) (4 * sizeof(GLfloat) + 3 * sizeof(GLfloat))));

  //kmMat4 finalMatrix = Matrix4::mul(state.transform,projectionMatrix);
  //kmMat4 inverseMatrix: Matrix4<f32> = Matrix4::from_nonuniform_scale(1.0, 1.0, 1.0);

  glCheck(glUniformMatrix4fv(gd->projection_matrix_uniform, 1, GL_FALSE, projectionMatrix.mat));
  //glCheck(glUniformMatrix4fv(gd->model_view_matrix_uniform, 1, GL_FALSE, modelViewMatrix.mat));
  glCheck(glUniformMatrix4fv(gd->view_matrix_uniform, 1, GL_FALSE, viewMatrix.mat));
  glCheck(glUniformMatrix4fv(gd->model_matrix_uniform, 1, GL_FALSE, modelMatrix.mat));

  glCheck(glUniform1i(gd->image_uniform, 0));

  glCheck(glDrawArrays(GL_TRIANGLES, 0, mesh->vertex_count));

  //GLuint quad_indexbuffer;
  //glCheck(glGenBuffers(1, &quad_indexbuffer));
  //glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_indexbuffer));
  //glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * mesh->triangle_count * 3, mesh->indices, GL_STATIC_DRAW));
  //glCheck(glDrawElements(GL_TRIANGLES, mesh->triangle_count * 3, GL_UNSIGNED_INT, 0));


  glCheck(glDisableVertexAttribArray(gd->vertex_attribute));
  glCheck(glDisableVertexAttribArray(gd->color_attribute));
  glCheck(glDisableVertexAttribArray(gd->tex_coord_attribute));
  glCheck(glUseProgram(GL_ZERO));

  glCheck(glBindTexture(GL_TEXTURE_2D, 0));
  glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));

  //glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
  //glCheck(glDeleteBuffers(1, &quad_indexbuffer));


}

void binocle_gd_draw_test_triangle(struct binocle_shader shader) {
  static const GLfloat g_quad_vertex_buffer_data[] = {
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.0f, 0.5f, 0.0f
  };

  GLuint quad_vertexbuffer;
  glCheck(glGenBuffers(1, &quad_vertexbuffer));
  glCheck(glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer));
  glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW));
  GLint pos_id;
  glCheck(pos_id = glGetAttribLocation(shader.program_id, "vertexPosition"));
  glCheck(glVertexAttribPointer(pos_id, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, 0));
  glCheck(glEnableVertexAttribArray(pos_id));
  glCheck(glDrawArrays(GL_TRIANGLES, 0, 3));

  glCheck(glDisableVertexAttribArray(pos_id));
  glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
  glCheck(glDeleteBuffers(1, &quad_vertexbuffer));
}

void binocle_gd_draw_test_cube(struct binocle_shader shader) {
  static GLfloat g_quad_vertex_buffer_data[] = {
    0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, 0.5f,
    -0.5f, -0.5f, 0.5f,
    -0.5f, -0.5f, -0.5f,
    0.5f, 0.5f, -0.5f,
    0.5f, 0.5f, 0.5f,
    -0.5f, 0.5f, 0.5f,
    -0.5f, 0.5f, -0.5f
  };

  static const GLuint index_buffer_data[] = {
    0, 1, 2,
    0, 2, 3,
    4, 7, 6,
    4, 6, 5,
    0, 4, 5,
    0, 5, 1,
    1, 5, 6,
    1, 6, 2,
    2, 6, 7,
    2, 7, 3,
    4, 0, 3,
    4, 3, 7
  };

  /*
  for (int i = 0 ; i < sizeof(g_quad_vertex_buffer_data) ; i++) {
    g_quad_vertex_buffer_data[i] /= 2;
  }
   */

  GLuint quad_vertexbuffer;
  glCheck(glGenBuffers(1, &quad_vertexbuffer));
  glCheck(glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer));
  glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW));
  GLint pos_id;
  glCheck(pos_id = glGetAttribLocation(shader.program_id, "vertexPosition"));
  glCheck(glVertexAttribPointer(pos_id, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, 0));
  glCheck(glEnableVertexAttribArray(pos_id));
  //glCheck(glDrawArrays(GL_TRIANGLES, 0, 8));

  GLuint quad_indexbuffer;
  glCheck(glGenBuffers(1, &quad_indexbuffer));
  glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_indexbuffer));
  glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW));
  glCheck(glDrawElements(GL_TRIANGLES, 12*3, GL_UNSIGNED_INT, 0));

  glCheck(glDisableVertexAttribArray(pos_id));
  glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
  glCheck(glDeleteBuffers(1, &quad_vertexbuffer));
  glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
  glCheck(glDeleteBuffers(1, &quad_indexbuffer));
}

