//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_BINOCLE_BACKEND_TYPES_H
#define BINOCLE_BINOCLE_BACKEND_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "binocle_color.h"
#include "binocle_blend.h"

#define BINOCLE_DEFAULT_UB_SIZE (4 * 1024 * 1024)
#define BINOCLE_DEFAULT_BUFFER_POOL_SIZE (128)
#define BINOCLE_DEFAULT_IMAGE_POOL_SIZE (128)
#define BINOCLE_DEFAULT_SHADER_POOL_SIZE (32)
#define BINOCLE_DEFAULT_PIPELINE_POOL_SIZE (64)
#define BINOCLE_DEFAULT_PASS_POOL_SIZE (16)
#define BINOCLE_DEFAULT_CONTEXT_POOL_SIZE (16)
#define BINOCLE_DEFAULT_SAMPLER_CACHE_CAPACITY (64)
#define BINOCLE_NUM_INFLIGHT_FRAMES (1)
#define BINOCLE_MAX_MIPMAPS (16)
#define BINOCLE_MAX_SHADERSTAGE_IMAGES (12)
#define BINOCLE_MAX_SHADERSTAGE_UBS (4)
#define BINOCLE_MAX_SHADERSTAGE_BUFFERS (8)
#define BINOCLE_MAX_COLOR_ATTACHMENTS (4)
#define BINOCLE_NUM_SHADER_STAGES (2)
#define BINOCLE_MAX_UB_MEMBERS (256) // (16)
#define BINOCLE_MAX_VERTEX_ATTRIBUTES (16)
#define BINOCLE_STRING_SIZE (16)
#define BINOCLE_DEF(val, def) (((val) == 0) ? (def) : (val))
#define BINOCLE_DEF_FLT(val, def) (((val) == 0.0f) ? (def) : (val))
#define BINOCLE_MIN(a,b) ((a<b)?a:b)
#define BINOCLE_MAX(a,b) ((a>b)?a:b)
#define BINOCLE_CLAMP(v,v0,v1) ((v<v0)?(v0):((v>v1)?(v1):(v)))
#define BINOCLE_FEQUAL(val,cmp,delta) (((val-cmp)> -delta)&&((val-cmp)<delta))
#define BINOCLE_RANGE(x) (binocle_range){ &x, sizeof(x) }
#define BINOCLE_RANGE_REF(x) &(binocle_range){ &x, sizeof(x) }

/* default clear values */
#ifndef BINOCLE_DEFAULT_CLEAR_RED
#define BINOCLE_DEFAULT_CLEAR_RED (0.5f)
#endif
#ifndef BINOCLE_DEFAULT_CLEAR_GREEN
#define BINOCLE_DEFAULT_CLEAR_GREEN (0.5f)
#endif
#ifndef BINOCLE_DEFAULT_CLEAR_BLUE
#define BINOCLE_DEFAULT_CLEAR_BLUE (0.5f)
#endif
#ifndef BINOCLE_DEFAULT_CLEAR_ALPHA
#define BINOCLE_DEFAULT_CLEAR_ALPHA (1.0f)
#endif
#ifndef BINOCLE_DEFAULT_CLEAR_DEPTH
#define BINOCLE_DEFAULT_CLEAR_DEPTH (1.0f)
#endif
#ifndef BINOCLE_DEFAULT_CLEAR_STENCIL
#define BINOCLE_DEFAULT_CLEAR_STENCIL (0)
#endif

typedef struct binocle_buffer { uint32_t id; } binocle_buffer;
typedef struct binocle_image { uint32_t id; } binocle_image;
typedef struct binocle_shader { uint32_t id; } binocle_shader;
typedef struct binocle_pipeline { uint32_t id; } binocle_pipeline;
typedef struct binocle_pass { uint32_t id; } binocle_pass;
typedef struct binocle_context { uint32_t id; } binocle_context;

/* fixed-size string */
typedef struct binocle_str_t {
  char buf[BINOCLE_STRING_SIZE];
} binocle_str_t;

typedef enum binocle_pixel_format {
  BINOCLE_PIXELFORMAT_DEFAULT,    /* value 0 reserved for default-init */
  BINOCLE_PIXELFORMAT_NONE,

  BINOCLE_PIXELFORMAT_R8,
  BINOCLE_PIXELFORMAT_R8SN,
  BINOCLE_PIXELFORMAT_R8UI,
  BINOCLE_PIXELFORMAT_R8SI,

  BINOCLE_PIXELFORMAT_R16,
  BINOCLE_PIXELFORMAT_R16SN,
  BINOCLE_PIXELFORMAT_R16UI,
  BINOCLE_PIXELFORMAT_R16SI,
  BINOCLE_PIXELFORMAT_R16F,
  BINOCLE_PIXELFORMAT_RG8,
  BINOCLE_PIXELFORMAT_RG8SN,
  BINOCLE_PIXELFORMAT_RG8UI,
  BINOCLE_PIXELFORMAT_RG8SI,

  BINOCLE_PIXELFORMAT_R32UI,
  BINOCLE_PIXELFORMAT_R32SI,
  BINOCLE_PIXELFORMAT_R32F,
  BINOCLE_PIXELFORMAT_RG16,
  BINOCLE_PIXELFORMAT_RG16SN,
  BINOCLE_PIXELFORMAT_RG16UI,
  BINOCLE_PIXELFORMAT_RG16SI,
  BINOCLE_PIXELFORMAT_RG16F,
  BINOCLE_PIXELFORMAT_RGBA8,
  BINOCLE_PIXELFORMAT_RGBA8SN,
  BINOCLE_PIXELFORMAT_RGBA8UI,
  BINOCLE_PIXELFORMAT_RGBA8SI,
  BINOCLE_PIXELFORMAT_BGRA8,
  BINOCLE_PIXELFORMAT_RGB10A2,
  BINOCLE_PIXELFORMAT_RG11B10F,

  BINOCLE_PIXELFORMAT_RG32UI,
  BINOCLE_PIXELFORMAT_RG32SI,
  BINOCLE_PIXELFORMAT_RG32F,
  BINOCLE_PIXELFORMAT_RGBA16,
  BINOCLE_PIXELFORMAT_RGBA16SN,
  BINOCLE_PIXELFORMAT_RGBA16UI,
  BINOCLE_PIXELFORMAT_RGBA16SI,
  BINOCLE_PIXELFORMAT_RGBA16F,

  BINOCLE_PIXELFORMAT_RGBA32UI,
  BINOCLE_PIXELFORMAT_RGBA32SI,
  BINOCLE_PIXELFORMAT_RGBA32F,

  BINOCLE_PIXELFORMAT_DEPTH,
  BINOCLE_PIXELFORMAT_DEPTH_STENCIL,

  BINOCLE_PIXELFORMAT_BC1_RGBA,
  BINOCLE_PIXELFORMAT_BC2_RGBA,
  BINOCLE_PIXELFORMAT_BC3_RGBA,
  BINOCLE_PIXELFORMAT_BC4_R,
  BINOCLE_PIXELFORMAT_BC4_RSN,
  BINOCLE_PIXELFORMAT_BC5_RG,
  BINOCLE_PIXELFORMAT_BC5_RGSN,
  BINOCLE_PIXELFORMAT_BC6H_RGBF,
  BINOCLE_PIXELFORMAT_BC6H_RGBUF,
  BINOCLE_PIXELFORMAT_BC7_RGBA,
  BINOCLE_PIXELFORMAT_PVRTC_RGB_2BPP,
  BINOCLE_PIXELFORMAT_PVRTC_RGB_4BPP,
  BINOCLE_PIXELFORMAT_PVRTC_RGBA_2BPP,
  BINOCLE_PIXELFORMAT_PVRTC_RGBA_4BPP,
  BINOCLE_PIXELFORMAT_ETC2_RGB8,
  BINOCLE_PIXELFORMAT_ETC2_RGB8A1,
  BINOCLE_PIXELFORMAT_ETC2_RGBA8,
  BINOCLE_PIXELFORMAT_ETC2_RG11,
  BINOCLE_PIXELFORMAT_ETC2_RG11SN,

  BINOCLE_PIXELFORMAT_NUM,
  BINOCLE_PIXELFORMAT_FORCE_U32 = 0x7FFFFFFF
} binocle_pixel_format;

/*
    binocle_index_type

    Indicates whether indexed rendering (fetching vertex-indices from an
    index buffer) is used, and if yes, the index data type (16- or 32-bits).
    This is used in the binocle_pipeline_desc.index_type member when creating a
    pipeline object.

    The default index type is BINOCLE_INDEXTYPE_NONE.
*/
typedef enum binocle_index_type {
  BINOCLE_INDEXTYPE_DEFAULT,   /* value 0 reserved for default-init */
  BINOCLE_INDEXTYPE_NONE,
  BINOCLE_INDEXTYPE_UINT16,
  BINOCLE_INDEXTYPE_UINT32,
  BINOCLE_INDEXTYPE_NUM,
  BINOCLE_INDEXTYPE_FORCE_U32 = 0x7FFFFFFF
} binocle_index_type;

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

typedef enum binocle_buffer_type {
  BINOCLE_BUFFERTYPE_DEFAULT,         /* value 0 reserved for default-init */
  BINOCLE_BUFFERTYPE_VERTEXBUFFER,
  BINOCLE_BUFFERTYPE_INDEXBUFFER,
  BINOCLE_BUFFERTYPE_NUM,
  BINOCLE_BUFFERTYPE_FORCE_U32 = 0x7FFFFFFF
} binocle_buffer_type;

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
    binocle_vertex_step

    Defines whether the input pointer of a vertex input stream is advanced
    'per vertex' or 'per instance'. The default step-func is
    BINOCLE_VERTEXSTEP_PER_VERTEX. BINOCLE_VERTEXSTEP_PER_INSTANCE is used with
    instanced-rendering.

    The vertex-step is part of the vertex-layout definition
    when creating pipeline objects.
*/
typedef enum binocle_vertex_step {
  BINOCLE_VERTEXSTEP_DEFAULT,     /* value 0 reserved for default-init */
  BINOCLE_VERTEXSTEP_PER_VERTEX,
  BINOCLE_VERTEXSTEP_PER_INSTANCE,
  BINOCLE_VERTEXSTEP_NUM,
  BINOCLE_VERTEXSTEP_FORCE_U32 = 0x7FFFFFFF
} binocle_vertex_step;

/*
    binocle_vertex_format

    The data type of a vertex component. This is used to describe
    the layout of vertex data when creating a pipeline object.
*/
typedef enum binocle_vertex_format {
  BINOCLE_VERTEXFORMAT_INVALID,
  BINOCLE_VERTEXFORMAT_FLOAT,
  BINOCLE_VERTEXFORMAT_FLOAT2,
  BINOCLE_VERTEXFORMAT_FLOAT3,
  BINOCLE_VERTEXFORMAT_FLOAT4,
  BINOCLE_VERTEXFORMAT_BYTE4,
  BINOCLE_VERTEXFORMAT_BYTE4N,
  BINOCLE_VERTEXFORMAT_UBYTE4,
  BINOCLE_VERTEXFORMAT_UBYTE4N,
  BINOCLE_VERTEXFORMAT_SHORT2,
  BINOCLE_VERTEXFORMAT_SHORT2N,
  BINOCLE_VERTEXFORMAT_USHORT2N,
  BINOCLE_VERTEXFORMAT_SHORT4,
  BINOCLE_VERTEXFORMAT_SHORT4N,
  BINOCLE_VERTEXFORMAT_USHORT4N,
  BINOCLE_VERTEXFORMAT_UINT10_N2,
  BINOCLE_VERTEXFORMAT_NUM,
  BINOCLE_VERTEXFORMAT_FORCE_U32 = 0x7FFFFFFF
} binocle_vertex_format;

/*
    binocle_compare_func

    The compare-function for depth- and stencil-ref tests.
    This is used when creating pipeline objects in the members:

    binocle_pipeline_desc
        .depth
            .compare
        .stencil
            .front.compare
            .back.compar

    The default compare func for depth- and stencil-tests is
    BINOCLE_COMPAREFUNC_ALWAYS.
*/
typedef enum binocle_compare_func {
  BINOCLE_COMPAREFUNC_DEFAULT,    /* value 0 reserved for default-init */
  BINOCLE_COMPAREFUNC_NEVER,
  BINOCLE_COMPAREFUNC_LESS,
  BINOCLE_COMPAREFUNC_EQUAL,
  BINOCLE_COMPAREFUNC_LESS_EQUAL,
  BINOCLE_COMPAREFUNC_GREATER,
  BINOCLE_COMPAREFUNC_NOT_EQUAL,
  BINOCLE_COMPAREFUNC_GREATER_EQUAL,
  BINOCLE_COMPAREFUNC_ALWAYS,
  BINOCLE_COMPAREFUNC_NUM,
  BINOCLE_COMPAREFUNC_FORCE_U32 = 0x7FFFFFFF
} binocle_compare_func;

/*
    binocle_stencil_op

    The operation performed on a currently stored stencil-value when a
    comparison test passes or fails. This is used when creating a pipeline
    object in the members:

    binocle_pipeline_desc
        .stencil
            .front
                .fail_op
                .depth_fail_op
                .pass_op
            .back
                .fail_op
                .depth_fail_op
                .pass_op

    The default value is BINOCLE_STENCILOP_KEEP.
*/
typedef enum binocle_stencil_op {
  BINOCLE_STENCILOP_DEFAULT,      /* value 0 reserved for default-init */
  BINOCLE_STENCILOP_KEEP,
  BINOCLE_STENCILOP_ZERO,
  BINOCLE_STENCILOP_REPLACE,
  BINOCLE_STENCILOP_INCR_CLAMP,
  BINOCLE_STENCILOP_DECR_CLAMP,
  BINOCLE_STENCILOP_INVERT,
  BINOCLE_STENCILOP_INCR_WRAP,
  BINOCLE_STENCILOP_DECR_WRAP,
  BINOCLE_STENCILOP_NUM,
  BINOCLE_STENCILOP_FORCE_U32 = 0x7FFFFFFF
} binocle_stencil_op;

/*
    binocle_blend_op

    Describes how the source and destination values are combined in the
    fragment blending operation. It is used in the following members when
    creating a pipeline object:

    binocle_pipeline_desc
        .colors[i]
            .blend
                .op_rgb
                .op_alpha

    The default value is BINOCLE_BLENDOP_ADD.
*/
typedef enum binocle_blend_op {
  BINOCLE_BLENDOP_DEFAULT,    /* value 0 reserved for default-init */
  BINOCLE_BLENDOP_ADD,
  BINOCLE_BLENDOP_SUBTRACT,
  BINOCLE_BLENDOP_REVERSE_SUBTRACT,
  BINOCLE_BLENDOP_NUM,
  BINOCLE_BLENDOP_FORCE_U32 = 0x7FFFFFFF
} binocle_blend_op;

/*
    binocle_color_mask

    Selects the active color channels when writing a fragment color to the
    framebuffer. This is used in the members
    binocle_pipeline_desc.colors[i].write_mask when creating a pipeline object.

    The default colormask is BINOCLE_COLORMASK_RGBA (write all colors channels)

    NOTE: since the color mask value 0 is reserved for the default value
    (BINOCLE_COLORMASK_RGBA), use BINOCLE_COLORMASK_NONE if all color channels
    should be disabled.
*/
typedef enum binocle_color_mask {
  BINOCLE_COLORMASK_DEFAULT = 0,    /* value 0 reserved for default-init */
  BINOCLE_COLORMASK_NONE   = 0x10,   /* special value for 'all channels disabled */
  BINOCLE_COLORMASK_R      = 0x1,
  BINOCLE_COLORMASK_G      = 0x2,
  BINOCLE_COLORMASK_RG     = 0x3,
  BINOCLE_COLORMASK_B      = 0x4,
  BINOCLE_COLORMASK_RB     = 0x5,
  BINOCLE_COLORMASK_GB     = 0x6,
  BINOCLE_COLORMASK_RGB    = 0x7,
  BINOCLE_COLORMASK_A      = 0x8,
  BINOCLE_COLORMASK_RA     = 0x9,
  BINOCLE_COLORMASK_GA     = 0xA,
  BINOCLE_COLORMASK_RGA    = 0xB,
  BINOCLE_COLORMASK_BA     = 0xC,
  BINOCLE_COLORMASK_RBA    = 0xD,
  BINOCLE_COLORMASK_GBA    = 0xE,
  BINOCLE_COLORMASK_RGBA   = 0xF,
  BINOCLE_COLORMASK_FORCE_U32 = 0x7FFFFFFF
} binocle_color_mask;

/*
    binocle_primitive_type

    This is the common subset of 3D primitive types supported across all 3D
    APIs. This is used in the binocle_pipeline_desc.primitive_type member when
    creating a pipeline object.

    The default primitive type is BINOCLE_PRIMITIVETYPE_TRIANGLES.
*/
typedef enum binocle_primitive_type {
  BINOCLE_PRIMITIVETYPE_DEFAULT,  /* value 0 reserved for default-init */
  BINOCLE_PRIMITIVETYPE_POINTS,
  BINOCLE_PRIMITIVETYPE_LINES,
  BINOCLE_PRIMITIVETYPE_LINE_STRIP,
  BINOCLE_PRIMITIVETYPE_TRIANGLES,
  BINOCLE_PRIMITIVETYPE_TRIANGLE_STRIP,
  BINOCLE_PRIMITIVETYPE_NUM,
  BINOCLE_PRIMITIVETYPE_FORCE_U32 = 0x7FFFFFFF
} binocle_primitive_type;

/*
    binocle_cull_mode

    The face-culling mode, this is used in the
    binocle_pipeline_desc.cull_mode member when creating a
    pipeline object.

    The default cull mode is BINOCLE_CULLMODE_NONE
*/
typedef enum binocle_cull_mode {
  BINOCLE_CULLMODE_DEFAULT,   /* value 0 reserved for default-init */
  BINOCLE_CULLMODE_NONE,
  BINOCLE_CULLMODE_FRONT,
  BINOCLE_CULLMODE_BACK,
  BINOCLE_CULLMODE_NUM,
  BINOCLE_CULLMODE_FORCE_U32 = 0x7FFFFFFF
} binocle_cull_mode;

/*
    binocle_face_winding

    The vertex-winding rule that determines a front-facing primitive. This
    is used in the member binocle_pipeline_desc.face_winding
    when creating a pipeline object.

    The default winding is BINOCLE_FACEWINDING_CW (clockwise)
*/
typedef enum binocle_face_winding {
  BINOCLE_FACEWINDING_DEFAULT,    /* value 0 reserved for default-init */
  BINOCLE_FACEWINDING_CCW,
  BINOCLE_FACEWINDING_CW,
  BINOCLE_FACEWINDING_NUM,
  BINOCLE_FACEWINDING_FORCE_U32 = 0x7FFFFFFF
} binocle_face_winding;

/*
    binocle_range is a pointer-size-pair struct used to pass memory blobs into
    the library. When initialized from a value type (array or struct), you can
    use the BINOCLE_RANGE() macro to build an binocle_range struct. For functions which
    take either a binocle_range pointer, or a (C++) binocle_range reference, use the
    BINOCLE_RANGE_REF macro as a solution which compiles both in C and C++.
*/
typedef struct binocle_range {
  const void* ptr;
  size_t size;
} binocle_range;

/*
    binocle_image_data

    Defines the content of an image through a 2D array
    of binocle_range structs. The first array dimension
    is the cubemap face, and the second array dimension the
    mipmap level.
*/
typedef struct binocle_image_data {
  binocle_range subimage[BINOCLE_CUBEFACE_NUM][BINOCLE_MAX_MIPMAPS];
} binocle_image_data;

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
  binocle_image_data data;
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
    type is used in the binocle_shader_image_desc to describe the
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
    bool force_gles2;
} binocle_gl_context_desc;

typedef struct binocle_mtl_context_desc {
  const void* mtl_view;
} binocle_mtl_context_desc;

typedef struct binocle_pipeline_common_t {
  binocle_shader shader_id;
  binocle_index_type index_type;
  bool vertex_layout_valid[BINOCLE_MAX_SHADERSTAGE_BUFFERS];
  int color_attachment_count;
  binocle_pixel_format color_formats[BINOCLE_MAX_COLOR_ATTACHMENTS];
  binocle_pixel_format depth_format;
  int sample_count;
  float depth_bias;
  float depth_bias_slope_scale;
  float depth_bias_clamp;
  binocle_color blend_color;
} binocle_pipeline_common_t;

typedef struct binocle_buffer_common_t {
  int size;
  int append_pos;
  bool append_overflow;
  binocle_buffer_type type;
  binocle_usage usage;
  uint32_t update_frame_index;
  uint32_t append_frame_index;
  int num_slots;
  int active_slot;
} binocle_buffer_common_t;

typedef struct binocle_buffer_layout_desc {
  int stride;
  binocle_vertex_step step_func;
  int step_rate;
} binocle_buffer_layout_desc;

typedef struct binocle_vertex_attr_desc {
  int buffer_index;
  int offset;
  binocle_vertex_format format;
} binocle_vertex_attr_desc;

typedef struct binocle_layout_desc {
  binocle_buffer_layout_desc buffers[BINOCLE_MAX_SHADERSTAGE_BUFFERS];
  binocle_vertex_attr_desc attrs[BINOCLE_MAX_VERTEX_ATTRIBUTES];
} binocle_layout_desc;

typedef struct binocle_depth_state {
  binocle_pixel_format pixel_format;
  binocle_compare_func compare;
  bool write_enabled;
  float bias;
  float bias_slope_scale;
  float bias_clamp;
} binocle_depth_state;

typedef struct binocle_stencil_face_state {
  binocle_compare_func compare;
  binocle_stencil_op fail_op;
  binocle_stencil_op depth_fail_op;
  binocle_stencil_op pass_op;
} binocle_stencil_face_state;

typedef struct binocle_stencil_state {
  bool enabled;
  binocle_stencil_face_state front;
  binocle_stencil_face_state back;
  uint8_t read_mask;
  uint8_t write_mask;
  uint8_t ref;
} binocle_stencil_state;

typedef struct binocle_blend_state {
  bool enabled;
  binocle_blend_factor src_factor_rgb;
  binocle_blend_factor dst_factor_rgb;
  binocle_blend_op op_rgb;
  binocle_blend_factor src_factor_alpha;
  binocle_blend_factor dst_factor_alpha;
  binocle_blend_op op_alpha;
} binocle_blend_state;

typedef struct binocle_color_state {
  binocle_pixel_format pixel_format;
  binocle_color_mask write_mask;
  binocle_blend_state blend;
} binocle_color_state;

typedef struct binocle_pipeline_desc {
  uint32_t _start_canary;
  binocle_shader shader;
  binocle_layout_desc layout;
  binocle_depth_state depth;
  binocle_stencil_state stencil;
  int color_count;
  binocle_color_state colors[BINOCLE_MAX_COLOR_ATTACHMENTS];
  binocle_primitive_type primitive_type;
  binocle_index_type index_type;
  binocle_cull_mode cull_mode;
  binocle_face_winding face_winding;
  int sample_count;
  binocle_color blend_color;
  bool alpha_to_coverage_enabled;
  const char* label;
  uint32_t _end_canary;
} binocle_pipeline_desc;

typedef struct binocle_pass_attachment_common_t {
  binocle_image image_id;
  int mip_level;
  int slice;
} binocle_pass_attachment_common_t;

typedef struct binocle_pass_common_t {
  int num_color_atts;
  binocle_pass_attachment_common_t color_atts[BINOCLE_MAX_COLOR_ATTACHMENTS];
  binocle_pass_attachment_common_t ds_att;
} binocle_pass_common_t;

typedef struct binocle_pass_attachment_desc {
  binocle_image image;
  int mip_level;
  int slice;      /* cube texture: face; array texture: layer; 3D texture: slice */
} binocle_pass_attachment_desc;

typedef struct binocle_pass_desc {
  uint32_t _start_canary;
  binocle_pass_attachment_desc color_attachments[BINOCLE_MAX_COLOR_ATTACHMENTS];
  binocle_pass_attachment_desc depth_stencil_attachment;
  const char* label;
  uint32_t _end_canary;
} binocle_pass_desc;

/*
    binocle_action

    Defines what action should be performed at the start of a render pass:

    BINOCLE_ACTION_CLEAR:    clear the render target image
    BINOCLE_ACTION_LOAD:     load the previous content of the render target image
    BINOCLE_ACTION_DONTCARE: leave the render target image content undefined

    This is used in the binocle_pass_action structure.

    The default action for all pass attachments is BINOCLE_ACTION_CLEAR, with the
    clear color rgba = {0.5f, 0.5f, 0.5f, 1.0f], depth=1.0 and stencil=0.

    If you want to override the default behaviour, it is important to not
    only set the clear color, but the 'action' field as well (as long as this
    is in its _BINOCLE_ACTION_DEFAULT, the value fields will be ignored).
*/
typedef enum binocle_action {
  BINOCLE_ACTION_DEFAULT,
  BINOCLE_ACTION_CLEAR,
  BINOCLE_ACTION_LOAD,
  BINOCLE_ACTION_DONTCARE,
  BINOCLE_ACTION_NUM,
  BINOCLE_ACTION_FORCE_U32 = 0x7FFFFFFF
} binocle_action;

typedef struct binocle_color_attachment_action {
  binocle_action action;
  binocle_color value;
} binocle_color_attachment_action;

typedef struct binocle_depth_attachment_action {
  binocle_action action;
  float value;
} binocle_depth_attachment_action;

typedef struct binocle_stencil_attachment_action {
  binocle_action action;
  uint8_t value;
} binocle_stencil_attachment_action;

/*
    binocle_pass_action

    The binocle_pass_action struct defines the actions to be performed
    at the start of a rendering pass in the functions binocle_begin_pass()
    and binocle_begin_default_pass().

    A separate action and clear values can be defined for each
    color attachment, and for the depth-stencil attachment.

    The default clear values are defined by the macros:

    - BINOCLE_DEFAULT_CLEAR_RED:     0.5f
    - BINOCLE_DEFAULT_CLEAR_GREEN:   0.5f
    - BINOCLE_DEFAULT_CLEAR_BLUE:    0.5f
    - BINOCLE_DEFAULT_CLEAR_ALPHA:   1.0f
    - BINOCLE_DEFAULT_CLEAR_DEPTH:   1.0f
    - BINOCLE_DEFAULT_CLEAR_STENCIL: 0
*/typedef struct binocle_pass_action {
  uint32_t _start_canary;
  binocle_color_attachment_action colors[BINOCLE_MAX_COLOR_ATTACHMENTS];
  binocle_depth_attachment_action depth;
  binocle_stencil_attachment_action stencil;
  uint32_t _end_canary;
} binocle_pass_action;

/*
    binocle_bindings

    The binocle_bindings structure defines the resource binding slots
    of the render pipeline, used as argument to the
    binocle_apply_bindings() function.

    A resource binding struct contains:

    - 1..N vertex buffers
    - 0..N vertex buffer offsets
    - 0..1 index buffers
    - 0..1 index buffer offsets
    - 0..N vertex shader stage images
    - 0..N fragment shader stage images

    The max number of vertex buffer and shader stage images
    are defined by the BINOCLE_MAX_SHADERSTAGE_BUFFERS and
    BINOCLE_MAX_SHADERSTAGE_IMAGES configuration constants.

    The optional buffer offsets can be used to put different unrelated
    chunks of vertex- and/or index-data into the same buffer objects.
*/
typedef struct binocle_bindings {
  uint32_t _start_canary;
  binocle_buffer vertex_buffers[BINOCLE_MAX_SHADERSTAGE_BUFFERS];
  int vertex_buffer_offsets[BINOCLE_MAX_SHADERSTAGE_BUFFERS];
  binocle_buffer index_buffer;
  int index_buffer_offset;
  binocle_image vs_images[BINOCLE_MAX_SHADERSTAGE_IMAGES];
  binocle_image fs_images[BINOCLE_MAX_SHADERSTAGE_IMAGES];
  uint32_t _end_canary;
} binocle_bindings;

/*
    binocle_buffer_desc

    Creation parameters for binocle_buffer objects, used in the
    binocle_make_buffer() call.

    The default configuration is:

    .size:      0       (*must* be >0 for buffers without data)
    .type:      BINOCLE_BUFFERTYPE_VERTEXBUFFER
    .usage:     BINOCLE_USAGE_IMMUTABLE
    .data.ptr   0       (*must* be valid for immutable buffers)
    .data.size  0       (*must* be > 0 for immutable buffers)
    .label      0       (optional string label for trace hooks)

    The label will be ignored, it is only useful
    when hooking into binocle_make_buffer() or binocle_init_buffer() via
    the binocle_install_trace_hooks() function.

    For immutable buffers which are initialized with initial data,
    keep the .size item zero-initialized, and set the size together with the
    pointer to the initial data in the .data item.

    For mutable buffers without initial data, keep the .data item
    zero-initialized, and set the buffer size in the .size item instead.

    You can also set both size values, but currently both size values must
    be identical (this may change in the future when the dynamic resource
    management may become more flexible).

    ADVANCED TOPIC: Injecting native 3D-API buffers:

    The following struct members allow to inject your own GL, Metal
    or D3D11 buffers into the backend:

    .gl_buffers[BINOCLE_NUM_INFLIGHT_FRAMES]
    .mtl_buffers[BINOCLE_NUM_INFLIGHT_FRAMES]
    .d3d11_buffer

    You must still provide all other struct items except the .data item, and
    these must match the creation parameters of the native buffers you
    provide. For BINOCLE_USAGE_IMMUTABLE, only provide a single native 3D-API
    buffer, otherwise you need to provide BINOCLE_NUM_INFLIGHT_FRAMES buffers
    (only for GL and Metal, not D3D11). Providing multiple buffers for GL and
    Metal is necessary because the backend will rotate through them when
    calling binocle_update_buffer() to prevent lock-stalls.

    Note that it is expected that immutable injected buffer have already been
    initialized with content, and the .content member must be 0!

    Also you need to call binocle_reset_state_cache() after calling native 3D-API
    functions, and before calling any backend function.
*/
typedef struct binocle_buffer_desc {
  uint32_t _start_canary;
  size_t size;
  binocle_buffer_type type;
  binocle_usage usage;
  binocle_range data;
  const char* label;
  /* GL specific */
  uint32_t gl_buffers[BINOCLE_NUM_INFLIGHT_FRAMES];
  /* Metal specific */
  const void* mtl_buffers[BINOCLE_NUM_INFLIGHT_FRAMES];
  uint32_t _end_canary;
} binocle_buffer_desc;

/*
    Runtime information about resource limits, returned by binocle_backend_query_limit()
*/
typedef struct binocle_limits {
  int max_image_size_2d;          // max width/height of BINOCLE_IMAGETYPE_2D images
  int max_image_size_cube;        // max width/height of BINOCLE_IMAGETYPE_CUBE images
  int max_image_size_3d;          // max width/height/depth of BINOCLE_IMAGETYPE_3D images
  int max_image_size_array;       // max width/height of BINOCLE_IMAGETYPE_ARRAY images
  int max_image_array_layers;     // max number of layers in BINOCLE_IMAGETYPE_ARRAY images
  int max_vertex_attrs;           // <= BINOCLE_MAX_VERTEX_ATTRIBUTES (only on some GLES2 impls)
} binocle_limits;

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
  uint32_t pipeline_pool_size;
  uint32_t pass_pool_size;
  uint32_t context_pool_size;
  uint32_t uniform_buffer_size;
  uint32_t sampler_cache_size;
  binocle_context_desc context;
} binocle_backend_desc;

#endif // BINOCLE_BINOCLE_BACKEND_TYPES_H
