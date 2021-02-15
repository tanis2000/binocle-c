//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_backend_metal.h"
#import <QuartzCore/CAMetalLayer.h>
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

void binocle_backend_mtl_init(binocle_mtl_backend_t *mtl, binocle_backend_desc *desc) {
  assert(desc);
  assert(desc->uniform_buffer_size > 0);

  binocle_backend_mtl_init_pool(mtl, desc);
  binocle_backend_mtl_init_sampler_cache(mtl, desc);
  binocle_backend_mtl_clear_state_cache(mtl);
  mtl->ub_size = desc->uniform_buffer_size;
  mtl->sem = dispatch_semaphore_create(BINOCLE_NUM_INFLIGHT_FRAMES);

  NSView *view = (__bridge NSView *)desc->context.mtl.mtl_view;
  CAMetalLayer *metal_layer = (CAMetalLayer *)view.layer;

  mtl->device = MTLCreateSystemDefaultDevice();
  mtl->cmd_queue = [mtl->device newCommandQueue];
  mtl->cmd_buffer = [mtl->cmd_queue commandBuffer];

  MTLRenderPassDescriptor *render_desc = [MTLRenderPassDescriptor renderPassDescriptor];
  mtl->cmd_encoder = [mtl->cmd_buffer renderCommandEncoderWithDescriptor:render_desc];

  MTLResourceOptions res_opts = MTLResourceCPUCacheModeWriteCombined;
#if defined(BINOCLE_MACOS)
  res_opts |= MTLResourceStorageModeManaged;
#endif
  mtl->uniform_buffer = [mtl->device newBufferWithLength:mtl->ub_size options: res_opts];

  mtl->valid = true;
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
