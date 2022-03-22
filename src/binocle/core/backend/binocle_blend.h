//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_BLEND_H
#define BINOCLE_BLEND_H

struct sg_blend_state;
enum sg_blend_factor;
enum sg_blend_op;

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
struct sg_blend_state binocle_blend_new(enum sg_blend_factor color_source_factor, enum sg_blend_factor color_destination_factor,
                                 enum sg_blend_op color_blend_equation, enum sg_blend_factor alpha_source_factor,
                                enum sg_blend_factor alpha_destination_factor,
                                 enum sg_blend_op alpha_blend_equation);

/**
 * \brief Helper to initialize a blending structure with basic values
 * @param source_factor
 * @param destination_factor
 * @param blend_equation
 * @return the initialized blending structure
 */
struct sg_blend_state binocle_blend_new_simple(enum sg_blend_factor source_factor, enum sg_blend_factor destination_factor,
                                        enum sg_blend_op blend_equation);

/**
 * \brief Returns default values for blending
 * @return the default values
 */
struct sg_blend_state binocle_blend_new_default();

#endif //BINOCLE_BLEND_H
