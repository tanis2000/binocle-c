#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#define SOKOL_IMPL
#if defined(BINOCLE_GL)
#if defined(__IPHONEOS__) || defined(__ANDROID__)
#define SOKOL_GLES3
#else
#define SOKOL_GLCORE
#endif
#elif defined(BINOCLE_METAL)
#define SOKOL_METAL
#endif
#include "sokol_log.h"
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

const void* binocle_sokol_mtk_get_drawable(void) {
  return (__bridge const void*) [metal_layer nextDrawable];
}

const void* binocle_sokol_mtk_get_depth_stencil_texture(void) {
    return (__bridge const void*) [metal_layer pixelFormat];
}

const void* binocle_sokol_mtk_get_msaa_color_texture(void) {
  return (__bridge const void*) [metal_layer pixelFormat];
}
//sg_context_desc binocle_metal_get_context(void) {
//  return (sg_context_desc) {
//    .sample_count = 1,
//    .metal = {
//      .device = (__bridge const void*) device,
//      .renderpass_descriptor_userdata_cb = binocle_sokol_mtk_get_render_pass_descriptor,
//      .drawable_userdata_cb = binocle_sokol_mtk_get_drawable,
//      .user_data = (void*)metal_layer,
//    }
//  };
//}

const void* binocle_metal_get_device(void) {
  return (__bridge const void*) device;
}

//GLuint binocle_sokol_tex_id(sg_image img_id) {
//  SOKOL_ASSERT(img_id.id != SG_INVALID_ID);
//  _sg_image_t* img = _sg_lookup_image(&_sg.pools, img_id.id);
//  SOKOL_ASSERT(img);
//  SOKOL_ASSERT(img->gl.target == GL_TEXTURE_2D);
//  SOKOL_ASSERT(0 != img->gl.tex[img->cmn.active_slot]);
//  return img->gl.tex[img->cmn.active_slot];
//}