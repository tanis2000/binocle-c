
#define SOKOL_IMPL
#if defined(BINOCLE_GL)
#if defined(__IPHONEOS__) || defined(__ANDROID__) || defined(__EMSCRIPTEN__)
#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#define SOKOL_GLES2
#else
#define SOKOL_GLCORE33
#endif
#elif defined(BINOCLE_METAL)
#define SOKOL_METAL
#endif
#include "sokol_gfx.h"