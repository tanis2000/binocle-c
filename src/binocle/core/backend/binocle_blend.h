//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_BLEND_H
#define BINOCLE_BLEND_H

/*
    binocle_blend_factor

    The source and destination factors in blending operations.
    This is used in the following members when creating a pipeline object:

    binocle_pipeline_desc
        .colors[i]
            .blend
                .src_factor_rgb
                .dst_factor_rgb
                .src_factor_alpha
                .dst_factor_alpha

    The default value is BINOCLE_BLENDFACTOR_ONE for source
    factors, and BINOCLE_BLENDFACTOR_ZERO for destination factors.
*/
typedef enum binocle_blend_factor {
  BINOCLE_BLENDFACTOR_DEFAULT,    /* value 0 reserved for default-init */
  BINOCLE_BLENDFACTOR_ZERO,
  BINOCLE_BLENDFACTOR_ONE,
  BINOCLE_BLENDFACTOR_SRC_COLOR,
  BINOCLE_BLENDFACTOR_ONE_MINUS_SRC_COLOR,
  BINOCLE_BLENDFACTOR_SRC_ALPHA,
  BINOCLE_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
  BINOCLE_BLENDFACTOR_DST_COLOR,
  BINOCLE_BLENDFACTOR_ONE_MINUS_DST_COLOR,
  BINOCLE_BLENDFACTOR_DST_ALPHA,
  BINOCLE_BLENDFACTOR_ONE_MINUS_DST_ALPHA,
  BINOCLE_BLENDFACTOR_SRC_ALPHA_SATURATED,
  BINOCLE_BLENDFACTOR_BLEND_COLOR,
  BINOCLE_BLENDFACTOR_ONE_MINUS_BLEND_COLOR,
  BINOCLE_BLENDFACTOR_BLEND_ALPHA,
  BINOCLE_BLENDFACTOR_ONE_MINUS_BLEND_ALPHA,
  BINOCLE_BLENDFACTOR_NUM,
  BINOCLE_BLENDFACTOR_FORCE_U32 = 0x7FFFFFFF
} binocle_blend_factor;

/**
 * \brief Enumeration of the blending equations
 *
 * The equations are mapped directly to their OpenGL equivalents,
 * specified by glBlendEquation() or glBlendEquationSeparate().
 */
typedef enum binocle_blend_equation {
  BINOCLE_BLEND_ADD,     ///< Pixel = Src * SrcFactor + Dst * DstFactor
  BINOCLE_BLEND_SUBTRACT ///< Pixel = Src * SrcFactor - Dst * DstFactor
} binocle_blend_equation;

/**
 * \brief Represents a blending structure with all the needed values
 */
typedef struct binocle_blend {
  binocle_blend_factor color_src_factor; ///< Source blending factor for the color channels
  binocle_blend_factor color_dst_factor; ///< Destination blending factor for the color channels
  binocle_blend_equation color_equation; ///< Blending equation for the color channels
  binocle_blend_factor alpha_src_factor; ///< Source blending factor for the alpha channel
  binocle_blend_factor alpha_dst_factor; ///< Destination blending factor for the alpha channel
  binocle_blend_equation alpha_equation; ///< Blending equation for the alpha channel
} binocle_blend;

/**
 * Initializes a new blending structure
 * @param color_source_factor
 * @param color_destination_factor
 * @param color_blend_equation
 * @param alpha_source_factor
 * @param alpha_destination_factor
 * @param alpha_blend_equation
 * @return the initialized blending structure
 */
binocle_blend binocle_blend_new(binocle_blend_factor color_source_factor, binocle_blend_factor color_destination_factor,
                                binocle_blend_equation color_blend_equation, binocle_blend_factor alpha_source_factor,
                                binocle_blend_factor alpha_destination_factor,
                                binocle_blend_equation alpha_blend_equation);

/**
 * \brief Helper to initialize a blending structure with basic values
 * @param source_factor
 * @param destination_factor
 * @param blend_equation
 * @return the initialized blending structure
 */
binocle_blend binocle_blend_new_simple(binocle_blend_factor source_factor, binocle_blend_factor destination_factor,
                                       binocle_blend_equation blend_equation);

/**
 * \brief Returns default values for blending
 * @return the default values
 */
binocle_blend binocle_blend_new_default();

#endif //BINOCLE_BLEND_H
