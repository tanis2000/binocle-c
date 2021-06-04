//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_backend_metal.h"
#import <QuartzCore/CAMetalLayer.h>
#include <stdbool.h>
#include "binocle_sampler_cache.h"
#include "binocle_backend.h"
#include "../binocle_log.h"

MTLSamplerAddressMode binocle_backend_mtl_address_mode(binocle_wrap w) {
  switch (w) {
  case BINOCLE_WRAP_REPEAT:
    return MTLSamplerAddressModeRepeat;
  case BINOCLE_WRAP_CLAMP_TO_EDGE:
    return MTLSamplerAddressModeClampToEdge;
#if defined(BINOCLE_MACOS)
  case BINOCLE_WRAP_CLAMP_TO_BORDER:
    return MTLSamplerAddressModeClampToBorderColor;
#else
  /* clamp-to-border not supported on iOS, fall back to clamp-to-edge */
  case BINOCLE_WRAP_CLAMP_TO_BORDER:
    return MTLSamplerAddressModeClampToEdge;
#endif
  case BINOCLE_WRAP_MIRRORED_REPEAT:
    return MTLSamplerAddressModeMirrorRepeat;
  default:
    assert(false);
    return (MTLSamplerAddressMode)0;
  }
}

MTLSamplerMinMagFilter binocle_backend_mtl_minmag_filter(binocle_filter f) {
  switch (f) {
  case BINOCLE_FILTER_NEAREST:
  case BINOCLE_FILTER_NEAREST_MIPMAP_NEAREST:
  case BINOCLE_FILTER_NEAREST_MIPMAP_LINEAR:
    return MTLSamplerMinMagFilterNearest;
  case BINOCLE_FILTER_LINEAR:
  case BINOCLE_FILTER_LINEAR_MIPMAP_NEAREST:
  case BINOCLE_FILTER_LINEAR_MIPMAP_LINEAR:
    return MTLSamplerMinMagFilterLinear;
  default:
    assert(false);
    return (MTLSamplerMinMagFilter)0;
  }
}

MTLSamplerMipFilter binocle_backend_mtl_mip_filter(binocle_filter f) {
  switch (f) {
  case BINOCLE_FILTER_NEAREST:
  case BINOCLE_FILTER_LINEAR:
    return MTLSamplerMipFilterNotMipmapped;
  case BINOCLE_FILTER_NEAREST_MIPMAP_NEAREST:
  case BINOCLE_FILTER_LINEAR_MIPMAP_NEAREST:
    return MTLSamplerMipFilterNearest;
  case BINOCLE_FILTER_NEAREST_MIPMAP_LINEAR:
  case BINOCLE_FILTER_LINEAR_MIPMAP_LINEAR:
    return MTLSamplerMipFilterLinear;
  default:
    assert(false);
    return (MTLSamplerMipFilter)0;
  }
}

MTLTextureType binocle_backend_mtl_texture_type(binocle_image_type t) {
  switch (t) {
  case BINOCLE_IMAGETYPE_2D:
    return MTLTextureType2D;
  case BINOCLE_IMAGETYPE_CUBE:
    return MTLTextureTypeCube;
  case BINOCLE_IMAGETYPE_3D:
    return MTLTextureType3D;
  case BINOCLE_IMAGETYPE_ARRAY:
    return MTLTextureType2DArray;
  default:
    assert(false);
    return (MTLTextureType)0;
  }
}

MTLPixelFormat binocle_backend_mtl_pixel_format(binocle_pixel_format fmt) {
  switch (fmt) {
  case BINOCLE_PIXELFORMAT_R8:
    return MTLPixelFormatR8Unorm;
  case BINOCLE_PIXELFORMAT_R8SN:
    return MTLPixelFormatR8Snorm;
  case BINOCLE_PIXELFORMAT_R8UI:
    return MTLPixelFormatR8Uint;
  case BINOCLE_PIXELFORMAT_R8SI:
    return MTLPixelFormatR8Sint;
  case BINOCLE_PIXELFORMAT_R16:
    return MTLPixelFormatR16Unorm;
  case BINOCLE_PIXELFORMAT_R16SN:
    return MTLPixelFormatR16Snorm;
  case BINOCLE_PIXELFORMAT_R16UI:
    return MTLPixelFormatR16Uint;
  case BINOCLE_PIXELFORMAT_R16SI:
    return MTLPixelFormatR16Sint;
  case BINOCLE_PIXELFORMAT_R16F:
    return MTLPixelFormatR16Float;
  case BINOCLE_PIXELFORMAT_RG8:
    return MTLPixelFormatRG8Unorm;
  case BINOCLE_PIXELFORMAT_RG8SN:
    return MTLPixelFormatRG8Snorm;
  case BINOCLE_PIXELFORMAT_RG8UI:
    return MTLPixelFormatRG8Uint;
  case BINOCLE_PIXELFORMAT_RG8SI:
    return MTLPixelFormatRG8Sint;
  case BINOCLE_PIXELFORMAT_R32UI:
    return MTLPixelFormatR32Uint;
  case BINOCLE_PIXELFORMAT_R32SI:
    return MTLPixelFormatR32Sint;
  case BINOCLE_PIXELFORMAT_R32F:
    return MTLPixelFormatR32Float;
  case BINOCLE_PIXELFORMAT_RG16:
    return MTLPixelFormatRG16Unorm;
  case BINOCLE_PIXELFORMAT_RG16SN:
    return MTLPixelFormatRG16Snorm;
  case BINOCLE_PIXELFORMAT_RG16UI:
    return MTLPixelFormatRG16Uint;
  case BINOCLE_PIXELFORMAT_RG16SI:
    return MTLPixelFormatRG16Sint;
  case BINOCLE_PIXELFORMAT_RG16F:
    return MTLPixelFormatRG16Float;
  case BINOCLE_PIXELFORMAT_RGBA8:
    return MTLPixelFormatRGBA8Unorm;
  case BINOCLE_PIXELFORMAT_RGBA8SN:
    return MTLPixelFormatRGBA8Snorm;
  case BINOCLE_PIXELFORMAT_RGBA8UI:
    return MTLPixelFormatRGBA8Uint;
  case BINOCLE_PIXELFORMAT_RGBA8SI:
    return MTLPixelFormatRGBA8Sint;
  case BINOCLE_PIXELFORMAT_BGRA8:
    return MTLPixelFormatBGRA8Unorm;
  case BINOCLE_PIXELFORMAT_RGB10A2:
    return MTLPixelFormatRGB10A2Unorm;
  case BINOCLE_PIXELFORMAT_RG11B10F:
    return MTLPixelFormatRG11B10Float;
  case BINOCLE_PIXELFORMAT_RG32UI:
    return MTLPixelFormatRG32Uint;
  case BINOCLE_PIXELFORMAT_RG32SI:
    return MTLPixelFormatRG32Sint;
  case BINOCLE_PIXELFORMAT_RG32F:
    return MTLPixelFormatRG32Float;
  case BINOCLE_PIXELFORMAT_RGBA16:
    return MTLPixelFormatRGBA16Unorm;
  case BINOCLE_PIXELFORMAT_RGBA16SN:
    return MTLPixelFormatRGBA16Snorm;
  case BINOCLE_PIXELFORMAT_RGBA16UI:
    return MTLPixelFormatRGBA16Uint;
  case BINOCLE_PIXELFORMAT_RGBA16SI:
    return MTLPixelFormatRGBA16Sint;
  case BINOCLE_PIXELFORMAT_RGBA16F:
    return MTLPixelFormatRGBA16Float;
  case BINOCLE_PIXELFORMAT_RGBA32UI:
    return MTLPixelFormatRGBA32Uint;
  case BINOCLE_PIXELFORMAT_RGBA32SI:
    return MTLPixelFormatRGBA32Sint;
  case BINOCLE_PIXELFORMAT_RGBA32F:
    return MTLPixelFormatRGBA32Float;
  case BINOCLE_PIXELFORMAT_DEPTH:
    return MTLPixelFormatDepth32Float;
  case BINOCLE_PIXELFORMAT_DEPTH_STENCIL:
    return MTLPixelFormatDepth32Float_Stencil8;
#if defined(BINOCLE_MACOS)
  case BINOCLE_PIXELFORMAT_BC1_RGBA:
    return MTLPixelFormatBC1_RGBA;
  case BINOCLE_PIXELFORMAT_BC2_RGBA:
    return MTLPixelFormatBC2_RGBA;
  case BINOCLE_PIXELFORMAT_BC3_RGBA:
    return MTLPixelFormatBC3_RGBA;
  case BINOCLE_PIXELFORMAT_BC4_R:
    return MTLPixelFormatBC4_RUnorm;
  case BINOCLE_PIXELFORMAT_BC4_RSN:
    return MTLPixelFormatBC4_RSnorm;
  case BINOCLE_PIXELFORMAT_BC5_RG:
    return MTLPixelFormatBC5_RGUnorm;
  case BINOCLE_PIXELFORMAT_BC5_RGSN:
    return MTLPixelFormatBC5_RGSnorm;
  case BINOCLE_PIXELFORMAT_BC6H_RGBF:
    return MTLPixelFormatBC6H_RGBFloat;
  case BINOCLE_PIXELFORMAT_BC6H_RGBUF:
    return MTLPixelFormatBC6H_RGBUfloat;
  case BINOCLE_PIXELFORMAT_BC7_RGBA:
    return MTLPixelFormatBC7_RGBAUnorm;
#else
  case BINOCLE_PIXELFORMAT_PVRTC_RGB_2BPP:
    return MTLPixelFormatPVRTC_RGB_2BPP;
  case BINOCLE_PIXELFORMAT_PVRTC_RGB_4BPP:
    return MTLPixelFormatPVRTC_RGB_4BPP;
  case BINOCLE_PIXELFORMAT_PVRTC_RGBA_2BPP:
    return MTLPixelFormatPVRTC_RGBA_2BPP;
  case BINOCLE_PIXELFORMAT_PVRTC_RGBA_4BPP:
    return MTLPixelFormatPVRTC_RGBA_4BPP;
  case BINOCLE_PIXELFORMAT_ETC2_RGB8:
    return MTLPixelFormatETC2_RGB8;
  case BINOCLE_PIXELFORMAT_ETC2_RGB8A1:
    return MTLPixelFormatETC2_RGB8A1;
  case BINOCLE_PIXELFORMAT_ETC2_RGBA8:
    return MTLPixelFormatEAC_RGBA8;
  case BINOCLE_PIXELFORMAT_ETC2_RG11:
    return MTLPixelFormatEAC_RG11Unorm;
  case BINOCLE_PIXELFORMAT_ETC2_RG11SN:
    return MTLPixelFormatEAC_RG11Snorm;
#endif
  default:
    return MTLPixelFormatInvalid;
  }
}

bool binocle_backend_mtl_is_pvrtc(binocle_pixel_format fmt) {
  switch (fmt) {
  case BINOCLE_PIXELFORMAT_PVRTC_RGB_2BPP:
  case BINOCLE_PIXELFORMAT_PVRTC_RGB_4BPP:
  case BINOCLE_PIXELFORMAT_PVRTC_RGBA_2BPP:
  case BINOCLE_PIXELFORMAT_PVRTC_RGBA_4BPP:
    return true;
  default:
    return false;
  }
}

MTLPrimitiveType binocle_backend_mtl_primitive_type(binocle_primitive_type t) {
  switch (t) {
  case BINOCLE_PRIMITIVETYPE_POINTS:
    return MTLPrimitiveTypePoint;
  case BINOCLE_PRIMITIVETYPE_LINES:
    return MTLPrimitiveTypeLine;
  case BINOCLE_PRIMITIVETYPE_LINE_STRIP:
    return MTLPrimitiveTypeLineStrip;
  case BINOCLE_PRIMITIVETYPE_TRIANGLES:
    return MTLPrimitiveTypeTriangle;
  case BINOCLE_PRIMITIVETYPE_TRIANGLE_STRIP:
    return MTLPrimitiveTypeTriangleStrip;
  default:
    assert(false);
    return (MTLPrimitiveType)0;
  }
}

MTLColorWriteMask binocle_backend_mtl_color_write_mask(binocle_color_mask m) {
  MTLColorWriteMask mtl_mask = MTLColorWriteMaskNone;
  if (m & BINOCLE_COLORMASK_R) {
    mtl_mask |= MTLColorWriteMaskRed;
  }
  if (m & BINOCLE_COLORMASK_G) {
    mtl_mask |= MTLColorWriteMaskGreen;
  }
  if (m & BINOCLE_COLORMASK_B) {
    mtl_mask |= MTLColorWriteMaskBlue;
  }
  if (m & BINOCLE_COLORMASK_A) {
    mtl_mask |= MTLColorWriteMaskAlpha;
  }
  return mtl_mask;
}

MTLBlendOperation binocle_backend_mtl_blend_op(binocle_blend_op op) {
  switch (op) {
  case BINOCLE_BLENDOP_ADD:
    return MTLBlendOperationAdd;
  case BINOCLE_BLENDOP_SUBTRACT:
    return MTLBlendOperationSubtract;
  case BINOCLE_BLENDOP_REVERSE_SUBTRACT:
    return MTLBlendOperationReverseSubtract;
  default:
    assert(false);
    return (MTLBlendOperation)0;
  }
}

MTLBlendFactor binocle_backend_mtl_blend_factor(binocle_blend_factor f) {
  switch (f) {
  case BINOCLE_BLENDFACTOR_ZERO:
    return MTLBlendFactorZero;
  case BINOCLE_BLENDFACTOR_ONE:
    return MTLBlendFactorOne;
  case BINOCLE_BLENDFACTOR_SRC_COLOR:
    return MTLBlendFactorSourceColor;
  case BINOCLE_BLENDFACTOR_ONE_MINUS_SRC_COLOR:
    return MTLBlendFactorOneMinusSourceColor;
  case BINOCLE_BLENDFACTOR_SRC_ALPHA:
    return MTLBlendFactorSourceAlpha;
  case BINOCLE_BLENDFACTOR_ONE_MINUS_SRC_ALPHA:
    return MTLBlendFactorOneMinusSourceAlpha;
  case BINOCLE_BLENDFACTOR_DST_COLOR:
    return MTLBlendFactorDestinationColor;
  case BINOCLE_BLENDFACTOR_ONE_MINUS_DST_COLOR:
    return MTLBlendFactorOneMinusDestinationColor;
  case BINOCLE_BLENDFACTOR_DST_ALPHA:
    return MTLBlendFactorDestinationAlpha;
  case BINOCLE_BLENDFACTOR_ONE_MINUS_DST_ALPHA:
    return MTLBlendFactorOneMinusDestinationAlpha;
  case BINOCLE_BLENDFACTOR_SRC_ALPHA_SATURATED:
    return MTLBlendFactorSourceAlphaSaturated;
  case BINOCLE_BLENDFACTOR_BLEND_COLOR:
    return MTLBlendFactorBlendColor;
  case BINOCLE_BLENDFACTOR_ONE_MINUS_BLEND_COLOR:
    return MTLBlendFactorOneMinusBlendColor;
  case BINOCLE_BLENDFACTOR_BLEND_ALPHA:
    return MTLBlendFactorBlendAlpha;
  case BINOCLE_BLENDFACTOR_ONE_MINUS_BLEND_ALPHA:
    return MTLBlendFactorOneMinusBlendAlpha;
  default:
    assert(false);
    return (MTLBlendFactor)0;
  }
}

MTLCompareFunction binocle_backend_mtl_compare_func(binocle_compare_func f) {
  switch (f) {
  case BINOCLE_COMPAREFUNC_NEVER:
    return MTLCompareFunctionNever;
  case BINOCLE_COMPAREFUNC_LESS:
    return MTLCompareFunctionLess;
  case BINOCLE_COMPAREFUNC_EQUAL:
    return MTLCompareFunctionEqual;
  case BINOCLE_COMPAREFUNC_LESS_EQUAL:
    return MTLCompareFunctionLessEqual;
  case BINOCLE_COMPAREFUNC_GREATER:
    return MTLCompareFunctionGreater;
  case BINOCLE_COMPAREFUNC_NOT_EQUAL:
    return MTLCompareFunctionNotEqual;
  case BINOCLE_COMPAREFUNC_GREATER_EQUAL:
    return MTLCompareFunctionGreaterEqual;
  case BINOCLE_COMPAREFUNC_ALWAYS:
    return MTLCompareFunctionAlways;
  default:
    assert(false);
    return (MTLCompareFunction)0;
  }
}

MTLStencilOperation binocle_backend_mtl_stencil_op(binocle_stencil_op op) {
  switch (op) {
  case BINOCLE_STENCILOP_KEEP:
    return MTLStencilOperationKeep;
  case BINOCLE_STENCILOP_ZERO:
    return MTLStencilOperationZero;
  case BINOCLE_STENCILOP_REPLACE:
    return MTLStencilOperationReplace;
  case BINOCLE_STENCILOP_INCR_CLAMP:
    return MTLStencilOperationIncrementClamp;
  case BINOCLE_STENCILOP_DECR_CLAMP:
    return MTLStencilOperationDecrementClamp;
  case BINOCLE_STENCILOP_INVERT:
    return MTLStencilOperationInvert;
  case BINOCLE_STENCILOP_INCR_WRAP:
    return MTLStencilOperationIncrementWrap;
  case BINOCLE_STENCILOP_DECR_WRAP:
    return MTLStencilOperationDecrementWrap;
  default:
    assert(false);
    return (MTLStencilOperation)0;
  }
}

MTLCullMode binocle_backend_mtl_cull_mode(binocle_cull_mode m) {
  switch (m) {
  case BINOCLE_CULLMODE_NONE:
    return MTLCullModeNone;
  case BINOCLE_CULLMODE_FRONT:
    return MTLCullModeFront;
  case BINOCLE_CULLMODE_BACK:
    return MTLCullModeBack;
  default:
    assert(false);
    return (MTLCullMode)0;
  }
}

MTLWinding binocle_backend_mtl_winding(binocle_face_winding w) {
  switch (w) {
  case BINOCLE_FACEWINDING_CW:
    return MTLWindingClockwise;
  case BINOCLE_FACEWINDING_CCW:
    return MTLWindingCounterClockwise;
  default:
    assert(false);
    return (MTLWinding)0;
  }
}

MTLIndexType binocle_backend_mtl_index_type(binocle_index_type t) {
  switch (t) {
  case BINOCLE_INDEXTYPE_UINT16:
    return MTLIndexTypeUInt16;
  case BINOCLE_INDEXTYPE_UINT32:
    return MTLIndexTypeUInt32;
  default:
    assert(false);
    return (MTLIndexType)0;
  }
}

int binocle_backend_mtl_index_size(binocle_index_type t) {
  switch (t) {
  case BINOCLE_INDEXTYPE_NONE:
    return 0;
  case BINOCLE_INDEXTYPE_UINT16:
    return 2;
  case BINOCLE_INDEXTYPE_UINT32:
    return 4;
  default:
    assert(false);
    return 0;
  }
}

MTLVertexFormat binocle_backend_mtl_vertex_format(binocle_vertex_format fmt) {
  switch (fmt) {
  case BINOCLE_VERTEXFORMAT_FLOAT:
    return MTLVertexFormatFloat;
  case BINOCLE_VERTEXFORMAT_FLOAT2:
    return MTLVertexFormatFloat2;
  case BINOCLE_VERTEXFORMAT_FLOAT3:
    return MTLVertexFormatFloat3;
  case BINOCLE_VERTEXFORMAT_FLOAT4:
    return MTLVertexFormatFloat4;
  case BINOCLE_VERTEXFORMAT_BYTE4:
    return MTLVertexFormatChar4;
  case BINOCLE_VERTEXFORMAT_BYTE4N:
    return MTLVertexFormatChar4Normalized;
  case BINOCLE_VERTEXFORMAT_UBYTE4:
    return MTLVertexFormatUChar4;
  case BINOCLE_VERTEXFORMAT_UBYTE4N:
    return MTLVertexFormatUChar4Normalized;
  case BINOCLE_VERTEXFORMAT_SHORT2:
    return MTLVertexFormatShort2;
  case BINOCLE_VERTEXFORMAT_SHORT2N:
    return MTLVertexFormatShort2Normalized;
  case BINOCLE_VERTEXFORMAT_USHORT2N:
    return MTLVertexFormatUShort2Normalized;
  case BINOCLE_VERTEXFORMAT_SHORT4:
    return MTLVertexFormatShort4;
  case BINOCLE_VERTEXFORMAT_SHORT4N:
    return MTLVertexFormatShort4Normalized;
  case BINOCLE_VERTEXFORMAT_USHORT4N:
    return MTLVertexFormatUShort4Normalized;
  case BINOCLE_VERTEXFORMAT_UINT10_N2:
    return MTLVertexFormatUInt1010102Normalized;
  default:
    assert(false);
    return (MTLVertexFormat)0;
  }
}

MTLVertexStepFunction
binocle_backend_mtl_step_function(binocle_vertex_step step) {
  switch (step) {
  case BINOCLE_VERTEXSTEP_PER_VERTEX:
    return MTLVertexStepFunctionPerVertex;
  case BINOCLE_VERTEXSTEP_PER_INSTANCE:
    return MTLVertexStepFunctionPerInstance;
  default:
    assert(false);
    return (MTLVertexStepFunction)0;
  }
}

MTLLoadAction binocle_backend_mtl_load_action(binocle_action a) {
  switch (a) {
  case BINOCLE_ACTION_CLEAR:
    return MTLLoadActionClear;
  case BINOCLE_ACTION_LOAD:
    return MTLLoadActionLoad;
  case BINOCLE_ACTION_DONTCARE:
    return MTLLoadActionDontCare;
  default:
    assert(false);
    return (MTLLoadAction)0;
  }
}

void binocle_backend_mtl_init_pool(binocle_mtl_backend_t *mtl, binocle_backend_desc *desc) {
  mtl->idpool.num_slots = 2 *
                             (
                               2 * desc->buffer_pool_size +
                               5 * desc->image_pool_size +
                               4 * desc->shader_pool_size
                             );
  mtl->idpool.pool = [NSMutableArray arrayWithCapacity:mtl->idpool.num_slots];
  NSNull* null = [NSNull null];
  for (uint32_t i = 0; i < mtl->idpool.num_slots; i++) {
    [mtl->idpool.pool addObject:null];
  }
  assert([mtl->idpool.pool count] == mtl->idpool.num_slots);
  /* a queue of currently free slot indices */
  mtl->idpool.free_queue_top = 0;
  mtl->idpool.free_queue = (uint32_t*)malloc(mtl->idpool.num_slots * sizeof(uint32_t));
  /* pool slot 0 is reserved! */
  for (int i = mtl->idpool.num_slots-1; i >= 1; i--) {
    mtl->idpool.free_queue[mtl->idpool.free_queue_top++] = (uint32_t)i;
  }
  /* a circular queue which holds release items (frame index
     when a resource is to be released, and the resource's
     pool index
  */
  mtl->idpool.release_queue_front = 0;
  mtl->idpool.release_queue_back = 0;
  mtl->idpool.release_queue = (binocle_mtl_release_item_t*)malloc(mtl->idpool.num_slots * sizeof(binocle_mtl_release_item_t));
  for (uint32_t i = 0; i < mtl->idpool.num_slots; i++) {
    mtl->idpool.release_queue[i].frame_index = 0;
    mtl->idpool.release_queue[i].slot_index = BINOCLE_MTL_INVALID_SLOT_INDEX;
  }
}

void binocle_backend_mtl_destroy_pool(binocle_mtl_backend_t *mtl) {
  free(mtl->idpool.release_queue);  mtl->idpool.release_queue = 0;
  free(mtl->idpool.free_queue);     mtl->idpool.free_queue = 0;
}

/* get a new free resource pool slot */
uint32_t binocle_backend_mtl_alloc_pool_slot(binocle_mtl_backend_t *mtl) {
  assert(mtl->idpool.free_queue_top > 0);
  const uint32_t slot_index = mtl->idpool.free_queue[--mtl->idpool.free_queue_top];
  assert((slot_index > 0) && (slot_index < mtl->idpool.num_slots));
  return slot_index;
}

/* put a free resource pool slot back into the free-queue */
void binocle_backend_mtl_free_pool_slot(binocle_mtl_backend_t *mtl, uint32_t slot_index) {
  assert(mtl->idpool.free_queue_top < mtl->idpool.num_slots);
  assert((slot_index > 0) && (slot_index < mtl->idpool.num_slots));
  mtl->idpool.free_queue[mtl->idpool.free_queue_top++] = slot_index;
}

/*  add an MTLResource to the pool, return pool index or 0 if input was 'nil' */
uint32_t binocle_backend_mtl_add_resource(binocle_mtl_backend_t *mtl, id res) {
  if (nil == res) {
    return BINOCLE_MTL_INVALID_SLOT_INDEX;
  }
  const uint32_t slot_index = binocle_backend_mtl_alloc_pool_slot(mtl);
  assert([NSNull null] == mtl->idpool.pool[slot_index]);
  mtl->idpool.pool[slot_index] = res;
  return slot_index;
}

/*  mark an MTLResource for release, this will put the resource into the
    deferred-release queue, and the resource will then be released N frames later,
    the special pool index 0 will be ignored (this means that a nil
    value was provided to _sg_mtl_add_resource()
*/
void binocle_backend_mtl_release_resource(binocle_mtl_backend_t *mtl, uint32_t frame_index, uint32_t slot_index) {
  if (slot_index == BINOCLE_MTL_INVALID_SLOT_INDEX) {
    return;
  }
  assert((slot_index > 0) && (slot_index < mtl->idpool.num_slots));
  assert([NSNull null] != mtl->idpool.pool[slot_index]);
  int release_index = mtl->idpool.release_queue_front++;
  if (mtl->idpool.release_queue_front >= mtl->idpool.num_slots) {
    /* wrap-around */
    mtl->idpool.release_queue_front = 0;
  }
  /* release queue full? */
  assert(mtl->idpool.release_queue_front != mtl->idpool.release_queue_back);
  assert(0 == mtl->idpool.release_queue[release_index].frame_index);
  const uint32_t safe_to_release_frame_index = frame_index + BINOCLE_NUM_INFLIGHT_FRAMES + 1;
  mtl->idpool.release_queue[release_index].frame_index = safe_to_release_frame_index;
  mtl->idpool.release_queue[release_index].slot_index = slot_index;
}

/* run garbage-collection pass on all resources in the release-queue */
void binocle_backend_mtl_garbage_collect(binocle_mtl_backend_t *mtl, uint32_t frame_index) {
  while (mtl->idpool.release_queue_back != mtl->idpool.release_queue_front) {
    if (frame_index < mtl->idpool.release_queue[mtl->idpool.release_queue_back].frame_index) {
      /* don't need to check further, release-items past this are too young */
      break;
    }
    /* safe to release this resource */
    const uint32_t slot_index = mtl->idpool.release_queue[mtl->idpool.release_queue_back].slot_index;
    assert((slot_index > 0) && (slot_index < mtl->idpool.num_slots));
    assert(mtl->idpool.pool[slot_index] != [NSNull null]);
    mtl->idpool.pool[slot_index] = [NSNull null];
    /* put the now free pool index back on the free queue */
    binocle_backend_mtl_free_pool_slot(mtl, slot_index);
    /* reset the release queue slot and advance the back index */
    mtl->idpool.release_queue[mtl->idpool.release_queue_back].frame_index = 0;
    mtl->idpool.release_queue[mtl->idpool.release_queue_back].slot_index = BINOCLE_MTL_INVALID_SLOT_INDEX;
    mtl->idpool.release_queue_back++;
    if (mtl->idpool.release_queue_back >= mtl->idpool.num_slots) {
      /* wrap-around */
      mtl->idpool.release_queue_back = 0;
    }
  }
}

id binocle_backend_mtl_id(binocle_mtl_backend_t *mtl, uint32_t slot_index) {
  return mtl->idpool.pool[slot_index];
}

void binocle_backend_mtl_init_sampler_cache(binocle_mtl_backend_t *mtl, binocle_backend_desc* desc) {
  assert(desc->sampler_cache_size > 0);
  binocle_smpcache_init(&mtl->sampler_cache, desc->sampler_cache_size);
}

/* destroy the sampler cache, and release all sampler objects */
void binocle_backend_mtl_destroy_sampler_cache(binocle_mtl_backend_t *mtl, uint32_t frame_index) {
  assert(mtl->sampler_cache.items);
  assert(mtl->sampler_cache.num_items <= mtl->sampler_cache.capacity);
  for (int i = 0; i < mtl->sampler_cache.num_items; i++) {
    binocle_backend_mtl_release_resource(mtl, frame_index, (uint32_t)binocle_smpcache_sampler(&mtl->sampler_cache, i));
  }
  binocle_smpcache_discard(&mtl->sampler_cache);
}

/*
    create and add an MTLSamplerStateObject and return its resource pool index,
    reuse identical sampler state if one exists
*/
uint32_t
binocle_backend_mtl_create_sampler(binocle_mtl_backend_t *mtl,
                                   id<MTLDevice> mtl_device,
                                   const binocle_image_desc *img_desc) {
  assert(img_desc);
  int index = binocle_smpcache_find_item(&mtl->sampler_cache, img_desc);
  if (index >= 0) {
    /* reuse existing sampler */
    return (uint32_t)binocle_smpcache_sampler(&mtl->sampler_cache, index);
  } else {
    /* create a new Metal sampler state object and add to sampler cache */
    MTLSamplerDescriptor *mtl_desc = [[MTLSamplerDescriptor alloc] init];
    mtl_desc.sAddressMode = binocle_backend_mtl_address_mode(img_desc->wrap_u);
    mtl_desc.tAddressMode = binocle_backend_mtl_address_mode(img_desc->wrap_v);
    if (BINOCLE_IMAGETYPE_3D == img_desc->type) {
      mtl_desc.rAddressMode =
        binocle_backend_mtl_address_mode(img_desc->wrap_w);
    }
#if defined(_BINOCLE_TARGET_MACOS)
    mtl_desc.borderColor =
      binocle_backend_mtl_border_color(img_desc->border_color);
#endif
    mtl_desc.minFilter =
      binocle_backend_mtl_minmag_filter(img_desc->min_filter);
    mtl_desc.magFilter =
      binocle_backend_mtl_minmag_filter(img_desc->mag_filter);
    mtl_desc.mipFilter = binocle_backend_mtl_mip_filter(img_desc->min_filter);
    mtl_desc.lodMinClamp = img_desc->min_lod;
    mtl_desc.lodMaxClamp = img_desc->max_lod;
    mtl_desc.maxAnisotropy = img_desc->max_anisotropy;
    mtl_desc.normalizedCoordinates = YES;
    id<MTLSamplerState> mtl_sampler =
      [mtl_device newSamplerStateWithDescriptor:mtl_desc];
    uint32_t sampler_handle =
      binocle_backend_mtl_add_resource(mtl, mtl_sampler);
    binocle_smpcache_add_item(&mtl->sampler_cache, img_desc, sampler_handle);
    return sampler_handle;
  }
}

void binocle_backend_mtl_clear_state_cache(binocle_mtl_backend_t *mtl) {
  memset(&mtl->state_cache, 0, sizeof(mtl->state_cache));
}

void binocle_backend_mtl_setup_backend(binocle_mtl_backend_t *mtl, binocle_backend_desc *desc) {
  assert(desc);
  assert(desc->context.mtl.mtl_view);
  assert(desc->uniform_buffer_size > 0);

  binocle_backend_mtl_init_pool(mtl, desc);
  binocle_backend_mtl_init_sampler_cache(mtl, desc);
  binocle_backend_mtl_clear_state_cache(mtl);
  mtl->valid = true;
  mtl->frame_index = 1;
  mtl->ub_size = desc->uniform_buffer_size;
  mtl->sem = dispatch_semaphore_create(BINOCLE_NUM_INFLIGHT_FRAMES);
  NSView *view = (__bridge NSView *)desc->context.mtl.mtl_view;
  CAMetalLayer *metal_layer = (CAMetalLayer *)view.layer;
  mtl->layer = metal_layer;
  mtl->device = MTLCreateSystemDefaultDevice();
  metal_layer.device = mtl->device;
  mtl->cmd_queue = [mtl->device newCommandQueue];
  mtl->cmd_buffer = [mtl->cmd_queue commandBuffer];

  MTLResourceOptions res_opts = MTLResourceCPUCacheModeWriteCombined;
#if defined(BINOCLE_MACOS)
  res_opts |= MTLResourceStorageModeManaged;
#endif
  for (int i = 0; i < BINOCLE_NUM_INFLIGHT_FRAMES; i++) {
    mtl->uniform_buffers[i] = [mtl->device
      newBufferWithLength:(NSUInteger)mtl->ub_size
                  options:res_opts
    ];
  }
  binocle_backend_mtl_init_caps();
}

void binocle_backend_image_common_init(binocle_image_common_t* cmn, const binocle_image_desc* desc) {
  cmn->type = desc->type;
  cmn->render_target = desc->render_target;
  cmn->width = desc->width;
  cmn->height = desc->height;
  cmn->depth = desc->depth;
  cmn->num_mipmaps = desc->num_mipmaps;
  cmn->usage = desc->usage;
  cmn->pixel_format = desc->pixel_format;
  cmn->sample_count = desc->sample_count;
  cmn->min_filter = desc->min_filter;
  cmn->mag_filter = desc->mag_filter;
  cmn->wrap_u = desc->wrap_u;
  cmn->wrap_v = desc->wrap_v;
  cmn->wrap_w = desc->wrap_w;
  cmn->border_color = desc->border_color;
  cmn->max_anisotropy = desc->max_anisotropy;
  cmn->upd_frame_index = 0;
  cmn->num_slots = (cmn->usage == BINOCLE_USAGE_IMMUTABLE) ? 1 : BINOCLE_NUM_INFLIGHT_FRAMES;
  cmn->active_slot = 0;
}

bool binocle_backend_mtl_init_texdesc_common(MTLTextureDescriptor *mtl_desc,
                                             binocle_image_t *img) {
  mtl_desc.textureType = binocle_backend_mtl_texture_type(img->cmn.type);
  mtl_desc.pixelFormat =
    binocle_backend_mtl_pixel_format(img->cmn.pixel_format);
  if (MTLPixelFormatInvalid == mtl_desc.pixelFormat) {
    binocle_log_warning("Unsupported texture pixel format!\n");
    return false;
  }
  mtl_desc.width = (NSUInteger)img->cmn.width;
  mtl_desc.height = (NSUInteger)img->cmn.height;
  if (BINOCLE_IMAGETYPE_3D == img->cmn.type) {
    mtl_desc.depth = (NSUInteger)img->cmn.depth;
  } else {
    mtl_desc.depth = 1;
  }
  mtl_desc.mipmapLevelCount = (NSUInteger)img->cmn.num_mipmaps;
  if (BINOCLE_IMAGETYPE_ARRAY == img->cmn.type) {
    mtl_desc.arrayLength = (NSUInteger)img->cmn.depth;
  } else {
    mtl_desc.arrayLength = 1;
  }
  mtl_desc.usage = MTLTextureUsageShaderRead;
  if (img->cmn.usage != BINOCLE_USAGE_IMMUTABLE) {
    mtl_desc.cpuCacheMode = MTLCPUCacheModeWriteCombined;
  }
#if defined(BINOCLE_MACOS)
  /* macOS: use managed textures */
  mtl_desc.resourceOptions = MTLResourceStorageModeManaged;
  mtl_desc.storageMode = MTLStorageModeManaged;
#else
  /* iOS: use CPU/GPU shared memory */
  mtl_desc.resourceOptions = MTLResourceStorageModeShared;
  mtl_desc.storageMode = MTLStorageModeShared;
#endif
  return true;
}

/* initialize MTLTextureDescritor with rendertarget attributes */
void binocle_backend_mtl_init_texdesc_rt(MTLTextureDescriptor* mtl_desc, binocle_image_t* img) {
  assert(img->cmn.render_target);
  /* reset the cpuCacheMode to 'default' */
  mtl_desc.cpuCacheMode = MTLCPUCacheModeDefaultCache;
  /* render targets are only visible to the GPU */
  mtl_desc.resourceOptions = MTLResourceStorageModePrivate;
  mtl_desc.storageMode = MTLStorageModePrivate;
  /* non-MSAA render targets are shader-readable */
  mtl_desc.usage = MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget;
}

/* initialize MTLTextureDescritor with MSAA attributes */
void binocle_backend_mtl_init_texdesc_rt_msaa(MTLTextureDescriptor* mtl_desc, binocle_image_t* img) {
  assert(img->cmn.sample_count > 1);
  /* reset the cpuCacheMode to 'default' */
  mtl_desc.cpuCacheMode = MTLCPUCacheModeDefaultCache;
  /* render targets are only visible to the GPU */
  mtl_desc.resourceOptions = MTLResourceStorageModePrivate;
  mtl_desc.storageMode = MTLStorageModePrivate;
  /* MSAA render targets are not shader-readable (instead they are resolved) */
  mtl_desc.usage = MTLTextureUsageRenderTarget;
  mtl_desc.textureType = MTLTextureType2DMultisample;
  mtl_desc.depth = 1;
  mtl_desc.arrayLength = 1;
  mtl_desc.mipmapLevelCount = 1;
  mtl_desc.sampleCount = img->cmn.sample_count;
}

void binocle_backend_mtl_copy_image_data(const binocle_image_t* img, __unsafe_unretained id<MTLTexture> mtl_tex, const binocle_image_data* data) {
  const int num_faces = (img->cmn.type == BINOCLE_IMAGETYPE_CUBE) ? 6:1;
  const int num_slices = (img->cmn.type == BINOCLE_IMAGETYPE_ARRAY) ? img->cmn.depth : 1;
  for (int face_index = 0; face_index < num_faces; face_index++) {
    for (int mip_index = 0; mip_index < img->cmn.num_mipmaps; mip_index++) {
      assert(data->subimage[face_index][mip_index].ptr);
      assert(data->subimage[face_index][mip_index].size > 0);
      const uint8_t* data_ptr = (const uint8_t*)data->subimage[face_index][mip_index].ptr;
      const int mip_width = BINOCLE_MAX(img->cmn.width >> mip_index, 1);
      const int mip_height = BINOCLE_MAX(img->cmn.height >> mip_index, 1);
      /* special case PVRTC formats: bytePerRow must be 0 */
      int bytes_per_row = 0;
      int bytes_per_slice = binocle_backend_surface_pitch(img->cmn.pixel_format, mip_width, mip_height, 1);
      if (!binocle_backend_mtl_is_pvrtc(img->cmn.pixel_format)) {
        bytes_per_row = binocle_backend_row_pitch(img->cmn.pixel_format, mip_width, 1);
      }
      MTLRegion region;
      if (img->cmn.type == BINOCLE_IMAGETYPE_3D) {
        const int mip_depth = BINOCLE_MAX(img->cmn.depth >> mip_index, 1);
        region = MTLRegionMake3D(0, 0, 0, (NSUInteger)mip_width, (NSUInteger)mip_height, (NSUInteger)mip_depth);
        /* FIXME: apparently the minimal bytes_per_image size for 3D texture
         is 4 KByte... somehow need to handle this */
      }
      else {
        region = MTLRegionMake2D(0, 0, (NSUInteger)mip_width, (NSUInteger)mip_height);
      }
      for (int slice_index = 0; slice_index < num_slices; slice_index++) {
        const int mtl_slice_index = (img->cmn.type == BINOCLE_IMAGETYPE_CUBE) ? face_index : slice_index;
        const int slice_offset = slice_index * bytes_per_slice;
        assert((slice_offset + bytes_per_slice) <= (int)data->subimage[face_index][mip_index].size);
        [mtl_tex replaceRegion:region
                   mipmapLevel:(NSUInteger)mip_index
                         slice:(NSUInteger)mtl_slice_index
                     withBytes:data_ptr + slice_offset
                   bytesPerRow:(NSUInteger)bytes_per_row
                 bytesPerImage:(NSUInteger)bytes_per_slice];
      }
    }
  }
}

binocle_resource_state
binocle_backend_mtl_create_image(binocle_mtl_backend_t *mtl, binocle_image_t *img,
                                 const binocle_image_desc *desc) {
  assert(img && desc);
  binocle_backend_image_common_init(&img->cmn, desc);
  const bool injected = (0 != desc->mtl_textures[0]);
  const bool msaa = (img->cmn.sample_count > 1);

  /* first initialize all Metal resource pool slots to 'empty' */
  for (int i = 0; i < BINOCLE_NUM_INFLIGHT_FRAMES; i++) {
    img->mtl.tex[i] = binocle_backend_mtl_add_resource(mtl, nil);
  }
  img->mtl.sampler_state = binocle_backend_mtl_add_resource(mtl, nil);
  img->mtl.depth_tex = binocle_backend_mtl_add_resource(mtl, nil);
  img->mtl.msaa_tex = binocle_backend_mtl_add_resource(mtl, nil);

  /* initialize a Metal texture descriptor with common attributes */
  MTLTextureDescriptor *mtl_desc = [[MTLTextureDescriptor alloc] init];
  if (!binocle_backend_mtl_init_texdesc_common(mtl_desc, img)) {
    return BINOCLE_RESOURCESTATE_FAILED;
  }

  /* special case depth-stencil-buffer? */
  if (binocle_backend_is_valid_rendertarget_depth_format(
        img->cmn.pixel_format)) {
    /* depth-stencil buffer texture must always be a render target */
    assert(img->cmn.render_target);
    assert(img->cmn.type == BINOCLE_IMAGETYPE_2D);
    assert(img->cmn.num_mipmaps == 1);
    assert(!injected);
    if (msaa) {
      binocle_backend_mtl_init_texdesc_rt_msaa(mtl_desc, img);
    } else {
      binocle_backend_mtl_init_texdesc_rt(mtl_desc, img);
    }
    id<MTLTexture> tex = [mtl->device newTextureWithDescriptor:mtl_desc];
    assert(nil != tex);
    img->mtl.depth_tex = binocle_backend_mtl_add_resource(mtl, tex);
  } else {
    /* create the color texture
        In case this is a render target without MSAA, add the relevant
        render-target descriptor attributes.
        In case this is a render target *with* MSAA, the color texture
        will serve as MSAA-resolve target (not as render target), and rendering
        will go into a separate render target texture of type
        MTLTextureType2DMultisample.
    */
    if (img->cmn.render_target && !msaa) {
      binocle_backend_mtl_init_texdesc_rt(mtl_desc, img);
    }
    for (int slot = 0; slot < img->cmn.num_slots; slot++) {
      id<MTLTexture> tex;
      if (injected) {
        assert(desc->mtl_textures[slot]);
        tex = (__bridge id<MTLTexture>) desc->mtl_textures[slot];
      }
      else {
        tex = [mtl->device newTextureWithDescriptor:mtl_desc];
        if ((img->cmn.usage == BINOCLE_USAGE_IMMUTABLE) && !img->cmn.render_target) {
          binocle_backend_mtl_copy_image_data(img, tex, &desc->data);
        }
      }
      img->mtl.tex[slot] = binocle_backend_mtl_add_resource(mtl, tex);
    }

    /* if MSAA color render target, create an additional MSAA render-surface
     * texture */
    if (img->cmn.render_target && msaa) {
      binocle_backend_mtl_init_texdesc_rt_msaa(mtl_desc, img);
      id<MTLTexture> tex = [mtl->device newTextureWithDescriptor:mtl_desc];
      img->mtl.msaa_tex = binocle_backend_mtl_add_resource(mtl, tex);
    }

    /* create (possibly shared) sampler state */
    img->mtl.sampler_state =
      binocle_backend_mtl_create_sampler(mtl, mtl->device, desc);
  }
  mtl_desc = nil;
  return BINOCLE_RESOURCESTATE_VALID;
}

void binocle_backend_mtl_destroy_image(binocle_mtl_backend_t *mtl, binocle_image_t *img) {
  assert(img);
  /* it's valid to call release resource with a 'null resource' */
  for (int slot = 0; slot < img->cmn.num_slots; slot++) {
    binocle_backend_mtl_release_resource(mtl, mtl->frame_index,
                                         img->mtl.tex[slot]);
  }
  binocle_backend_mtl_release_resource(mtl, mtl->frame_index, img->mtl.depth_tex);
  binocle_backend_mtl_release_resource(mtl, mtl->frame_index, img->mtl.msaa_tex);
  /* NOTE: sampler state objects are shared and not released until shutdown */
}

id<MTLLibrary> binocle_backend_mtl_compile_library(binocle_mtl_backend_t *mtl, const char* src) {
  NSError* err = NULL;
  id<MTLLibrary> lib = [mtl->device
    newLibraryWithSource:[NSString stringWithUTF8String:src]
                 options:nil
                   error:&err
  ];
  if (err) {
    binocle_log_error([err.localizedDescription UTF8String]);
  }
  return lib;
}

id<MTLLibrary> binocle_backend_mtl_library_from_bytecode(binocle_mtl_backend_t *mtl, const uint8_t* ptr, int num_bytes) {
  NSError* err = NULL;
  dispatch_data_t lib_data = dispatch_data_create(ptr, num_bytes, NULL, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
  id<MTLLibrary> lib = [mtl->device newLibraryWithData:lib_data error:&err];
  if (err) {
    binocle_log_error([err.localizedDescription UTF8String]);
  }
  lib_data = nil;
  return lib;
}

binocle_resource_state
binocle_backend_mtl_create_shader(binocle_mtl_backend_t *mtl, binocle_shader_t *shd,
                                  const binocle_shader_desc *desc) {
  assert(shd && desc);

  binocle_backend_shader_common_init(&shd->cmn, desc);

  /* create metal libray objects and lookup entry functions */
  id<MTLLibrary> vs_lib;
  id<MTLLibrary> fs_lib;
  id<MTLFunction> vs_func;
  id<MTLFunction> fs_func;
  const char *vs_entry = desc->vs.entry;
  const char *fs_entry = desc->fs.entry;
  if (desc->vs.byte_code && desc->fs.byte_code) {
    /* separate byte code provided */
    vs_lib = binocle_backend_mtl_library_from_bytecode(mtl, desc->vs.byte_code,
                                           desc->vs.byte_code_size);
    fs_lib = binocle_backend_mtl_library_from_bytecode(mtl, desc->fs.byte_code,
                                           desc->fs.byte_code_size);
    if (nil == vs_lib || nil == fs_lib) {
      return BINOCLE_RESOURCESTATE_FAILED;
    }
    vs_func =
      [vs_lib newFunctionWithName:[NSString stringWithUTF8String:vs_entry]];
    fs_func =
      [fs_lib newFunctionWithName:[NSString stringWithUTF8String:fs_entry]];
  } else if (desc->vs.source && desc->fs.source) {
    /* separate sources provided */
    vs_lib = binocle_backend_mtl_compile_library(mtl, desc->vs.source);
    fs_lib = binocle_backend_mtl_compile_library(mtl, desc->fs.source);
    if (nil == vs_lib || nil == fs_lib) {
      return BINOCLE_RESOURCESTATE_FAILED;
    }
    vs_func =
      [vs_lib newFunctionWithName:[NSString stringWithUTF8String:vs_entry]];
    fs_func =
      [fs_lib newFunctionWithName:[NSString stringWithUTF8String:fs_entry]];
  } else {
    return BINOCLE_RESOURCESTATE_FAILED;
  }
  if (nil == vs_func) {
    binocle_log_error("vertex shader entry function not found\n");
    return BINOCLE_RESOURCESTATE_FAILED;
  }
  if (nil == fs_func) {
    binocle_log_error("fragment shader entry function not found\n");
    return BINOCLE_RESOURCESTATE_FAILED;
  }
  /* it is legal to call _sg_mtl_add_resource with a nil value, this will return
   * a special 0xFFFFFFFF index */
  shd->mtl.stage[BINOCLE_SHADERSTAGE_VS].mtl_lib = binocle_backend_mtl_add_resource(mtl, vs_lib);
  shd->mtl.stage[BINOCLE_SHADERSTAGE_FS].mtl_lib = binocle_backend_mtl_add_resource(mtl, fs_lib);
  shd->mtl.stage[BINOCLE_SHADERSTAGE_VS].mtl_func = binocle_backend_mtl_add_resource(mtl, vs_func);
  shd->mtl.stage[BINOCLE_SHADERSTAGE_FS].mtl_func = binocle_backend_mtl_add_resource(mtl, fs_func);
  return BINOCLE_RESOURCESTATE_VALID;
}

void binocle_backend_mtl_destroy_shader(binocle_mtl_backend_t *mtl,
                                        binocle_shader_t *shd) {
  assert(shd);
  /* it is valid to call _sg_mtl_release_resource with a 'null resource' */
  binocle_backend_mtl_release_resource(
    mtl, mtl->frame_index, shd->mtl.stage[BINOCLE_SHADERSTAGE_VS].mtl_func);
  binocle_backend_mtl_release_resource(
    mtl, mtl->frame_index, shd->mtl.stage[BINOCLE_SHADERSTAGE_VS].mtl_lib);
  binocle_backend_mtl_release_resource(
    mtl, mtl->frame_index, shd->mtl.stage[BINOCLE_SHADERSTAGE_FS].mtl_func);
  binocle_backend_mtl_release_resource(
    mtl, mtl->frame_index, shd->mtl.stage[BINOCLE_SHADERSTAGE_FS].mtl_lib);
}

void binocle_backend_mtl_clear(binocle_mtl_backend_t *mtl, struct binocle_color color) {
  assert(mtl->cmd_queue);
  assert(nil == mtl->cmd_encoder);

  MTLRenderPassDescriptor* pass_desc = [MTLRenderPassDescriptor renderPassDescriptor];
    id<CAMetalDrawable> cur_drawable = [mtl->layer nextDrawable];
    pass_desc.colorAttachments[0].texture = cur_drawable.texture;
  pass_desc.colorAttachments[0].loadAction = MTLLoadActionClear;
  pass_desc.colorAttachments[0].storeAction = MTLStoreActionStore;
  pass_desc.colorAttachments[0].clearColor = MTLClearColorMake(color.r, color.g, color.b, color.a);

    if (mtl->cmd_buffer == nil) {
        mtl->cmd_buffer = [mtl->cmd_queue commandBufferWithUnretainedReferences];
    }
  mtl->cmd_encoder = [mtl->cmd_buffer renderCommandEncoderWithDescriptor:pass_desc];
  [mtl->cmd_encoder endEncoding];
  mtl->cmd_encoder = nil;
    [mtl->cmd_buffer presentDrawable:cur_drawable];
  [mtl->cmd_buffer commit];
  mtl->cmd_buffer = nil;
}

void binocle_backend_mtl_bind_uniform_buffers(binocle_mtl_backend_t *mtl) {
  assert(nil != mtl->cmd_encoder);
  for (int slot = 0; slot < BINOCLE_MAX_SHADERSTAGE_UBS; slot++) {
    [mtl->cmd_encoder
      setVertexBuffer:mtl->uniform_buffers[mtl->cur_frame_rotate_index]
               offset:0
              atIndex:(NSUInteger)slot];
    [mtl->cmd_encoder
      setFragmentBuffer:mtl->uniform_buffers[mtl->cur_frame_rotate_index]
                 offset:0
                atIndex:(NSUInteger)slot];
  }
}

binocle_resource_state binocle_backend_mtl_create_pipeline(
  binocle_mtl_backend_t *mtl, binocle_pipeline_t *pip, binocle_shader_t *shd,
  const binocle_pipeline_desc *desc) {
  assert(pip && shd && desc);
  assert(desc->shader.id == shd->slot.id);

  pip->shader = shd;
  binocle_backend_pipeline_common_init(&pip->cmn, desc);

  binocle_primitive_type prim_type = desc->primitive_type;
  pip->mtl.prim_type = binocle_backend_mtl_primitive_type(prim_type);
  pip->mtl.index_size = binocle_backend_mtl_index_size(pip->cmn.index_type);
  if (BINOCLE_INDEXTYPE_NONE != pip->cmn.index_type) {
    pip->mtl.index_type = binocle_backend_mtl_index_type(pip->cmn.index_type);
  }
  pip->mtl.cull_mode = binocle_backend_mtl_cull_mode(desc->cull_mode);
  pip->mtl.winding = binocle_backend_mtl_winding(desc->face_winding);
  pip->mtl.stencil_ref = desc->stencil.ref;

  /* create vertex-descriptor */
  MTLVertexDescriptor *vtx_desc = [MTLVertexDescriptor vertexDescriptor];
  for (NSUInteger attr_index = 0; attr_index < BINOCLE_MAX_VERTEX_ATTRIBUTES;
       attr_index++) {
    const binocle_vertex_attr_desc *a_desc = &desc->layout.attrs[attr_index];
    if (a_desc->format == BINOCLE_VERTEXFORMAT_INVALID) {
      break;
    }
    assert(a_desc->buffer_index < BINOCLE_MAX_SHADERSTAGE_BUFFERS);
    vtx_desc.attributes[attr_index].format =
      binocle_backend_mtl_vertex_format(a_desc->format);
    vtx_desc.attributes[attr_index].offset = (NSUInteger)a_desc->offset;
    vtx_desc.attributes[attr_index].bufferIndex =
      (NSUInteger)(a_desc->buffer_index + BINOCLE_MAX_SHADERSTAGE_UBS);
    pip->cmn.vertex_layout_valid[a_desc->buffer_index] = true;
  }
  for (NSUInteger layout_index = 0;
       layout_index < BINOCLE_MAX_SHADERSTAGE_BUFFERS; layout_index++) {
    if (pip->cmn.vertex_layout_valid[layout_index]) {
      const binocle_buffer_layout_desc *l_desc =
        &desc->layout.buffers[layout_index];
      const NSUInteger mtl_vb_slot = layout_index + BINOCLE_MAX_SHADERSTAGE_UBS;
      assert(l_desc->stride > 0);
      vtx_desc.layouts[mtl_vb_slot].stride = (NSUInteger)l_desc->stride;
      vtx_desc.layouts[mtl_vb_slot].stepFunction =
        binocle_backend_mtl_step_function(l_desc->step_func);
      vtx_desc.layouts[mtl_vb_slot].stepRate = (NSUInteger)l_desc->step_rate;
    }
  }

  /* render-pipeline descriptor */
  MTLRenderPipelineDescriptor *rp_desc =
    [[MTLRenderPipelineDescriptor alloc] init];
  rp_desc.vertexDescriptor = vtx_desc;
  assert(shd->mtl.stage[BINOCLE_SHADERSTAGE_VS].mtl_func !=
         BINOCLE_MTL_INVALID_SLOT_INDEX);
  rp_desc.vertexFunction = binocle_backend_mtl_id(
    mtl, shd->mtl.stage[BINOCLE_SHADERSTAGE_VS].mtl_func);
  assert(shd->mtl.stage[BINOCLE_SHADERSTAGE_FS].mtl_func !=
         BINOCLE_MTL_INVALID_SLOT_INDEX);
  rp_desc.fragmentFunction = binocle_backend_mtl_id(
    mtl, shd->mtl.stage[BINOCLE_SHADERSTAGE_FS].mtl_func);
  rp_desc.sampleCount = (NSUInteger)desc->sample_count;
  rp_desc.alphaToCoverageEnabled = desc->alpha_to_coverage_enabled;
  rp_desc.alphaToOneEnabled = NO;
  rp_desc.rasterizationEnabled = YES;
  rp_desc.depthAttachmentPixelFormat =
    binocle_backend_mtl_pixel_format(desc->depth.pixel_format);
  if (desc->depth.pixel_format == BINOCLE_PIXELFORMAT_DEPTH_STENCIL) {
    rp_desc.stencilAttachmentPixelFormat =
      binocle_backend_mtl_pixel_format(desc->depth.pixel_format);
  }
  /* FIXME: this only works on macOS 10.13!
  for (int i = 0; i <
  (BINOCLE_MAX_SHADERSTAGE_UBS+BINOCLE_MAX_SHADERSTAGE_BUFFERS); i++) {
      rp_desc.vertexBuffers[i].mutability = MTLMutabilityImmutable;
  }
  for (int i = 0; i < BINOCLE_MAX_SHADERSTAGE_UBS; i++) {
      rp_desc.fragmentBuffers[i].mutability = MTLMutabilityImmutable;
  }
  */
  for (NSUInteger i = 0; i < (NSUInteger)desc->color_count; i++) {
    assert(i < BINOCLE_MAX_COLOR_ATTACHMENTS);
    const binocle_color_state *cs = &desc->colors[i];
    rp_desc.colorAttachments[i].pixelFormat =
      binocle_backend_mtl_pixel_format(cs->pixel_format);
    rp_desc.colorAttachments[i].writeMask =
      binocle_backend_mtl_color_write_mask(cs->write_mask);
    rp_desc.colorAttachments[i].blendingEnabled = cs->blend.enabled;
    rp_desc.colorAttachments[i].alphaBlendOperation =
      binocle_backend_mtl_blend_op(cs->blend.op_alpha);
    rp_desc.colorAttachments[i].rgbBlendOperation =
      binocle_backend_mtl_blend_op(cs->blend.op_rgb);
    rp_desc.colorAttachments[i].destinationAlphaBlendFactor =
      binocle_backend_mtl_blend_factor(cs->blend.dst_factor_alpha);
    rp_desc.colorAttachments[i].destinationRGBBlendFactor =
      binocle_backend_mtl_blend_factor(cs->blend.dst_factor_rgb);
    rp_desc.colorAttachments[i].sourceAlphaBlendFactor =
      binocle_backend_mtl_blend_factor(cs->blend.src_factor_alpha);
    rp_desc.colorAttachments[i].sourceRGBBlendFactor =
      binocle_backend_mtl_blend_factor(cs->blend.src_factor_rgb);
  }
  NSError *err = NULL;
  id<MTLRenderPipelineState> mtl_rps =
    [mtl->device newRenderPipelineStateWithDescriptor:rp_desc error:&err];
  rp_desc = nil;
  if (nil == mtl_rps) {
    assert(err);
    binocle_log_error([err.localizedDescription UTF8String]);
    return BINOCLE_RESOURCESTATE_FAILED;
  }

  /* depth-stencil-state */
  MTLDepthStencilDescriptor *ds_desc = [[MTLDepthStencilDescriptor alloc] init];
  ds_desc.depthCompareFunction =
    binocle_backend_mtl_compare_func(desc->depth.compare);
  ds_desc.depthWriteEnabled = desc->depth.write_enabled;
  if (desc->stencil.enabled) {
    const binocle_stencil_face_state *sb = &desc->stencil.back;
    ds_desc.backFaceStencil = [[MTLStencilDescriptor alloc] init];
    ds_desc.backFaceStencil.stencilFailureOperation =
      binocle_backend_mtl_stencil_op(sb->fail_op);
    ds_desc.backFaceStencil.depthFailureOperation =
      binocle_backend_mtl_stencil_op(sb->depth_fail_op);
    ds_desc.backFaceStencil.depthStencilPassOperation =
      binocle_backend_mtl_stencil_op(sb->pass_op);
    ds_desc.backFaceStencil.stencilCompareFunction =
      binocle_backend_mtl_compare_func(sb->compare);
    ds_desc.backFaceStencil.readMask = desc->stencil.read_mask;
    ds_desc.backFaceStencil.writeMask = desc->stencil.write_mask;
    const binocle_stencil_face_state *sf = &desc->stencil.front;
    ds_desc.frontFaceStencil = [[MTLStencilDescriptor alloc] init];
    ds_desc.frontFaceStencil.stencilFailureOperation =
      binocle_backend_mtl_stencil_op(sf->fail_op);
    ds_desc.frontFaceStencil.depthFailureOperation =
      binocle_backend_mtl_stencil_op(sf->depth_fail_op);
    ds_desc.frontFaceStencil.depthStencilPassOperation =
      binocle_backend_mtl_stencil_op(sf->pass_op);
    ds_desc.frontFaceStencil.stencilCompareFunction =
      binocle_backend_mtl_compare_func(sf->compare);
    ds_desc.frontFaceStencil.readMask = desc->stencil.read_mask;
    ds_desc.frontFaceStencil.writeMask = desc->stencil.write_mask;
  }
  id<MTLDepthStencilState> mtl_dss =
    [mtl->device newDepthStencilStateWithDescriptor:ds_desc];
  ds_desc = nil;
  pip->mtl.rps = binocle_backend_mtl_add_resource(mtl, mtl_rps);
  pip->mtl.dss = binocle_backend_mtl_add_resource(mtl, mtl_dss);
  return BINOCLE_RESOURCESTATE_VALID;
}

void binocle_backend_mtl_destroy_pipeline(binocle_mtl_backend_t *mtl, binocle_pipeline_t* pip) {
  assert(pip);
  /* it's valid to call release resource with a 'null resource' */
  binocle_backend_mtl_release_resource(mtl, mtl->frame_index, pip->mtl.rps);
  binocle_backend_mtl_release_resource(mtl, mtl->frame_index, pip->mtl.dss);
}

binocle_resource_state
binocle_backend_mtl_create_pass(binocle_pass_t *pass,
                                binocle_image_t **att_images,
                                const binocle_pass_desc *desc) {
  assert(pass && desc);
  assert(att_images && att_images[0]);

  binocle_pass_common_init(&pass->cmn, desc);

  /* copy image pointers */
  const binocle_pass_attachment_desc *att_desc;
  for (int i = 0; i < pass->cmn.num_color_atts; i++) {
    att_desc = &desc->color_attachments[i];
    if (att_desc->image.id != BINOCLE_INVALID_ID) {
      assert(att_desc->image.id != BINOCLE_INVALID_ID);
      assert(0 == pass->mtl.color_atts[i].image);
      assert(att_images[i] && (att_images[i]->slot.id == att_desc->image.id));
      assert(binocle_backend_is_valid_rendertarget_color_format(
        att_images[i]->cmn.pixel_format));
      pass->mtl.color_atts[i].image = att_images[i];
    }
  }
  assert(0 == pass->mtl.ds_att.image);
  att_desc = &desc->depth_stencil_attachment;
  if (att_desc->image.id != BINOCLE_INVALID_ID) {
    const int ds_img_index = BINOCLE_MAX_COLOR_ATTACHMENTS;
    assert(att_images[ds_img_index] &&
           (att_images[ds_img_index]->slot.id == att_desc->image.id));
    assert(binocle_backend_is_valid_rendertarget_depth_format(
      att_images[ds_img_index]->cmn.pixel_format));
    pass->mtl.ds_att.image = att_images[ds_img_index];
  }
  return BINOCLE_RESOURCESTATE_VALID;
}

void binocle_backend_mtl_destroy_pass(binocle_pass_t* pass) {
  assert(pass);
  (void)(pass);
}

binocle_image_t* binocle_backend_mtl_pass_color_image(const binocle_pass_t* pass, int index) {
  assert(pass && (index >= 0) && (index < BINOCLE_MAX_COLOR_ATTACHMENTS));
  /* NOTE: may return null */
  return pass->mtl.color_atts[index].image;
}

binocle_image_t* binocle_backend_mtl_pass_ds_image(const binocle_pass_t* pass) {
  /* NOTE: may return null */
  assert(pass);
  return pass->mtl.ds_att.image;
}

void binocle_backend_mtl_begin_pass(binocle_mtl_backend_t *mtl, binocle_pass_t* pass, const binocle_pass_action* action, int w, int h) {
  assert(action);
  assert(!mtl->in_pass);
  assert(mtl->cmd_queue);
  assert(nil == mtl->cmd_encoder);
  assert(mtl->renderpass_descriptor_cb || mtl->renderpass_descriptor_userdata_cb);
  mtl->in_pass = true;
  mtl->cur_width = w;
  mtl->cur_height = h;
  binocle_backend_mtl_clear_state_cache(mtl);

  /* if this is the first pass in the frame, create a command buffer */
  if (nil == mtl->cmd_buffer) {
    /* block until the oldest frame in flight has finished */
    dispatch_semaphore_wait(mtl->sem, DISPATCH_TIME_FOREVER);
    mtl->cmd_buffer = [mtl->cmd_queue commandBufferWithUnretainedReferences];
  }

  /* if this is first pass in frame, get uniform buffer base pointer */
  if (0 == mtl->cur_ub_base_ptr) {
    mtl->cur_ub_base_ptr = (uint8_t*)[mtl->uniform_buffers[mtl->cur_frame_rotate_index] contents];
  }

  /* initialize a render pass descriptor */
  MTLRenderPassDescriptor* pass_desc = nil;
  if (pass) {
    /* offscreen render pass */
    pass_desc = [MTLRenderPassDescriptor renderPassDescriptor];
  }
  else {
    /* default render pass, call user-provided callback to provide render pass descriptor */
    if (mtl->renderpass_descriptor_cb) {
      pass_desc = (__bridge MTLRenderPassDescriptor*) mtl->renderpass_descriptor_cb();
    }
    else {
      pass_desc = (__bridge MTLRenderPassDescriptor*) mtl->renderpass_descriptor_userdata_cb(mtl->user_data);
    }

  }
  if (pass_desc) {
    mtl->pass_valid = true;
  }
  else {
    /* default pass descriptor will not be valid if window is minimized,
       don't do any rendering in this case */
    mtl->pass_valid = false;
    return;
  }
  if (pass) {
    /* setup pass descriptor for offscreen rendering */
    assert(pass->slot.state == BINOCLE_RESOURCESTATE_VALID);
    for (NSUInteger i = 0; i < (NSUInteger)pass->cmn.num_color_atts; i++) {
      const binocle_pass_attachment_t* cmn_att = &pass->cmn.color_atts[i];
      const binocle_mtl_attachment_t* mtl_att = &pass->mtl.color_atts[i];
      const binocle_image_t* att_img = mtl_att->image;
      assert(att_img->slot.state == BINOCLE_RESOURCESTATE_VALID);
      assert(att_img->slot.id == cmn_att->image_id.id);
      const bool is_msaa = (att_img->cmn.sample_count > 1);
      pass_desc.colorAttachments[i].loadAction = binocle_backend_mtl_load_action(action->colors[i].action);
      pass_desc.colorAttachments[i].storeAction = is_msaa ? MTLStoreActionMultisampleResolve : MTLStoreActionStore;
      binocle_color c = action->colors[i].value;
      pass_desc.colorAttachments[i].clearColor = MTLClearColorMake(c.r, c.g, c.b, c.a);
      if (is_msaa) {
        assert(att_img->mtl.msaa_tex != BINOCLE_MTL_INVALID_SLOT_INDEX);
        assert(att_img->mtl.tex[mtl_att->image->cmn.active_slot] != BINOCLE_MTL_INVALID_SLOT_INDEX);
        pass_desc.colorAttachments[i].texture = binocle_backend_mtl_id(mtl, att_img->mtl.msaa_tex);
        pass_desc.colorAttachments[i].resolveTexture = binocle_backend_mtl_id(mtl, att_img->mtl.tex[att_img->cmn.active_slot]);
        pass_desc.colorAttachments[i].resolveLevel = (NSUInteger)cmn_att->mip_level;
        switch (att_img->cmn.type) {
        case BINOCLE_IMAGETYPE_CUBE:
        case BINOCLE_IMAGETYPE_ARRAY:
          pass_desc.colorAttachments[i].resolveSlice = (NSUInteger)cmn_att->slice;
          break;
        case BINOCLE_IMAGETYPE_3D:
          pass_desc.colorAttachments[i].resolveDepthPlane = (NSUInteger)cmn_att->slice;
          break;
        default: break;
        }
      }
      else {
        assert(att_img->mtl.tex[att_img->cmn.active_slot] != BINOCLE_MTL_INVALID_SLOT_INDEX);
        pass_desc.colorAttachments[i].texture = binocle_backend_mtl_id(mtl, att_img->mtl.tex[att_img->cmn.active_slot]);
        pass_desc.colorAttachments[i].level = (NSUInteger)cmn_att->mip_level;
        switch (att_img->cmn.type) {
        case BINOCLE_IMAGETYPE_CUBE:
        case BINOCLE_IMAGETYPE_ARRAY:
          pass_desc.colorAttachments[i].slice = (NSUInteger)cmn_att->slice;
          break;
        case BINOCLE_IMAGETYPE_3D:
          pass_desc.colorAttachments[i].depthPlane = (NSUInteger)cmn_att->slice;
          break;
        default: break;
        }
      }
    }
    const binocle_image_t* ds_att_img = pass->mtl.ds_att.image;
    if (0 != ds_att_img) {
      assert(ds_att_img->slot.state == BINOCLE_RESOURCESTATE_VALID);
      assert(ds_att_img->slot.id == pass->cmn.ds_att.image_id.id);
      assert(ds_att_img->mtl.depth_tex != BINOCLE_MTL_INVALID_SLOT_INDEX);
      pass_desc.depthAttachment.texture = binocle_backend_mtl_id(mtl, ds_att_img->mtl.depth_tex);
      pass_desc.depthAttachment.loadAction = binocle_backend_mtl_load_action(action->depth.action);
      pass_desc.depthAttachment.clearDepth = action->depth.value;
      if (binocle_backend_is_depth_stencil_format(ds_att_img->cmn.pixel_format)) {
        pass_desc.stencilAttachment.texture = binocle_backend_mtl_id(mtl, ds_att_img->mtl.depth_tex);
        pass_desc.stencilAttachment.loadAction = binocle_backend_mtl_load_action(action->stencil.action);
        pass_desc.stencilAttachment.clearStencil = action->stencil.value;
      }
    }
  }
  else {
    /* setup pass descriptor for default rendering */
    pass_desc.colorAttachments[0].loadAction = binocle_backend_mtl_load_action(action->colors[0].action);
    binocle_color c = action->colors[0].value;
    pass_desc.colorAttachments[0].clearColor = MTLClearColorMake(c.r, c.g, c.b, c.a);
    pass_desc.depthAttachment.loadAction = binocle_backend_mtl_load_action(action->depth.action);
    pass_desc.depthAttachment.clearDepth = action->depth.value;
    pass_desc.stencilAttachment.loadAction = binocle_backend_mtl_load_action(action->stencil.action);
    pass_desc.stencilAttachment.clearStencil = action->stencil.value;
  }

  /* create a render command encoder, this might return nil if window is minimized */
  mtl->cmd_encoder = [mtl->cmd_buffer renderCommandEncoderWithDescriptor:pass_desc];
  if (nil == mtl->cmd_encoder) {
    mtl->pass_valid = false;
    return;
  }

  /* bind the global uniform buffer, this only happens once per pass */
  binocle_backend_mtl_bind_uniform_buffers(mtl);
}

void binocle_backend_mtl_end_pass(binocle_mtl_backend_t *mtl) {
  assert(mtl->in_pass);
  mtl->in_pass = false;
  mtl->pass_valid = false;
  if (nil != mtl->cmd_encoder) {
    [mtl->cmd_encoder endEncoding];
    /* NOTE: MTLRenderCommandEncoder is autoreleased */
    mtl->cmd_encoder = nil;
  }
}

void binocle_backend_mtl_commit(binocle_mtl_backend_t *mtl) {
  assert(!mtl->in_pass);
  assert(!mtl->pass_valid);
  assert(mtl->drawable_cb || mtl->drawable_userdata_cb);
  assert(nil == mtl->cmd_encoder);
  assert(nil != mtl->cmd_buffer);

#if defined(BINOCLE_MACOS)
  [mtl->uniform_buffers[mtl->cur_frame_rotate_index] didModifyRange:NSMakeRange(0, (NSUInteger)mtl->cur_ub_offset)];
#endif

  /* present, commit and signal semaphore when done */
  id<MTLDrawable> cur_drawable = nil;
  if (mtl->drawable_cb) {
    cur_drawable = (__bridge id<MTLDrawable>) mtl->drawable_cb();
  }
  else {
    cur_drawable = (__bridge id<MTLDrawable>) mtl->drawable_userdata_cb(mtl->user_data);
  }
  [mtl->cmd_buffer presentDrawable:cur_drawable];
  [mtl->cmd_buffer addCompletedHandler:^(id<MTLCommandBuffer> cmd_buffer) {
    (void)(cmd_buffer);
    dispatch_semaphore_signal(mtl->sem);
  }];
  [mtl->cmd_buffer commit];

  /* garbage-collect resources pending for release */
  binocle_backend_mtl_garbage_collect(mtl, mtl->frame_index);

  /* rotate uniform buffer slot */
  if (++mtl->cur_frame_rotate_index >= BINOCLE_NUM_INFLIGHT_FRAMES) {
    mtl->cur_frame_rotate_index = 0;
  }
  mtl->frame_index++;
  mtl->cur_ub_offset = 0;
  mtl->cur_ub_base_ptr = 0;
  /* NOTE: MTLCommandBuffer is autoreleased */
  mtl->cmd_buffer = nil;
}

void binocle_backend_mtl_apply_pipeline(binocle_mtl_backend_t *mtl, binocle_pipeline_t* pip) {
  assert(pip);
  assert(pip->shader && (pip->cmn.shader_id.id == pip->shader->slot.id));
  assert(mtl->in_pass);
  if (!mtl->pass_valid) {
    return;
  }
  assert(nil != mtl->cmd_encoder);

  if ((mtl->state_cache.cur_pipeline != pip) || (mtl->state_cache.cur_pipeline_id.id != pip->slot.id)) {
    mtl->state_cache.cur_pipeline = pip;
    mtl->state_cache.cur_pipeline_id.id = pip->slot.id;
    binocle_color c = pip->cmn.blend_color;
    [mtl->cmd_encoder setBlendColorRed:c.r green:c.g blue:c.b alpha:c.a];
    [mtl->cmd_encoder setCullMode:pip->mtl.cull_mode];
    [mtl->cmd_encoder setFrontFacingWinding:pip->mtl.winding];
    [mtl->cmd_encoder setStencilReferenceValue:pip->mtl.stencil_ref];
    [mtl->cmd_encoder setDepthBias:pip->cmn.depth_bias slopeScale:pip->cmn.depth_bias_slope_scale clamp:pip->cmn.depth_bias_clamp];
    assert(pip->mtl.rps != BINOCLE_MTL_INVALID_SLOT_INDEX);
    [mtl->cmd_encoder setRenderPipelineState:binocle_backend_mtl_id(mtl, pip->mtl.rps)];
    assert(pip->mtl.dss != BINOCLE_MTL_INVALID_SLOT_INDEX);
    [mtl->cmd_encoder setDepthStencilState:binocle_backend_mtl_id(mtl, pip->mtl.dss)];
  }
}

void binocle_backend_mtl_apply_bindings(
  binocle_mtl_backend_t *mtl,
  binocle_pipeline_t* pip,
  binocle_buffer_t** vbs, const int* vb_offsets, int num_vbs,
  binocle_buffer_t* ib, int ib_offset,
  binocle_image_t** vs_imgs, int num_vs_imgs,
  binocle_image_t** fs_imgs, int num_fs_imgs)
{
//  _SOKOL_UNUSED(pip);
  assert(mtl->in_pass);
  if (!mtl->pass_valid) {
    return;
  }
  assert(nil != mtl->cmd_encoder);

  /* store index buffer binding, this will be needed later in sg_draw() */
  mtl->state_cache.cur_indexbuffer = ib;
  mtl->state_cache.cur_indexbuffer_offset = ib_offset;
  if (ib) {
    assert(pip->cmn.index_type != BINOCLE_INDEXTYPE_NONE);
    mtl->state_cache.cur_indexbuffer_id.id = ib->slot.id;
  }
  else {
    assert(pip->cmn.index_type == BINOCLE_INDEXTYPE_NONE);
    mtl->state_cache.cur_indexbuffer_id.id = BINOCLE_INVALID_ID;
  }

  /* apply vertex buffers */
  NSUInteger slot;
  for (slot = 0; slot < (NSUInteger)num_vbs; slot++) {
    const binocle_buffer_t* vb = vbs[slot];
    if ((mtl->state_cache.cur_vertexbuffers[slot] != vb) ||
        (mtl->state_cache.cur_vertexbuffer_offsets[slot] != vb_offsets[slot]) ||
        (mtl->state_cache.cur_vertexbuffer_ids[slot].id != vb->slot.id))
    {
      mtl->state_cache.cur_vertexbuffers[slot] = vb;
      mtl->state_cache.cur_vertexbuffer_offsets[slot] = vb_offsets[slot];
      mtl->state_cache.cur_vertexbuffer_ids[slot].id = vb->slot.id;
      const NSUInteger mtl_slot = BINOCLE_MAX_SHADERSTAGE_UBS + slot;
      assert(vb->mtl.buf[vb->cmn.active_slot] != BINOCLE_MTL_INVALID_SLOT_INDEX);
      [mtl->cmd_encoder setVertexBuffer:binocle_backend_mtl_id(mtl, vb->mtl.buf[vb->cmn.active_slot])
                                    offset:(NSUInteger)vb_offsets[slot]
                                   atIndex:mtl_slot];
    }
  }

  /* apply vertex shader images */
  for (slot = 0; slot < (NSUInteger)num_vs_imgs; slot++) {
    const binocle_image_t* img = vs_imgs[slot];
    if ((mtl->state_cache.cur_vs_images[slot] != img) || (mtl->state_cache.cur_vs_image_ids[slot].id != img->slot.id)) {
      mtl->state_cache.cur_vs_images[slot] = img;
      mtl->state_cache.cur_vs_image_ids[slot].id = img->slot.id;
      assert(img->mtl.tex[img->cmn.active_slot] != BINOCLE_MTL_INVALID_SLOT_INDEX);
      [mtl->cmd_encoder setVertexTexture:binocle_backend_mtl_id(mtl, img->mtl.tex[img->cmn.active_slot]) atIndex:slot];
      assert(img->mtl.sampler_state != BINOCLE_MTL_INVALID_SLOT_INDEX);
      [mtl->cmd_encoder setVertexSamplerState:binocle_backend_mtl_id(mtl, img->mtl.sampler_state) atIndex:slot];
    }
  }

  /* apply fragment shader images */
  for (slot = 0; slot < (NSUInteger)num_fs_imgs; slot++) {
    const binocle_image_t* img = fs_imgs[slot];
    if ((mtl->state_cache.cur_fs_images[slot] != img) || (mtl->state_cache.cur_fs_image_ids[slot].id != img->slot.id)) {
      mtl->state_cache.cur_fs_images[slot] = img;
      mtl->state_cache.cur_fs_image_ids[slot].id = img->slot.id;
      assert(img->mtl.tex[img->cmn.active_slot] != BINOCLE_MTL_INVALID_SLOT_INDEX);
      [mtl->cmd_encoder setFragmentTexture:binocle_backend_mtl_id(mtl, img->mtl.tex[img->cmn.active_slot]) atIndex:slot];
      assert(img->mtl.sampler_state != BINOCLE_MTL_INVALID_SLOT_INDEX);
      [mtl->cmd_encoder setFragmentSamplerState:binocle_backend_mtl_id(mtl, img->mtl.sampler_state) atIndex:slot];
    }
  }
}

void binocle_backend_mtl_apply_uniforms(binocle_mtl_backend_t *mtl, binocle_shader_stage stage_index, int ub_index, const binocle_range* data) {
  assert(mtl->in_pass);
  if (!mtl->pass_valid) {
    return;
  }
  assert(nil != mtl->cmd_encoder);
  assert(((size_t)mtl->cur_ub_offset + data->size) <= (size_t)mtl->ub_size);
  assert((mtl->cur_ub_offset & (BINOCLE_MTL_UB_ALIGN-1)) == 0);
  assert(mtl->state_cache.cur_pipeline && mtl->state_cache.cur_pipeline->shader);
  assert(mtl->state_cache.cur_pipeline->slot.id == mtl->state_cache.cur_pipeline_id.id);
  assert(mtl->state_cache.cur_pipeline->shader->slot.id == mtl->state_cache.cur_pipeline->cmn.shader_id.id);
  assert(ub_index < mtl->state_cache.cur_pipeline->shader->cmn.stage[stage_index].num_uniform_blocks);
  assert(data->size <= mtl->state_cache.cur_pipeline->shader->cmn.stage[stage_index].uniform_blocks[ub_index].size);

  /* copy to global uniform buffer, record offset into cmd encoder, and advance offset */
  uint8_t* dst = &mtl->cur_ub_base_ptr[mtl->cur_ub_offset];
  memcpy(dst, data->ptr, data->size);
  if (stage_index == BINOCLE_SHADERSTAGE_VS) {
    [mtl->cmd_encoder setVertexBufferOffset:(NSUInteger)mtl->cur_ub_offset atIndex:(NSUInteger)ub_index];
  }
  else {
    [mtl->cmd_encoder setFragmentBufferOffset:(NSUInteger)mtl->cur_ub_offset atIndex:(NSUInteger)ub_index];
  }
  mtl->cur_ub_offset = BINOCLE_ROUNDUP(mtl->cur_ub_offset + (int)data->size, BINOCLE_MTL_UB_ALIGN);
}

void binocle_backend_mtl_draw(binocle_mtl_backend_t *mtl, int base_element, int num_elements, int num_instances) {
  assert(mtl->in_pass);
  if (!mtl->pass_valid) {
    return;
  }
  assert(nil != mtl->cmd_encoder);
  assert(mtl->state_cache.cur_pipeline && (mtl->state_cache.cur_pipeline->slot.id == mtl->state_cache.cur_pipeline_id.id));
  if (BINOCLE_INDEXTYPE_NONE != mtl->state_cache.cur_pipeline->cmn.index_type) {
    /* indexed rendering */
    assert(mtl->state_cache.cur_indexbuffer && (mtl->state_cache.cur_indexbuffer->slot.id == mtl->state_cache.cur_indexbuffer_id.id));
    const binocle_buffer_t* ib = mtl->state_cache.cur_indexbuffer;
    assert(ib->mtl.buf[ib->cmn.active_slot] != BINOCLE_MTL_INVALID_SLOT_INDEX);
    const NSUInteger index_buffer_offset = (NSUInteger) (mtl->state_cache.cur_indexbuffer_offset + base_element * mtl->state_cache.cur_pipeline->mtl.index_size);
    [mtl->cmd_encoder drawIndexedPrimitives:mtl->state_cache.cur_pipeline->mtl.prim_type
                                    indexCount:(NSUInteger)num_elements
                                     indexType:mtl->state_cache.cur_pipeline->mtl.index_type
                                   indexBuffer:binocle_backend_mtl_id(mtl, ib->mtl.buf[ib->cmn.active_slot])
                             indexBufferOffset:index_buffer_offset
                                 instanceCount:(NSUInteger)num_instances];
  }
  else {
    /* non-indexed rendering */
    [mtl->cmd_encoder drawPrimitives:mtl->state_cache.cur_pipeline->mtl.prim_type
                            vertexStart:(NSUInteger)base_element
                            vertexCount:(NSUInteger)num_elements
                          instanceCount:(NSUInteger)num_instances];
  }
}