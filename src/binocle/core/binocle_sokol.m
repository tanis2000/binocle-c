#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#define SOKOL_IMPL
#if defined(BINOCLE_GL)
#if defined(__IPHONEOS__) || defined(__ANDROID__)
#define SOKOL_GLES3
#else
#define SOKOL_GLCORE33
#endif
#elif defined(BINOCLE_METAL)
#define SOKOL_METAL
#endif
#include "sokol_gfx.h"

static id<CAMetalDrawable> cached_drawable_hack;
static CAMetalLayer *metal_layer;
static id<MTLDevice> device;

void binocle_metal_init(void *mtl_view) {
#if defined(__IPHONEOS__)
  UIView *view = (__bridge UIView *)mtl_view;
#else
  NSView *view = (__bridge NSView *)mtl_view;
#endif
  metal_layer = (CAMetalLayer *)view.layer;
  device = MTLCreateSystemDefaultDevice();
  metal_layer.device = device;
}

static const void* binocle_sokol_mtk_get_render_pass_descriptor(void* user_data) {
  CAMetalLayer *ml = user_data;
  MTLRenderPassDescriptor *desc = [MTLRenderPassDescriptor renderPassDescriptor];
  cached_drawable_hack = [ml nextDrawable];
  desc.colorAttachments[0].texture = cached_drawable_hack.texture;
  desc.depthAttachment.loadAction = MTLLoadActionDontCare;
  desc.stencilAttachment.loadAction = MTLLoadActionDontCare;
  return desc;
}

static const void* binocle_sokol_mtk_get_drawable(void* user_data) {
  return cached_drawable_hack;
}

sg_context_desc binocle_metal_get_context(void) {
  return (sg_context_desc) {
    .sample_count = 1,
    .metal = {
      .device = (__bridge const void*) device,
      .renderpass_descriptor_userdata_cb = binocle_sokol_mtk_get_render_pass_descriptor,
      .drawable_userdata_cb = binocle_sokol_mtk_get_drawable,
      .user_data = (void*)metal_layer,
    }
  };
}
