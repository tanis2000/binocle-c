//
// Created by Valerio Santinelli on 19/04/18.
//

#include <string.h>
#include "binocle_gd.h"
#include "binocle_vpct.h"
#include "binocle_material.h"
#include "binocle_viewport_adapter.h"
#include "binocle_shader.h"
#include "binocle_texture.h"
#include "binocle_camera.h"

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
  // not sure why
  glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, res.texture, 0));

  // set up renderbuffer (depth buffer)
  glCheck(glBindRenderbuffer(GL_RENDERBUFFER, res.render_buffer));
  glCheck(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height));
  glCheck(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, res.render_buffer));

  // clean up
  glCheck(glBindTexture(GL_TEXTURE_2D, 0));
  glCheck(glBindRenderbuffer(GL_RENDERBUFFER, 0));
  glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));

  return res;
}

void binocle_gd_set_uniform_float(struct binocle_shader shader, const char *name, float value) {
  GLint id = glGetUniformLocation(shader.program_id, name);
  glCheck(glUniform1f(id, value));
}

void binocle_gd_set_uniform_float2(struct binocle_shader shader, const char *name, float value1, float value2) {
  GLint id = glGetUniformLocation(shader.program_id, name);
  glCheck(glUniform2f(id, value1, value2));
}

void binocle_gd_set_uniform_float3(struct binocle_shader shader, const char *name, float value1, float value2, float value3) {
  GLint id = glGetUniformLocation(shader.program_id, name);
  glCheck(glUniform3f(id, value1, value2, value3));
}

void binocle_gd_set_uniform_float4(struct binocle_shader shader, const char *name, float value1, float value2, float value3, float value4) {
  GLint id = glGetUniformLocation(shader.program_id, name);
  glCheck(glUniform4f(id, value1, value2, value3, value4));
}

void binocle_gd_set_uniform_render_target_as_texture(struct binocle_shader shader, const char *name,
                                                     binocle_render_target render_target) {
  GLint id = glGetUniformLocation(shader.program_id, name);
  glCheck(glUniform1i(id, 0));
  glCheck(glActiveTexture(GL_TEXTURE0));
  glCheck(glBindTexture(GL_TEXTURE_2D, render_target.texture));
}

void binocle_gd_set_uniform_mat4(struct binocle_shader shader, const char *name, kmMat4 mat) {
  GLint id = glGetUniformLocation(shader.program_id, name);
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
  GLint id = glGetAttribLocation(shader.program_id, "position");
  glCheck(glVertexAttribPointer(id, 2, GL_FLOAT, false, 0, 0));
  glCheck(glEnableVertexAttribArray(id));
  glCheck(glDrawArrays(GL_TRIANGLES, 0, 6));
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
  GLint id = glGetAttribLocation(shader.program_id, "position");
  glCheck(glVertexAttribPointer(id, 2, GL_FLOAT, false, 0, 0));
  glCheck(glEnableVertexAttribArray(id));
  id = glGetUniformLocation(shader.program_id, "texture");
  glCheck(glUniform1i(id, 0));
  glCheck(glActiveTexture(GL_TEXTURE0));
  glCheck(glBindTexture(GL_TEXTURE_2D, render_target.texture));
  // Sets the frame buffer to use as the screen
#if defined(__IPHONEOS__)
  SDL_SysWMinfo info;
  SDL_VERSION(&info.version);
  SDL_GetWindowWMInfo(SDL_GL_GetCurrentWindow(), &info);
  id = info.info.uikit.framebuffer;
  glCheck(glBindFramebuffer(GL_FRAMEBUFFER, id));
#else
  glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));
#endif
  glCheck(glDrawArrays(GL_TRIANGLES, 0, 6));
  glCheck(glDeleteBuffers(1, &quad_vertexbuffer));
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
  glCheck(projection_matrix_uniform = glGetUniformLocation(binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_FLAT].program_id, "projectionMatrix"));
  GLint view_matrix_uniform;
  glCheck(view_matrix_uniform = glGetUniformLocation(binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_FLAT].program_id, "viewMatrix"));
  GLint model_matrix_uniform;
  glCheck(model_matrix_uniform = glGetUniformLocation(binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_FLAT].program_id, "modelMatrix"));
  glCheck(glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, projectionMatrix.mat));
  glCheck(glUniformMatrix4fv(view_matrix_uniform, 1, GL_FALSE, viewMatrix.mat));
  glCheck(glUniformMatrix4fv(model_matrix_uniform, 1, GL_FALSE, modelMatrix.mat));
  //glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));
  glCheck(glDrawArrays(GL_TRIANGLES, 0, 6));

  glCheck(glDisableVertexAttribArray(pos_id));
  glCheck(glUseProgram(GL_ZERO));
  glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
}
