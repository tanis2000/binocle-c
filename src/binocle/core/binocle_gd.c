//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include <string.h>
#include "binocle_gd.h"
#include "backend/binocle_vpct.h"
#include "backend/binocle_material.h"
#include "binocle_viewport_adapter.h"
#include "binocle_render_state.h"
#include "backend/binocle_backend.h"
#include "binocle_camera.h"
#include "binocle_log.h"
#include "binocle_model.h"
#include "binocle_window.h"

typedef struct binocle_gd_flat_shader_vs_params_t {
  kmMat4 projectionMatrix;
  kmMat4 viewMatrix;
  kmMat4 modelMatrix;
} binocle_gd_flat_shader_vs_params_t;

typedef struct binocle_gd_flat_shader_fs_params_t {
  float color[4];
} binocle_gd_flat_shader_fs_params_t;

const char *binocle_shader_flat_vertex_src = "\
\n\
\
attribute vec3 vertexPosition;\
attribute vec4 vertexColor;\
\
uniform mat4 projectionMatrix;\
uniform mat4 viewMatrix;\
uniform mat4 modelMatrix;\
\
void main(void) {\
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);\
    gl_PointSize = 1.0;\
}\
\0";

const char *binocle_shader_flat_src = "\
\n\
uniform vec4 color;\
void main(void)\
{\
    gl_FragColor = color;\n\
    //gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n\
}\
\0";

binocle_gd binocle_gd_new() {
  binocle_gd res = {0};
  res.vertices = malloc(sizeof(binocle_vpct) * BINOCLE_GD_MAX_VERTICES);
  res.commands = malloc(sizeof(binocle_gd_command_t) * BINOCLE_GD_MAX_COMMANDS);
  return res;
}

void binocle_gd_destroy(binocle_gd *gd) {
  if (gd->vertices != NULL) {
    free(gd->vertices);
    gd->vertices = NULL;
  }

  if (gd->commands != NULL) {
    free(gd->commands);
    gd->commands = NULL;
  }

  // TODO: deinitialize the pipeline, pass and backend
}

void binocle_gd_init(binocle_gd *gd, binocle_window *win) {
#if defined(BINOCLE_GL)
  binocle_backend_desc desc = {0};
#elif defined(BINOCLE_METAL)
  binocle_backend_desc desc = {
    .context.mtl = win->mtl_view
  };
#endif
  binocle_backend_setup(&desc);
}

void binocle_gd_setup_default_pipeline(binocle_gd *gd, uint32_t offscreen_width, uint32_t offscreen_height, binocle_shader offscreen_shader, binocle_shader display_shader) {
  // Create the render target image
  binocle_image_desc rt_desc = {
      .render_target = true,
      .width = offscreen_width,
      .height = offscreen_height,
      .min_filter = BINOCLE_FILTER_LINEAR,
      .mag_filter = BINOCLE_FILTER_LINEAR,
#ifdef BINOCLE_GL
      .pixel_format = BINOCLE_PIXELFORMAT_RGBA8,
#else
      .pixel_format = BINOCLE_PIXELFORMAT_BGRA8,
#endif
      .sample_count = 1,
  };
  gd->offscreen.render_target = binocle_backend_make_image(&rt_desc);

  // Clear screen action for the offscreen render target
  binocle_color off_clear_color = binocle_color_azure();
  binocle_pass_action offscreen_action = {
      .colors[0] = {
          .action = BINOCLE_ACTION_CLEAR,
          .value = {
              .r = off_clear_color.r,
              .g = off_clear_color.g,
              .b = off_clear_color.b,
              .a = off_clear_color.a,
          }
      }
  };
  gd->offscreen.action = offscreen_action;

  // Render pass that renders to the offscreen render target
  gd->offscreen.pass = binocle_backend_make_pass(&(binocle_pass_desc){
      .color_attachments[0].image = gd->offscreen.render_target,
  });

  // Pipeline state object for the offscreen rendered sprite
  gd->offscreen.pip = binocle_backend_make_pipeline(&(binocle_pipeline_desc) {
      .layout = {
          .attrs = {
              [0] = { .format = BINOCLE_VERTEXFORMAT_FLOAT2 }, // position
              [1] = { .format = BINOCLE_VERTEXFORMAT_FLOAT4 }, // color
              [2] = { .format = BINOCLE_VERTEXFORMAT_FLOAT2 }, // texture uv
          },
      },
      .shader = offscreen_shader,
//      .index_type = BINOCLE_INDEXTYPE_UINT16,
      .index_type = BINOCLE_INDEXTYPE_NONE,
      .depth = {
          .pixel_format = BINOCLE_PIXELFORMAT_NONE,
          .compare = BINOCLE_COMPAREFUNC_NEVER,
          .write_enabled = false,
      },
      .stencil = {
          .enabled = false,
      },
      .colors = {
        [0] = {
#ifdef BINOCLE_GL
            .pixel_format = BINOCLE_PIXELFORMAT_RGBA8,
#else
            .pixel_format = BINOCLE_PIXELFORMAT_BGRA8,
#endif
          .blend = {
          .enabled = true,
          .src_factor_rgb = BINOCLE_BLENDFACTOR_SRC_ALPHA,
          .dst_factor_rgb = BINOCLE_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        }
        }
      }
  });

  binocle_buffer_desc vbuf_desc = {
    .type = BINOCLE_BUFFERTYPE_VERTEXBUFFER,
    .usage = BINOCLE_USAGE_STREAM,
    .size = sizeof(binocle_vpct) * BINOCLE_GD_MAX_VERTICES,
  };
  gd->offscreen.vbuf = binocle_backend_make_buffer(&vbuf_desc);

  binocle_buffer_desc ibuf_desc = {
    .type = BINOCLE_BUFFERTYPE_INDEXBUFFER,
    .usage = BINOCLE_USAGE_STREAM,
  };
  gd->offscreen.ibuf = binocle_backend_make_buffer(&ibuf_desc);

  gd->offscreen.bind = (binocle_bindings){
      .vertex_buffers = {
          [0] = gd->offscreen.vbuf,
      },
  };


  // Render to screen pipeline

  // Clear screen action for the actual screen
  binocle_color clear_color = binocle_color_green();
  binocle_pass_action default_action = {
    .colors[0] = {
      .action = BINOCLE_ACTION_CLEAR,
      .value = {
        .r = clear_color.r,
        .g = clear_color.g,
        .b = clear_color.b,
        .a = clear_color.a,
      }
    }
  };
  gd->display.action = default_action;

  // Pipeline state object for the screen (default) pass
  gd->display.pip = binocle_backend_make_pipeline(&(binocle_pipeline_desc){
    .layout = {
      .attrs = {
        [0] = { .format = BINOCLE_VERTEXFORMAT_FLOAT3 }, // position
        [1] = { .format = BINOCLE_VERTEXFORMAT_FLOAT4 }, // color
        [2] = { .format = BINOCLE_VERTEXFORMAT_FLOAT2 }, // texture uv
      }
    },
    .shader = display_shader,
    .index_type = BINOCLE_INDEXTYPE_UINT16,
#if !defined(BINOCLE_GL)
    .depth = {
      .pixel_format = BINOCLE_PIXELFORMAT_NONE,
      .compare = BINOCLE_COMPAREFUNC_NEVER,
      .write_enabled = false,
    },
    .stencil = {
      .enabled = false,
    },
#endif
    .colors = {
#ifdef BINOCLE_GL
      [0] = { .pixel_format = BINOCLE_PIXELFORMAT_RGBA8 }
#else
      [0] = { .pixel_format = BINOCLE_PIXELFORMAT_BGRA8 }
#endif
    }
  });

  float vertices[] = {
    /* pos                  color                       uvs */
    -1.0f, -1.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,     0.0f, 0.0f,
    1.0f, -1.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,     1.0f, 0.0f,
    1.0f,  1.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,     1.0f, 1.0f,
    -1.0f,  1.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,     0.0f, 1.0f,
  };
  binocle_buffer_desc display_vbuf_desc = {
    .data = BINOCLE_RANGE(vertices)
  };
  gd->display.vbuf = binocle_backend_make_buffer(&display_vbuf_desc);

  uint16_t indices[] = {
    0, 1, 2,  0, 2, 3,
  };
  binocle_buffer_desc display_ibuf_desc = {
    .type = BINOCLE_BUFFERTYPE_INDEXBUFFER,
    .data = BINOCLE_RANGE(indices)
  };
  gd->display.ibuf = binocle_backend_make_buffer(&display_ibuf_desc);

  gd->display.bind = (binocle_bindings){
    .vertex_buffers = {
      [0] = gd->display.vbuf,
    },
    .index_buffer = gd->display.ibuf,
    .fs_images = {
      [0] = gd->offscreen.render_target,
    }
  };
}

void binocle_gd_draw(binocle_gd *gd, const struct binocle_vpct *vertices, size_t vertex_count, binocle_material material,
                     kmAABB2 viewport, binocle_camera *camera) {
  binocle_gd_command_t *cmd = &gd->commands[gd->num_commands];
  cmd->num_vertices = vertex_count;
  cmd->base_vertex = gd->num_vertices;
  cmd->img = material.albedo_texture;

  kmMat4 *cameraTransformMatrix = NULL;
  if (camera != NULL) {
    cameraTransformMatrix = binocle_camera_get_transform_matrix(camera);
  }

  cmd->uniforms.projectionMatrix = binocle_math_create_orthographic_matrix_off_center(viewport.min.x, viewport.max.x,
                                                                                          viewport.min.y, viewport.max.y, -1000.0f,
                                                                                          1000.0f);
  kmMat4Identity(&cmd->uniforms.viewMatrix);

  if (cameraTransformMatrix != NULL) {
    kmMat4Multiply(&cmd->uniforms.viewMatrix, &cmd->uniforms.viewMatrix, cameraTransformMatrix);
  }

  kmMat4Identity(&cmd->uniforms.modelMatrix);

  gd->num_commands++;

  for (uint32_t i = 0 ; i < vertex_count ; i++) {
    binocle_vpct *v = &gd->vertices[gd->num_vertices];
    v->pos.x = vertices[i].pos.x;
    v->pos.y = vertices[i].pos.y;
    v->color.r = vertices[i].color.r;
    v->color.g = vertices[i].color.g;
    v->color.b = vertices[i].color.b;
    v->color.a = vertices[i].color.a;
    v->tex.x = vertices[i].tex.x;
    v->tex.y = vertices[i].tex.y;
    gd->num_vertices++;
  }

//  LEGACY_binocle_backend_draw(vertices, vertex_count, material, viewport, cameraTransformMatrix);
}

void binocle_gd_render_offscreen(binocle_gd *gd) {
  binocle_backend_update_buffer(gd->offscreen.vbuf, &(binocle_range){ .ptr=gd->vertices, .size=gd->num_vertices * sizeof(binocle_vpct) });

  binocle_backend_begin_pass(gd->offscreen.pass, &gd->offscreen.action);

  for (uint32_t i = 0 ; i < gd->num_commands ; i++) {
    binocle_gd_command_t *cmd = &gd->commands[i];

    gd->offscreen.bind.fs_images[0] = cmd->img;
    gd->offscreen.bind.vertex_buffers[0] = gd->offscreen.vbuf;

    binocle_backend_apply_pipeline(gd->offscreen.pip);
    binocle_backend_apply_bindings(&gd->offscreen.bind);
    binocle_backend_apply_uniforms(BINOCLE_SHADERSTAGE_VS, 0, &BINOCLE_RANGE(cmd->uniforms));
    binocle_backend_draw(cmd->base_vertex, cmd->num_vertices, 1);

  }
  binocle_backend_end_pass();
  gd->num_commands = 0;
  gd->num_vertices = 0;
}

void binocle_gd_render_screen(binocle_gd *gd, struct binocle_window *window, float design_width, float design_height, kmAABB2 viewport) {
  // Render the offscreen to the display

  typedef struct screen_vs_params_t {
    kmMat4 transform;
  } screen_vs_params_t;

  typedef struct screen_fs_params_t {
    float resolution[2];
    float scale[2];
    float viewport[2];
  } screen_fs_params_t;

  screen_vs_params_t screen_vs_params;
  screen_fs_params_t screen_fs_params;

  kmMat4Identity(&screen_vs_params.transform);

  screen_fs_params.resolution[0] = design_width;
  screen_fs_params.resolution[1] = design_height;
  screen_fs_params.scale[0] = 1;
  screen_fs_params.scale[1] = 1;
  screen_fs_params.viewport[0] = viewport.min.x;
  screen_fs_params.viewport[1] = viewport.min.y;


  gd->display.bind.fs_images[0] = gd->offscreen.render_target;

  binocle_backend_begin_default_pass(&gd->display.action, window->width, window->height);
  binocle_backend_apply_pipeline(gd->display.pip);
  binocle_backend_apply_bindings(&gd->display.bind);
  binocle_backend_apply_uniforms(BINOCLE_SHADERSTAGE_VS, 0, &BINOCLE_RANGE(screen_vs_params));
  binocle_backend_apply_uniforms(BINOCLE_SHADERSTAGE_FS, 0, &BINOCLE_RANGE(screen_fs_params));
  binocle_backend_draw(0, 6, 1);
  binocle_backend_end_pass();

  binocle_backend_commit();
}

void binocle_gd_render(binocle_gd *gd, struct binocle_window *window, float design_width, float design_height, kmAABB2 viewport) {
  binocle_gd_render_offscreen(gd);
  binocle_gd_render_screen(gd, window, design_width, design_height, viewport);
}

void binocle_gd_setup_flat_pipeline(binocle_gd *gd) {
  binocle_shader_desc screen_shader_desc = {
#ifdef BINOCLE_GL
    .vs.source = binocle_shader_flat_vertex_src,
    .vs.uniform_blocks[0] = {
      .size = sizeof(struct binocle_gd_flat_shader_vs_params_t),
      .uniforms = {
        [0] = { .name = "projectionMatrix", .type = BINOCLE_UNIFORMTYPE_MAT4 },
        [1] = { .name = "viewMatrix", .type = BINOCLE_UNIFORMTYPE_MAT4 },
        [2] = { .name = "modelMatrix", .type = BINOCLE_UNIFORMTYPE_MAT4 },
      },
    },
#else
    .vs.byte_code = screen_vs_bytecode,
    .vs.byte_code_size = sizeof(screen_vs_bytecode),
#endif
    .attrs = {
      [0].name = "vertexPosition",
    },
#ifdef BINOCLE_GL
    .fs.source = binocle_shader_flat_src,
#else
    .fs.byte_code = screen_fs_bytecode,
    .fs.byte_code_size = sizeof(screen_fs_bytecode),
#endif
    .fs.uniform_blocks[0] = {
      .size = sizeof(struct binocle_gd_flat_shader_fs_params_t),
      .uniforms = {
        [0] = { .name = "color", .type = BINOCLE_UNIFORMTYPE_FLOAT4 },
      },
    },
  };
  binocle_shader shader = binocle_backend_make_shader(&screen_shader_desc);

  binocle_pass_action action = {
    .colors[0] = {
      .action = BINOCLE_ACTION_DONTCARE,
      .value = {0.0f, 1.0f, 0.0f, 1.0f},
    }
  };
  gd->flat.action = action;

  gd->flat.pass = binocle_backend_make_pass(&(binocle_pass_desc){
    .color_attachments[0].image = gd->offscreen.render_target,
  });

  gd->flat.pip = binocle_backend_make_pipeline(&(binocle_pipeline_desc) {
    .layout = {
      .attrs = {
        [0] = { .format = BINOCLE_VERTEXFORMAT_FLOAT2 }, // position
      },
    },
    .shader = shader,
    .index_type = BINOCLE_INDEXTYPE_NONE,
    .depth = {
      .pixel_format = BINOCLE_PIXELFORMAT_NONE,
      .compare = BINOCLE_COMPAREFUNC_NEVER,
      .write_enabled = false,
    },
    .stencil = {
      .enabled = false,
    },
    .colors = {
      [0] = {
#ifdef BINOCLE_GL
        .pixel_format = BINOCLE_PIXELFORMAT_RGBA8,
#else
        .pixel_format = BINOCLE_PIXELFORMAT_BGRA8,
#endif
        .blend = {
          .enabled = false,
          .src_factor_rgb = BINOCLE_BLENDFACTOR_SRC_ALPHA,
          .dst_factor_rgb = BINOCLE_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        }
      }
    }
  });

  binocle_buffer_desc vbuf_desc = {
    .type = BINOCLE_BUFFERTYPE_VERTEXBUFFER,
    .usage = BINOCLE_USAGE_STREAM,
    .size = sizeof(float) * 2 * 1024,
  };
  gd->flat.vbuf = binocle_backend_make_buffer(&vbuf_desc);

  binocle_buffer_desc ibuf_desc = {
    .type = BINOCLE_BUFFERTYPE_INDEXBUFFER,
    .usage = BINOCLE_USAGE_STREAM,
  };
  gd->flat.ibuf = binocle_backend_make_buffer(&ibuf_desc);

  gd->flat.bind = (binocle_bindings){
    .vertex_buffers = {
      [0] = gd->flat.vbuf,
    },
  };
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

void binocle_gd_apply_viewport(kmAABB2 viewport) {
  binocle_backend_apply_viewport(viewport.min.x, viewport.min.y, viewport.max.x, viewport.max.y);
}

void binocle_gd_apply_blend_mode(const binocle_blend blend_mode) {
  binocle_backend_apply_blend_mode(blend_mode);
}

void binocle_gd_apply_shader(binocle_gd *gd, binocle_shader shader) {
  binocle_backend_apply_shader(shader);
}

void binocle_gd_apply_texture(binocle_image texture) {
  binocle_backend_apply_texture(texture);
}

void binocle_gd_apply_3d_texture(binocle_material *material) {
  binocle_backend_apply_3d_texture(material);
}

void binocle_gd_set_uniform_float(struct binocle_shader *shader, const char *name, float value) {
//  GLint id = 0;
//  glCheck(id = glGetUniformLocation(shader->program_id, name));
//  if (id == -1) {
//    binocle_log_error("Cannot find uniform called %s", name);
//    return;
//  }
//  glCheck(glUniform1f(id, value));
}

void binocle_gd_set_uniform_float2(binocle_shader shader, const char *name, float value1, float value2) {
  binocle_backend_set_uniform_float2(shader, name, value1, value2);
}

void binocle_gd_set_uniform_float3(struct binocle_shader *shader, const char *name, float value1, float value2,
                                   float value3) {
//  GLint id = 0;
//  glCheck(id = glGetUniformLocation(shader->program_id, name));
//  if (id == -1) {
//    binocle_log_error("Cannot find uniform called %s", name);
//    return;
//  }
//  glCheck(glUniform3f(id, value1, value2, value3));
}

void
binocle_gd_set_uniform_float4(struct binocle_shader *shader, const char *name, float value1, float value2, float value3,
                              float value4) {
//  GLint id = 0;
//  glCheck(id = glGetUniformLocation(shader->program_id, name));
//  if (id == -1) {
//    binocle_log_error("Cannot find uniform called %s", name);
//    return;
//  }
//  glCheck(glUniform4f(id, value1, value2, value3, value4));
}

void binocle_gd_set_uniform_render_target_as_texture(struct binocle_shader *shader, const char *name,
                                                     binocle_image render_target) {
//  GLint id = 0;
//  glCheck(id = glGetUniformLocation(shader->program_id, name));
//  if (id == -1) {
//    binocle_log_error("Cannot find uniform called %s", name);
//    return;
//  }
//  glCheck(glUniform1i(id, 0));
//  glCheck(glActiveTexture(GL_TEXTURE0));
//  glCheck(glBindTexture(GL_TEXTURE_2D, render_target.texture));
}

void binocle_gd_set_uniform_vec3(struct binocle_shader *shader, const char *name, kmVec3 vec) {
//  GLint id = 0;
//  glCheck(id = glGetUniformLocation(shader->program_id, name));
//  if (id == -1) {
//    binocle_log_error("Cannot find uniform called %s", name);
//    return;
//  }
//  glCheck(glUniform3fv(id, 1, (GLfloat *)&vec));
}

void binocle_gd_set_uniform_mat4(binocle_shader shader, const char *name, kmMat4 mat) {
  binocle_backend_set_uniform_mat4(shader, name, mat);
}

void binocle_gd_clear(struct binocle_color color) {
  binocle_backend_clear(color);
}

void binocle_gd_set_render_target(binocle_image render_target) {
  binocle_backend_set_render_target(render_target);
}

void binocle_gd_draw_quad(binocle_gd *gd, binocle_image image) {
  static const GLfloat g_quad_vertex_buffer_data[] = {
      -1.0f, -1.0f,
      1.0f, -1.0f,
      -1.0f, 1.0f,
      -1.0f, 1.0f,
      1.0f, -1.0f,
      1.0f, 1.0f,
  };

//
//  GLuint quad_vertexbuffer;
//  glCheck(glGenBuffers(1, &quad_vertexbuffer));
//  glCheck(glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer));
//  glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW));
//  GLint pos_id;
//  glCheck(pos_id = glGetAttribLocation(shader->program_id, "position"));
//  glCheck(glVertexAttribPointer(pos_id, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, 0));
//  glCheck(glEnableVertexAttribArray(pos_id));
//  glCheck(glDrawArrays(GL_TRIANGLES, 0, 6));
//
//  glCheck(glDisableVertexAttribArray(pos_id));
//  glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
//  glCheck(glDeleteBuffers(1, &quad_vertexbuffer));
}

void binocle_gd_draw_quad_to_screen(binocle_shader shader, binocle_image render_target) {
  binocle_backend_draw_quad_to_screen(shader, render_target);
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

  binocle_gd_flat_shader_vs_params_t vs_params = {
    .modelMatrix = modelMatrix,
    .projectionMatrix = projectionMatrix,
    .viewMatrix = viewMatrix,
  };
  binocle_gd_flat_shader_fs_params_t fs_params = {
    .color[0] = col.r,
    .color[1] = col.g,
    .color[2] = col.b,
    .color[3] = col.a,
  };
  int offset = binocle_backend_append_buffer(gd->flat.vbuf, &(binocle_range){.ptr = g_quad_vertex_buffer_data, .size=sizeof(float)*6*2});
  gd->flat.bind.vertex_buffer_offsets[0] = offset;
  binocle_backend_begin_pass(gd->flat.pass, &gd->flat.action);
  binocle_backend_apply_pipeline(gd->flat.pip);
  binocle_backend_apply_bindings(&gd->flat.bind);
  binocle_backend_apply_uniforms(BINOCLE_SHADERSTAGE_VS, 0, BINOCLE_RANGE_REF(vs_params));
  binocle_backend_apply_uniforms(BINOCLE_SHADERSTAGE_FS, 0, BINOCLE_RANGE_REF(fs_params));
  binocle_backend_draw(0, 6, 1);
  binocle_backend_end_pass();

//
//  //GLuint quad_vertexbuffer;
//  //glCheck(glGenBuffers(1, &quad_vertexbuffer));
//  //glCheck(glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer));
//  glCheck(glBindBuffer(GL_ARRAY_BUFFER, gd->vbo));
//  glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_DYNAMIC_DRAW));
//  glCheck(glUseProgram(binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_FLAT].program_id));
//  GLint color_id;
//  glCheck(color_id = glGetUniformLocation(binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_FLAT].program_id, "color"));
//  GLint pos_id;
//  glCheck(
//      pos_id = glGetAttribLocation(binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_FLAT].program_id, "vertexPosition"));
//  glCheck(glUniform4f(color_id, col.r, col.g, col.b, col.a));
//  binocle_gd_apply_viewport(viewport);
//  glCheck(glVertexAttribPointer(pos_id, 2, GL_FLOAT, false, sizeof(GLfloat) * 2, 0));
//  glCheck(glEnableVertexAttribArray(pos_id));
//  GLint projection_matrix_uniform;
//  glCheck(
//      projection_matrix_uniform = glGetUniformLocation(binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_FLAT].program_id,
//                                                       "projectionMatrix"));
//  GLint view_matrix_uniform;
//  glCheck(view_matrix_uniform = glGetUniformLocation(binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_FLAT].program_id,
//                                                     "viewMatrix"));
//  GLint model_matrix_uniform;
//  glCheck(model_matrix_uniform = glGetUniformLocation(binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_FLAT].program_id,
//                                                      "modelMatrix"));
//  glCheck(glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, projectionMatrix.mat));
//  glCheck(glUniformMatrix4fv(view_matrix_uniform, 1, GL_FALSE, viewMatrix.mat));
//  glCheck(glUniformMatrix4fv(model_matrix_uniform, 1, GL_FALSE, modelMatrix.mat));
//  //glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));
//  glCheck(glDrawArrays(GL_TRIANGLES, 0, 6));
//
//  glCheck(glDisableVertexAttribArray(pos_id));
//  glCheck(glUseProgram(GL_ZERO));
//  glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
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

  float angle = atan2f(end.y - start.y, end.x - start.x);
  float length = kmVec2DistanceBetween(&start, &end);
  kmAABB2 rect;
  rect.min.x = 0;
  rect.min.y = 0;
  rect.max.x = 1;
  rect.max.y = 1;

  kmMat4 trans;
  kmMat4Translation(&trans, start.x, start.y, 0);

  kmMat4 scale;
  kmMat4Scaling(&scale, length, 1, 0); // y is the thickness of the line

  kmMat4 rot;
  kmMat4RotationZ(&rot, angle);

  kmMat4Multiply(&viewMatrix, &viewMatrix, &trans);
  kmMat4Multiply(&viewMatrix, &viewMatrix, &rot);
  kmMat4Multiply(&viewMatrix, &viewMatrix, &scale);

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

  binocle_gd_flat_shader_vs_params_t vs_params = {
    .modelMatrix = modelMatrix,
    .projectionMatrix = projectionMatrix,
    .viewMatrix = viewMatrix,
  };
  binocle_gd_flat_shader_fs_params_t fs_params = {
    .color[0] = col.r,
    .color[1] = col.g,
    .color[2] = col.b,
    .color[3] = col.a,
  };
  int offset = binocle_backend_append_buffer(gd->flat.vbuf, BINOCLE_RANGE_REF(vertex_buffer_data));
  gd->flat.bind.vertex_buffer_offsets[0] = offset;
  binocle_backend_begin_pass(gd->flat.pass, &gd->flat.action);
  binocle_backend_apply_pipeline(gd->flat.pip);
  binocle_backend_apply_bindings(&gd->flat.bind);
  binocle_backend_apply_uniforms(BINOCLE_SHADERSTAGE_VS, 0, BINOCLE_RANGE_REF(vs_params));
  binocle_backend_apply_uniforms(BINOCLE_SHADERSTAGE_FS, 0, BINOCLE_RANGE_REF(fs_params));
  binocle_backend_draw(0, 3 * 32, 1);
  binocle_backend_end_pass();








//
//  //GLuint quad_vertexbuffer;
//  //glCheck(glGenBuffers(1, &quad_vertexbuffer));
//  //glCheck(glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer));
//  glCheck(glBindBuffer(GL_ARRAY_BUFFER, gd->vbo));
//  glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_DYNAMIC_DRAW));
//  glCheck(glUseProgram(binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_FLAT].program_id));
//  GLint color_id;
//  glCheck(color_id = glGetUniformLocation(binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_FLAT].program_id, "color"));
//  GLint pos_id;
//  glCheck(
//    pos_id = glGetAttribLocation(binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_FLAT].program_id, "vertexPosition"));
//  glCheck(glUniform4f(color_id, col.r, col.g, col.b, col.a));
//  binocle_gd_apply_viewport(viewport);
//  glCheck(glVertexAttribPointer(pos_id, 2, GL_FLOAT, false, sizeof(GLfloat) * 2, 0));
//  glCheck(glEnableVertexAttribArray(pos_id));
//  GLint projection_matrix_uniform;
//  glCheck(
//    projection_matrix_uniform = glGetUniformLocation(binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_FLAT].program_id,
//                                                     "projectionMatrix"));
//  GLint view_matrix_uniform;
//  glCheck(view_matrix_uniform = glGetUniformLocation(binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_FLAT].program_id,
//                                                     "viewMatrix"));
//  GLint model_matrix_uniform;
//  glCheck(model_matrix_uniform = glGetUniformLocation(binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_FLAT].program_id,
//                                                      "modelMatrix"));
//  glCheck(glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, projectionMatrix.mat));
//  glCheck(glUniformMatrix4fv(view_matrix_uniform, 1, GL_FALSE, viewMatrix.mat));
//  glCheck(glUniformMatrix4fv(model_matrix_uniform, 1, GL_FALSE, modelMatrix.mat));
//  //glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));
//  glCheck(glDrawArrays(GL_TRIANGLES, 0, 3 * 32));
//
//  glCheck(glDisableVertexAttribArray(pos_id));
//  glCheck(glUseProgram(GL_ZERO));
//  glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));

}

void binocle_gd_draw_with_state(binocle_gd *gd, const binocle_vpct *vertices, size_t vertex_count, binocle_render_state *render_state) {
  binocle_gd_command_t *cmd = &gd->commands[gd->num_commands];
  cmd->num_vertices = vertex_count;
  cmd->base_vertex = gd->num_vertices;
  cmd->img = *render_state->texture;

  kmMat4 cameraTransformMatrix = render_state->transform;

  cmd->uniforms.projectionMatrix = binocle_math_create_orthographic_matrix_off_center(render_state->viewport.min.x, render_state->viewport.max.x,
                                                                                      render_state->viewport.min.y, render_state->viewport.max.y, -1000.0f,
                                                                                      1000.0f);
  kmMat4Identity(&cmd->uniforms.viewMatrix);
  kmMat4Multiply(&cmd->uniforms.viewMatrix, &cmd->uniforms.viewMatrix, &cameraTransformMatrix);

  kmMat4Identity(&cmd->uniforms.modelMatrix);

  gd->num_commands++;

  for (uint32_t i = 0 ; i < vertex_count ; i++) {
    binocle_vpct *v = &gd->vertices[gd->num_vertices];
    v->pos.x = vertices[i].pos.x;
    v->pos.y = vertices[i].pos.y;
    v->color.r = vertices[i].color.r;
    v->color.g = vertices[i].color.g;
    v->color.b = vertices[i].color.b;
    v->color.a = vertices[i].color.a;
    v->tex.x = vertices[i].tex.x;
    v->tex.y = vertices[i].tex.y;
    gd->num_vertices++;
  }
}

void binocle_gd_draw_mesh(binocle_gd *gd, const struct binocle_mesh *mesh, kmAABB2 viewport, struct binocle_camera_3d *camera) {
//  if (camera == NULL) {
//    binocle_log_warning("Missing camera for call to binocle_gd_draw_mesh");
//    return;
//  }
//  binocle_gd_apply_3d_gl_states();
//  binocle_gd_apply_viewport(viewport);
//  binocle_gd_apply_blend_mode(mesh->material->blend_mode);
//  binocle_gd_apply_shader(gd, mesh->material->shader);
//  binocle_gd_apply_3d_texture(mesh->material);
//
//  kmMat4 projectionMatrix;
//  kmMat4Identity(&projectionMatrix);
//  /*
//  kmMat4 projectionMatrix = binocle_math_create_orthographic_matrix_off_center(viewport.min.x, viewport.max.x,
//                                                                               viewport.min.y, viewport.max.y, -1000.0f,
//                                                                               1000.0f);
//                                                                               */
//
//  kmMat4PerspectiveProjection(&projectionMatrix, camera->fov_y, viewport.max.x / viewport.max.y, camera->near_distance /*camera->near + camera->position.z*/, camera->far_distance /*camera->position.z + camera->far*/);
//
//  /*
//  kmVec3 eye;
//  eye.x = 0;
//  eye.y = 0;
//  eye.z = 0.0f;
//  kmVec3 center;
//  center.x = 0;
//  center.y = 0;
//  center.z = 0;
//  kmVec3 up;
//  up.x = 0;
//  up.y = 1.0f;
//  up.z = 0;
//  kmMat4LookAt(&projectionMatrix, &eye, &center, &up);
//  */
//
//  kmMat4 viewMatrix;
//  kmMat4Identity(&viewMatrix);
//  /*
//  kmMat4 trans;
//  kmMat4Translation(&trans, 0, 0, -500.0f);
//  kmMat4Multiply(&viewMatrix, &viewMatrix, &trans);
//  */
//  kmMat4Multiply(&viewMatrix, &viewMatrix, binocle_camera_3d_get_transform_matrix(camera));
//
//  kmMat4 modelMatrix;
//  modelMatrix = mesh->transform;
//  /*
//  kmMat4Identity(&modelMatrix);
//  kmMat4 rot;
//  kmVec3 axis;
//  axis.x = 1.0f;
//  axis.y = 0;
//  axis.z = 0;
//  kmMat4RotationAxisAngle(&rot, &axis, kmDegreesToRadians(-55.0f));
//  kmMat4Multiply(&modelMatrix, &modelMatrix, &rot);
//   */
//
//  glCheck(glEnableVertexAttribArray(gd->vertex_attribute));
//  glCheck(glEnableVertexAttribArray(gd->color_attribute));
//  glCheck(glEnableVertexAttribArray(gd->tex_coord_attribute));
//  glCheck(glEnableVertexAttribArray(gd->normal_attribute));
//
//  glCheck(glBindBuffer(GL_ARRAY_BUFFER, gd->vbo));
//  glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(binocle_vpctn) * mesh->vertex_count, mesh->vertices, GL_STATIC_DRAW));
//
//  glCheck(glVertexAttribPointer(gd->vertex_attribute, 3, GL_FLOAT, GL_FALSE, sizeof(binocle_vpctn), 0));
//  glCheck(glVertexAttribPointer(gd->color_attribute, 4, GL_FLOAT, GL_FALSE, sizeof(binocle_vpctn),
//                                (void *) (3 * sizeof(GLfloat))));
//  glCheck(glVertexAttribPointer(gd->tex_coord_attribute, 2, GL_FLOAT, GL_FALSE, sizeof(binocle_vpctn),
//                                (void *) (4 * sizeof(GLfloat) + 3 * sizeof(GLfloat))));
//  glCheck(glVertexAttribPointer(gd->normal_attribute, 3, GL_FLOAT, GL_FALSE, sizeof(binocle_vpctn),
//                                (void *) (4 * sizeof(GLfloat) + 3 * sizeof(GLfloat) + 2 * sizeof(GLfloat))));
//
//  //kmMat4 finalMatrix = Matrix4::mul(state.transform,projectionMatrix);
//  //kmMat4 inverseMatrix: Matrix4<f32> = Matrix4::from_nonuniform_scale(1.0, 1.0, 1.0);
//
//  glCheck(glUniformMatrix4fv(gd->projection_matrix_uniform, 1, GL_FALSE, projectionMatrix.mat));
//  //glCheck(glUniformMatrix4fv(gd->model_view_matrix_uniform, 1, GL_FALSE, modelViewMatrix.mat));
//  glCheck(glUniformMatrix4fv(gd->view_matrix_uniform, 1, GL_FALSE, viewMatrix.mat));
//  glCheck(glUniformMatrix4fv(gd->model_matrix_uniform, 1, GL_FALSE, modelMatrix.mat));
//
//  glCheck(glUniform1i(gd->image_uniform, 0));
//
//  glCheck(glDrawArrays(GL_TRIANGLES, 0, mesh->vertex_count));
//
//  //GLuint quad_indexbuffer;
//  //glCheck(glGenBuffers(1, &quad_indexbuffer));
//  //glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_indexbuffer));
//  //glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * mesh->triangle_count * 3, mesh->indices, GL_STATIC_DRAW));
//  //glCheck(glDrawElements(GL_TRIANGLES, mesh->triangle_count * 3, GL_UNSIGNED_INT, 0));
//
//
//  glCheck(glDisableVertexAttribArray(gd->vertex_attribute));
//  glCheck(glDisableVertexAttribArray(gd->color_attribute));
//  glCheck(glDisableVertexAttribArray(gd->tex_coord_attribute));
//  glCheck(glDisableVertexAttribArray(gd->normal_attribute));
//  glCheck(glUseProgram(GL_ZERO));
//
//  glCheck(glBindTexture(GL_TEXTURE_2D, 0));
//  glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
//
//  //glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
//  //glCheck(glDeleteBuffers(1, &quad_indexbuffer));
//

}

void binocle_gd_draw_test_triangle(struct binocle_shader *shader) {
//  static const GLfloat g_quad_vertex_buffer_data[] = {
//    -0.5f, -0.5f, 0.0f,
//    0.5f, -0.5f, 0.0f,
//    0.0f, 0.5f, 0.0f
//  };
//
//  GLuint quad_vertexbuffer;
//  glCheck(glGenBuffers(1, &quad_vertexbuffer));
//  glCheck(glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer));
//  glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW));
//  GLint pos_id;
//  glCheck(pos_id = glGetAttribLocation(shader->program_id, "vertexPosition"));
//  glCheck(glVertexAttribPointer(pos_id, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, 0));
//  glCheck(glEnableVertexAttribArray(pos_id));
//  glCheck(glDrawArrays(GL_TRIANGLES, 0, 3));
//
//  glCheck(glDisableVertexAttribArray(pos_id));
//  glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
//  glCheck(glDeleteBuffers(1, &quad_vertexbuffer));
}

void binocle_gd_draw_test_cube(struct binocle_shader *shader) {
//  static GLfloat g_quad_vertex_buffer_data[] = {
//    0.5f, -0.5f, -0.5f,
//    0.5f, -0.5f, 0.5f,
//    -0.5f, -0.5f, 0.5f,
//    -0.5f, -0.5f, -0.5f,
//    0.5f, 0.5f, -0.5f,
//    0.5f, 0.5f, 0.5f,
//    -0.5f, 0.5f, 0.5f,
//    -0.5f, 0.5f, -0.5f
//  };
//
//  static const GLuint index_buffer_data[] = {
//    0, 1, 2,
//    0, 2, 3,
//    4, 7, 6,
//    4, 6, 5,
//    0, 4, 5,
//    0, 5, 1,
//    1, 5, 6,
//    1, 6, 2,
//    2, 6, 7,
//    2, 7, 3,
//    4, 0, 3,
//    4, 3, 7
//  };
//
//  /*
//  for (int i = 0 ; i < sizeof(g_quad_vertex_buffer_data) ; i++) {
//    g_quad_vertex_buffer_data[i] /= 2;
//  }
//   */
//
//  GLuint quad_vertexbuffer;
//  glCheck(glGenBuffers(1, &quad_vertexbuffer));
//  glCheck(glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer));
//  glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW));
//  GLint pos_id;
//  glCheck(pos_id = glGetAttribLocation(shader->program_id, "vertexPosition"));
//  glCheck(glVertexAttribPointer(pos_id, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, 0));
//  glCheck(glEnableVertexAttribArray(pos_id));
//  //glCheck(glDrawArrays(GL_TRIANGLES, 0, 8));
//
//  GLuint quad_indexbuffer;
//  glCheck(glGenBuffers(1, &quad_indexbuffer));
//  glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_indexbuffer));
//  glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW));
//  glCheck(glDrawElements(GL_TRIANGLES, 12*3, GL_UNSIGNED_INT, 0));
//
//  glCheck(glDisableVertexAttribArray(pos_id));
//  glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
//  glCheck(glDeleteBuffers(1, &quad_vertexbuffer));
//  glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
//  glCheck(glDeleteBuffers(1, &quad_indexbuffer));
}

