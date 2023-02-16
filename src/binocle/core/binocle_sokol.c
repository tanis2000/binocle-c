
#define SOKOL_IMPL
#if defined(BINOCLE_GL)
  #if defined(__EMSCRIPTEN__)
    #define GL_GLEXT_PROTOTYPES
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
    #include <emscripten/emscripten.h>
    #define SOKOL_GLES2
  #elif defined(__IPHONEOS__) || defined(__ANDROID__)
    #define SOKOL_GLES3
  #else
    #define SOKOL_GLCORE33
  #endif
#elif defined(BINOCLE_METAL)
  #define SOKOL_METAL
#endif
#include "sokol_log.h"
#include "sokol_gfx.h"

GLuint binocle_sokol_tex_id(sg_image img_id) {
  SOKOL_ASSERT(img_id.id != SG_INVALID_ID);
  _sg_image_t* img = _sg_lookup_image(&_sg.pools, img_id.id);
  SOKOL_ASSERT(img);
  SOKOL_ASSERT(img->gl.target == GL_TEXTURE_2D);
  SOKOL_ASSERT(0 != img->gl.tex[img->cmn.active_slot]);
  return img->gl.tex[img->cmn.active_slot];
}