//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_BINOCLE_BACKEND_TYPES_H
#define BINOCLE_BINOCLE_BACKEND_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#define BINOCLE_DEFAULT_UB_SIZE (4 * 1024 * 1024)
#define BINOCLE_DEFAULT_BUFFER_POOL_SIZE (128)
#define BINOCLE_DEFAULT_IMAGE_POOL_SIZE (128)
#define BINOCLE_DEFAULT_SHADER_POOL_SIZE (32)
#define BINOCLE_DEFAULT_SAMPLER_CACHE_CAPACITY (64)
#define BINOCLE_NUM_INFLIGHT_FRAMES (1)
#define BINOCLE_MAX_MIPMAPS (16)
#define BINOCLE_MAX_SHADERSTAGE_IMAGES (12)
#define BINOCLE_MAX_SHADERSTAGE_UBS (4)
#define BINOCLE_NUM_SHADER_STAGES (2)
#define BINOCLE_MAX_UB_MEMBERS (16)
#define BINOCLE_MAX_VERTEX_ATTRIBUTES (16)
#define BINOCLE_STRING_SIZE (16)
#define BINOCLE_DEF(val, def) (((val) == 0) ? (def) : (val))
#define BINOCLE_DEF_FLT(val, def) (((val) == 0.0f) ? (def) : (val))
#define BINOCLE_CLAMP(v,v0,v1) ((v<v0)?(v0):((v>v1)?(v1):(v)))

/* fixed-size string */
typedef struct binocle_str_t {
  char buf[BINOCLE_STRING_SIZE];
} binocle_str_t;

typedef enum binocle_pixel_format {
  BINOCLE_PIXEL_FORMAT_DEFAULT,
  BINOCLE_PIXEL_FORMAT_NONE,
  BINOCLE_PIXEL_FORMAT_RGB,
  BINOCLE_PIXEL_FORMAT_RGBA,
  BINOCLE_PIXEL_FORMAT_RGBA8,
  BINOCLE_PIXEL_FORMAT_BGRA8,
  BINOCLE_PIXEL_FORMAT_DEPTH,
  BINOCLE_PIXEL_FORMAT_DEPTH_STENCIL,
  BINOCLE_PIXEL_FORMAT_NUM,
} binocle_pixel_format;

typedef enum binocle_image_type {
  BINOCLE_IMAGETYPE_DEFAULT,  /* value 0 reserved for default-init */
  BINOCLE_IMAGETYPE_2D,
  BINOCLE_IMAGETYPE_CUBE,
  BINOCLE_IMAGETYPE_3D,
  BINOCLE_IMAGETYPE_ARRAY,
  BINOCLE_IMAGETYPE_NUM,
  BINOCLE_IMAGETYPE_FORCE_U32 = 0x7FFFFFFF
} binocle_image_type;

typedef enum binocle_usage {
  BINOCLE_USAGE_DEFAULT,      /* value 0 reserved for default-init */
  BINOCLE_USAGE_IMMUTABLE,
  BINOCLE_USAGE_DYNAMIC,
  BINOCLE_USAGE_STREAM,
  BINOCLE_USAGE_NUM,
  BINOCLE_USAGE_FORCE_U32 = 0x7FFFFFFF
} binocle_usage;

typedef enum binocle_filter {
  BINOCLE_FILTER_DEFAULT, /* value 0 reserved for default-init */
  BINOCLE_FILTER_NEAREST,
  BINOCLE_FILTER_LINEAR,
  BINOCLE_FILTER_NEAREST_MIPMAP_NEAREST,
  BINOCLE_FILTER_NEAREST_MIPMAP_LINEAR,
  BINOCLE_FILTER_LINEAR_MIPMAP_NEAREST,
  BINOCLE_FILTER_LINEAR_MIPMAP_LINEAR,
  BINOCLE_FILTER_NUM,
  BINOCLE_FILTER_FORCE_U32 = 0x7FFFFFFF
} binocle_filter;

typedef enum binocle_wrap {
  BINOCLE_WRAP_DEFAULT,   /* value 0 reserved for default-init */
  BINOCLE_WRAP_REPEAT,
  BINOCLE_WRAP_CLAMP_TO_EDGE,
  BINOCLE_WRAP_CLAMP_TO_BORDER,
  BINOCLE_WRAP_MIRRORED_REPEAT,
  BINOCLE_WRAP_NUM,
  BINOCLE_WRAP_FORCE_U32 = 0x7FFFFFFF
} binocle_wrap;

typedef enum binocle_border_color {
  BINOCLE_BORDERCOLOR_DEFAULT,    /* value 0 reserved for default-init */
  BINOCLE_BORDERCOLOR_TRANSPARENT_BLACK,
  BINOCLE_BORDERCOLOR_OPAQUE_BLACK,
  BINOCLE_BORDERCOLOR_OPAQUE_WHITE,
  BINOCLE_BORDERCOLOR_NUM,
  BINOCLE_BORDERCOLOR_FORCE_U32 = 0x7FFFFFFF
} binocle_border_color;

typedef enum binocle_cube_face {
  BINOCLE_CUBEFACE_POS_X,
  BINOCLE_CUBEFACE_NEG_X,
  BINOCLE_CUBEFACE_POS_Y,
  BINOCLE_CUBEFACE_NEG_Y,
  BINOCLE_CUBEFACE_POS_Z,
  BINOCLE_CUBEFACE_NEG_Z,
  BINOCLE_CUBEFACE_NUM,
  BINOCLE_CUBEFACE_FORCE_U32 = 0x7FFFFFFF
} binocle_cube_face;

/*
    binocle_subimage_content

    Pointer to and size of a subimage-surface data, this is
    used to describe the initial content of immutable-usage images,
    or for updating a dynamic- or stream-usage images.

    For 3D- or array-textures, one binocle_subimage_content item
    describes an entire mipmap level consisting of all array- or
    3D-slices of the mipmap level. It is only possible to update
    an entire mipmap level, not parts of it.
*/
typedef struct binocle_subimage_content {
  const void* ptr;    /* pointer to subimage data */
  int size;           /* size in bytes of pointed-to subimage data */
} binocle_subimage_content;

/*
    binocle_image_content

    Defines the content of an image through a 2D array
    of binocle_subimage_content structs. The first array dimension
    is the cubemap face, and the second array dimension the
    mipmap level.
*/
typedef struct binocle_image_content {
  binocle_subimage_content subimage[BINOCLE_CUBEFACE_NUM][BINOCLE_MAX_MIPMAPS];
} binocle_image_content;

typedef struct binocle_image_desc {
  binocle_image_type type;
  bool render_target;
  int width;
  int height;
  union {
    int depth;
    int layers;
  };
  int num_mipmaps;
  binocle_usage usage;
  binocle_pixel_format pixel_format;
  int sample_count;
  binocle_filter min_filter;
  binocle_filter mag_filter;
  binocle_wrap wrap_u;
  binocle_wrap wrap_v;
  binocle_wrap wrap_w;
  binocle_border_color border_color;
  uint32_t max_anisotropy;
  float min_lod;
  float max_lod;
  binocle_image_content content;
  const char* label;
  /* GL specific */
  uint32_t gl_textures[BINOCLE_NUM_INFLIGHT_FRAMES];
  uint32_t gl_texture_target;
  /* Metal specific */
  const void* mtl_textures[BINOCLE_NUM_INFLIGHT_FRAMES];
} binocle_image_desc;

typedef struct binocle_image_common_t {
  binocle_image_type type;
  bool render_target;
  int width;
  int height;
  int depth;
  int num_mipmaps;
  binocle_usage usage;
  binocle_pixel_format pixel_format;
  int sample_count;
  binocle_filter min_filter;
  binocle_filter mag_filter;
  binocle_wrap wrap_u;
  binocle_wrap wrap_v;
  binocle_wrap wrap_w;
  binocle_border_color border_color;
  uint32_t max_anisotropy;
  uint32_t upd_frame_index;
  int num_slots;
  int active_slot;
} binocle_image_common_t;

/*
    Runtime information about a pixel format, returned
    by binocle_query_pixelformat().
*/
typedef struct binocle_pixelformat_info {
  bool sample;        /* pixel format can be sampled in shaders */
  bool filter;        /* pixel format can be sampled with filtering */
  bool render;        /* pixel format can be used as render target */
  bool blend;         /* alpha-blending is supported */
  bool msaa;          /* pixel format can be used as MSAA render target */
  bool depth;         /* pixel format is a depth format */
} binocle_pixelformat_info;

/*
    binocle_uniform_type

    The data type of a uniform block member. This is used to
    describe the internal layout of uniform blocks when creating
    a shader object.
*/
typedef enum binocle_uniform_type {
  BINOCLE_UNIFORMTYPE_INVALID,
  BINOCLE_UNIFORMTYPE_FLOAT,
  BINOCLE_UNIFORMTYPE_FLOAT2,
  BINOCLE_UNIFORMTYPE_FLOAT3,
  BINOCLE_UNIFORMTYPE_FLOAT4,
  BINOCLE_UNIFORMTYPE_MAT4,
  _BINOCLE_UNIFORMTYPE_NUM,
  _BINOCLE_UNIFORMTYPE_FORCE_U32 = 0x7FFFFFFF
} binocle_uniform_type;

/*
    binocle_sampler_type

    Indicates the basic data type of a shader's texture sampler which
    can be float , unsigned integer or signed integer. The sampler
    type is used in the sg_shader_image_desc to describe the
    sampler type of a shader's texture sampler binding.

    The default sampler type is BINOCLE_SAMPLERTYPE_FLOAT.
*/
typedef enum binocle_sampler_type {
  _BINOCLE_SAMPLERTYPE_DEFAULT,  /* value 0 reserved for default-init */
  BINOCLE_SAMPLERTYPE_FLOAT,
  BINOCLE_SAMPLERTYPE_SINT,
  BINOCLE_SAMPLERTYPE_UINT,
} binocle_sampler_type;

/*
    binocle_shader_stage

    There are 2 shader stages: vertex- and fragment-shader-stage.
    Each shader stage consists of:

    - one slot for a shader function (provided as source- or byte-code)
    - BINOCLE_MAX_SHADERSTAGE_UBS slots for uniform blocks
    - BINOCLE_MAX_SHADERSTAGE_IMAGES slots for images used as textures by
      the shader function
*/
typedef enum binocle_shader_stage {
  BINOCLE_SHADERSTAGE_VS,
  BINOCLE_SHADERSTAGE_FS,
  _BINOCLE_SHADERSTAGE_FORCE_U32 = 0x7FFFFFFF
} binocle_shader_stage;

typedef struct binocle_shader_attr_desc {
  const char* name;           /* GLSL vertex attribute name (only required for GLES2) */
} binocle_shader_attr_desc;

typedef struct binocle_shader_uniform_desc {
  const char* name;
  binocle_uniform_type type;
  int array_count;
} binocle_shader_uniform_desc;

typedef struct binocle_shader_uniform_block_desc {
  int size;
  binocle_shader_uniform_desc uniforms[BINOCLE_MAX_UB_MEMBERS];
} binocle_shader_uniform_block_desc;

typedef struct binocle_shader_image_desc {
  const char* name;
  binocle_image_type type;         /* FIXME: should this be renamed to 'image_type'? */
  binocle_sampler_type sampler_type;
} binocle_shader_image_desc;

typedef struct binocle_shader_stage_desc {
  const char* source;
  const uint8_t* byte_code;
  int byte_code_size;
  const char* entry;
  binocle_shader_uniform_block_desc uniform_blocks[BINOCLE_MAX_SHADERSTAGE_UBS];
  binocle_shader_image_desc images[BINOCLE_MAX_SHADERSTAGE_IMAGES];
} binocle_shader_stage_desc;

typedef struct binocle_shader_desc {
  uint32_t _start_canary;
  binocle_shader_attr_desc attrs[BINOCLE_MAX_VERTEX_ATTRIBUTES];
  binocle_shader_stage_desc vs;
  binocle_shader_stage_desc fs;
  const char* label;
  uint32_t _end_canary;
} binocle_shader_desc;

typedef struct binocle_uniform_block_t {
  int size;
} binocle_uniform_block_t;

typedef struct binocle_shader_image_t {
  binocle_image_type type;
  binocle_sampler_type sampler_type;
} binocle_shader_image_t;

typedef struct binocle_shader_stage_t {
  int num_uniform_blocks;
  int num_images;
  binocle_uniform_block_t uniform_blocks[BINOCLE_MAX_SHADERSTAGE_UBS];
  binocle_shader_image_t images[BINOCLE_MAX_SHADERSTAGE_IMAGES];
} binocle_shader_stage_t;

typedef struct binocle_shader_common_t {
  binocle_shader_stage_t stage[BINOCLE_NUM_SHADER_STAGES];
} binocle_shader_common_t;

typedef struct binocle_gl_context_desc {
} binocle_gl_context_desc;

typedef struct binocle_mtl_context_desc {
  void *mtl_view;
  uint32_t ub_size;
} binocle_mtl_context_desc;

typedef struct binocle_context_desc {
  binocle_pixel_format color_format;
  binocle_pixel_format depth_format;
  int sample_count;
  binocle_gl_context_desc gl;
  binocle_mtl_context_desc mtl;
} binocle_context_desc;

typedef struct binocle_backend_desc {
  uint32_t buffer_pool_size;
  uint32_t image_pool_size;
  uint32_t shader_pool_size;
  uint32_t uniform_buffer_size;
  uint32_t sampler_cache_size;
  binocle_context_desc context;
} binocle_backend_desc;

#endif // BINOCLE_BINOCLE_BACKEND_TYPES_H
