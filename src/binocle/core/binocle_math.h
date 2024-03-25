//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_MATH_H
#define BINOCLE_MATH_H

#include <stdbool.h>
#include <stdint.h>
#include <kazmath/kazmath.h>

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef MINOF
#define MINOF(a, b, c, d) MIN(a, MIN(b, MIN(c, d)))
#endif

#ifndef MAXOF
#define MAXOF(a, b, c, d) MAX(a, MAX(b, MAX(c, d)))
#endif

/* SIGNOF
 If value is positive or zero, return 1
 If negative return -1
 */
#ifndef SIGNOF
#define SIGNOF(value) ((value >= 0) ? 1 : -1)
#endif

#ifndef M_PI
#define M_PI        3.14159265358979323846264338327950288   /* pi             */
#endif

#ifndef M_PI_2
#define M_PI_2      1.57079632679489661923132169163975144   /* pi/2           */
#endif

static kmMat4 binocle_math_create_orthographic_matrix_off_center(float left, float right, float bottom, float top, float z_near_plane, float z_far_plane);

//#ifdef BINOCLE_MATH_IMPL
static kmMat4 binocle_math_create_orthographic_matrix_off_center(
  float left, float right, float bottom, float top, float z_near_plane, float z_far_plane) {
  kmMat4 m;

  kmMat4Identity(&m);

  kmScalar tx = -((right + left) / (right - left));
  kmScalar ty = -((top + bottom) / (top - bottom));
  kmScalar tz = -((z_far_plane + z_near_plane) / (z_far_plane - z_near_plane));

  m.mat[0] = 2 / (right - left);
  m.mat[5] = 2 / (top - bottom);
  m.mat[10] = -2 / (z_far_plane - z_near_plane);
  m.mat[12] = tx;
  m.mat[13] = ty;
  m.mat[14] = tz;

  return m;
}
//#endif //BINOCLE_MATH_IMPL

static bool binocle_math_is_pow_2(uint32_t value)
{
  bool result = ((value & ~(value - 1)) == value);
  return result;
}

#define binocle_math_align_pow_2(value, alignment) (((value) + ((alignment) - 1)) & ~(((value) - (value)) + (alignment) - 1))

#endif //BINOCLE_MATH_H
