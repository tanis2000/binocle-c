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
#include "binocle_camera.h"
#include "binocle_log.h"
#include "binocle_model.h"
#include "binocle_window.h"
#include "binocle_sokol.h"
#include <ksort/ksort.h>

typedef struct binocle_gd_flat_shader_vs_params_t {
  kmMat4 projectionMatrix;
  kmMat4 viewMatrix;
  kmMat4 modelMatrix;
} binocle_gd_flat_shader_vs_params_t;

typedef struct binocle_gd_flat_shader_fs_params_t {
  float color[4];
} binocle_gd_flat_shader_fs_params_t;

const char *binocle_shader_flat_vertex_src_gles =
"#version 300 es\n"
"precision mediump float;\n"
"precision mediump int;\n"
"in vec3 vertexPosition;\n"
"in vec4 vertexColor;\n"
"in vec2 vertexTexture;\n"
"\n"
"uniform mat4 projectionMatrix;\n"
"uniform mat4 viewMatrix;\n"
"uniform mat4 modelMatrix;\n"
"\n"
"out vec4 color;\n"
"\n"
"void main(void) {\n"
"    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);\n"
"    gl_PointSize = 1.0;\n"
"    color = vertexColor;\n"
"}\n";

const char *binocle_shader_flat_frag_src_gles =
"#version 300 es\n"
"precision mediump float;\n"
"precision mediump int;\n"
"in vec4 color;\n"
"out vec4 fragColor;\n"
"void main(void)\n"
"{\n"
"    fragColor = color;\n"
"    //gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
"}\n";

const char *binocle_shader_flat_vertex_src_gl33 =
"#version 330\n"
"in vec3 vertexPosition;\n"
"in vec4 vertexColor;\n"
"in vec2 vertexTexture;\n"
"\n"
"uniform mat4 projectionMatrix;\n"
"uniform mat4 viewMatrix;\n"
"uniform mat4 modelMatrix;\n"
"\n"
"out vec4 color;"
"\n"
"void main(void) {\n"
"    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);\n"
"    //gl_Position = vec4(0.0, 0.0, 0.0, 1.0);\n"
"    gl_PointSize = 1.0;\n"
"    color = vertexColor;\n"
"}\n";

const char *binocle_shader_flat_src_gl33 =
"#version 330\n"
"in vec4 color;\n"
"out vec4 fragColor;\n"
"void main(void)\n"
"{\n"
"    fragColor = color;\n"
"    //fragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
"}\n";

binocle_gd binocle_gd_new() {
  binocle_gd res = {0};
  res.vertices = malloc(sizeof(binocle_vpct) * BINOCLE_GD_MAX_VERTICES);
  res.commands = malloc(sizeof(binocle_gd_command_t) * BINOCLE_GD_MAX_COMMANDS);
  memset(res.commands, 0, sizeof(binocle_gd_command_t) * BINOCLE_GD_MAX_COMMANDS);
  res.flat_vertices = malloc(sizeof(binocle_vpct) * BINOCLE_GD_MAX_VERTICES);
  res.flat_commands = malloc(sizeof(binocle_gd_command_t) * BINOCLE_GD_MAX_COMMANDS);
  memset(res.flat_commands, 0, sizeof(binocle_gd_command_t) * BINOCLE_GD_MAX_COMMANDS);
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

  if (gd->flat_vertices != NULL) {
    free(gd->flat_vertices);
    gd->flat_vertices = NULL;
  }

  if (gd->flat_commands != NULL) {
    free(gd->flat_commands);
    gd->flat_commands = NULL;
  }

  // TODO: deinitialize the pipeline, pass and backend
}

void binocle_gd_init(binocle_gd *gd, binocle_window *win) {
#if defined(BINOCLE_GL)
  sg_desc desc = {
    .logger.func = slog_func,
  };
#elif defined(BINOCLE_METAL)
  binocle_metal_init(win->mtl_view);
  sg_desc desc = {
    .context = binocle_metal_get_context(),
    .logger.func = slog_func,
  };
#endif
  sg_setup(&desc);
  assert(sg_isvalid());
}

void binocle_gd_setup_default_pipeline(binocle_gd *gd, uint32_t offscreen_width, uint32_t offscreen_height, sg_shader offscreen_shader, sg_shader display_shader) {
  // Create the render target image
  sg_image_desc rt_desc = {
      .render_target = true,
      .width = offscreen_width,
      .height = offscreen_height,
//      .min_filter = SG_FILTER_LINEAR,
//      .mag_filter = SG_FILTER_LINEAR,
#ifdef SOKOL_GLES3
    .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
    .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
    .wrap_w = SG_WRAP_CLAMP_TO_EDGE,
#else
//    .wrap_u = SG_WRAP_CLAMP_TO_BORDER,
//    .wrap_v = SG_WRAP_CLAMP_TO_BORDER,
//    .wrap_w = SG_WRAP_CLAMP_TO_BORDER,
#endif
#ifdef BINOCLE_GL
      .pixel_format = SG_PIXELFORMAT_RGBA8,
#else
      .pixel_format = SG_PIXELFORMAT_BGRA8,
#endif
      .sample_count = 1,
      .label = "offscreen"
  };
  gd->offscreen.render_target = sg_make_image(&rt_desc);

  // Clear screen action for the offscreen render target
  sg_color off_clear_color = binocle_color_azure();
  sg_pass_action offscreen_action = {
      .colors[0] = {
      .load_action = SG_LOADACTION_CLEAR,
      .clear_value =  {
              .r = off_clear_color.r,
              .g = off_clear_color.g,
              .b = off_clear_color.b,
              .a = off_clear_color.a,
          }
      }
  };
  gd->offscreen.action = offscreen_action;

  // Render pass that renders to the offscreen render target
  gd->offscreen.pass = sg_make_pass(&(sg_pass_desc){
      .color_attachments[0].image = gd->offscreen.render_target,
    .label = "offscreen-pass",
  });

  binocle_log_info("Setting up offscreen pipeline");
  // Pipeline state object for the offscreen rendered sprite
  gd->offscreen.pip = sg_make_pipeline(&(sg_pipeline_desc) {
    .label = "offscreen-pipeline",
      .layout = {
          .attrs = {
              [0] = { .format = SG_VERTEXFORMAT_FLOAT2 }, // position
              [1] = { .format = SG_VERTEXFORMAT_FLOAT4 }, // color
              [2] = { .format = SG_VERTEXFORMAT_FLOAT2 }, // texture uv
          },
      },
      .shader = offscreen_shader,
//      .index_type = SG_INDEXTYPE_UINT16,
      .index_type = SG_INDEXTYPE_NONE,
      .depth = {
          .pixel_format = SG_PIXELFORMAT_NONE,
          .compare = SG_COMPAREFUNC_NEVER,
          .write_enabled = false,
      },
      .stencil = {
          .enabled = false,
      },
      .colors = {
        [0] = {
#ifdef BINOCLE_GL
            .pixel_format = SG_PIXELFORMAT_RGBA8,
#else
            .pixel_format = SG_PIXELFORMAT_BGRA8,
#endif
          .blend = {
          .enabled = true,
          .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
          .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        }
        }
      }
  });

  binocle_log_info("Done setting up offscreen pipeline");

  sg_buffer_desc vbuf_desc = {
    .type = SG_BUFFERTYPE_VERTEXBUFFER,
    .usage = SG_USAGE_STREAM,
    .size = sizeof(binocle_vpct) * BINOCLE_GD_MAX_VERTICES,
  };
  gd->offscreen.vbuf = sg_make_buffer(&vbuf_desc);

  sg_buffer_desc ibuf_desc = {
    .type = SG_BUFFERTYPE_INDEXBUFFER,
    .usage = SG_USAGE_STREAM,
    .size = sizeof(uint16_t) * BINOCLE_GD_MAX_INDICES,
  };
  gd->offscreen.ibuf = sg_make_buffer(&ibuf_desc);

  gd->offscreen.bind = (sg_bindings){
      .vertex_buffers = {
          [0] = gd->offscreen.vbuf,
      },
  };
  gd->offscreen.bind.fs.samplers[0] = sg_make_sampler(&(sg_sampler_desc){
    .label = "offscreen-sampler",
    .min_filter = SG_FILTER_LINEAR,
    .mag_filter = SG_FILTER_LINEAR,
    .mipmap_filter = SG_FILTER_NONE,
  });

  // Render to screen pipeline

  // Clear screen action for the actual screen
  sg_color clear_color = binocle_color_green();
  sg_pass_action default_action = {
    .colors[0] = {
      .load_action = SG_LOADACTION_CLEAR,
      .clear_value = {
        .r = clear_color.r,
        .g = clear_color.g,
        .b = clear_color.b,
        .a = clear_color.a,
      }
    }
  };
  gd->display.action = default_action;

  binocle_log_info("Setting up screen pipeline");

  // Pipeline state object for the screen (default) pass
  gd->display.pip = sg_make_pipeline(&(sg_pipeline_desc){
    .label = "display-pipeline",
    .layout = {
      .attrs = {
        [0] = { .format = SG_VERTEXFORMAT_FLOAT3 }, // position
//        [1] = { .format = SG_VERTEXFORMAT_FLOAT4 }, // color
//        [2] = { .format = SG_VERTEXFORMAT_FLOAT2 }, // texture uv
      }
    },
    .shader = display_shader,
    .index_type = SG_INDEXTYPE_UINT16,
#if !defined(BINOCLE_GL)
    .depth = {
      .pixel_format = SG_PIXELFORMAT_NONE,
      .compare = SG_COMPAREFUNC_NEVER,
      .write_enabled = false,
    },
    .stencil = {
      .enabled = false,
    },
#endif
    .colors = {
#ifdef BINOCLE_GL
      [0] = {
        .pixel_format = SG_PIXELFORMAT_RGBA8
      }
#else
      [0] = { .pixel_format = SG_PIXELFORMAT_BGRA8 }
#endif
    }
  });
  binocle_log_info("Done setting up screen pipeline");

//  float vertices[] = {
//    /* pos                  color                       uvs */
//    -1.0f, -1.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,     0.0f, 0.0f,
//    1.0f, -1.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,     1.0f, 0.0f,
//    1.0f,  1.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,     1.0f, 1.0f,
//    -1.0f,  1.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,     0.0f, 1.0f,
//  };
  float vertices[] = {
    /* pos                  color                       uvs */
    -1.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
  };
  sg_buffer_desc display_vbuf_desc = {
    .data = SG_RANGE(vertices)
  };
  gd->display.vbuf = sg_make_buffer(&display_vbuf_desc);

  uint16_t indices[] = {
    0, 1, 2,  0, 2, 3,
  };
  sg_buffer_desc display_ibuf_desc = {
    .type = SG_BUFFERTYPE_INDEXBUFFER,
    .data = SG_RANGE(indices)
  };
  gd->display.ibuf = sg_make_buffer(&display_ibuf_desc);

  gd->display.bind = (sg_bindings){
    .vertex_buffers = {
      [0] = gd->display.vbuf,
    },
    .index_buffer = gd->display.ibuf,
    .fs.images = {
      [0] = gd->offscreen.render_target,
    }
  };
  gd->display.bind.fs.samplers[0] = sg_make_sampler(&(sg_sampler_desc){
    .label = "display-sampler",
    .min_filter = SG_FILTER_LINEAR,
    .mag_filter = SG_FILTER_LINEAR,
    .mipmap_filter = SG_FILTER_NONE,
  });
}

void binocle_gd_draw(binocle_gd *gd, const struct binocle_vpct *vertices, size_t vertex_count, binocle_material material,
                     kmAABB2 viewport, binocle_camera *camera, float depth) {
  binocle_gd_command_t *cmd = &gd->commands[gd->num_commands];
  cmd->num_vertices = vertex_count;
  cmd->base_vertex = gd->num_vertices;
  cmd->img = material.albedo_texture;
  cmd->depth = depth;
  if (sg_query_pipeline_state(material.pip) == SG_RESOURCESTATE_VALID) {
    cmd->pip = material.pip;
    memcpy(cmd->custom_fs_uniforms, material.custom_fs_uniforms, 1024);
    cmd->shader_desc = material.shader_desc;
  } else {
    cmd->pip.id = 0;
  }

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

#define binocle_gd_command_lt(a, b) ((a).depth < (b).depth)
KSORT_INIT(binocle_gd_sort_commands, binocle_gd_command_t, binocle_gd_command_lt)

void binocle_gd_render_offscreen(binocle_gd *gd) {
  if (gd->num_vertices == 0) {
    return;
  }
  sg_update_buffer(gd->offscreen.vbuf, &(sg_range){ .ptr=gd->vertices, .size=gd->num_vertices * sizeof(binocle_vpct) });

  sg_begin_pass(gd->offscreen.pass, &gd->offscreen.action);

  ks_mergesort(binocle_gd_sort_commands, gd->num_commands, gd->commands, 0);

  for (uint32_t i = 0 ; i < gd->num_commands ; i++) {
    binocle_gd_command_t *cmd = &gd->commands[i];

    gd->offscreen.bind.fs.images[0] = cmd->img;
    gd->offscreen.bind.vertex_buffers[0] = gd->offscreen.vbuf;

    if (sg_query_pipeline_state(cmd->pip) == SG_RESOURCESTATE_VALID) {
      sg_apply_pipeline(cmd->pip);
      sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &SG_RANGE(cmd->uniforms));
      sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, &(sg_range){.ptr = cmd->custom_fs_uniforms, .size = cmd->shader_desc.fs.uniform_blocks[0].size});
    } else {
      sg_apply_pipeline(gd->offscreen.pip);
      sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &SG_RANGE(cmd->uniforms));
    }
    sg_apply_bindings(&gd->offscreen.bind);
    sg_draw(cmd->base_vertex, cmd->num_vertices, 1);

  }
  sg_end_pass();
  gd->num_commands = 0;
  gd->num_vertices = 0;
}

void binocle_gd_render_screen(binocle_gd *gd, struct binocle_window *window, float design_width, float design_height, kmAABB2 viewport, kmMat4 matrix, float scale) {
  // Render the offscreen to the display

  typedef struct screen_vs_params_t {
    kmMat4 transform;
  } screen_vs_params_t;

  typedef struct screen_fs_params_t {
    float resolution[2];
    float scale[2];
    float viewport[2];
    uint8_t _pad_24[8];
  } screen_fs_params_t;

  screen_vs_params_t screen_vs_params = {0};
  screen_fs_params_t screen_fs_params = {0};

//  screen_vs_params.transform = matrix;
  kmMat4Identity(&screen_vs_params.transform);

  screen_fs_params.resolution[0] = design_width;
  screen_fs_params.resolution[1] = design_height;
  screen_fs_params.scale[0] = scale;
  screen_fs_params.scale[1] = scale;
  screen_fs_params.viewport[0] = viewport.min.x;
  screen_fs_params.viewport[1] = viewport.min.y;

  gd->display.bind.fs.images[0] = gd->offscreen.render_target;

  sg_begin_default_pass(&gd->display.action, window->width, window->height);
  sg_apply_pipeline(gd->display.pip);
  sg_apply_bindings(&gd->display.bind);
  sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &SG_RANGE(screen_vs_params));
  sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, &SG_RANGE(screen_fs_params));
  sg_draw(0, 6, 1);
  sg_end_pass();

  sg_commit();
}

void binocle_gd_render(binocle_gd *gd, struct binocle_window *window, float design_width, float design_height, kmAABB2 viewport, kmMat4 matrix, float scale) {
  binocle_gd_render_offscreen(gd);
  binocle_gd_render_flat(gd);
  binocle_gd_render_screen(gd, window, design_width, design_height, viewport, matrix, scale);
}

void binocle_gd_setup_flat_pipeline(binocle_gd *gd) {
#if defined(BINOCLE_MACOS) && defined(BINOCLE_METAL)
#include "../../assets/metal/default-metal-macosx.h"
#include "../../assets/metal/screen-metal-macosx.h"
#endif
  sg_shader_desc screen_shader_desc = {
#ifdef BINOCLE_GL
#if defined(__IPHONEOS__) || defined(__ANDROID__) || defined(__EMSCRIPTEN__)
    .vs.source = binocle_shader_flat_vertex_src_gles,
#else
    .vs.source = binocle_shader_flat_vertex_src_gl33,
#endif
    .vs.uniform_blocks[0] = {
      .size = sizeof(struct binocle_gd_flat_shader_vs_params_t),
      .uniforms = {
        [0] = { .name = "projectionMatrix", .type = SG_UNIFORMTYPE_MAT4 },
        [1] = { .name = "viewMatrix", .type = SG_UNIFORMTYPE_MAT4 },
        [2] = { .name = "modelMatrix", .type = SG_UNIFORMTYPE_MAT4 },
      },
    },
#else
    .vs.bytecode.ptr = screen_vs_bytecode,
    .vs.bytecode.size = sizeof(screen_vs_bytecode),
#endif
    .attrs = {
      [0].name = "vertexPosition",
      [1].name = "vertexColor",
      [2].name = "vertexTexture",
    },
#ifdef BINOCLE_GL
#if defined(__IPHONEOS__) || defined(__ANDROID__) || defined(__EMSCRIPTEN__)
    .fs.source = binocle_shader_flat_frag_src_gles,
#else
    .fs.source = binocle_shader_flat_src_gl33,
#endif
#else
    .fs.bytecode.ptr = screen_fs_bytecode,
    .fs.bytecode.size = sizeof(screen_fs_bytecode),
#endif
//    .fs.uniform_blocks[0] = {
//      .size = sizeof(struct binocle_gd_flat_shader_fs_params_t),
//      .uniforms = {
//        [0] = { .name = "color", .type = SG_UNIFORMTYPE_FLOAT4 },
//      },
//    },
  };
  binocle_log_info("Compiling flat shader");
  sg_shader shader = sg_make_shader(&screen_shader_desc);
  binocle_log_info("Done compiling flat shader");

  sg_pass_action action = {
    .colors[0] = {
      .load_action = SG_LOADACTION_DONTCARE,
      .clear_value = {0.0f, 1.0f, 0.0f, 1.0f},
    }
  };
  gd->flat.action = action;

  gd->flat.pass = sg_make_pass(&(sg_pass_desc){
    .color_attachments[0].image = gd->offscreen.render_target,
  });

  binocle_log_info("Setting up flat pipeline");
  gd->flat.pip = sg_make_pipeline(&(sg_pipeline_desc) {
    .layout = {
      .attrs = {
        [0] = { .format = SG_VERTEXFORMAT_FLOAT2 }, // position
        [1] = { .format = SG_VERTEXFORMAT_FLOAT4 }, // color
        [2] = { .format = SG_VERTEXFORMAT_FLOAT2 }, // texture
      },
    },
    .shader = shader,
    .index_type = SG_INDEXTYPE_NONE,
    .depth = {
      .pixel_format = SG_PIXELFORMAT_NONE,
      .compare = SG_COMPAREFUNC_NEVER,
      .write_enabled = false,
    },
    .stencil = {
      .enabled = false,
    },
    .colors = {
      [0] = {
#ifdef BINOCLE_GL
        .pixel_format = SG_PIXELFORMAT_RGBA8,
#else
        .pixel_format = SG_PIXELFORMAT_BGRA8,
#endif
        .blend = {
          .enabled = true,
          .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
          .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        }
      }
    }
  });
  binocle_log_info("Done setting up flat pipeline");

  sg_buffer_desc vbuf_desc = {
    .type = SG_BUFFERTYPE_VERTEXBUFFER,
    .usage = SG_USAGE_STREAM,
    .size = sizeof(binocle_vpct) * 4096,
  };
  gd->flat.vbuf = sg_make_buffer(&vbuf_desc);

  gd->flat.bind = (sg_bindings){
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
  sg_apply_viewport(viewport.min.x, viewport.min.y, viewport.max.x, viewport.max.y, false);
}

void binocle_gd_apply_blend_mode(const sg_blend_state blend_mode) {
  assert(false);
}

void binocle_gd_apply_shader(binocle_gd *gd, sg_shader shader) {
  assert(false);
}

void binocle_gd_apply_texture(sg_image texture) {
  assert(false);
}

void binocle_gd_apply_3d_texture(binocle_material *material) {
  assert(false);
}

void binocle_gd_set_uniform_float(struct sg_shader *shader, const char *name, float value) {
//  GLint id = 0;
//  glCheck(id = glGetUniformLocation(shader->program_id, name));
//  if (id == -1) {
//    binocle_log_error("Cannot find uniform called %s", name);
//    return;
//  }
//  glCheck(glUniform1f(id, value));
}

void binocle_gd_set_uniform_float2(sg_shader shader, const char *name, float value1, float value2) {
  assert(false);
}

void binocle_gd_set_uniform_float3(struct sg_shader *shader, const char *name, float value1, float value2,
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
binocle_gd_set_uniform_float4(struct sg_shader *shader, const char *name, float value1, float value2, float value3,
                              float value4) {
//  GLint id = 0;
//  glCheck(id = glGetUniformLocation(shader->program_id, name));
//  if (id == -1) {
//    binocle_log_error("Cannot find uniform called %s", name);
//    return;
//  }
//  glCheck(glUniform4f(id, value1, value2, value3, value4));
}

void binocle_gd_set_uniform_render_target_as_texture(struct sg_shader *shader, const char *name,
                                                     sg_image render_target) {
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

void binocle_gd_set_uniform_vec3(struct sg_shader *shader, const char *name, kmVec3 vec) {
//  GLint id = 0;
//  glCheck(id = glGetUniformLocation(shader->program_id, name));
//  if (id == -1) {
//    binocle_log_error("Cannot find uniform called %s", name);
//    return;
//  }
//  glCheck(glUniform3fv(id, 1, (GLfloat *)&vec));
}

void binocle_gd_set_uniform_mat4(sg_shader shader, const char *name, kmMat4 mat) {
  assert(false);
}

void binocle_gd_set_offscreen_clear_color(binocle_gd *gd, struct sg_color color) {
  gd->offscreen.action.colors[0].clear_value = color;
}

void binocle_gd_set_render_target(sg_image render_target) {
  assert(false);
}

void binocle_gd_draw_quad(binocle_gd *gd, sg_image image) {
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

void binocle_gd_draw_quad_to_screen(binocle_gd *gd, sg_shader shader, sg_image render_target) {
  float vertices[] = {
    // positions            colors
    -0.5f,  0.5f, 0.5f,     1.0f, 0.0f, 0.0f, 1.0f,
    0.5f,  0.5f, 0.5f,     0.0f, 1.0f, 0.0f, 1.0f,
    0.5f, -0.5f, 0.5f,     0.0f, 0.0f, 1.0f, 1.0f,
    -0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 0.0f, 1.0f,
  };
  sg_buffer_desc vbuf_desc = {
    .data = SG_RANGE(vertices)
  };
  sg_buffer vbuf = sg_make_buffer(&vbuf_desc);

  /* create an index buffer */
  uint16_t indices[] = {
    0, 1, 2,    // first triangle
    0, 2, 3,    // second triangle
  };
  sg_buffer_desc ibuf_desc = {
    .type = SG_BUFFERTYPE_INDEXBUFFER,
    .data = SG_RANGE(indices)
  };
  sg_buffer ibuf = sg_make_buffer(&ibuf_desc);

  /* define the resource bindings */
  sg_bindings bind = {
    .vertex_buffers[0] = vbuf,
    .index_buffer = ibuf
  };

  // TODO: this is bad.. we create the pipeline at runtime and recreate it over and over without destroying it
  /* create a pipeline object (default render state is fine) */
  sg_pipeline pip = sg_make_pipeline(&(sg_pipeline_desc){
    .shader = shader,
    .index_type = SG_INDEXTYPE_UINT16,
    .layout = {
      /* test to provide attr offsets, but no buffer stride, this should compute the stride */
      .attrs = {
        /* vertex attrs can also be bound by location instead of name (but not in GLES2) */
        [0] = { .offset=0, .format=SG_VERTEXFORMAT_FLOAT3 },
        [1] = { .offset=12, .format=SG_VERTEXFORMAT_FLOAT4 }
      }
    }
  });

  /* default pass action */
  sg_pass_action pass_action = { 0 };
  sg_begin_pass(gd->display.pass, &pass_action);
  sg_apply_pipeline(pip);
  sg_apply_bindings(&bind);
  sg_draw(0, 6, 1);
  sg_end_pass();
}

void binocle_gd_draw_flat(binocle_gd *gd, const struct binocle_vpct *vertices, size_t vertex_count,
                     kmAABB2 viewport, binocle_camera *camera, kmMat4 *view_matrix, float depth) {
  binocle_gd_command_t *cmd = &gd->flat_commands[gd->flat_num_commands];
  cmd->num_vertices = vertex_count;
  cmd->base_vertex = gd->flat_num_vertices;
  cmd->depth = depth;

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

  if (view_matrix != NULL) {
    kmMat4Multiply(&cmd->uniforms.viewMatrix, &cmd->uniforms.viewMatrix, view_matrix);
  }

  kmMat4Identity(&cmd->uniforms.modelMatrix);

  gd->flat_num_commands++;

  for (uint32_t i = 0 ; i < vertex_count ; i++) {
    binocle_vpct *v = &gd->flat_vertices[gd->flat_num_vertices];
    v->pos.x = vertices[i].pos.x;
    v->pos.y = vertices[i].pos.y;
    v->color.r = vertices[i].color.r;
    v->color.g = vertices[i].color.g;
    v->color.b = vertices[i].color.b;
    v->color.a = vertices[i].color.a;
    v->tex.x = vertices[i].tex.x;
    v->tex.y = vertices[i].tex.y;
    gd->flat_num_vertices++;
  }
}

void binocle_gd_draw_rect(binocle_gd *gd, kmAABB2 rect, sg_color col, kmAABB2 viewport, binocle_camera *camera, kmMat4 *view_matrix, float depth) {
  static binocle_vpct g_quad_vertex_buffer_data[6] = {0};
  g_quad_vertex_buffer_data[0] = (binocle_vpct){
    .pos = {.x = rect.min.x, .y = rect.min.y},
    .color = {.r = col.r, .g = col.g, .b = col.b, .a = col.a}
  };
  g_quad_vertex_buffer_data[1] = (binocle_vpct){
    .pos = {.x = rect.max.x, .y = rect.min.y},
    .color = {.r = col.r, .g = col.g, .b = col.b, .a = col.a}
  };
  g_quad_vertex_buffer_data[2] = (binocle_vpct){
    .pos = {.x = rect.min.x, .y = rect.max.y},
    .color = {.r = col.r, .g = col.g, .b = col.b, .a = col.a}
  };
  g_quad_vertex_buffer_data[3] = (binocle_vpct){
    .pos = {.x = rect.min.x, .y = rect.max.y},
    .color = {.r = col.r, .g = col.g, .b = col.b, .a = col.a}
  };
  g_quad_vertex_buffer_data[4] = (binocle_vpct){
    .pos = {.x = rect.max.x, .y = rect.min.y},
    .color = {.r = col.r, .g = col.g, .b = col.b, .a = col.a}
  };
  g_quad_vertex_buffer_data[5] = (binocle_vpct){
    .pos = {.x = rect.max.x, .y = rect.max.y},
    .color = {.r = col.r, .g = col.g, .b = col.b, .a = col.a}
  };

  binocle_gd_draw_flat(gd, g_quad_vertex_buffer_data, 6, viewport, camera, view_matrix, depth);
}

void binocle_gd_render_flat(binocle_gd *gd) {
  if (gd->flat_num_vertices == 0) {
    return;
  }
  sg_update_buffer(gd->flat.vbuf, &(sg_range){ .ptr=gd->flat_vertices, .size=gd->flat_num_vertices * sizeof(binocle_vpct) });

  sg_begin_pass(gd->flat.pass, &gd->flat.action);

  for (uint32_t i = 0 ; i < gd->flat_num_commands ; i++) {
    binocle_gd_command_t *cmd = &gd->flat_commands[i];

    gd->flat.bind.vertex_buffers[0] = gd->flat.vbuf;

    sg_apply_pipeline(gd->flat.pip);
    sg_apply_bindings(&gd->flat.bind);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, SG_RANGE_REF(cmd->uniforms));
    sg_draw(cmd->base_vertex, cmd->num_vertices, 1);

  }
  sg_end_pass();
  gd->flat_num_commands = 0;
  gd->flat_num_vertices = 0;
}

void binocle_gd_draw_rect_outline(binocle_gd *gd, kmAABB2 rect, sg_color col, kmAABB2 viewport, binocle_camera *camera, float depth) {
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

  binocle_gd_draw_rect(gd, rect_bottom, col, viewport, camera, NULL, depth);
  binocle_gd_draw_rect(gd, rect_top, col, viewport, camera, NULL, depth);
  binocle_gd_draw_rect(gd, rect_left, col, viewport, camera, NULL, depth);
  binocle_gd_draw_rect(gd, rect_right, col, viewport, camera, NULL, depth);
}

void binocle_gd_draw_line(binocle_gd *gd, kmVec2 start, kmVec2 end, sg_color col, kmAABB2 viewport, binocle_camera *camera, float depth) {

  float angle = atan2f(end.y - start.y, end.x - start.x);
  float length = kmVec2DistanceBetween(&start, &end);
  kmAABB2 rect;
  rect.min.x = 0;
  rect.min.y = 0;
  rect.max.x = 1;
  rect.max.y = 1;

  kmMat4 viewMatrix;
  kmMat4Identity(&viewMatrix);

  kmMat4 trans;
  kmMat4Translation(&trans, start.x, start.y, 0);

  kmMat4 scale;
  kmMat4Scaling(&scale, length, 1, 0); // y is the thickness of the line

  kmMat4 rot;
  kmMat4RotationZ(&rot, angle);

  kmMat4Multiply(&viewMatrix, &viewMatrix, &trans);
  kmMat4Multiply(&viewMatrix, &viewMatrix, &rot);
  kmMat4Multiply(&viewMatrix, &viewMatrix, &scale);

  binocle_gd_draw_rect(gd, rect, col, viewport, camera, &viewMatrix, depth);
}

void binocle_gd_draw_circle(binocle_gd *gd, kmVec2 center, float radius, sg_color col, kmAABB2 viewport, binocle_camera *camera, float depth) {
  static binocle_vpct vertex_buffer_data[3 * 32] = {0};

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

    vertex_buffer_data[count] = (binocle_vpct){
      .pos = {.x = v0.x, .y = v0.y},
      .color = {.r = col.r, .g = col.g, .b = col.b, .a = col.a}
    };
    count++;
    vertex_buffer_data[count] = (binocle_vpct){
      .pos = {.x = v1.x, .y = v1.y},
      .color = {.r = col.r, .g = col.g, .b = col.b, .a = col.a}
    };
    count++;
    vertex_buffer_data[count] = (binocle_vpct){
      .pos = {.x = v2.x, .y = v2.y},
      .color = {.r = col.r, .g = col.g, .b = col.b, .a = col.a}
    };
    count++;

    theta += increment;
  }
  binocle_gd_draw_flat(gd, vertex_buffer_data, circle_segments * 3, viewport, camera, NULL, depth);
}

void binocle_gd_draw_with_state(binocle_gd *gd, const binocle_vpct *vertices, size_t vertex_count, binocle_render_state *render_state, float depth) {
  binocle_gd_command_t *cmd = &gd->commands[gd->num_commands];
  cmd->num_vertices = vertex_count;
  cmd->base_vertex = gd->num_vertices;
  cmd->img = render_state->material->albedo_texture;
  cmd->depth = depth;

  if (sg_query_pipeline_state(render_state->material->pip) == SG_RESOURCESTATE_VALID) {
    cmd->pip = render_state->material->pip;
    memcpy(cmd->custom_fs_uniforms, render_state->material->custom_fs_uniforms, 1024);
    cmd->shader_desc = render_state->material->shader_desc;
  } else {
    cmd->pip.id = 0;
  }

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

void binocle_gd_draw_test_triangle(struct sg_shader *shader) {
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

void binocle_gd_draw_test_cube(struct sg_shader *shader) {
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

sg_shader_desc binocle_gd_create_offscreen_shader_desc(const char *shader_vs_src, const char *shader_fs_src) {
  sg_shader_desc shader_desc = {
#ifdef BINOCLE_GL
    .vs.source = shader_vs_src,
#else
    .vs.byte_code = default_vs_bytecode,
    .vs.byte_code_size = sizeof(default_vs_bytecode),
#endif
    .attrs = {
      [0].name = "vertexPosition",
      [1].name = "vertexColor",
      [2].name = "vertexTCoord",
    },
    .vs.uniform_blocks[0] = {
      .size = sizeof(float) * 16 * 3,
      .uniforms = {
        [0] = { .name = "projectionMatrix", .type = SG_UNIFORMTYPE_MAT4},
        [1] = { .name = "viewMatrix", .type = SG_UNIFORMTYPE_MAT4},
        [2] = { .name = "modelMatrix", .type = SG_UNIFORMTYPE_MAT4},
      }
    },
#ifdef BINOCLE_GL
    .fs.source = shader_fs_src,
#else
    .fs.byte_code = default_fs_bytecode,
    .fs.byte_code_size = sizeof(default_fs_bytecode),
#endif
    .fs.images[0] = {.used = true, .image_type = SG_IMAGETYPE_2D},
    .fs.samplers[0] = {.used = true, .sampler_type = SG_SAMPLERTYPE_FILTERING},
    .fs.image_sampler_pairs[0] = {.used = true,
                                  .glsl_name = "tex0",
                                  .image_slot = 0,
                                  .sampler_slot = 0},
  };
  return shader_desc;
}

size_t binocle_gd_compute_uniform_block_size(sg_shader_uniform_block_desc desc) {
  size_t total_size = 0;
  for (int i = 0 ; i < SG_MAX_UB_MEMBERS ; i++) {
    size_t size = 0;
    switch(desc.uniforms[i].type) {
      case SG_UNIFORMTYPE_MAT4:
        size = sizeof(float) * 16;
        break;
      case SG_UNIFORMTYPE_FLOAT:
        size = sizeof(float);
        break;
      case SG_UNIFORMTYPE_FLOAT2:
        size = sizeof(float) * 2;
        break;
      case SG_UNIFORMTYPE_FLOAT3:
        size = sizeof(float) * 3;
        break;
      case SG_UNIFORMTYPE_FLOAT4:
        size = sizeof(float) * 4;
        break;
      case SG_UNIFORMTYPE_INT:
        size = sizeof(int);
        break;
      case SG_UNIFORMTYPE_INT2:
        size = sizeof(int) * 2;
        break;
      case SG_UNIFORMTYPE_INT3:
        size = sizeof(int) * 3;
        break;
      case SG_UNIFORMTYPE_INT4:
        size = sizeof(int) * 4;
        break;
      default:
        break;
    }
    total_size += size;
  }
  return total_size;
}

void binocle_gd_add_uniform_to_shader_desc(sg_shader_desc *shader_desc, sg_shader_stage stage, size_t idx, const char *uniform_name, sg_uniform_type uniform_type) {
  switch (stage) {
    case SG_SHADERSTAGE_VS:
      shader_desc->vs.uniform_blocks[0].uniforms[idx].name = SDL_strdup(uniform_name);;
      shader_desc->vs.uniform_blocks[0].uniforms[idx].type = uniform_type;
      break;
    case SG_SHADERSTAGE_FS:
      shader_desc->fs.uniform_blocks[0].uniforms[idx].name = SDL_strdup(uniform_name);;
      shader_desc->fs.uniform_blocks[0].uniforms[idx].type = uniform_type;
      break;
    default:
      break;
  }
  shader_desc->vs.uniform_blocks[0].size = binocle_gd_compute_uniform_block_size(shader_desc->vs.uniform_blocks[0]);
  shader_desc->fs.uniform_blocks[0].size = binocle_gd_compute_uniform_block_size(shader_desc->fs.uniform_blocks[0]);
}

sg_shader binocle_gd_create_shader(sg_shader_desc desc) {
  return sg_make_shader(&desc);
}

sg_pipeline binocle_gd_create_offscreen_pipeline(sg_shader shader) {
  sg_pipeline pip = sg_make_pipeline(&(sg_pipeline_desc) {
    .layout = {
      .attrs = {
        [0] = { .format = SG_VERTEXFORMAT_FLOAT2 }, // position
        [1] = { .format = SG_VERTEXFORMAT_FLOAT4 }, // color
        [2] = { .format = SG_VERTEXFORMAT_FLOAT2 }, // texture uv
      },
    },
    .shader = shader,
//      .index_type = SG_INDEXTYPE_UINT16,
    .index_type = SG_INDEXTYPE_NONE,
    .depth = {
      .pixel_format = SG_PIXELFORMAT_NONE,
      .compare = SG_COMPAREFUNC_NEVER,
      .write_enabled = false,
    },
    .stencil = {
      .enabled = false,
    },
    .colors = {
      [0] = {
#ifdef BINOCLE_GL
        .pixel_format = SG_PIXELFORMAT_RGBA8,
#else
        .pixel_format = SG_PIXELFORMAT_BGRA8,
#endif
        .blend = {
          .enabled = true,
          .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
          .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        }
      }
    }
  });
  return pip;
}

