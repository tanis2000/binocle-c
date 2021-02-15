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

GLuint binocle_backend_gl_factor_to_gl_constant(binocle_blend_factor blend_factor) {
  switch (blend_factor) {
  case BINOCLE_BLEND_ZERO:
    return GL_ZERO;
  case BINOCLE_BLEND_ONE:
    return GL_ONE;
  case BINOCLE_BLEND_SRCCOLOR:
    return GL_SRC_COLOR;
  case BINOCLE_BLEND_ONEMINUSSRCCOLOR:
    return GL_ONE_MINUS_SRC_COLOR;
  case BINOCLE_BLEND_DSTCOLOR:
    return GL_DST_COLOR;
  case BINOCLE_BLEND_ONEMINUSDSTCOLOR:
    return GL_ONE_MINUS_DST_COLOR;
  case BINOCLE_BLEND_SRCALPHA:
    return GL_SRC_ALPHA;
  case BINOCLE_BLEND_ONEMINUSSRCALPHA:
    return GL_ONE_MINUS_SRC_ALPHA;
  case BINOCLE_BLEND_DSTALPHA:
    return GL_DST_ALPHA;
  case BINOCLE_BLEND_ONEMINUSDSTALPHA:
    return GL_ONE_MINUS_DST_ALPHA;
  }

  SDL_Log("Invalid value for Binocle::BlendMode::Factor! Fallback to Binocle::BlendMode::Zero.");
  return GL_ZERO;
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

void binocle_backend_gl_init_limits(binocle_gl_backend_t *gl) {
  assert(glGetError() == GL_NO_ERROR);
  GLint gl_int;
//  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &gl_int);
//  assert(glGetError() == GL_NO_ERROR);
//  _sg.limits.max_image_size_2d = gl_int;
//  _sg.limits.max_image_size_array = gl_int;
//  glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &gl_int);
//  assert(glGetError() == GL_NO_ERROR);
//  _sg.limits.max_image_size_cube = gl_int;
//  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &gl_int);
//  assert(glGetError() == GL_NO_ERROR);
//  if (gl_int > SG_MAX_VERTEX_ATTRIBUTES) {
//    gl_int = SG_MAX_VERTEX_ATTRIBUTES;
//  }
//  _sg.limits.max_vertex_attrs = gl_int;
//#if !defined(BINOCLE_GLES2)
//  if (!gl->gles2) {
//    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &gl_int);
//    assert(glGetError() == GL_NO_ERROR);
//    _sg.limits.max_image_size_3d = gl_int;
//    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &gl_int);
//    assert(glGetError() == GL_NO_ERROR);
//    _sg.limits.max_image_array_layers = gl_int;
//  }
//#endif
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
  // Create a new vertex buffer object
  glCheck(glGenBuffers(1, &gl->vbo));
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
    binocle_backend_gl_factor_to_gl_constant(blend_mode.color_src_factor),
    binocle_backend_gl_factor_to_gl_constant(blend_mode.color_dst_factor)));
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

void binocle_backend_gl_draw(binocle_gl_backend_t *gl, const struct binocle_vpct *vertices, size_t vertex_count, struct binocle_blend blend,
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
//      _SG_GL_CHECK_ERROR();
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
              desc->content.subimage[face_index][mip_index].ptr;
            const int data_size =
              desc->content.subimage[face_index][mip_index].size;
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
  {
    GLenum fmt =
      binocle_backend_gl_teximage_format(desc->pixel_format);

    GLuint fb[1];
    glCheck(glGenFramebuffers(1, fb));
    img->gl.frame_buffer = fb[0];

    GLuint rb[1];
    glCheck(glGenRenderbuffers(1, rb));
    img->gl.depth_render_buffer = rb[0];

    // set up framebuffer

    // bind the framebuffer
    glCheck(glBindFramebuffer(GL_FRAMEBUFFER, img->gl.frame_buffer));

    // bind the newly created texture: all future texture functions will modify
    // this texture
    glCheck(glBindTexture(GL_TEXTURE_2D, img->gl.tex[0]));
    // Give an empty image to OpenGL ( the last "0" )
    glCheck(glTexImage2D(GL_TEXTURE_2D, 0, fmt, img->cmn.width, img->cmn.height,
                         0, fmt, GL_UNSIGNED_BYTE, 0));
    // filtering
    glCheck(
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    glCheck(
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    // attach the texture to the bound framebuffer object
    glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D, img->gl.tex[0], 0));

    // set up renderbuffer (depth buffer)
    glCheck(glBindRenderbuffer(GL_RENDERBUFFER, img->gl.depth_render_buffer));
    glCheck(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16,
                                  img->cmn.width, img->cmn.height));
    glCheck(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                      GL_RENDERBUFFER,
                                      img->gl.depth_render_buffer));

    // clean up
    glCheck(glBindTexture(GL_TEXTURE_2D, 0));
    glCheck(glBindRenderbuffer(GL_RENDERBUFFER, 0));
    glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));
  }
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
