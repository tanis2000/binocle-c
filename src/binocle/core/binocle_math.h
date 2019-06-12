//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_MATH_H
#define BINOCLE_MATH_H

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

#if defined(__EMSCRIPTEN__)
/*  Even though these might be more useful as long doubles, POSIX requires
    that they be double-precision literals.                                   */
#define M_E         2.71828182845904523536028747135266250   /* e              */
#define M_LOG2E     1.44269504088896340735992468100189214   /* log2(e)        */
#define M_LOG10E    0.434294481903251827651128918916605082  /* log10(e)       */
#define M_LN2       0.693147180559945309417232121458176568  /* loge(2)        */
#define M_LN10      2.30258509299404568401799145468436421   /* loge(10)       */
#define M_PI        3.14159265358979323846264338327950288   /* pi             */
#define M_PI_2      1.57079632679489661923132169163975144   /* pi/2           */
#define M_PI_4      0.785398163397448309615660845819875721  /* pi/4           */
#define M_1_PI      0.318309886183790671537767526745028724  /* 1/pi           */
#define M_2_PI      0.636619772367581343075535053490057448  /* 2/pi           */
#define M_2_SQRTPI  1.12837916709551257389615890312154517   /* 2/sqrt(pi)     */
#define M_SQRT2     1.41421356237309504880168872420969808   /* sqrt(2)        */
#define M_SQRT1_2   0.707106781186547524400844362104849039  /* 1/sqrt(2)      */
#endif // defined(__EMSCRIPTEN__)

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

#endif //BINOCLE_MATH_H
