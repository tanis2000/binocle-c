//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_backend_gl.h"
#include "binocle_blend.h"
#include "../binocle_log.h"
#include "binocle_texture.h"
#include "binocle_material.h"
#include "binocle_vpct.h"
#include "binocle_backend.h"

void binocle_backend_gl_check_error(const char *file, unsigned int line, const char *expression) {
  // Get the last error
  GLenum errorCode = glGetError();

  if (errorCode != GL_NO_ERROR) {
    char error[1024] = "Unknown error";
    char description[1024] = "No description";

    // Decode the error code
    switch (errorCode) {
    case GL_INVALID_ENUM: {
      strcpy(error, "GL_INVALID_ENUM");
      strcpy(description, "An unacceptable value has been specified for an enumerated argument.");
      break;
    }

    case GL_INVALID_VALUE: {
      strcpy(error, "GL_INVALID_VALUE");
      strcpy(description, "A numeric argument is out of range.");
      break;
    }

    case GL_INVALID_OPERATION: {
      strcpy(error, "GL_INVALID_OPERATION");
      strcpy(description, "The specified operation is not allowed in the current state.");
      break;
    }

    case GL_STACK_OVERFLOW: {
      strcpy(error, "GL_STACK_OVERFLOW");
      strcpy(description, "This command would cause a stack overflow.");
      break;
    }

    case GL_STACK_UNDERFLOW: {
      strcpy(error, "GL_STACK_UNDERFLOW");
      strcpy(description, "This command would cause a stack underflow.");
      break;
    }

    case GL_OUT_OF_MEMORY: {
      strcpy(error, "GL_OUT_OF_MEMORY");
      strcpy(description, "There is not enough memory left to execute the command.");
      break;
    }
    default: {
      strcpy(error, "UNKNOWN");
      strcpy(description, "An unknown error has occurred.");
    }

      /*case GLEXT_GL_INVALID_FRAMEBUFFER_OPERATION:
      {
        error = "GL_INVALID_FRAMEBUFFER_OPERATION";
        description = "The object bound to FRAMEBUFFER_BINDING is not \"framebuffer complete\".";
        break;
      }*/
    }

    // Log the error
    SDL_Log("An internal OpenGL call failed in %s (%d).\nExpression:\n%s\nError description:\n %s\n%s", file, line,
            expression, error, description);
    SDL_Quit();
  }
}

GLenum binocle_backend_gl_blend_factor(binocle_blend_factor f) {
  switch (f) {
  case BINOCLE_BLENDFACTOR_ZERO:
    return GL_ZERO;
  case BINOCLE_BLENDFACTOR_ONE:
    return GL_ONE;
  case BINOCLE_BLENDFACTOR_SRC_COLOR:
    return GL_SRC_COLOR;
  case BINOCLE_BLENDFACTOR_ONE_MINUS_SRC_COLOR:
    return GL_ONE_MINUS_SRC_COLOR;
  case BINOCLE_BLENDFACTOR_SRC_ALPHA:
    return GL_SRC_ALPHA;
  case BINOCLE_BLENDFACTOR_ONE_MINUS_SRC_ALPHA:
    return GL_ONE_MINUS_SRC_ALPHA;
  case BINOCLE_BLENDFACTOR_DST_COLOR:
    return GL_DST_COLOR;
  case BINOCLE_BLENDFACTOR_ONE_MINUS_DST_COLOR:
    return GL_ONE_MINUS_DST_COLOR;
  case BINOCLE_BLENDFACTOR_DST_ALPHA:
    return GL_DST_ALPHA;
  case BINOCLE_BLENDFACTOR_ONE_MINUS_DST_ALPHA:
    return GL_ONE_MINUS_DST_ALPHA;
  case BINOCLE_BLENDFACTOR_SRC_ALPHA_SATURATED:
    return GL_SRC_ALPHA_SATURATE;
  case BINOCLE_BLENDFACTOR_BLEND_COLOR:
    return GL_CONSTANT_COLOR;
  case BINOCLE_BLENDFACTOR_ONE_MINUS_BLEND_COLOR:
    return GL_ONE_MINUS_CONSTANT_COLOR;
  case BINOCLE_BLENDFACTOR_BLEND_ALPHA:
    return GL_CONSTANT_ALPHA;
  case BINOCLE_BLENDFACTOR_ONE_MINUS_BLEND_ALPHA:
    return GL_ONE_MINUS_CONSTANT_ALPHA;
  default:
    assert(false);
    return 0;
  }
}

GLenum binocle_backend_gl_blend_op(binocle_blend_op op) {
switch (op) {
case BINOCLE_BLENDOP_ADD:                return GL_FUNC_ADD;
case BINOCLE_BLENDOP_SUBTRACT:           return GL_FUNC_SUBTRACT;
case BINOCLE_BLENDOP_REVERSE_SUBTRACT:   return GL_FUNC_REVERSE_SUBTRACT;
default: assert(false); return 0;
}
}

GLuint binocle_backend_gl_equation_to_gl_constant(binocle_blend_equation blend_equation) {
  switch (blend_equation) {
  case BINOCLE_BLEND_ADD:
    return GL_FUNC_ADD;
  case BINOCLE_BLEND_SUBTRACT:
    return GL_FUNC_SUBTRACT;
  }

  SDL_Log("Invalid value for Binocle::BlendMode::Equation! Fallback to Binocle::BlendMode::Add.");
  return GL_FUNC_ADD;
}

GLenum binocle_backend_gl_depth_attachment_format(binocle_pixel_format fmt) {
  switch (fmt) {
  case BINOCLE_PIXELFORMAT_DEPTH:
    return GL_DEPTH_COMPONENT16;
  case BINOCLE_PIXELFORMAT_DEPTH_STENCIL:
    return GL_DEPTH24_STENCIL8;
  default:
    assert(false);
    return 0;
  }
}

GLenum binocle_backend_gl_teximage_format(binocle_pixel_format fmt) {
  switch (fmt) {
  case BINOCLE_PIXELFORMAT_R8:
  case BINOCLE_PIXELFORMAT_R8SN:
  case BINOCLE_PIXELFORMAT_R16:
  case BINOCLE_PIXELFORMAT_R16SN:
  case BINOCLE_PIXELFORMAT_R16F:
  case BINOCLE_PIXELFORMAT_R32F:
#if defined(SOKOL_GLES2)
    return GL_LUMINANCE;
#else
//    if (backend.gl.gles2) {
//      return GL_LUMINANCE;
//    }
//    else
    {
      return GL_RED;
    }
#endif
#if !defined(SOKOL_GLES2)
  case BINOCLE_PIXELFORMAT_R8UI:
  case BINOCLE_PIXELFORMAT_R8SI:
  case BINOCLE_PIXELFORMAT_R16UI:
  case BINOCLE_PIXELFORMAT_R16SI:
  case BINOCLE_PIXELFORMAT_R32UI:
  case BINOCLE_PIXELFORMAT_R32SI:
//    return GL_RED_INTEGER;
  assert(false);
  return 0;
  case BINOCLE_PIXELFORMAT_RG8:
  case BINOCLE_PIXELFORMAT_RG8SN:
  case BINOCLE_PIXELFORMAT_RG16:
  case BINOCLE_PIXELFORMAT_RG16SN:
  case BINOCLE_PIXELFORMAT_RG16F:
  case BINOCLE_PIXELFORMAT_RG32F:
    return GL_RG;
  case BINOCLE_PIXELFORMAT_RG8UI:
  case BINOCLE_PIXELFORMAT_RG8SI:
  case BINOCLE_PIXELFORMAT_RG16UI:
  case BINOCLE_PIXELFORMAT_RG16SI:
  case BINOCLE_PIXELFORMAT_RG32UI:
  case BINOCLE_PIXELFORMAT_RG32SI:
    return GL_RG_INTEGER;
#endif
  case BINOCLE_PIXELFORMAT_RGBA8:
  case BINOCLE_PIXELFORMAT_RGBA8SN:
  case BINOCLE_PIXELFORMAT_RGBA16:
  case BINOCLE_PIXELFORMAT_RGBA16SN:
  case BINOCLE_PIXELFORMAT_RGBA16F:
  case BINOCLE_PIXELFORMAT_RGBA32F:
  case BINOCLE_PIXELFORMAT_RGB10A2:
    return GL_RGBA;
#if !defined(SOKOL_GLES2)
  case BINOCLE_PIXELFORMAT_RGBA8UI:
  case BINOCLE_PIXELFORMAT_RGBA8SI:
  case BINOCLE_PIXELFORMAT_RGBA16UI:
  case BINOCLE_PIXELFORMAT_RGBA16SI:
  case BINOCLE_PIXELFORMAT_RGBA32UI:
  case BINOCLE_PIXELFORMAT_RGBA32SI:
//    return GL_RGBA_INTEGER;
    assert(false);
    return 0;
#endif
  case BINOCLE_PIXELFORMAT_RG11B10F:
    return GL_RGB;
  case BINOCLE_PIXELFORMAT_DEPTH:
    return GL_DEPTH_COMPONENT;
  case BINOCLE_PIXELFORMAT_DEPTH_STENCIL:
    return GL_DEPTH_STENCIL;
  case BINOCLE_PIXELFORMAT_BC1_RGBA:
    return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
  case BINOCLE_PIXELFORMAT_BC2_RGBA:
    return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
  case BINOCLE_PIXELFORMAT_BC3_RGBA:
    return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
  case BINOCLE_PIXELFORMAT_BC4_R:
    return GL_COMPRESSED_RED_RGTC1;
  case BINOCLE_PIXELFORMAT_BC4_RSN:
    return GL_COMPRESSED_SIGNED_RED_RGTC1;
  case BINOCLE_PIXELFORMAT_BC5_RG:
    assert(false);
    return 0;
//    return GL_COMPRESSED_RED_GREEN_RGTC2;
  case BINOCLE_PIXELFORMAT_BC5_RGSN:
    assert(false);
    return 0;
//    return GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2;
  case BINOCLE_PIXELFORMAT_BC6H_RGBF:
    assert(false);
    return 0;
//    return GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB;
  case BINOCLE_PIXELFORMAT_BC6H_RGBUF:
    assert(false);
    return 0;
//    return GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB;
  case BINOCLE_PIXELFORMAT_BC7_RGBA:
    assert(false);
    return 0;
//    return GL_COMPRESSED_RGBA_BPTC_UNORM_ARB;
  case BINOCLE_PIXELFORMAT_PVRTC_RGB_2BPP:
    assert(false);
    return 0;
//    return GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
  case BINOCLE_PIXELFORMAT_PVRTC_RGB_4BPP:
    assert(false);
    return 0;
//    return GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
  case BINOCLE_PIXELFORMAT_PVRTC_RGBA_2BPP:
    assert(false);
    return 0;
//    return GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
  case BINOCLE_PIXELFORMAT_PVRTC_RGBA_4BPP:
    assert(false);
    return 0;
//    return GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
  case BINOCLE_PIXELFORMAT_ETC2_RGB8:
    assert(false);
    return 0;
//    return GL_COMPRESSED_RGB8_ETC2;
  case BINOCLE_PIXELFORMAT_ETC2_RGB8A1:
    assert(false);
    return 0;
//    return GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
  case BINOCLE_PIXELFORMAT_ETC2_RGBA8:
    assert(false);
    return 0;
//    return GL_COMPRESSED_RGBA8_ETC2_EAC;
  case BINOCLE_PIXELFORMAT_ETC2_RG11:
    assert(false);
    return 0;
//    return GL_COMPRESSED_RG11_EAC;
  case BINOCLE_PIXELFORMAT_ETC2_RG11SN:
    assert(false);
    return 0;
//    return GL_COMPRESSED_SIGNED_RG11_EAC;
  default:
    assert(false); return 0;
  }
}

GLenum binocle_backend_gl_buffer_target(binocle_buffer_type t) {
switch (t) {
case BINOCLE_BUFFERTYPE_VERTEXBUFFER:    return GL_ARRAY_BUFFER;
case BINOCLE_BUFFERTYPE_INDEXBUFFER:     return GL_ELEMENT_ARRAY_BUFFER;
default: assert(false); return 0;
}
}

GLenum binocle_backend_gl_texture_target(binocle_image_type t) {
  switch (t) {
  case BINOCLE_IMAGETYPE_2D:
    return GL_TEXTURE_2D;
  case BINOCLE_IMAGETYPE_CUBE:
    return GL_TEXTURE_CUBE_MAP;
  //#if !defined(BINOCLE_GLES2)
  // case BINOCLE_IMAGETYPE_3D:       return GL_TEXTURE_3D;
  // case BINOCLE_IMAGETYPE_ARRAY:    return GL_TEXTURE_2D_ARRAY;
  //#endif
  default:
    assert(false);
    return 0;
  }
}

GLenum binocle_backend_gl_usage(binocle_usage u) {
switch (u) {
case BINOCLE_USAGE_IMMUTABLE:    return GL_STATIC_DRAW;
case BINOCLE_USAGE_DYNAMIC:      return GL_DYNAMIC_DRAW;
case BINOCLE_USAGE_STREAM:       return GL_STREAM_DRAW;
default: assert(false); return 0;
}
}

GLenum binocle_backend_gl_teximage_internal_format(binocle_pixel_format fmt) {
#if defined(BINOCLE_GLES2)
  return binocle_backend_gl_teximage_format(fmt);
#else
//  if (backend.gl.gles2) {
//    return binocle_backend_gl_teximage_format(fmt);
//  }
//  else 
  {
    switch (fmt) {
    case BINOCLE_PIXELFORMAT_R8:         return GL_R8;
    case BINOCLE_PIXELFORMAT_R8SN:
      assert(false);
      return 0;
//      return GL_R8_SNORM;
    case BINOCLE_PIXELFORMAT_R8UI:       return GL_R8UI;
    case BINOCLE_PIXELFORMAT_R8SI:       return GL_R8I;
#if !defined(SOKOL_GLES3)
    case BINOCLE_PIXELFORMAT_R16:        return GL_R16;
    case BINOCLE_PIXELFORMAT_R16SN:
      assert(false);
      return 0;
//      return GL_R16_SNORM;
#endif
    case BINOCLE_PIXELFORMAT_R16UI:      return GL_R16UI;
    case BINOCLE_PIXELFORMAT_R16SI:      return GL_R16I;
    case BINOCLE_PIXELFORMAT_R16F:       return GL_R16F;
    case BINOCLE_PIXELFORMAT_RG8:        return GL_RG8;
    case BINOCLE_PIXELFORMAT_RG8SN:
      assert(false);
      return 0;
//      return GL_RG8_SNORM;
    case BINOCLE_PIXELFORMAT_RG8UI:      return GL_RG8UI;
    case BINOCLE_PIXELFORMAT_RG8SI:      return GL_RG8I;
    case BINOCLE_PIXELFORMAT_R32UI:      return GL_R32UI;
    case BINOCLE_PIXELFORMAT_R32SI:      return GL_R32I;
    case BINOCLE_PIXELFORMAT_R32F:       return GL_R32F;
#if !defined(SOKOL_GLES3)
    case BINOCLE_PIXELFORMAT_RG16:       return GL_RG16;
    case BINOCLE_PIXELFORMAT_RG16SN:
      assert(false);
      return 0;
//      return GL_RG16_SNORM;
#endif
    case BINOCLE_PIXELFORMAT_RG16UI:     return GL_RG16UI;
    case BINOCLE_PIXELFORMAT_RG16SI:     return GL_RG16I;
    case BINOCLE_PIXELFORMAT_RG16F:      return GL_RG16F;
    case BINOCLE_PIXELFORMAT_RGBA8:      return GL_RGBA8;
    case BINOCLE_PIXELFORMAT_RGBA8SN:
      assert(false);
      return 0;
//      return GL_RGBA8_SNORM;
    case BINOCLE_PIXELFORMAT_RGBA8UI:
      assert(false);
      return 0;
//      return GL_RGBA8UI;
    case BINOCLE_PIXELFORMAT_RGBA8SI:
      assert(false);
      return 0;
//      return GL_RGBA8I;
    case BINOCLE_PIXELFORMAT_RGB10A2:    return GL_RGB10_A2;
    case BINOCLE_PIXELFORMAT_RG11B10F:
      assert(false);
      return 0;
//      return GL_R11F_G11F_B10F;
    case BINOCLE_PIXELFORMAT_RG32UI:     return GL_RG32UI;
    case BINOCLE_PIXELFORMAT_RG32SI:     return GL_RG32I;
    case BINOCLE_PIXELFORMAT_RG32F:      return GL_RG32F;
#if !defined(SOKOL_GLES3)
    case BINOCLE_PIXELFORMAT_RGBA16:     return GL_RGBA16;
    case BINOCLE_PIXELFORMAT_RGBA16SN:
      assert(false);
      return 0;
//      return GL_RGBA16_SNORM;
#endif
    case BINOCLE_PIXELFORMAT_RGBA16UI:
      assert(false);
      return 0;
//      return GL_RGBA16UI;
    case BINOCLE_PIXELFORMAT_RGBA16SI:
      assert(false);
      return 0;
//      return GL_RGBA16I;
    case BINOCLE_PIXELFORMAT_RGBA16F:
      assert(false);
      return 0;
//      return GL_RGBA16F;
    case BINOCLE_PIXELFORMAT_RGBA32UI:
      assert(false);
      return 0;
//      return GL_RGBA32UI;
    case BINOCLE_PIXELFORMAT_RGBA32SI:
      assert(false);
      return 0;
//      return GL_RGBA32I;
    case BINOCLE_PIXELFORMAT_RGBA32F:
      assert(false);
      return 0;
//      return GL_RGBA32F;
    case BINOCLE_PIXELFORMAT_DEPTH:      return GL_DEPTH_COMPONENT16;
    case BINOCLE_PIXELFORMAT_DEPTH_STENCIL:      return GL_DEPTH24_STENCIL8;
    case BINOCLE_PIXELFORMAT_BC1_RGBA:           return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
    case BINOCLE_PIXELFORMAT_BC2_RGBA:           return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
    case BINOCLE_PIXELFORMAT_BC3_RGBA:           return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    case BINOCLE_PIXELFORMAT_BC4_R:              return GL_COMPRESSED_RED_RGTC1;
    case BINOCLE_PIXELFORMAT_BC4_RSN:            return GL_COMPRESSED_SIGNED_RED_RGTC1;
    case BINOCLE_PIXELFORMAT_BC5_RG:
      assert(false);
      return 0;
//      return GL_COMPRESSED_RED_GREEN_RGTC2;
    case BINOCLE_PIXELFORMAT_BC5_RGSN:
      assert(false);
      return 0;
//      return GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2;
    case BINOCLE_PIXELFORMAT_BC6H_RGBF:
      assert(false);
      return 0;
//      return GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB;
    case BINOCLE_PIXELFORMAT_BC6H_RGBUF:
      assert(false);
      return 0;
//      return GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB;
    case BINOCLE_PIXELFORMAT_BC7_RGBA:
      assert(false);
      return 0;
//      return GL_COMPRESSED_RGBA_BPTC_UNORM_ARB;
    case BINOCLE_PIXELFORMAT_PVRTC_RGB_2BPP:
      assert(false);
      return 0;
//      return GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
    case BINOCLE_PIXELFORMAT_PVRTC_RGB_4BPP:
      assert(false);
      return 0;
//      return GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
    case BINOCLE_PIXELFORMAT_PVRTC_RGBA_2BPP:
      assert(false);
      return 0;
//      return GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
    case BINOCLE_PIXELFORMAT_PVRTC_RGBA_4BPP:
      assert(false);
      return 0;
//      return GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
    case BINOCLE_PIXELFORMAT_ETC2_RGB8:
      assert(false);
      return 0;
//      return GL_COMPRESSED_RGB8_ETC2;
    case BINOCLE_PIXELFORMAT_ETC2_RGB8A1:
      assert(false);
      return 0;
//      return GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
    case BINOCLE_PIXELFORMAT_ETC2_RGBA8:
      assert(false);
      return 0;
//      return GL_COMPRESSED_RGBA8_ETC2_EAC;
    case BINOCLE_PIXELFORMAT_ETC2_RG11:
      assert(false);
      return 0;
//      return GL_COMPRESSED_RG11_EAC;
    case BINOCLE_PIXELFORMAT_ETC2_RG11SN:
      assert(false);
      return 0;
//      return GL_COMPRESSED_SIGNED_RG11_EAC;
    default: assert(false); return 0;
    }
  }
#endif
}

GLenum binocle_backend_gl_wrap(binocle_wrap w) {
  switch (w) {
  case BINOCLE_WRAP_CLAMP_TO_EDGE:
    return GL_CLAMP_TO_EDGE;
#if defined(SOKOL_GLCORE33)
  case BINOCLE_WRAP_CLAMP_TO_BORDER:
    return GL_CLAMP_TO_BORDER;
#else
  case BINOCLE_WRAP_CLAMP_TO_BORDER:
    return GL_CLAMP_TO_EDGE;
#endif
  case BINOCLE_WRAP_REPEAT:
    return GL_REPEAT;
  case BINOCLE_WRAP_MIRRORED_REPEAT:
    return GL_MIRRORED_REPEAT;
  default:
    assert(false);
    return 0;
  }
}

GLenum binocle_backend_gl_filter(binocle_filter f) {
  switch (f) {
  case BINOCLE_FILTER_NEAREST:
    return GL_NEAREST;
  case BINOCLE_FILTER_LINEAR:
    return GL_LINEAR;
  case BINOCLE_FILTER_NEAREST_MIPMAP_NEAREST:
    return GL_NEAREST_MIPMAP_NEAREST;
  case BINOCLE_FILTER_NEAREST_MIPMAP_LINEAR:
    return GL_NEAREST_MIPMAP_LINEAR;
  case BINOCLE_FILTER_LINEAR_MIPMAP_NEAREST:
    return GL_LINEAR_MIPMAP_NEAREST;
  case BINOCLE_FILTER_LINEAR_MIPMAP_LINEAR:
    return GL_LINEAR_MIPMAP_LINEAR;
  default:
    assert(false);
    return 0;
  }
}

GLenum binocle_backend_gl_teximage_type(binocle_pixel_format fmt) {
  switch (fmt) {
  case BINOCLE_PIXELFORMAT_R8:
  case BINOCLE_PIXELFORMAT_R8UI:
  case BINOCLE_PIXELFORMAT_RG8:
  case BINOCLE_PIXELFORMAT_RG8UI:
  case BINOCLE_PIXELFORMAT_RGBA8:
  case BINOCLE_PIXELFORMAT_RGBA8UI:
  case BINOCLE_PIXELFORMAT_BGRA8:
    return GL_UNSIGNED_BYTE;
  case BINOCLE_PIXELFORMAT_R8SN:
  case BINOCLE_PIXELFORMAT_R8SI:
  case BINOCLE_PIXELFORMAT_RG8SN:
  case BINOCLE_PIXELFORMAT_RG8SI:
  case BINOCLE_PIXELFORMAT_RGBA8SN:
  case BINOCLE_PIXELFORMAT_RGBA8SI:
    return GL_BYTE;
  case BINOCLE_PIXELFORMAT_R16:
  case BINOCLE_PIXELFORMAT_R16UI:
  case BINOCLE_PIXELFORMAT_RG16:
  case BINOCLE_PIXELFORMAT_RG16UI:
  case BINOCLE_PIXELFORMAT_RGBA16:
  case BINOCLE_PIXELFORMAT_RGBA16UI:
    return GL_UNSIGNED_SHORT;
  case BINOCLE_PIXELFORMAT_R16SN:
  case BINOCLE_PIXELFORMAT_R16SI:
  case BINOCLE_PIXELFORMAT_RG16SN:
  case BINOCLE_PIXELFORMAT_RG16SI:
  case BINOCLE_PIXELFORMAT_RGBA16SN:
  case BINOCLE_PIXELFORMAT_RGBA16SI:
    return GL_SHORT;
  case BINOCLE_PIXELFORMAT_R16F:
  case BINOCLE_PIXELFORMAT_RG16F:
  case BINOCLE_PIXELFORMAT_RGBA16F:
    return GL_HALF_FLOAT;
  case BINOCLE_PIXELFORMAT_R32UI:
  case BINOCLE_PIXELFORMAT_RG32UI:
  case BINOCLE_PIXELFORMAT_RGBA32UI:
    return GL_UNSIGNED_INT;
  case BINOCLE_PIXELFORMAT_R32SI:
  case BINOCLE_PIXELFORMAT_RG32SI:
  case BINOCLE_PIXELFORMAT_RGBA32SI:
    return GL_INT;
  case BINOCLE_PIXELFORMAT_R32F:
  case BINOCLE_PIXELFORMAT_RG32F:
  case BINOCLE_PIXELFORMAT_RGBA32F:
    return GL_FLOAT;
#if !defined(SOKOL_GLES2)
  case BINOCLE_PIXELFORMAT_RGB10A2:
    return GL_UNSIGNED_INT_2_10_10_10_REV;
  case BINOCLE_PIXELFORMAT_RG11B10F:
    assert(false);
    return 0;
//    return GL_UNSIGNED_INT_10F_11F_11F_REV;
#endif
  case BINOCLE_PIXELFORMAT_DEPTH:
    return GL_UNSIGNED_SHORT;
  case BINOCLE_PIXELFORMAT_DEPTH_STENCIL:
    return GL_UNSIGNED_INT_24_8;
  default:
    assert(false); return 0;
  }
}

GLenum binocle_backend_gl_cubeface_target(int face_index) {
  switch (face_index) {
  case 0:
    return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
  case 1:
    return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
  case 2:
    return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
  case 3:
    return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
  case 4:
    return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
  case 5:
    return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
  default:
    assert(false);
    return 0;
  }
}

GLenum binocle_backend_gl_shader_stage(binocle_shader_stage stage) {
  switch (stage) {
  case BINOCLE_SHADERSTAGE_VS:
    return GL_VERTEX_SHADER;
  case BINOCLE_SHADERSTAGE_FS:
    return GL_FRAGMENT_SHADER;
  default:
    assert(false);
    return 0;
  }
}

GLint binocle_backend_gl_vertexformat_size(binocle_vertex_format fmt) {
  switch (fmt) {
  case BINOCLE_VERTEXFORMAT_FLOAT:
    return 1;
  case BINOCLE_VERTEXFORMAT_FLOAT2:
    return 2;
  case BINOCLE_VERTEXFORMAT_FLOAT3:
    return 3;
  case BINOCLE_VERTEXFORMAT_FLOAT4:
    return 4;
  case BINOCLE_VERTEXFORMAT_BYTE4:
    return 4;
  case BINOCLE_VERTEXFORMAT_BYTE4N:
    return 4;
  case BINOCLE_VERTEXFORMAT_UBYTE4:
    return 4;
  case BINOCLE_VERTEXFORMAT_UBYTE4N:
    return 4;
  case BINOCLE_VERTEXFORMAT_SHORT2:
    return 2;
  case BINOCLE_VERTEXFORMAT_SHORT2N:
    return 2;
  case BINOCLE_VERTEXFORMAT_USHORT2N:
    return 2;
  case BINOCLE_VERTEXFORMAT_SHORT4:
    return 4;
  case BINOCLE_VERTEXFORMAT_SHORT4N:
    return 4;
  case BINOCLE_VERTEXFORMAT_USHORT4N:
    return 4;
  case BINOCLE_VERTEXFORMAT_UINT10_N2:
    return 4;
  default:
    assert(false);
    return 0;
  }
}

GLenum binocle_backend_gl_vertexformat_type(binocle_vertex_format fmt) {
  switch (fmt) {
  case BINOCLE_VERTEXFORMAT_FLOAT:
  case BINOCLE_VERTEXFORMAT_FLOAT2:
  case BINOCLE_VERTEXFORMAT_FLOAT3:
  case BINOCLE_VERTEXFORMAT_FLOAT4:
    return GL_FLOAT;
  case BINOCLE_VERTEXFORMAT_BYTE4:
  case BINOCLE_VERTEXFORMAT_BYTE4N:
    return GL_BYTE;
  case BINOCLE_VERTEXFORMAT_UBYTE4:
  case BINOCLE_VERTEXFORMAT_UBYTE4N:
    return GL_UNSIGNED_BYTE;
  case BINOCLE_VERTEXFORMAT_SHORT2:
  case BINOCLE_VERTEXFORMAT_SHORT2N:
  case BINOCLE_VERTEXFORMAT_SHORT4:
  case BINOCLE_VERTEXFORMAT_SHORT4N:
    return GL_SHORT;
  case BINOCLE_VERTEXFORMAT_USHORT2N:
  case BINOCLE_VERTEXFORMAT_USHORT4N:
    return GL_UNSIGNED_SHORT;
  case BINOCLE_VERTEXFORMAT_UINT10_N2:
    return GL_UNSIGNED_INT_2_10_10_10_REV;
  default:
    assert(false);
    return 0;
  }
}

GLboolean
binocle_backend_gl_vertexformat_normalized(binocle_vertex_format fmt) {
  switch (fmt) {
  case BINOCLE_VERTEXFORMAT_BYTE4N:
  case BINOCLE_VERTEXFORMAT_UBYTE4N:
  case BINOCLE_VERTEXFORMAT_SHORT2N:
  case BINOCLE_VERTEXFORMAT_USHORT2N:
  case BINOCLE_VERTEXFORMAT_SHORT4N:
  case BINOCLE_VERTEXFORMAT_USHORT4N:
  case BINOCLE_VERTEXFORMAT_UINT10_N2:
    return GL_TRUE;
  default:
    return GL_FALSE;
  }
}

GLenum binocle_backend_gl_primitive_type(binocle_primitive_type t) {
  switch (t) {
  case BINOCLE_PRIMITIVETYPE_POINTS:
    return GL_POINTS;
  case BINOCLE_PRIMITIVETYPE_LINES:
    return GL_LINES;
  case BINOCLE_PRIMITIVETYPE_LINE_STRIP:
    return GL_LINE_STRIP;
  case BINOCLE_PRIMITIVETYPE_TRIANGLES:
    return GL_TRIANGLES;
  case BINOCLE_PRIMITIVETYPE_TRIANGLE_STRIP:
    return GL_TRIANGLE_STRIP;
  default:
    assert(false);
    return 0;
  }
}

GLenum binocle_backend_gl_index_type(binocle_index_type t) {
  switch (t) {
  case BINOCLE_INDEXTYPE_NONE:
    return 0;
  case BINOCLE_INDEXTYPE_UINT16:
    return GL_UNSIGNED_SHORT;
  case BINOCLE_INDEXTYPE_UINT32:
    return GL_UNSIGNED_INT;
  default:
    assert(false);
    return 0;
  }
}

GLenum binocle_backend_gl_compare_func(binocle_compare_func cmp) {
  switch (cmp) {
  case BINOCLE_COMPAREFUNC_NEVER:
    return GL_NEVER;
  case BINOCLE_COMPAREFUNC_LESS:
    return GL_LESS;
  case BINOCLE_COMPAREFUNC_EQUAL:
    return GL_EQUAL;
  case BINOCLE_COMPAREFUNC_LESS_EQUAL:
    return GL_LEQUAL;
  case BINOCLE_COMPAREFUNC_GREATER:
    return GL_GREATER;
  case BINOCLE_COMPAREFUNC_NOT_EQUAL:
    return GL_NOTEQUAL;
  case BINOCLE_COMPAREFUNC_GREATER_EQUAL:
    return GL_GEQUAL;
  case BINOCLE_COMPAREFUNC_ALWAYS:
    return GL_ALWAYS;
  default:
    assert(false);
    return 0;
  }
}

GLenum binocle_backend_gl_stencil_op(binocle_stencil_op op) {
  switch (op) {
  case BINOCLE_STENCILOP_KEEP:
    return GL_KEEP;
  case BINOCLE_STENCILOP_ZERO:
    return GL_ZERO;
  case BINOCLE_STENCILOP_REPLACE:
    return GL_REPLACE;
  case BINOCLE_STENCILOP_INCR_CLAMP:
    return GL_INCR;
  case BINOCLE_STENCILOP_DECR_CLAMP:
    return GL_DECR;
  case BINOCLE_STENCILOP_INVERT:
    return GL_INVERT;
  case BINOCLE_STENCILOP_INCR_WRAP:
    return GL_INCR_WRAP;
  case BINOCLE_STENCILOP_DECR_WRAP:
    return GL_DECR_WRAP;
  default:
    assert(false);
    return 0;
  }
}

void binocle_backend_gl_init_limits(binocle_gl_backend_t *gl) {
  assert(glGetError() == GL_NO_ERROR);
  GLint gl_int;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &gl_int);
  assert(glGetError() == GL_NO_ERROR);
  gl->limits.max_image_size_2d = gl_int;
  gl->limits.max_image_size_array = gl_int;
  glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &gl_int);
  assert(glGetError() == GL_NO_ERROR);
  gl->limits.max_image_size_cube = gl_int;
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &gl_int);
  assert(glGetError() == GL_NO_ERROR);
  if (gl_int > BINOCLE_MAX_VERTEX_ATTRIBUTES) {
    gl_int = BINOCLE_MAX_VERTEX_ATTRIBUTES;
  }
  gl->limits.max_vertex_attrs = gl_int;
#if !defined(SOKOL_GLES2)
  if (!gl->gles2) {
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &gl_int);
    assert(glGetError() == GL_NO_ERROR);
    gl->limits.max_image_size_3d = gl_int;
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS_EXT, &gl_int);
    assert(glGetError() == GL_NO_ERROR);
    gl->limits.max_image_array_layers = gl_int;
  }
#endif
  if (gl->ext_anisotropic) {
    glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &gl_int);
    assert(glGetError() == GL_NO_ERROR);
    gl->max_anisotropy = gl_int;
  }
  else {
    gl->max_anisotropy = 1;
  }
  glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &gl_int);
  assert(glGetError() == GL_NO_ERROR);
  gl->max_combined_texture_image_units = gl_int;
}

void binocle_backend_gl_init(binocle_gl_backend_t *gl) {
  binocle_backend_gl_init_limits(gl);
  binocle_backend_gl_reset_state_cache(gl);
  // Create a new vertex buffer object (legacy stuff)
  glCheck(glGenBuffers(1, &gl->vbo));
}

void binocle_backend_gl_setup_backend(binocle_gl_backend_t *gl, const binocle_backend_desc* desc) {
  /* assumes that _sg.gl is already zero-initialized */
  gl->valid = true;
#if defined(BINOCLE_GLES2) || defined(BINOCLE_GLES3)
  gl->gles2 = desc->context.gl.force_gles2;
#else
//  _SOKOL_UNUSED(desc);
  gl->gles2 = false;
#endif

  /* clear initial GL error state */
#if defined(BINOCLE_DEBUG)
  while (glGetError() != GL_NO_ERROR);
#endif
#if defined(BINOCLE_GLCORE33)
  binocle_backend_gl_init_caps_glcore33();
#elif defined(BINOCLE_GLES3)
  if (gl->gles2) {
            binocle_backend_gl_init_caps_gles2();
        }
        else {
            binocle_backend_gl_init_caps_gles3();
        }
#else
  // for the time being we replace this with our own
  binocle_backend_gl_init(gl);
//  binocle_backend_gl_init_caps_gles2();
#endif
}

void binocle_backend_gl_apply_default_state() {
  glCheck(glDisable(GL_CULL_FACE));
  glCheck(glDisable(GL_DEPTH_TEST));

  glCheck(glEnable(GL_BLEND));

  // This is for better 2D textures placement
  //glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
  //glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

  /**
   * GL_NEAREST	Returns the value of the texture element that is nearest (in Manhattan distance) to the center of
   * the pixel being textured.
   *
   * GL_LINEAR	Returns the weighted average of the four texture elements that are closest to the center of the pixel being textured.
   * These can include border texture elements, depending on the values of GL_TEXTURE_WRAP_S and GL_TEXTURE_WRAP_T,
   * and on the exact mapping.
   */
  // Enable the following for better 2D pixel rendering
  //glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
  //glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

  glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
}

void binocle_backend_gl_apply_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
  glCheck(glViewport(x, y, w, h));
}

void binocle_backend_gl_apply_blend_mode(const struct binocle_blend blend_mode) {
  glCheck(glBlendFunc(
    binocle_backend_gl_blend_factor(blend_mode.color_src_factor),
    binocle_backend_gl_blend_factor(blend_mode.color_dst_factor)));
  glCheck(glBlendEquation(binocle_backend_gl_equation_to_gl_constant(blend_mode.color_equation)));
}

void binocle_backend_gl_apply_shader(binocle_gl_backend_t *gl,
                                     binocle_gl_shader *shader) {
  GLint id;
  glCheck(id = glGetAttribLocation(shader->gl.prog, "vertexPosition"));
  gl->vertex_attribute = id;
  gl->tex_coord_attribute = glGetAttribLocation(shader->gl.prog, "vertexTCoord");
  gl->color_attribute = glGetAttribLocation(shader->gl.prog, "vertexColor");
  gl->normal_attribute = glGetAttribLocation(shader->gl.prog, "vertexNormal");
  gl->projection_matrix_uniform = glGetUniformLocation(shader->gl.prog, "projectionMatrix");
  gl->view_matrix_uniform = glGetUniformLocation(shader->gl.prog, "viewMatrix");
  gl->model_matrix_uniform = glGetUniformLocation(shader->gl.prog, "modelMatrix");
  gl->image_uniform = glGetUniformLocation(shader->gl.prog, "tex0");
  glCheck(glUseProgram(shader->gl.prog));
}

void binocle_backend_gl_apply_texture(binocle_image_t *texture) {
  glCheck(glActiveTexture(GL_TEXTURE0));
  glCheck(glBindTexture(GL_TEXTURE_2D, texture->gl.tex[0]));
  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
}

void binocle_backend_gl_apply_3d_texture(binocle_image_t *albedo, binocle_image_t *normal) {
  // Diffuse texture
  glCheck(glActiveTexture(GL_TEXTURE0));
  glCheck(glBindTexture(GL_TEXTURE_2D, albedo->gl.tex[0]));
  // Specular texture
  glCheck(glActiveTexture(GL_TEXTURE1));
  glCheck(glBindTexture(GL_TEXTURE_2D, normal->gl.tex[0]));
  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
}

void LEGACY_binocle_backend_gl_draw(binocle_gl_backend_t *gl, const struct binocle_vpct *vertices, size_t vertex_count, struct binocle_blend blend,
                             binocle_gl_shader *shader, binocle_image_t *albedo,
                     struct kmAABB2 viewport, struct kmMat4 *cameraTransformMatrix) {
  binocle_backend_gl_apply_default_state();
  binocle_backend_gl_apply_viewport(viewport.min.x, viewport.min.y, viewport.max.x, viewport.max.y);
  binocle_backend_gl_apply_blend_mode(blend);
  binocle_backend_gl_apply_shader(gl, shader);
  binocle_backend_gl_apply_texture(albedo);

  kmMat4 projectionMatrix = binocle_math_create_orthographic_matrix_off_center(viewport.min.x, viewport.max.x,
                                                                               viewport.min.y, viewport.max.y, -1000.0f,
                                                                               1000.0f);
  //kmMat4 modelViewMatrix = binocle_gd_create_model_view_matrix(0.0f, 0.0f, 1.0f, 0.0f);
  kmMat4 viewMatrix;
  kmMat4Identity(&viewMatrix);
  // TODO: apply the viewport_adapter scale_matrix here
  //kmMat4Scaling(&viewMatrix, 2.0f, 2.0f, 2.0f);

  if (cameraTransformMatrix != NULL) {
    kmMat4Multiply(&viewMatrix, &viewMatrix, cameraTransformMatrix);
  }

  kmMat4 modelMatrix;
  kmMat4Identity(&modelMatrix);

  glCheck(glEnableVertexAttribArray(gl->vertex_attribute));
  glCheck(glEnableVertexAttribArray(gl->color_attribute));
  glCheck(glEnableVertexAttribArray(gl->tex_coord_attribute));

  glCheck(glBindBuffer(GL_ARRAY_BUFFER, gl->vbo));
  glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(binocle_vpct) * vertex_count, vertices, GL_STATIC_DRAW));

  glCheck(glVertexAttribPointer(gl->vertex_attribute, 2, GL_FLOAT, GL_FALSE, sizeof(binocle_vpct), 0));
  glCheck(glVertexAttribPointer(gl->color_attribute, 4, GL_FLOAT, GL_FALSE, sizeof(binocle_vpct),
                                (void *) (2 * sizeof(GLfloat))));
  glCheck(glVertexAttribPointer(
    gl->tex_coord_attribute, 2, GL_FLOAT, GL_FALSE, sizeof(binocle_vpct),
                                (void *) (4 * sizeof(GLfloat) + 2 * sizeof(GLfloat))));

  //kmMat4 finalMatrix = Matrix4::mul(state.transform,projectionMatrix);
  //kmMat4 inverseMatrix: Matrix4<f32> = Matrix4::from_nonuniform_scale(1.0, 1.0, 1.0);

  glCheck(glUniformMatrix4fv(gl->projection_matrix_uniform, 1, GL_FALSE, projectionMatrix.mat));
  //glCheck(glUniformMatrix4fv(gd->model_view_matrix_uniform, 1, GL_FALSE, modelViewMatrix.mat));
  glCheck(glUniformMatrix4fv(gl->view_matrix_uniform, 1, GL_FALSE, viewMatrix.mat));
  glCheck(glUniformMatrix4fv(gl->model_matrix_uniform, 1, GL_FALSE, modelMatrix.mat));

  glCheck(glUniform1i(gl->image_uniform, 0));

  glCheck(glDrawArrays(GL_TRIANGLES, 0, vertex_count));

  glCheck(glDisableVertexAttribArray(gl->vertex_attribute));
  glCheck(glDisableVertexAttribArray(gl->color_attribute));
  glCheck(glDisableVertexAttribArray(gl->tex_coord_attribute));
  glCheck(glUseProgram(GL_ZERO));

  glCheck(glBindTexture(GL_TEXTURE_2D, 0));
  glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

binocle_resource_state binocle_backend_gl_create_render_target(binocle_render_target_t *rt, uint32_t width, uint32_t height, bool use_depth, binocle_pixel_format format) {
  /*
   * glGenBuffers creates regular buffers for vertex data, etc.
   * glGenFrameBuffers creates a framebuffer object primarily used as render targets for offscreen rendering.
   * glGenRenderBuffers creates a renderbuffer object that are specifically used with framebuffer objects for any depth-testing required.
   */

//  binocle_render_target_t *res = SDL_malloc(sizeof(binocle_render_target_t));

  GLenum fmt = binocle_backend_gl_teximage_format(format);

  GLuint fb[1];
  glCheck(glGenFramebuffers(1, fb));
  rt->frame_buffer = fb[0];

  GLuint rb[1];
  glCheck(glGenRenderbuffers(1, rb));
  rt->render_buffer = rb[0];

  GLuint t[1];
  glCheck(glGenTextures(1, t));
  rt->texture = t[0];

  // set up framebuffer

  // bind the framebuffer
  glCheck(glBindFramebuffer(GL_FRAMEBUFFER, rt->frame_buffer));

  // bind the newly created texture: all future texture functions will modify this texture
  glCheck(glBindTexture(GL_TEXTURE_2D, rt->texture));
  // Give an empty image to OpenGL ( the last "0" )
  glCheck(glTexImage2D(GL_TEXTURE_2D, 0, fmt, width, height, 0, fmt, GL_UNSIGNED_BYTE, 0));
  // filtering
  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
  glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
  // attach the texture to the bound framebuffer object
  glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rt->texture, 0));

  // set up renderbuffer (depth buffer)
  glCheck(glBindRenderbuffer(GL_RENDERBUFFER, rt->render_buffer));
  glCheck(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height));
  glCheck(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rt->render_buffer));

  // clean up
  glCheck(glBindTexture(GL_TEXTURE_2D, 0));
  glCheck(glBindRenderbuffer(GL_RENDERBUFFER, 0));
  glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));

  //if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
  //  binocle_log_error("Framebuffer isn't complete");
  //}
  return BINOCLE_RESOURCESTATE_VALID;
}

void binocle_backend_gl_destroy_render_target(binocle_render_target_t *render_target) {
  glCheck(glDeleteTextures(1, &render_target->texture));
  render_target->texture = GL_NONE;
  glCheck(glDeleteRenderbuffers(1, &render_target->render_buffer));
  render_target->render_buffer = GL_NONE;
  glCheck(glDeleteFramebuffers(1, &render_target->frame_buffer));
  render_target->frame_buffer = GL_NONE;
//  SDL_free(render_target);
}

void binocle_backend_gl_set_render_target(binocle_image_t *render_target) {
  if (render_target == NULL) {
    glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    glCheck(glBindRenderbuffer(GL_RENDERBUFFER, 0));
  } else {
    glCheck(glBindFramebuffer(GL_FRAMEBUFFER, render_target->gl.frame_buffer));
    // NOTE: this might not be needed unless we use MSAA and it'' not OpenGLES2
    //glCheck(glBindRenderbuffer(GL_RENDERBUFFER, render_target->render_buffer));
  }
}

void binocle_backend_gl_clear(struct binocle_color color) {
  glCheck(glClearColor(color.r, color.g, color.b, color.a));
  glCheck(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void binocle_backend_gl_set_uniform_float2(binocle_gl_shader *shader, const char *name, float value1, float value2) {
  GLint id = 0;
  glCheck(id = glGetUniformLocation(shader->gl.prog, name));
  if (id == -1) {
    binocle_log_error("Cannot find uniform called %s", name);
    return;
  }
  glCheck(glUniform2f(id, value1, value2));
}

void binocle_backend_gl_set_uniform_mat4(binocle_gl_shader *shader, const char *name, struct kmMat4 mat) {
  GLint id = 0;
  glCheck(id = glGetUniformLocation(shader->gl.prog, name));
  if (id == -1) {
    binocle_log_error("Cannot find uniform called %s", name);
    return;
  }
  glCheck(glUniformMatrix4fv(id, 1, GL_FALSE, mat.mat));
}

void binocle_backend_gl_draw_quad_to_screen(
  binocle_gl_shader *shader, binocle_image_t *render_target) {
  static const GLfloat g_quad_vertex_buffer_data[] = {
      -1.0f, -1.0f,
      1.0f, -1.0f,
      -1.0f, 1.0f,
      -1.0f, 1.0f,
      1.0f, -1.0f,
      1.0f, 1.0f,
  };

  GLuint quad_vertexbuffer;
  glCheck(glGenBuffers(1, &quad_vertexbuffer));
  glCheck(glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer));
  glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW));
  GLint pos_id;
  glCheck(pos_id = glGetAttribLocation(shader->gl.prog, "position"));
  glCheck(glVertexAttribPointer(pos_id, 2, GL_FLOAT, false, 0, 0));
  glCheck(glEnableVertexAttribArray(pos_id));
  GLint tex_id;
  glCheck(tex_id = glGetUniformLocation(shader->gl.prog, "texture"));
  glCheck(glUniform1i(tex_id, 0));
  glCheck(glActiveTexture(GL_TEXTURE0));
  glCheck(glBindTexture(GL_TEXTURE_2D, render_target->gl.tex[0]));
  // Sets the frame buffer to use as the screen
#if defined(__IPHONEOS__)
  SDL_SysWMinfo info;
  SDL_VERSION(&info.version);
  SDL_GetWindowWMInfo(SDL_GL_GetCurrentWindow(), &info);
  GLuint id = info.info.uikit.framebuffer;
  glCheck(glBindFramebuffer(GL_FRAMEBUFFER, id));
#else
  glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));
#endif
  glCheck(glDrawArrays(GL_TRIANGLES, 0, 6));

  glCheck(glDisableVertexAttribArray(pos_id));
  glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
  glCheck(glDeleteBuffers(1, &quad_vertexbuffer));
  glCheck(glBindTexture(GL_TEXTURE_2D, 0));
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

void binocle_backend_gl_init_attr(binocle_gl_attr_t* attr) {
  attr->vb_index = -1;
  attr->divisor = -1;
}

/*-- state cache implementation ----------------------------------------------*/
void binocle_backend_gl_cache_clear_buffer_bindings(binocle_gl_backend_t *gl, bool force) {
  if (force || (gl->cache.vertex_buffer != 0)) {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    gl->cache.vertex_buffer = 0;
  }
  if (force || (gl->cache.index_buffer != 0)) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    gl->cache.index_buffer = 0;
  }
}

void binocle_backend_gl_cache_bind_buffer(binocle_gl_backend_t *gl, GLenum target, GLuint buffer) {
  assert((GL_ARRAY_BUFFER == target) || (GL_ELEMENT_ARRAY_BUFFER == target));
  if (target == GL_ARRAY_BUFFER) {
    if (gl->cache.vertex_buffer != buffer) {
      gl->cache.vertex_buffer = buffer;
      glBindBuffer(target, buffer);
    }
  }
  else {
    if (gl->cache.index_buffer != buffer) {
      gl->cache.index_buffer = buffer;
      glBindBuffer(target, buffer);
    }
  }
}

void binocle_backend_gl_cache_store_buffer_binding(binocle_gl_backend_t *gl, GLenum target) {
  if (target == GL_ARRAY_BUFFER) {
    gl->cache.stored_vertex_buffer = gl->cache.vertex_buffer;
  }
  else {
    gl->cache.stored_index_buffer = gl->cache.index_buffer;
  }
}

void binocle_backend_gl_cache_restore_buffer_binding(binocle_gl_backend_t *gl, GLenum target) {
  if (target == GL_ARRAY_BUFFER) {
    if (gl->cache.stored_vertex_buffer != 0) {
      /* we only care restoring valid ids */
      binocle_backend_gl_cache_bind_buffer(gl, target, gl->cache.stored_vertex_buffer);
      gl->cache.stored_vertex_buffer = 0;
    }
  }
  else {
    if (gl->cache.stored_index_buffer != 0) {
      /* we only care restoring valid ids */
      binocle_backend_gl_cache_bind_buffer(gl, target, gl->cache.stored_index_buffer);
      gl->cache.stored_index_buffer = 0;
    }
  }
}

/* called when from _sg_gl_destroy_buffer() */
void binocle_backend_gl_cache_invalidate_buffer(binocle_gl_backend_t *gl, GLuint buf) {
  if (buf == gl->cache.vertex_buffer) {
    gl->cache.vertex_buffer = 0;
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
  if (buf == gl->cache.index_buffer) {
    gl->cache.index_buffer = 0;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }
  if (buf == gl->cache.stored_vertex_buffer) {
    gl->cache.stored_vertex_buffer = 0;
  }
  if (buf == gl->cache.stored_index_buffer) {
    gl->cache.stored_index_buffer = 0;
  }
  for (int i = 0; i < BINOCLE_MAX_VERTEX_ATTRIBUTES; i++) {
    if (buf == gl->cache.attrs[i].gl_vbuf) {
      gl->cache.attrs[i].gl_vbuf = 0;
    }
  }
}

void binocle_backend_gl_cache_active_texture(binocle_gl_backend_t *gl, GLenum texture) {
  if (gl->cache.cur_active_texture != texture) {
    gl->cache.cur_active_texture = texture;
    glActiveTexture(texture);
  }
}

void binocle_backend_gl_cache_clear_texture_bindings(binocle_gl_backend_t *gl, bool force) {
  for (int i = 0; (i < BINOCLE_MAX_SHADERSTAGE_IMAGES) && (i < gl->max_combined_texture_image_units); i++) {
    if (force || (gl->cache.textures[i].texture != 0)) {
      GLenum gl_texture_slot = GL_TEXTURE0 + i;
      glActiveTexture(gl_texture_slot);
      glBindTexture(GL_TEXTURE_2D, 0);
      glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
//#if !defined(SOKOL_GLES2)
//      if (!gl->gles2) {
//        glBindTexture(GL_TEXTURE_3D, 0);
//        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
//      }
//#endif
      gl->cache.textures[i].target = 0;
      gl->cache.textures[i].texture = 0;
      gl->cache.cur_active_texture = gl_texture_slot;
    }
  }
}

void binocle_backend_gl_cache_bind_texture(binocle_gl_backend_t *gl, int slot_index, GLenum target, GLuint texture) {
  /* it's valid to call this function with target=0 and/or texture=0
     target=0 will unbind the previous binding, texture=0 will clear
     the new binding
  */
  assert(slot_index < BINOCLE_MAX_SHADERSTAGE_IMAGES);
  if (slot_index >= gl->max_combined_texture_image_units) {
    return;
  }
  binocle_gl_texture_bind_slot *slot = &gl->cache.textures[slot_index];
  if ((slot->target != target) || (slot->texture != texture)) {
    binocle_backend_gl_cache_active_texture(gl, GL_TEXTURE0 + slot_index);
    /* if the target has changed, clear the previous binding on that target */
    if ((target != slot->target) && (slot->target != 0)) {
      glBindTexture(slot->target, 0);
    }
    /* apply new binding (texture can be 0 to unbind) */
    if (target != 0) {
      glBindTexture(target, texture);
    }
    slot->target = target;
    slot->texture = texture;
  }
}

void binocle_backend_gl_cache_store_texture_binding(binocle_gl_backend_t *gl, int slot_index) {
  assert(slot_index < BINOCLE_MAX_SHADERSTAGE_IMAGES);
  gl->cache.stored_texture = gl->cache.textures[slot_index];
}

void binocle_backend_gl_cache_restore_texture_binding(binocle_gl_backend_t *gl, int slot_index) {
  assert(slot_index < BINOCLE_MAX_SHADERSTAGE_IMAGES);
  binocle_gl_texture_bind_slot *slot = &gl->cache.stored_texture;
  if (slot->texture != 0) {
    /* we only care restoring valid ids */
    assert(slot->target != 0);
    binocle_backend_gl_cache_bind_texture(gl, slot_index, slot->target, slot->texture);
    slot->target = 0;
    slot->texture = 0;
  }
}

/* called from _sg_gl_destroy_texture() */
void binocle_backend_gl_cache_invalidate_texture(binocle_gl_backend_t *gl, GLuint tex) {
  for (int i = 0; i < BINOCLE_MAX_SHADERSTAGE_IMAGES; i++) {
    binocle_gl_texture_bind_slot* slot = &gl->cache.textures[i];
    if (tex == slot->texture) {
      binocle_backend_gl_cache_active_texture(gl, GL_TEXTURE0 + i);
      glBindTexture(slot->target, 0);
      slot->target = 0;
      slot->texture = 0;
    }
  }
  if (tex == gl->cache.stored_texture.texture) {
    gl->cache.stored_texture.target = 0;
    gl->cache.stored_texture.texture = 0;
  }
}

/* called from _sg_gl_destroy_shader() */
void binocle_backend_gl_cache_invalidate_program(binocle_gl_backend_t *gl, GLuint prog) {
  if (prog == gl->cache.prog) {
    gl->cache.prog = 0;
    glUseProgram(0);
  }
}

void binocle_backend_gl_reset_state_cache(binocle_gl_backend_t *gl) {
  if (true /*gl->cur_context*/) {
    assert(glGetError() == GL_NO_ERROR);
//#if !defined(SOKOL_GLES2)
//    if (!gl->gles2) {
//      glBindVertexArray(gl->cur_context->vao);
//      _BINOCLE_GL_CHECK_ERROR();
//    }
//#endif
    memset(&gl->cache, 0, sizeof(gl->cache));
    binocle_backend_gl_cache_clear_buffer_bindings(gl, true);
    assert(glGetError() == GL_NO_ERROR);
    binocle_backend_gl_cache_clear_texture_bindings(gl, true);
    assert(glGetError() == GL_NO_ERROR);
    for (uint32_t i = 0; i < BINOCLE_MAX_VERTEX_ATTRIBUTES/*backend.limits.max_vertex_attrs*/; i++) {
      binocle_backend_gl_init_attr(&gl->cache.attrs[i].gl_attr);
      glDisableVertexAttribArray(i);
      assert(glGetError() == GL_NO_ERROR);
    }
    gl->cache.cur_primitive_type = GL_TRIANGLES;

    /* shader program */
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&gl->cache.prog);
    assert(glGetError() == GL_NO_ERROR);

    /* depth-stencil state */
//    binocle_backend_gl_init_depth_stencil_state(&gl->cache.ds);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    glDepthMask(GL_FALSE);
    glDisable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0, 0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilMask(0);

    /* blend state */
//    binocle_backend_gl_init_blend_state(&gl->cache.blend);
    glDisable(GL_BLEND);
    glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glBlendColor(0.0f, 0.0f, 0.0f, 0.0f);

    /* rasterizer state */
//    binocle_backend_gl_init_rasterizer_state(&gl->cache.rast);
    glPolygonOffset(0.0f, 0.0f);
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_CULL_FACE);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glEnable(GL_SCISSOR_TEST);
    glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    glEnable(GL_DITHER);
    glDisable(GL_POLYGON_OFFSET_FILL);
#if defined(SOKOL_GLCORE33)
    glEnable(GL_MULTISAMPLE);
            glEnable(GL_PROGRAM_POINT_SIZE);
#endif
  }
}

binocle_resource_state
binocle_backend_gl_create_image(binocle_gl_backend_t *gl, binocle_image_t *img,
                                const binocle_image_desc *desc) {
  assert(img && desc);
  assert(glGetError() == GL_NO_ERROR);
  binocle_backend_image_common_init(&img->cmn, desc);
  img->gl.ext_textures = (0 != desc->gl_textures[0]);

  /* check if texture format is support */
//  if (!binocle_backend_gl_supported_texture_format(img->cmn.pixel_format)) {
//    binocle_log_error("texture format not supported by GL context\n");
//    return BINOCLE_RESOURCESTATE_FAILED;
//  }
  /* check for optional texture types */
//  if ((img->cmn.type == BINOCLE_IMAGETYPE_3D) && !_sg.features.imagetype_3d) {
//    binocle_log_error("3D textures not supported by GL context\n");
//    return BINOCLE_RESOURCESTATE_FAILED;
//  }
//  if ((img->cmn.type == BINOCLE_IMAGETYPE_ARRAY) && !_sg.features.imagetype_array) {
//    binocle_log_error("array textures not supported by GL context\n");
//    return BINOCLE_RESOURCESTATE_FAILED;
//  }

#if !defined(BINOCLE_GLES2)
  bool msaa = false;
  if (!gl->gles2) {
    msaa = (img->cmn.sample_count > 1) /*&& (_sg.features.msaa_render_targets)*/;
  }
#endif

  if (binocle_backend_is_valid_rendertarget_depth_format(
        img->cmn.pixel_format)) {
    /* special case depth-stencil-buffer? */
    assert((img->cmn.usage == BINOCLE_USAGE_IMMUTABLE) &&
           (img->cmn.num_slots == 1));
    assert(
      !img->gl
         .ext_textures); /* cannot provide external texture for depth images */
    glGenRenderbuffers(1, &img->gl.depth_render_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, img->gl.depth_render_buffer);
    GLenum gl_depth_format =
      binocle_backend_gl_depth_attachment_format(img->cmn.pixel_format);
#if !defined(BINOCLE_GLES2)
    if (!gl->gles2 && msaa) {
      glRenderbufferStorageMultisample(GL_RENDERBUFFER, img->cmn.sample_count,
                                       gl_depth_format, img->cmn.width,
                                       img->cmn.height);
    } else
#endif
    {
      glRenderbufferStorage(GL_RENDERBUFFER, gl_depth_format, img->cmn.width,
                            img->cmn.height);
    }
  } else {
    /* regular color texture */
    img->gl.target = binocle_backend_gl_texture_target(img->cmn.type);
    const GLenum gl_internal_format =
      binocle_backend_gl_teximage_internal_format(img->cmn.pixel_format);

/* if this is a MSAA render target, need to create a separate render buffer */
#if !defined(BINOCLE_GLES2)
    if (!gl->gles2 && img->cmn.render_target && msaa) {
      glGenRenderbuffers(1, &img->gl.msaa_render_buffer);
      glBindRenderbuffer(GL_RENDERBUFFER, img->gl.msaa_render_buffer);
      glRenderbufferStorageMultisample(GL_RENDERBUFFER, img->cmn.sample_count,
                                       gl_internal_format, img->cmn.width,
                                       img->cmn.height);
    }
#endif

    if (img->gl.ext_textures) {
      /* inject externally GL textures */
      for (int slot = 0; slot < img->cmn.num_slots; slot++) {
        assert(desc->gl_textures[slot]);
        img->gl.tex[slot] = desc->gl_textures[slot];
      }
      if (desc->gl_texture_target) {
        img->gl.target = (GLenum)desc->gl_texture_target;
      }
    } else {
      /* create our own GL texture(s) */
      const GLenum gl_format =
        binocle_backend_gl_teximage_format(img->cmn.pixel_format);
      const bool is_compressed =
        binocle_backend_is_compressed_pixel_format(img->cmn.pixel_format);
      for (int slot = 0; slot < img->cmn.num_slots; slot++) {
        glGenTextures(1, &img->gl.tex[slot]);
        binocle_backend_gl_cache_store_texture_binding(gl, 0);
        binocle_backend_gl_cache_bind_texture(gl, 0, img->gl.target,
                                              img->gl.tex[slot]);
        GLenum gl_min_filter = binocle_backend_gl_filter(img->cmn.min_filter);
        GLenum gl_mag_filter = binocle_backend_gl_filter(img->cmn.mag_filter);
        glTexParameteri(img->gl.target, GL_TEXTURE_MIN_FILTER, gl_min_filter);
        glTexParameteri(img->gl.target, GL_TEXTURE_MAG_FILTER, gl_mag_filter);
        if (gl->ext_anisotropic && (img->cmn.max_anisotropy > 1)) {
          GLint max_aniso = (GLint)img->cmn.max_anisotropy;
          if (max_aniso > gl->max_anisotropy) {
            max_aniso = gl->max_anisotropy;
          }
          glTexParameteri(img->gl.target, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                          max_aniso);
        }
        if (img->cmn.type == BINOCLE_IMAGETYPE_CUBE) {
          glTexParameteri(img->gl.target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
          glTexParameteri(img->gl.target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        } else {
          glTexParameteri(img->gl.target, GL_TEXTURE_WRAP_S,
                          binocle_backend_gl_wrap(img->cmn.wrap_u));
          glTexParameteri(img->gl.target, GL_TEXTURE_WRAP_T,
                          binocle_backend_gl_wrap(img->cmn.wrap_v));
#if !defined(BINOCLE_GLES2)
          if (!gl->gles2 && (img->cmn.type == BINOCLE_IMAGETYPE_3D)) {
            glTexParameteri(img->gl.target, GL_TEXTURE_WRAP_R,
                            binocle_backend_gl_wrap(img->cmn.wrap_w));
          }
#endif
#if defined(BINOCLE_GLCORE33)
          float border[4];
          switch (img->cmn.border_color) {
          case BINOCLE_BORDERCOLOR_TRANSPARENT_BLACK:
            border[0] = 0.0f;
            border[1] = 0.0f;
            border[2] = 0.0f;
            border[3] = 0.0f;
            break;
          case BINOCLE_BORDERCOLOR_OPAQUE_WHITE:
            border[0] = 1.0f;
            border[1] = 1.0f;
            border[2] = 1.0f;
            border[3] = 1.0f;
            break;
          default:
            border[0] = 0.0f;
            border[1] = 0.0f;
            border[2] = 0.0f;
            border[3] = 1.0f;
            break;
          }
          glTexParameterfv(img->gl.target, GL_TEXTURE_BORDER_COLOR, border);
#endif
        }
#if !defined(BINOCLE_GLES2)
        if (!gl->gles2) {
          /* GL spec has strange defaults for mipmap min/max lod: -1000 to +1000
           */
          const float min_lod =
            BINOCLE_CLAMP(desc->min_lod, 0.0f, 1000.0f);
          const float max_lod = BINOCLE_CLAMP(desc->max_lod, 0.0f, 1000.0f);
          glTexParameterf(img->gl.target, GL_TEXTURE_MIN_LOD, min_lod);
          glTexParameterf(img->gl.target, GL_TEXTURE_MAX_LOD, max_lod);
        }
#endif
        const int num_faces = img->cmn.type == BINOCLE_IMAGETYPE_CUBE ? 6 : 1;
        int data_index = 0;
        for (int face_index = 0; face_index < num_faces; face_index++) {
          for (int mip_index = 0; mip_index < img->cmn.num_mipmaps;
               mip_index++, data_index++) {
            GLenum gl_img_target = img->gl.target;
            if (BINOCLE_IMAGETYPE_CUBE == img->cmn.type) {
              gl_img_target = binocle_backend_gl_cubeface_target(face_index);
            }
            const GLvoid *data_ptr =
              desc->data.subimage[face_index][mip_index].ptr;
            const int data_size =
              desc->data.subimage[face_index][mip_index].size;
            int mip_width = img->cmn.width >> mip_index;
            if (mip_width == 0) {
              mip_width = 1;
            }
            int mip_height = img->cmn.height >> mip_index;
            if (mip_height == 0) {
              mip_height = 1;
            }
            if ((BINOCLE_IMAGETYPE_2D == img->cmn.type) ||
                (BINOCLE_IMAGETYPE_CUBE == img->cmn.type)) {
              if (is_compressed) {
                glCompressedTexImage2D(gl_img_target, mip_index,
                                       gl_internal_format, mip_width,
                                       mip_height, 0, data_size, data_ptr);
              } else {
                const GLenum gl_type =
                  binocle_backend_gl_teximage_type(img->cmn.pixel_format);
                glTexImage2D(gl_img_target, mip_index, gl_internal_format,
                             mip_width, mip_height, 0, gl_format, gl_type,
                             data_ptr);
              }
            }
#if !defined(BINOCLE_GLES2)
            else if (!gl->gles2 &&
                     ((BINOCLE_IMAGETYPE_3D == img->cmn.type) ||
                      (BINOCLE_IMAGETYPE_ARRAY == img->cmn.type))) {
              int mip_depth = img->cmn.depth;
              if (BINOCLE_IMAGETYPE_3D == img->cmn.type) {
                mip_depth >>= mip_index;
              }
              if (mip_depth == 0) {
                mip_depth = 1;
              }
              if (is_compressed) {
                glCompressedTexImage3D(
                  gl_img_target, mip_index, gl_internal_format, mip_width,
                  mip_height, mip_depth, 0, data_size, data_ptr);
              } else {
                const GLenum gl_type =
                  binocle_backend_gl_teximage_type(img->cmn.pixel_format);
                glTexImage3D(gl_img_target, mip_index, gl_internal_format,
                             mip_width, mip_height, mip_depth, 0, gl_format,
                             gl_type, data_ptr);
              }
            }
#endif
          }
        }
        binocle_backend_gl_cache_restore_texture_binding(gl, 0);
      }
    }
  }

  // Begin hack to get render targets working without render passes
//  {
//    GLenum fmt =
//      binocle_backend_gl_teximage_format(desc->pixel_format);
//
//    GLuint fb[1];
//    glCheck(glGenFramebuffers(1, fb));
//    img->gl.frame_buffer = fb[0];
//
//    GLuint rb[1];
//    glCheck(glGenRenderbuffers(1, rb));
//    img->gl.depth_render_buffer = rb[0];
//
//    // set up framebuffer
//
//    // bind the framebuffer
//    glCheck(glBindFramebuffer(GL_FRAMEBUFFER, img->gl.frame_buffer));
//
//    // bind the newly created texture: all future texture functions will modify
//    // this texture
//    glCheck(glBindTexture(GL_TEXTURE_2D, img->gl.tex[0]));
//    // Give an empty image to OpenGL ( the last "0" )
//    glCheck(glTexImage2D(GL_TEXTURE_2D, 0, fmt, img->cmn.width, img->cmn.height,
//                         0, fmt, GL_UNSIGNED_BYTE, 0));
//    // filtering
//    glCheck(
//      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
//    glCheck(
//      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
//    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
//    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
//    // attach the texture to the bound framebuffer object
//    glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
//                                   GL_TEXTURE_2D, img->gl.tex[0], 0));
//
//    // set up renderbuffer (depth buffer)
//    glCheck(glBindRenderbuffer(GL_RENDERBUFFER, img->gl.depth_render_buffer));
//    glCheck(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16,
//                                  img->cmn.width, img->cmn.height));
//    glCheck(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
//                                      GL_RENDERBUFFER,
//                                      img->gl.depth_render_buffer));
//
//    // clean up
//    glCheck(glBindTexture(GL_TEXTURE_2D, 0));
//    glCheck(glBindRenderbuffer(GL_RENDERBUFFER, 0));
//    glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));
//  }
  // End hack to get render targets working without render passes

  assert(glGetError() == GL_NO_ERROR);

  return BINOCLE_RESOURCESTATE_VALID;
}

void binocle_backend_gl_destroy_image(binocle_gl_backend_t *gl, binocle_image_t* img) {
  assert(img);
  assert(glGetError() == GL_NO_ERROR);
  for (int slot = 0; slot < img->cmn.num_slots; slot++) {
    if (img->gl.tex[slot]) {
      binocle_backend_gl_cache_invalidate_texture(gl, img->gl.tex[slot]);
      if (!img->gl.ext_textures) {
        glDeleteTextures(1, &img->gl.tex[slot]);
      }
    }
  }
  if (img->gl.depth_render_buffer) {
    glDeleteRenderbuffers(1, &img->gl.depth_render_buffer);
  }
  if (img->gl.msaa_render_buffer) {
    glDeleteRenderbuffers(1, &img->gl.msaa_render_buffer);
  }
  assert(glGetError() == GL_NO_ERROR);
}

GLuint binocle_backend_gl_compile_shader(binocle_shader_stage stage,
                                         const char *src) {
  assert(src);
  assert(glGetError() == GL_NO_ERROR);
  GLuint gl_shd = glCreateShader(binocle_backend_gl_shader_stage(stage));
  glShaderSource(gl_shd, 1, &src, 0);
  glCompileShader(gl_shd);
  GLint compile_status = 0;
  glGetShaderiv(gl_shd, GL_COMPILE_STATUS, &compile_status);
  if (!compile_status) {
    /* compilation failed, log error and delete shader */
    GLint log_len = 0;
    glGetShaderiv(gl_shd, GL_INFO_LOG_LENGTH, &log_len);
    if (log_len > 0) {
      GLchar *log_buf = (GLchar *)malloc(log_len);
      glGetShaderInfoLog(gl_shd, log_len, &log_len, log_buf);
      binocle_log_error(log_buf);
      free(log_buf);
    }
    glDeleteShader(gl_shd);
    gl_shd = 0;
  }
  assert(glGetError() == GL_NO_ERROR);
  return gl_shd;
}

binocle_resource_state
binocle_backend_gl_create_shader(binocle_gl_backend_t *gl, binocle_shader_t *sha,
                                 const binocle_shader_desc *desc) {
  assert(sha && desc);
  assert(!sha->gl.prog);
  assert(glGetError() == GL_NO_ERROR);

  binocle_backend_shader_common_init(&sha->cmn, desc);

  /* copy vertex attribute names over, these are required for GLES2, and optional for GLES3 and GL3.x */
  for (int i = 0; i < BINOCLE_MAX_VERTEX_ATTRIBUTES; i++) {
    binocle_backend_strcpy(&sha->gl.attrs[i].name, desc->attrs[i].name);
  }

  GLuint gl_vs = binocle_backend_gl_compile_shader(BINOCLE_SHADERSTAGE_VS, desc->vs.source);
  GLuint gl_fs = binocle_backend_gl_compile_shader(BINOCLE_SHADERSTAGE_FS, desc->fs.source);
  if (!(gl_vs && gl_fs)) {
    return BINOCLE_RESOURCESTATE_FAILED;
  }
  GLuint gl_prog = glCreateProgram();
  glAttachShader(gl_prog, gl_vs);
  glAttachShader(gl_prog, gl_fs);
  glLinkProgram(gl_prog);
  glDeleteShader(gl_vs);
  glDeleteShader(gl_fs);
  assert(glGetError() == GL_NO_ERROR);

  GLint link_status;
  glGetProgramiv(gl_prog, GL_LINK_STATUS, &link_status);
  if (!link_status) {
    GLint log_len = 0;
    glGetProgramiv(gl_prog, GL_INFO_LOG_LENGTH, &log_len);
    if (log_len > 0) {
      GLchar* log_buf = (GLchar*) malloc(log_len);
      glGetProgramInfoLog(gl_prog, log_len, &log_len, log_buf);
      binocle_log_error(log_buf);
      free(log_buf);
    }
    glDeleteProgram(gl_prog);
    return BINOCLE_RESOURCESTATE_FAILED;
  }
  sha->gl.prog = gl_prog;

  /* resolve uniforms */
  assert(glGetError() == GL_NO_ERROR);
  for (int stage_index = 0; stage_index < BINOCLE_NUM_SHADER_STAGES; stage_index++) {
    const binocle_shader_stage_desc* stage_desc = (stage_index == BINOCLE_SHADERSTAGE_VS)? &desc->vs : &desc->fs;
    binocle_gl_shader_stage_t* gl_stage = &sha->gl.stage[stage_index];
    for (int ub_index = 0; ub_index < sha->cmn.stage[stage_index].num_uniform_blocks; ub_index++) {
      const binocle_shader_uniform_block_desc* ub_desc = &stage_desc->uniform_blocks[ub_index];
      assert(ub_desc->size > 0);
      binocle_gl_uniform_block_t* ub = &gl_stage->uniform_blocks[ub_index];
      assert(ub->num_uniforms == 0);
      int cur_uniform_offset = 0;
      for (int u_index = 0; u_index < BINOCLE_MAX_UB_MEMBERS; u_index++) {
        const binocle_shader_uniform_desc* u_desc = &ub_desc->uniforms[u_index];
        if (u_desc->type == BINOCLE_UNIFORMTYPE_INVALID) {
          break;
        }
        binocle_gl_uniform_t* u = &ub->uniforms[u_index];
        u->type = u_desc->type;
        u->count = (uint8_t) u_desc->array_count;
        u->offset = (uint16_t) cur_uniform_offset;
        cur_uniform_offset += binocle_backend_uniform_size(u->type, u->count);
        if (u_desc->name) {
          u->gl_loc = glGetUniformLocation(gl_prog, u_desc->name);
        }
        else {
          u->gl_loc = u_index;
        }
        ub->num_uniforms++;
      }
      assert(ub_desc->size == cur_uniform_offset);
    }
  }

  /* resolve image locations */
  assert(glGetError() == GL_NO_ERROR);
  GLuint cur_prog = 0;
  glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&cur_prog);
  glUseProgram(gl_prog);
  int gl_tex_slot = 0;
  for (int stage_index = 0; stage_index < BINOCLE_NUM_SHADER_STAGES; stage_index++) {
    const binocle_shader_stage_desc* stage_desc = (stage_index == BINOCLE_SHADERSTAGE_VS)? &desc->vs : &desc->fs;
    binocle_gl_shader_stage_t* gl_stage = &sha->gl.stage[stage_index];
    for (int img_index = 0; img_index < sha->cmn.stage[stage_index].num_images; img_index++) {
      const binocle_shader_image_desc* img_desc = &stage_desc->images[img_index];
      assert(img_desc->type != BINOCLE_IMAGETYPE_DEFAULT);
      binocle_gl_shader_image_t* gl_img = &gl_stage->images[img_index];
      GLint gl_loc = img_index;
      if (img_desc->name) {
        gl_loc = glGetUniformLocation(gl_prog, img_desc->name);
      }
      if (gl_loc != -1) {
        gl_img->gl_tex_slot = gl_tex_slot++;
        glUniform1i(gl_loc, gl_img->gl_tex_slot);
      }
      else {
        gl_img->gl_tex_slot = -1;
      }
    }
  }
  /* it's legal to call glUseProgram with 0 */
  glUseProgram(cur_prog);
  assert(glGetError() == GL_NO_ERROR);
  return BINOCLE_RESOURCESTATE_VALID;
}

void binocle_backend_gl_destroy_shader(binocle_gl_backend_t *gl, binocle_shader_t* sha) {
  assert(sha);
  assert(glGetError() == GL_NO_ERROR);
  if (sha->gl.prog) {
    binocle_backend_gl_cache_invalidate_program(gl, sha->gl.prog);
    glDeleteProgram(sha->gl.prog);
  }
  assert(glGetError() == GL_NO_ERROR);
}

binocle_resource_state binocle_backend_gl_create_pipeline(
  binocle_gl_backend_t *gl, binocle_pipeline_t *pip, binocle_shader_t *shd,
  const binocle_pipeline_desc *desc) {
  assert(pip && shd && desc);
  assert(!pip->shader && pip->cmn.shader_id.id == BINOCLE_INVALID_ID);
  assert(desc->shader.id == shd->slot.id);
  assert(shd->gl.prog);
  pip->shader = shd;
  binocle_backend_pipeline_common_init(&pip->cmn, desc);
  pip->gl.primitive_type = desc->primitive_type;
  pip->gl.depth = desc->depth;
  pip->gl.stencil = desc->stencil;
  // FIXME: blend color and write mask per draw-buffer-attachment (requires GL4)
  pip->gl.blend = desc->colors[0].blend;
  for (int i = 0; i < BINOCLE_MAX_COLOR_ATTACHMENTS; i++) {
    pip->gl.color_write_mask[i] = desc->colors[i].write_mask;
  }
  pip->gl.cull_mode = desc->cull_mode;
  pip->gl.face_winding = desc->face_winding;
  pip->gl.sample_count = desc->sample_count;
  pip->gl.alpha_to_coverage_enabled = desc->alpha_to_coverage_enabled;

  /* resolve vertex attributes */
  for (int attr_index = 0; attr_index < BINOCLE_MAX_VERTEX_ATTRIBUTES;
       attr_index++) {
    pip->gl.attrs[attr_index].vb_index = -1;
  }
  for (int attr_index = 0; attr_index < gl->limits.max_vertex_attrs;
       attr_index++) {
    const binocle_vertex_attr_desc *a_desc = &desc->layout.attrs[attr_index];
    if (a_desc->format == BINOCLE_VERTEXFORMAT_INVALID) {
      break;
    }
    assert(a_desc->buffer_index < BINOCLE_MAX_SHADERSTAGE_BUFFERS);
    const binocle_buffer_layout_desc *l_desc =
      &desc->layout.buffers[a_desc->buffer_index];
    const binocle_vertex_step step_func = l_desc->step_func;
    const int step_rate = l_desc->step_rate;
    GLint attr_loc = attr_index;
    if (!binocle_backend_strempty(&shd->gl.attrs[attr_index].name)) {
      attr_loc = glGetAttribLocation(
        pip->shader->gl.prog,
        binocle_backend_strptr(&shd->gl.attrs[attr_index].name));
    }
    assert(attr_loc < (GLint)gl->limits.max_vertex_attrs);
    if (attr_loc != -1) {
      binocle_gl_attr_t *gl_attr = &pip->gl.attrs[attr_loc];
      assert(gl_attr->vb_index == -1);
      gl_attr->vb_index = (int8_t)a_desc->buffer_index;
      if (step_func == BINOCLE_VERTEXSTEP_PER_VERTEX) {
        gl_attr->divisor = 0;
      } else {
        gl_attr->divisor = (int8_t)step_rate;
      }
      assert(l_desc->stride > 0);
      gl_attr->stride = (uint8_t)l_desc->stride;
      gl_attr->offset = a_desc->offset;
      gl_attr->size =
        (uint8_t)binocle_backend_gl_vertexformat_size(a_desc->format);
      gl_attr->type = binocle_backend_gl_vertexformat_type(a_desc->format);
      gl_attr->normalized =
        binocle_backend_gl_vertexformat_normalized(a_desc->format);
      pip->cmn.vertex_layout_valid[a_desc->buffer_index] = true;
    } else {
      binocle_log_warning("Vertex attribute not found in shader: ");
      binocle_log_warning(
        binocle_backend_strptr(&shd->gl.attrs[attr_index].name));
    }
  }
  return BINOCLE_RESOURCESTATE_VALID;
}

void binocle_backend_gl_destroy_pipeline(binocle_pipeline_t *pip) {
  assert(pip);
  (void)(pip);
  /* empty */
}

/*
    binocle_backend_gl_create_pass

    att_imgs must point to a _sg_image* att_imgs[BINOCLE_MAX_COLOR_ATTACHMENTS+1] array,
    first entries are the color attachment images (or nullptr), last entry
    is the depth-stencil image (or nullptr).
*/
binocle_resource_state
binocle_backend_gl_create_pass(binocle_gl_backend_t *gl, binocle_pass_t *pass,
                               binocle_image_t **att_images,
                               const binocle_pass_desc *desc) {
  assert(pass && att_images && desc);
  assert(att_images && att_images[0]);
  assert(glGetError() == GL_NO_ERROR);

  binocle_pass_common_init(&pass->cmn, desc);

  /* copy image pointers */
  const binocle_pass_attachment_desc *att_desc;
  for (int i = 0; i < pass->cmn.num_color_atts; i++) {
    att_desc = &desc->color_attachments[i];
    assert(att_desc->image.id != BINOCLE_INVALID_ID);
    assert(0 == pass->gl.color_atts[i].image);
    assert(att_images[i] && (att_images[i]->slot.id == att_desc->image.id));
    assert(binocle_backend_is_valid_rendertarget_color_format(
      att_images[i]->cmn.pixel_format));
    pass->gl.color_atts[i].image = att_images[i];
  }
  assert(0 == pass->gl.ds_att.image);
  att_desc = &desc->depth_stencil_attachment;
  if (att_desc->image.id != BINOCLE_INVALID_ID) {
    const int ds_img_index = BINOCLE_MAX_COLOR_ATTACHMENTS;
    assert(att_images[ds_img_index] &&
           (att_images[ds_img_index]->slot.id == att_desc->image.id));
    assert(binocle_backend_is_valid_rendertarget_depth_format(
      att_images[ds_img_index]->cmn.pixel_format));
    pass->gl.ds_att.image = att_images[ds_img_index];
  }

  /* store current framebuffer binding (restored at end of function) */
  GLuint gl_orig_fb;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *)&gl_orig_fb);

  /* create a framebuffer object */
  glGenFramebuffers(1, &pass->gl.fb);
  glBindFramebuffer(GL_FRAMEBUFFER, pass->gl.fb);

  /* attach msaa render buffer or textures */
  const bool is_msaa = (0 != att_images[0]->gl.msaa_render_buffer);
  if (is_msaa) {
    for (int i = 0; i < BINOCLE_MAX_COLOR_ATTACHMENTS; i++) {
      const binocle_image_t *att_img = pass->gl.color_atts[i].image;
      if (att_img) {
        const GLuint gl_render_buffer = att_img->gl.msaa_render_buffer;
        assert(gl_render_buffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                  (GLenum)(GL_COLOR_ATTACHMENT0 + i),
                                  GL_RENDERBUFFER, gl_render_buffer);
      }
    }
  } else {
    for (int i = 0; i < BINOCLE_MAX_COLOR_ATTACHMENTS; i++) {
      const binocle_image_t *att_img = pass->gl.color_atts[i].image;
      const int mip_level = pass->cmn.color_atts[i].mip_level;
      const int slice = pass->cmn.color_atts[i].slice;
      if (att_img) {
        const GLuint gl_tex = att_img->gl.tex[0];
        assert(gl_tex);
        const GLenum gl_att = (GLenum)(GL_COLOR_ATTACHMENT0 + i);
        switch (att_img->cmn.type) {
        case BINOCLE_IMAGETYPE_2D:
          glFramebufferTexture2D(GL_FRAMEBUFFER, gl_att, GL_TEXTURE_2D, gl_tex,
                                 mip_level);
          break;
        case BINOCLE_IMAGETYPE_CUBE:
          glFramebufferTexture2D(GL_FRAMEBUFFER, gl_att,
                                 binocle_backend_gl_cubeface_target(slice),
                                 gl_tex, mip_level);
          break;
        default:
/* 3D- or array-texture */
#if !defined(SOKOL_GLES2)
          if (!gl->gles2) {
            glFramebufferTextureLayer(GL_FRAMEBUFFER, gl_att, gl_tex, mip_level,
                                      slice);
          }
#endif
          break;
        }
      }
    }
  }

  /* attach depth-stencil buffer to framebuffer */
  if (pass->gl.ds_att.image) {
    const GLuint gl_render_buffer =
      pass->gl.ds_att.image->gl.depth_render_buffer;
    assert(gl_render_buffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, gl_render_buffer);
    if (binocle_backend_is_depth_stencil_format(
          pass->gl.ds_att.image->cmn.pixel_format)) {
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                                GL_RENDERBUFFER, gl_render_buffer);
    }
  }

  /* check if framebuffer is complete */
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    binocle_log_error("Framebuffer completeness check failed!\n");
    return BINOCLE_RESOURCESTATE_FAILED;
  }

/* setup color attachments for the framebuffer */
#if !defined(SOKOL_GLES2)
  if (!gl->gles2) {
    GLenum att[BINOCLE_MAX_COLOR_ATTACHMENTS] = {
      GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
      GL_COLOR_ATTACHMENT3};
    glDrawBuffers(pass->cmn.num_color_atts, att);
  }
#endif

  /* create MSAA resolve framebuffers if necessary */
  if (is_msaa) {
    for (int i = 0; i < BINOCLE_MAX_COLOR_ATTACHMENTS; i++) {
      binocle_gl_attachment_t *gl_att = &pass->gl.color_atts[i];
      binocle_pass_attachment_t *cmn_att = &pass->cmn.color_atts[i];
      if (gl_att->image) {
        assert(0 == gl_att->gl_msaa_resolve_buffer);
        glGenFramebuffers(1, &gl_att->gl_msaa_resolve_buffer);
        glBindFramebuffer(GL_FRAMEBUFFER, gl_att->gl_msaa_resolve_buffer);
        const GLuint gl_tex = gl_att->image->gl.tex[0];
        assert(gl_tex);
        switch (gl_att->image->cmn.type) {
        case BINOCLE_IMAGETYPE_2D:
          glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                 GL_TEXTURE_2D, gl_tex, cmn_att->mip_level);
          break;
        case BINOCLE_IMAGETYPE_CUBE:
          glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            binocle_backend_gl_cubeface_target(cmn_att->slice), gl_tex,
            cmn_att->mip_level);
          break;
        default:
#if !defined(SOKOL_GLES2)
          if (!gl->gles2) {
            glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                      gl_tex, cmn_att->mip_level,
                                      cmn_att->slice);
          }
#endif
          break;
        }
        /* check if framebuffer is complete */
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
            GL_FRAMEBUFFER_COMPLETE) {
          binocle_log_error(
            "Framebuffer completeness check failed (msaa resolve buffer)!\n");
          return BINOCLE_RESOURCESTATE_FAILED;
        }
/* setup color attachments for the framebuffer */
#if !defined(SOKOL_GLES2)
        if (!gl->gles2) {
          const GLenum gl_draw_bufs = GL_COLOR_ATTACHMENT0;
          glDrawBuffers(1, &gl_draw_bufs);
        }
#endif
      }
    }
  }

  /* restore original framebuffer binding */
  glBindFramebuffer(GL_FRAMEBUFFER, gl_orig_fb);
  assert(glGetError() == GL_NO_ERROR);
  return BINOCLE_RESOURCESTATE_VALID;
}

void binocle_backend_gl_destroy_pass(binocle_pass_t* pass) {
  assert(pass);
  assert(glGetError() == GL_NO_ERROR);
  if (0 != pass->gl.fb) {
    glDeleteFramebuffers(1, &pass->gl.fb);
  }
  for (int i = 0; i < BINOCLE_MAX_COLOR_ATTACHMENTS; i++) {
    if (pass->gl.color_atts[i].gl_msaa_resolve_buffer) {
      glDeleteFramebuffers(1, &pass->gl.color_atts[i].gl_msaa_resolve_buffer);
    }
  }
  if (pass->gl.ds_att.gl_msaa_resolve_buffer) {
    glDeleteFramebuffers(1, &pass->gl.ds_att.gl_msaa_resolve_buffer);
  }
  assert(glGetError() == GL_NO_ERROR);
}

void binocle_backend_gl_begin_pass(binocle_gl_backend_t *gl, binocle_pass_t* pass, const binocle_pass_action* action, int w, int h) {
  /* FIXME: what if a texture used as render target is still bound, should we
     unbind all currently bound textures in begin pass? */
  assert(action);
  assert(!gl->in_pass);
  assert(glGetError() == GL_NO_ERROR);
  gl->in_pass = true;
  gl->cur_pass = pass; /* can be 0 */
  if (pass) {
    gl->cur_pass_id.id = pass->slot.id;
  }
  else {
    gl->cur_pass_id.id = BINOCLE_INVALID_ID;
  }
  gl->cur_pass_width = w;
  gl->cur_pass_height = h;

  /* number of color attachments */
  const int num_color_atts = pass ? pass->cmn.num_color_atts : 1;

  /* bind the render pass framebuffer */
  if (pass) {
    /* offscreen pass */
    assert(pass->gl.fb);
    glBindFramebuffer(GL_FRAMEBUFFER, pass->gl.fb);
  }
  else {
    /* default pass */
    assert(gl->cur_context);
    glBindFramebuffer(GL_FRAMEBUFFER, gl->cur_context->default_framebuffer);
  }
  glViewport(0, 0, w, h);
  glScissor(0, 0, w, h);

  /* clear color and depth-stencil attachments if needed */
  bool clear_color = false;
  for (int i = 0; i < num_color_atts; i++) {
    if (BINOCLE_ACTION_CLEAR == action->colors[i].action) {
      clear_color = true;
      break;
    }
  }
  const bool clear_depth = (action->depth.action == BINOCLE_ACTION_CLEAR);
  const bool clear_stencil = (action->stencil.action == BINOCLE_ACTION_CLEAR);

  bool need_pip_cache_flush = false;
  if (clear_color) {
    bool need_color_mask_flush = false;
    // NOTE: not a bug to iterate over all possible color attachments
    for (int i = 0; i < BINOCLE_MAX_COLOR_ATTACHMENTS; i++) {
      if (BINOCLE_COLORMASK_RGBA != gl->cache.color_write_mask[i]) {
        need_pip_cache_flush = true;
        need_color_mask_flush = true;
        gl->cache.color_write_mask[i] = BINOCLE_COLORMASK_RGBA;
      }
    }
    if (need_color_mask_flush) {
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    }
  }
  if (clear_depth) {
    if (!gl->cache.depth.write_enabled) {
      need_pip_cache_flush = true;
      gl->cache.depth.write_enabled = true;
      glDepthMask(GL_TRUE);
    }
    if (gl->cache.depth.compare != BINOCLE_COMPAREFUNC_ALWAYS) {
      need_pip_cache_flush = true;
      gl->cache.depth.compare = BINOCLE_COMPAREFUNC_ALWAYS;
      glDepthFunc(GL_ALWAYS);
    }
  }
  if (clear_stencil) {
    if (gl->cache.stencil.write_mask != 0xFF) {
      need_pip_cache_flush = true;
      gl->cache.stencil.write_mask = 0xFF;
      glStencilMask(0xFF);
    }
  }
  if (need_pip_cache_flush) {
    /* we messed with the state cache directly, need to clear cached
       pipeline to force re-evaluation in next sg_apply_pipeline() */
    gl->cache.cur_pipeline = 0;
    gl->cache.cur_pipeline_id.id = BINOCLE_INVALID_ID;
  }
  bool use_mrt_clear = (0 != pass);
//#if defined(SOKOL_GLES2)
  use_mrt_clear = false;
//#else
//  if (gl->gles2) {
//    use_mrt_clear = false;
//  }
//#endif
  if (!use_mrt_clear) {
    GLbitfield clear_mask = 0;
    if (clear_color) {
      clear_mask |= GL_COLOR_BUFFER_BIT;
      const binocle_color c = action->colors[0].value;
      glClearColor(c.r, c.g, c.b, c.a);
    }
    if (clear_depth) {
      clear_mask |= GL_DEPTH_BUFFER_BIT;
//#ifdef SOKOL_GLCORE33
      glClearDepth(action->depth.value);
//#else
//      glClearDepthf(action->depth.value);
//#endif
    }
    if (clear_stencil) {
      clear_mask |= GL_STENCIL_BUFFER_BIT;
      glClearStencil(action->stencil.value);
    }
    if (0 != clear_mask) {
      glClear(clear_mask);
    }
  }
//#if !defined SOKOL_GLES2
//  else {
//    assert(pass);
//    for (int i = 0; i < num_color_atts; i++) {
//      if (action->colors[i].action == BINOCLE_ACTION_CLEAR) {
//        glClearBufferfv(GL_COLOR, i, &action->colors[i].value.r);
//      }
//    }
//    if (pass->gl.ds_att.image) {
//      if (clear_depth && clear_stencil) {
//        glClearBufferfi(GL_DEPTH_STENCIL, 0, action->depth.value, action->stencil.value);
//      }
//      else if (clear_depth) {
//        glClearBufferfv(GL_DEPTH, 0, &action->depth.value);
//      }
//      else if (clear_stencil) {
//        GLint val = (GLint) action->stencil.value;
//        glClearBufferiv(GL_STENCIL, 0, &val);
//      }
//    }
//  }
//#endif
  assert(glGetError() == GL_NO_ERROR);
}

binocle_image_t* binocle_backend_gl_pass_color_image(const binocle_pass_t* pass, int index) {
  assert(pass && (index >= 0) && (index < BINOCLE_MAX_COLOR_ATTACHMENTS));
  /* NOTE: may return null */
  return pass->gl.color_atts[index].image;
}

void binocle_backend_gl_end_pass(binocle_gl_backend_t *gl) {
  assert(gl->in_pass);
  assert(glGetError() == GL_NO_ERROR);

  /* if this was an offscreen pass, and MSAA rendering was used, need
     to resolve into the pass images */
#if !defined(SOKOL_GLES2)
  if (!gl->gles2 && gl->cur_pass) {
    /* check if the pass object is still valid */
    const binocle_pass_t* pass = gl->cur_pass;
    assert(pass->slot.id == gl->cur_pass_id.id);
    bool is_msaa = (0 != gl->cur_pass->gl.color_atts[0].gl_msaa_resolve_buffer);
    if (is_msaa) {
      assert(pass->gl.fb);
      glBindFramebuffer(GL_READ_FRAMEBUFFER, pass->gl.fb);
      assert(pass->gl.color_atts[0].image);
      const int w = pass->gl.color_atts[0].image->cmn.width;
      const int h = pass->gl.color_atts[0].image->cmn.height;
      for (int att_index = 0; att_index < BINOCLE_MAX_COLOR_ATTACHMENTS; att_index++) {
        const binocle_gl_attachment_t* gl_att = &pass->gl.color_atts[att_index];
        if (gl_att->image) {
          assert(gl_att->gl_msaa_resolve_buffer);
          glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl_att->gl_msaa_resolve_buffer);
          glReadBuffer((GLenum)(GL_COLOR_ATTACHMENT0 + att_index));
          glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }
        else {
          break;
        }
      }
    }
  }
#endif
  gl->cur_pass = 0;
  gl->cur_pass_id.id = BINOCLE_INVALID_ID;
  gl->cur_pass_width = 0;
  gl->cur_pass_height = 0;

  assert(gl->cur_context);
  glBindFramebuffer(GL_FRAMEBUFFER, gl->cur_context->default_framebuffer);
  gl->in_pass = false;
  assert(glGetError() == GL_NO_ERROR);
}

void binocle_backend_gl_apply_pipeline(binocle_gl_backend_t *gl, binocle_pipeline_t* pip) {
  assert(pip);
  assert(pip->shader && (pip->cmn.shader_id.id == pip->shader->slot.id));
  assert(glGetError() == GL_NO_ERROR);
  if ((gl->cache.cur_pipeline != pip) || (gl->cache.cur_pipeline_id.id != pip->slot.id)) {
    gl->cache.cur_pipeline = pip;
    gl->cache.cur_pipeline_id.id = pip->slot.id;
    gl->cache.cur_primitive_type = binocle_backend_gl_primitive_type(pip->gl.primitive_type);
    gl->cache.cur_index_type = binocle_backend_gl_index_type(pip->cmn.index_type);

    /* update depth state */
    {
      const binocle_depth_state* state_ds = &pip->gl.depth;
      binocle_depth_state* cache_ds = &gl->cache.depth;
      if (state_ds->compare != cache_ds->compare) {
        cache_ds->compare = state_ds->compare;
        glDepthFunc(binocle_backend_gl_compare_func(state_ds->compare));
      }
      if (state_ds->write_enabled != cache_ds->write_enabled) {
        cache_ds->write_enabled = state_ds->write_enabled;
        glDepthMask(state_ds->write_enabled);
      }
      if (!BINOCLE_FEQUAL(state_ds->bias, cache_ds->bias, 0.000001f) ||
          !BINOCLE_FEQUAL(state_ds->bias_slope_scale, cache_ds->bias_slope_scale, 0.000001f))
      {
        /* according to ANGLE's D3D11 backend:
            D3D11 SlopeScaledDepthBias ==> GL polygonOffsetFactor
            D3D11 DepthBias ==> GL polygonOffsetUnits
            DepthBiasClamp has no meaning on GL
        */
        cache_ds->bias = state_ds->bias;
        cache_ds->bias_slope_scale = state_ds->bias_slope_scale;
        glPolygonOffset(state_ds->bias_slope_scale, state_ds->bias);
        bool po_enabled = true;
        if (BINOCLE_FEQUAL(state_ds->bias, 0.0f, 0.000001f) &&
          BINOCLE_FEQUAL(state_ds->bias_slope_scale, 0.0f, 0.000001f))
        {
          po_enabled = false;
        }
        if (po_enabled != gl->cache.polygon_offset_enabled) {
          gl->cache.polygon_offset_enabled = po_enabled;
          if (po_enabled) {
            glEnable(GL_POLYGON_OFFSET_FILL);
          }
          else {
            glDisable(GL_POLYGON_OFFSET_FILL);
          }
        }
      }
    }

    /* update stencil state */
    {
      const binocle_stencil_state* state_ss = &pip->gl.stencil;
      binocle_stencil_state* cache_ss = &gl->cache.stencil;
      if (state_ss->enabled != cache_ss->enabled) {
        cache_ss->enabled = state_ss->enabled;
        if (state_ss->enabled) {
          glEnable(GL_STENCIL_TEST);
        }
        else {
          glDisable(GL_STENCIL_TEST);
        }
      }
      if (state_ss->write_mask != cache_ss->write_mask) {
        cache_ss->write_mask = state_ss->write_mask;
        glStencilMask(state_ss->write_mask);
      }
      for (int i = 0; i < 2; i++) {
        const binocle_stencil_face_state* state_sfs = (i==0)? &state_ss->front : &state_ss->back;
        binocle_stencil_face_state* cache_sfs = (i==0)? &cache_ss->front : &cache_ss->back;
        GLenum gl_face = (i==0)? GL_FRONT : GL_BACK;
        if ((state_sfs->compare != cache_sfs->compare) ||
            (state_ss->read_mask != cache_ss->read_mask) ||
            (state_ss->ref != cache_ss->ref))
        {
          cache_sfs->compare = state_sfs->compare;
          glStencilFuncSeparate(gl_face,
                                binocle_backend_gl_compare_func(state_sfs->compare),
                                state_ss->ref,
                                state_ss->read_mask);
        }
        if ((state_sfs->fail_op != cache_sfs->fail_op) ||
            (state_sfs->depth_fail_op != cache_sfs->depth_fail_op) ||
            (state_sfs->pass_op != cache_sfs->pass_op))
        {
          cache_sfs->fail_op = state_sfs->fail_op;
          cache_sfs->depth_fail_op = state_sfs->depth_fail_op;
          cache_sfs->pass_op = state_sfs->pass_op;
          glStencilOpSeparate(gl_face,
                              binocle_backend_gl_stencil_op(state_sfs->fail_op),
                              binocle_backend_gl_stencil_op(state_sfs->depth_fail_op),
                              binocle_backend_gl_stencil_op(state_sfs->pass_op));
        }
      }
      cache_ss->read_mask = state_ss->read_mask;
      cache_ss->ref = state_ss->ref;
    }

    /* update blend state
        FIXME: separate blend state per color attachment not support, needs GL4
    */
    {
      const binocle_blend_state* state_bs = &pip->gl.blend;
      binocle_blend_state* cache_bs = &gl->cache.blend;
      if (state_bs->enabled != cache_bs->enabled) {
        cache_bs->enabled = state_bs->enabled;
        if (state_bs->enabled) {
          glEnable(GL_BLEND);
        }
        else {
          glDisable(GL_BLEND);
        }
      }
      if ((state_bs->src_factor_rgb != cache_bs->src_factor_rgb) ||
          (state_bs->dst_factor_rgb != cache_bs->dst_factor_rgb) ||
          (state_bs->src_factor_alpha != cache_bs->src_factor_alpha) ||
          (state_bs->dst_factor_alpha != cache_bs->dst_factor_alpha))
      {
        cache_bs->src_factor_rgb = state_bs->src_factor_rgb;
        cache_bs->dst_factor_rgb = state_bs->dst_factor_rgb;
        cache_bs->src_factor_alpha = state_bs->src_factor_alpha;
        cache_bs->dst_factor_alpha = state_bs->dst_factor_alpha;
        glBlendFuncSeparate(binocle_backend_gl_blend_factor(state_bs->src_factor_rgb),
                            binocle_backend_gl_blend_factor(state_bs->dst_factor_rgb),
                            binocle_backend_gl_blend_factor(state_bs->src_factor_alpha),
                            binocle_backend_gl_blend_factor(state_bs->dst_factor_alpha));
      }
      if ((state_bs->op_rgb != cache_bs->op_rgb) || (state_bs->op_alpha != cache_bs->op_alpha)) {
        cache_bs->op_rgb = state_bs->op_rgb;
        cache_bs->op_alpha = state_bs->op_alpha;
        glBlendEquationSeparate(binocle_backend_gl_blend_op(state_bs->op_rgb), binocle_backend_gl_blend_op(state_bs->op_alpha));
      }
    }

    /* standalone state */
    for (GLuint i = 0; i < (GLuint)pip->cmn.color_attachment_count; i++) {
      if (pip->gl.color_write_mask[i] != gl->cache.color_write_mask[i]) {
        const binocle_color_mask cm = pip->gl.color_write_mask[i];
        gl->cache.color_write_mask[i] = cm;
#ifdef SOKOL_GLCORE33
        glColorMaski(i,
                                (cm & BINOCLE_COLORMASK_R) != 0,
                                (cm & BINOCLE_COLORMASK_G) != 0,
                                (cm & BINOCLE_COLORMASK_B) != 0,
                                (cm & BINOCLE_COLORMASK_A) != 0);
#else
        if (0 == i) {
          glColorMask((cm & BINOCLE_COLORMASK_R) != 0,
                      (cm & BINOCLE_COLORMASK_G) != 0,
                      (cm & BINOCLE_COLORMASK_B) != 0,
                      (cm & BINOCLE_COLORMASK_A) != 0);
        }
#endif
      }
    }

    if (!BINOCLE_FEQUAL(pip->cmn.blend_color.r, gl->cache.blend_color.r, 0.0001f) ||
        !BINOCLE_FEQUAL(pip->cmn.blend_color.g, gl->cache.blend_color.g, 0.0001f) ||
        !BINOCLE_FEQUAL(pip->cmn.blend_color.b, gl->cache.blend_color.b, 0.0001f) ||
        !BINOCLE_FEQUAL(pip->cmn.blend_color.a, gl->cache.blend_color.a, 0.0001f))
    {
      binocle_color c = pip->cmn.blend_color;
      gl->cache.blend_color = c;
      glBlendColor(c.r, c.g, c.b, c.a);
    }
    if (pip->gl.cull_mode != gl->cache.cull_mode) {
      gl->cache.cull_mode = pip->gl.cull_mode;
      if (BINOCLE_CULLMODE_NONE == pip->gl.cull_mode) {
        glDisable(GL_CULL_FACE);
      }
      else {
        glEnable(GL_CULL_FACE);
        GLenum gl_mode = (BINOCLE_CULLMODE_FRONT == pip->gl.cull_mode) ? GL_FRONT : GL_BACK;
        glCullFace(gl_mode);
      }
    }
    if (pip->gl.face_winding != gl->cache.face_winding) {
      gl->cache.face_winding = pip->gl.face_winding;
      GLenum gl_winding = (BINOCLE_FACEWINDING_CW == pip->gl.face_winding) ? GL_CW : GL_CCW;
      glFrontFace(gl_winding);
    }
    if (pip->gl.alpha_to_coverage_enabled != gl->cache.alpha_to_coverage_enabled) {
      gl->cache.alpha_to_coverage_enabled = pip->gl.alpha_to_coverage_enabled;
      if (pip->gl.alpha_to_coverage_enabled) {
        glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
      }
      else {
        glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
      }
    }
#ifdef SOKOL_GLCORE33
    if (pip->gl.sample_count != gl->cache.sample_count) {
            gl->cache.sample_count = pip->gl.sample_count;
            if (pip->gl.sample_count > 1) {
                glEnable(GL_MULTISAMPLE);
            }
            else {
                glDisable(GL_MULTISAMPLE);
            }
        }
#endif

    /* bind shader program */
    if (pip->shader->gl.prog != gl->cache.prog) {
      gl->cache.prog = pip->shader->gl.prog;
      glUseProgram(pip->shader->gl.prog);
    }
  }
  assert(glGetError() == GL_NO_ERROR);
}

void binocle_backend_gl_apply_bindings(
  binocle_gl_backend_t *gl,
  binocle_pipeline_t* pip,
  binocle_buffer_t** vbs, const int* vb_offsets, int num_vbs,
  binocle_buffer_t* ib, int ib_offset,
  binocle_image_t** vs_imgs, int num_vs_imgs,
  binocle_image_t** fs_imgs, int num_fs_imgs)
{
  assert(pip);
//  _SOKOL_UNUSED(num_fs_imgs);
//  _SOKOL_UNUSED(num_vs_imgs);
//  _SOKOL_UNUSED(num_vbs);
  assert(glGetError() == GL_NO_ERROR);

  /* bind textures */
  assert(glGetError() == GL_NO_ERROR);
  for (int stage_index = 0; stage_index < BINOCLE_NUM_SHADER_STAGES; stage_index++) {
    const binocle_shader_stage_t* stage = &pip->shader->cmn.stage[stage_index];
    const binocle_gl_shader_stage_t* gl_stage = &pip->shader->gl.stage[stage_index];
    binocle_image_t** imgs = (stage_index == BINOCLE_SHADERSTAGE_VS)? vs_imgs : fs_imgs;
    assert(((stage_index == BINOCLE_SHADERSTAGE_VS)? num_vs_imgs : num_fs_imgs) == stage->num_images);
    for (int img_index = 0; img_index < stage->num_images; img_index++) {
      const binocle_gl_shader_image_t* gl_shd_img = &gl_stage->images[img_index];
      if (gl_shd_img->gl_tex_slot != -1) {
        binocle_image_t* img = imgs[img_index];
        const GLuint gl_tex = img->gl.tex[img->cmn.active_slot];
        assert(img && img->gl.target);
        assert((gl_shd_img->gl_tex_slot != -1) && gl_tex);
        binocle_backend_gl_cache_bind_texture(gl, gl_shd_img->gl_tex_slot, img->gl.target, gl_tex);
      }
    }
  }
  assert(glGetError() == GL_NO_ERROR);

  /* index buffer (can be 0) */
  const GLuint gl_ib = ib ? ib->gl.buf[ib->cmn.active_slot] : 0;
  binocle_backend_gl_cache_bind_buffer(gl, GL_ELEMENT_ARRAY_BUFFER, gl_ib);
  gl->cache.cur_ib_offset = ib_offset;

  /* vertex attributes */
  for (GLuint attr_index = 0; attr_index < (GLuint)BINOCLE_MAX_VERTEX_ATTRIBUTES/*_sg.limits.max_vertex_attrs*/; attr_index++) {
    binocle_gl_attr_t* attr = &pip->gl.attrs[attr_index];
    binocle_gl_cache_attr_t* cache_attr = &gl->cache.attrs[attr_index];
    bool cache_attr_dirty = false;
    int vb_offset = 0;
    GLuint gl_vb = 0;
    if (attr->vb_index >= 0) {
      /* attribute is enabled */
      assert(attr->vb_index < num_vbs);
      binocle_buffer_t* vb = vbs[attr->vb_index];
      assert(vb);
      gl_vb = vb->gl.buf[vb->cmn.active_slot];
      vb_offset = vb_offsets[attr->vb_index] + attr->offset;
      if ((gl_vb != cache_attr->gl_vbuf) ||
          (attr->size != cache_attr->gl_attr.size) ||
          (attr->type != cache_attr->gl_attr.type) ||
          (attr->normalized != cache_attr->gl_attr.normalized) ||
          (attr->stride != cache_attr->gl_attr.stride) ||
          (vb_offset != cache_attr->gl_attr.offset) ||
          (cache_attr->gl_attr.divisor != attr->divisor))
      {
        binocle_backend_gl_cache_bind_buffer(gl, GL_ARRAY_BUFFER, gl_vb);
        glVertexAttribPointer(attr_index, attr->size, attr->type,
                              attr->normalized, attr->stride,
                              (const GLvoid*)(GLintptr)vb_offset);
#ifdef SOKOL_INSTANCING_ENABLED
        if (_sg.features.instancing) {
                        glVertexAttribDivisor(attr_index, (GLuint)attr->divisor);
                    }
#endif
        cache_attr_dirty = true;
      }
      if (cache_attr->gl_attr.vb_index == -1) {
        glEnableVertexAttribArray(attr_index);
        cache_attr_dirty = true;
      }
    }
    else {
      /* attribute is disabled */
      if (cache_attr->gl_attr.vb_index != -1) {
        glDisableVertexAttribArray(attr_index);
        cache_attr_dirty = true;
      }
    }
    if (cache_attr_dirty) {
      cache_attr->gl_attr = *attr;
      cache_attr->gl_attr.offset = vb_offset;
      cache_attr->gl_vbuf = gl_vb;
    }
  }
  assert(glGetError() == GL_NO_ERROR);
}

void binocle_backend_gl_activate_context(binocle_gl_backend_t *gl, binocle_context_t* ctx) {
  assert(gl->valid);
  /* NOTE: ctx can be 0 to unset the current context */
  gl->cur_context = ctx;
  binocle_backend_gl_reset_state_cache(gl);
}

binocle_resource_state binocle_backend_gl_create_context(binocle_gl_backend_t *gl, binocle_context_t* ctx) {
assert(ctx);
  assert(0 == ctx->default_framebuffer);
  assert(glGetError() == GL_NO_ERROR);
glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&ctx->default_framebuffer);
  assert(glGetError() == GL_NO_ERROR);
//#if !defined(SOKOL_GLES2)
//if (!gl->gles2) {
//assert(0 == ctx->vao);
//glGenVertexArrays(1, &ctx->vao);
//glBindVertexArray(ctx->vao);
//  assert(glGetError() == GL_NO_ERROR);
//}
//#endif
return BINOCLE_RESOURCESTATE_VALID;
}

void binocle_backend_gl_destroy_context(binocle_gl_backend_t *gl, binocle_context_t* ctx) {
  assert(ctx);
//#if !defined(SOKOL_GLES2)
//  if (!gl->gles2) {
//    if (ctx->vao) {
//      glDeleteVertexArrays(1, &ctx->vao);
//    }
//    assert(glGetError() == GL_NO_ERROR);
//  }
//#else
//  _SOKOL_UNUSED(ctx);
//#endif
}

void binocle_backend_gl_apply_uniforms(binocle_gl_backend_t *gl, binocle_shader_stage stage_index, int ub_index, const binocle_range* data) {
  assert(gl->cache.cur_pipeline);
  assert(gl->cache.cur_pipeline->slot.id == gl->cache.cur_pipeline_id.id);
  assert(gl->cache.cur_pipeline->shader->slot.id == gl->cache.cur_pipeline->cmn.shader_id.id);
  assert(gl->cache.cur_pipeline->shader->cmn.stage[stage_index].num_uniform_blocks > ub_index);
  assert(gl->cache.cur_pipeline->shader->cmn.stage[stage_index].uniform_blocks[ub_index].size == data->size);
  const binocle_gl_shader_stage_t* gl_stage = &gl->cache.cur_pipeline->shader->gl.stage[stage_index];
  const binocle_gl_uniform_block_t* gl_ub = &gl_stage->uniform_blocks[ub_index];
  for (int u_index = 0; u_index < gl_ub->num_uniforms; u_index++) {
    const binocle_gl_uniform_t* u = &gl_ub->uniforms[u_index];
    assert(u->type != BINOCLE_UNIFORMTYPE_INVALID);
    if (u->gl_loc == -1) {
      continue;
    }
    GLfloat* ptr = (GLfloat*) (((uint8_t*)data->ptr) + u->offset);
    switch (u->type) {
    case BINOCLE_UNIFORMTYPE_INVALID:
      break;
    case BINOCLE_UNIFORMTYPE_FLOAT:
      glUniform1fv(u->gl_loc, u->count, ptr);
      break;
    case BINOCLE_UNIFORMTYPE_FLOAT2:
      glUniform2fv(u->gl_loc, u->count, ptr);
      break;
    case BINOCLE_UNIFORMTYPE_FLOAT3:
      glUniform3fv(u->gl_loc, u->count, ptr);
      break;
    case BINOCLE_UNIFORMTYPE_FLOAT4:
      glUniform4fv(u->gl_loc, u->count, ptr);
      break;
    case BINOCLE_UNIFORMTYPE_MAT4:
      glUniformMatrix4fv(u->gl_loc, u->count, GL_FALSE, ptr);
      break;
    default:
      assert(false);
      break;
    }
  }
}

void binocle_backend_gl_draw(binocle_gl_backend_t *gl, int base_element, int num_elements, int num_instances) {
  const GLenum i_type = gl->cache.cur_index_type;
  const GLenum p_type = gl->cache.cur_primitive_type;
  if (0 != i_type) {
    /* indexed rendering */
    const int i_size = (i_type == GL_UNSIGNED_SHORT) ? 2 : 4;
    const int ib_offset = gl->cache.cur_ib_offset;
    const GLvoid* indices = (const GLvoid*)(GLintptr)(base_element*i_size+ib_offset);
    if (num_instances == 1) {
      glDrawElements(p_type, num_elements, i_type, indices);
    }
    else {
//      if (_sg.features.instancing) {
//        glDrawElementsInstanced(p_type, num_elements, i_type, indices, num_instances);
//      }
    }
  }
  else {
    /* non-indexed rendering */
    if (num_instances == 1) {
      glDrawArrays(p_type, base_element, num_elements);
    }
    else {
//      if (_sg.features.instancing) {
//        glDrawArraysInstanced(p_type, base_element, num_elements, num_instances);
//      }
    }
  }
}

void binocle_backend_gl_commit(binocle_gl_backend_t *gl) {
  assert(!gl->in_pass);
  /* "soft" clear bindings (only those that are actually bound) */
  binocle_backend_gl_cache_clear_buffer_bindings(gl, false);
  binocle_backend_gl_cache_clear_texture_bindings(gl, false);
}

binocle_resource_state binocle_backend_gl_create_buffer(binocle_gl_backend_t *gl, binocle_buffer_t* buf, const binocle_buffer_desc* desc) {
assert(buf && desc);
  assert(glGetError() == GL_NO_ERROR);
binocle_backend_buffer_common_init(&buf->cmn, desc);
buf->gl.ext_buffers = (0 != desc->gl_buffers[0]);
GLenum gl_target = binocle_backend_gl_buffer_target(buf->cmn.type);
GLenum gl_usage  = binocle_backend_gl_usage(buf->cmn.usage);
for (int slot = 0; slot < buf->cmn.num_slots; slot++) {
GLuint gl_buf = 0;
if (buf->gl.ext_buffers) {
  assert(desc->gl_buffers[slot]);
gl_buf = desc->gl_buffers[slot];
}
else {
glGenBuffers(1, &gl_buf);
  binocle_backend_gl_cache_store_buffer_binding(gl, gl_target);
  binocle_backend_gl_cache_bind_buffer(gl, gl_target, gl_buf);
glBufferData(gl_target, buf->cmn.size, 0, gl_usage);
if (buf->cmn.usage == BINOCLE_USAGE_IMMUTABLE) {
  assert(desc->data.ptr);
glBufferSubData(gl_target, 0, buf->cmn.size, desc->data.ptr);
}
  binocle_backend_gl_cache_restore_buffer_binding(gl, gl_target);
}
buf->gl.buf[slot] = gl_buf;
}
  assert(glGetError() == GL_NO_ERROR);
return BINOCLE_RESOURCESTATE_VALID;
}
