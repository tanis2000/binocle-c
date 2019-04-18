//
// Created by Valerio Santinelli on 2019-04-18.
//

#include "binocle_bezier.h"
#include "binocle_math.h"

void binocle_bezier_table(kmVec2 P[4], kmVec2 results[BINOCLE_BEZIER_STEPS + 1]) {
  static float C[(BINOCLE_BEZIER_STEPS + 1) * 4], *K = 0;
  if (!K) {
    K = C;
    for (unsigned step = 0; step <= BINOCLE_BEZIER_STEPS; ++step) {
      float t = (float) step / (float) BINOCLE_BEZIER_STEPS;
      C[step * 4 + 0] = (1 - t)*(1 - t)*(1 - t);   // * P0
      C[step * 4 + 1] = 3 * (1 - t)*(1 - t) * t; // * P1
      C[step * 4 + 2] = 3 * (1 - t) * t*t;     // * P2
      C[step * 4 + 3] = t * t*t;               // * P3
    }
  }
  for (unsigned step = 0; step <= BINOCLE_BEZIER_STEPS; ++step) {
    kmVec2 point = {
      K[step * 4 + 0] * P[0].x + K[step * 4 + 1] * P[1].x + K[step * 4 + 2] * P[2].x + K[step * 4 + 3] * P[3].x,
      K[step * 4 + 0] * P[0].y + K[step * 4 + 1] * P[1].y + K[step * 4 + 2] * P[2].y + K[step * 4 + 3] * P[3].y
    };
    results[step] = point;
  }
}

float binocle_bezier_value(float dt01, float P[4]) {
  kmVec2 Q[4] = { { 0, 0 }, { P[0], P[1] }, { P[2], P[3] }, { 1, 1 } };
  kmVec2 results[BINOCLE_BEZIER_STEPS + 1];
  binocle_bezier_table(Q, results);
  return results[(int) ((dt01 < 0 ? 0 : dt01 > 1 ? 1 : dt01) * BINOCLE_BEZIER_STEPS)].y;
}